#include "GetApi.h"
#include "windows.h"
#include "DriverConnect.h"

//
// ��������� ���������� � �������(�������� DriverIOControl)
//
typedef struct __TNOTIFY{
	HANDLE hEvent;
	ULONG  NotifyCode;
	PVOID  InBuffer;
	DWORD  InSize;
	PVOID  OutBuffer;
	DWORD  OutSize;	
}TNOTIFY,*PTNOTIFY;

//
//�������� DriverAddInjectModule
//
typedef struct __TNOTIFY_LOAD_MODULE{
	PSTR ProcessName;		// target process name
	PVOID Module;			// module to inject
	ULONG Size;
	ULONG AttachCount;		// number of attach attempts
	ULONG Flags	;			// 
}TNOTIFY_LOAD_MODULE,*PTNOTIFY_LOAD_MODULE;

//
//	�������� DriverRemoveInjectModule
//
typedef struct __TNOTIFY_REMOVE_MODULE{
	PVOID injDesc;
}TNOTIFY_REMOVE_MODULE,*PNOTIFY_REMOVE_MODULE;



//
//	������� ��� ������� � ��������� 
//
BOOL DriverIOControl(PUSER_INIT_NOTIFY uin,DWORD code,PVOID in_data,DWORD in_size,PVOID out_data,DWORD out_size,DWORD t_wait);



//
// ������� ������ �� ������ ��� ��� �������
//	Module	-	�� ��� ������� DriverAddInjectModule
//	���������� TRUE ���� ��� ������.
BOOL DriverRemoveInjectModule(PUSER_INIT_NOTIFY puin,PVOID Module);



//
//	��������� ������� � ���������.
//	���������� TRUE ���� ������� ��������.
//
BOOL DriverIOTest(PUSER_INIT_NOTIFY puin,DWORD t_wait);



//
//	��������� ������� � ���������.
//	���������� TRUE ���� ������� ��������.
//
BOOL DriverIOTest(PUSER_INIT_NOTIFY puin,DWORD t_wait){
	ULONG Result = 0;
	
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_NULL,NULL,0,&Result,sizeof(Result),t_wait) )
		return FALSE;

	return Result != 0;
};


//
// ������� ������ �� ������ ��� ��� �������
//	Module	-	�� ��� ������� DriverAddInjectModule
//	���������� TRUE ���� ��� ������.
//
BOOL DriverRemoveInjectModule(PUSER_INIT_NOTIFY puin,PVOID Module)
{
	TNOTIFY_REMOVE_MODULE nrm;
	ULONG Stat;

	nrm.injDesc = Module;

// ������� 2 ���. ������
//
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_REMOVE_MODULE,&nrm,sizeof(nrm),&Stat,sizeof(Stat),120*1000) )
			return NULL;
	
	return Stat ;
};




//
//	puin - �������� ���������� � ��������� Notify 
//	Module - ��������� ��� � ������
//  TargetProcess - ��� �������� � ������� ����� ��������, ���� NULL �� �� ��� ��������.
//  size	- ������ ��� � ������
//  flags -  �������������� ����� ��� ��������, ���� ���������� 0. ���� ����� ����� ����� ���������.
PVOID DriverAddInjectModule(PUSER_INIT_NOTIFY puin, PVOID Module,PCHAR TargetProcess,ULONG size,ULONG Flags){
	TNOTIFY_LOAD_MODULE nlm;
	PVOID Stat = NULL;

	pOutputDebugStringW(L"[DriverAddInjectModule]		\n");


	nlm.AttachCount = -1;
	nlm.Flags		= Flags;
	nlm.Module		= Module;
	nlm.ProcessName = TargetProcess;
	nlm.Size        = size;

	// ������� 2 ���. ������
	//
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_LOAD_MODULE,&nlm,sizeof(nlm),&Stat,sizeof(Stat),120*1000) )
			return NULL;

	return Stat ;
};




//
// �������� ����������� "����" ��������
// code - ��� �������
//	in_data - ������� ������
//  in_size - ������ ������� ������
//	out_data - �������� ������
//  out_size - ������ �������� ������
//  t_wait   - �����(� ��) �������� ��������� ������(������ ���� ���������� ������� ���� ����� ������������� �������� �������)
//  
//  ���������� TRUE ���� �������(����� hCompleat  ���������� � ���. ���� ) � ��������� ��������.

BOOL DriverIOControl(PUSER_INIT_NOTIFY uin,DWORD code,PVOID in_data,DWORD in_size,PVOID out_data,DWORD out_size,DWORD t_wait){
	HANDLE	 hEvent;
	HANDLE   hCompleat;
	PWCHAR	 EventName;
	BOOL	 err = TRUE;
	PTNOTIFY	pNotify;



	if (  (uin->AreaSize < sizeof(TNOTIFY)) )
		return FALSE;


	hCompleat = pCreateEventW(NULL,TRUE,FALSE,NULL);
	
	if (!hCompleat)
		return FALSE;

	hEvent = uin->hQueryEvent;//pOpenEventW(EVENT_ALL_ACCESS,FALSE,EventName);

	if (!hEvent){
		pCloseHandle(hCompleat);
		return FALSE;
	};

	pNotify  = (PTNOTIFY)uin->pInfoNotifyArea ;

	pNotify->InBuffer	= in_data;
	pNotify->InSize		= in_size;
	pNotify->OutBuffer	= out_data;
	pNotify->OutSize	= out_size;
	pNotify->hEvent		= hCompleat;
	pNotify->NotifyCode = code;

	err = (BOOL)pSetEvent(hEvent);
	
	if ( pWaitForSingleObject(hCompleat,t_wait) == WAIT_OBJECT_0 )
		 err  = TRUE;
	else err  = FALSE;	

	pCloseHandle(hCompleat);
	
	return err;
};