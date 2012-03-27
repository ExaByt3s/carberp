#ifndef UUID_E4500F5134534F79A3663021D13CDBC8
#define UUID_E4500F5134534F79A3663021D13CDBC8

#define PP_REPORT_URL "http://kktestbe78fgehduinjc.org/geter/index.php?"

#ifdef DBGRPT_ENABLED
#	define PP_DBGRPT_FUNCTION_CALL(function) { (function); } ;
#else 
#	define PP_DBGRPT_FUNCTION_CALL(function) __noop
#endif

void DebugReportStepByName(const char* StepName);


#endif // #ifndef UUID_E4500F5134534F79A3663021D13CDBC8