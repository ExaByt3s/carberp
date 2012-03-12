//---------------------------------------------------------------------------
#pragma hdrstop



#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   Файл включен в модули проекта
	//   используем отдельные настройки
	//****************************************
	#define USE_AZ_CONFIG
#endif


#include "Strings.h"
#include "Config.h"
//-----------------------------------------------------------------------------



#ifdef USE_AZ_CONFIG

	char AZ_HOSTS[AZ_HOSTS_SIZE] = "__AZ_HOSTS__\0\0";

	#define AZ_HOSTS_HASH 0xE0203A42 /* __AZ_HOSTS__ */

#endif



string GetAzHost()
{
	// Функция возвращает первый рабочий хост из
	// массива хостов системы AZ
	#ifdef USE_AZ_CONFIG
		return GetActiveHostFromBuf2(AZ_HOSTS, AZ_HOSTS_HASH, true);
	#else
    	return GetActiveHost2();
	#endif
}
//-----------------------------------------------------------------------------

string GetAzURL(const char*  Path)
{
	//  Функция возвращает полный адрес на
	//  основе Хоста системы и переданного пути
	string Host = GetAzHost();
	if (Host.IsEmpty()) return Host;
	TURL URL;
	URL.Host = Host;
	URL.Path = Path;

    return URL.URL();
}
//-----------------------------------------------------------------------------

