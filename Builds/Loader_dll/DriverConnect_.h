#ifndef __Driver_Connect___
#define __Driver_Connect___

#define IO_CONTROL_DRIVER_NULL				0
#define IO_CONTROL_DRIVER_LOAD_MODULE		1
#define IO_CONTROL_DRIVER_REMOVE_MODULE		2


typedef struct __USER_INIT_NOTIFY{
	PVOID pInfoNotifyArea;				//	указатель куда нуно писать данные драйверу
	ULONG AreaSize;						//	 размер области
	HANDLE hQueryEvent;					// евент для обращения к драйверу
}USER_INIT_NOTIFY,*PUSER_INIT_NOTIFY;



//
//	puin			- значение переданное в процедуру Notify 
//	Module			- указатель длл в памяти
//  TargetProcess	- имя процесса в который нужно внедрить, если NULL то во все процессы.
//  size			- размер длл в памяти
//  Flags			- зарезервировано
PVOID DriverAddInjectModule(PUSER_INIT_NOTIFY puin, PVOID Module,PCHAR TargetProcess,ULONG size,ULONG Flags);




#endif