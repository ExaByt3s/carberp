#ifndef __Driver_Connect___
#define __Driver_Connect___

#define IO_CONTROL_DRIVER_NULL				0
#define IO_CONTROL_DRIVER_LOAD_MODULE		1
#define IO_CONTROL_DRIVER_REMOVE_MODULE		2


typedef struct __USER_INIT_NOTIFY{
	PVOID pInfoNotifyArea;				//	��������� ���� ���� ������ ������ ��������
	ULONG AreaSize;						//	 ������ �������
	HANDLE hQueryEvent;					// ����� ��� ��������� � ��������
}USER_INIT_NOTIFY,*PUSER_INIT_NOTIFY;



//
//	puin			- �������� ���������� � ��������� Notify 
//	Module			- ��������� ��� � ������
//  TargetProcess	- ��� �������� � ������� ����� ��������, ���� NULL �� �� ��� ��������.
//  size			- ������ ��� � ������
//  Flags			- ���������������
PVOID DriverAddInjectModule(PUSER_INIT_NOTIFY puin, PVOID Module,PCHAR TargetProcess,ULONG size,ULONG Flags);




#endif