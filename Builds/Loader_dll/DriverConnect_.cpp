#include "GetApi.h"
#include "windows.h"
#include "DriverConnect.h"

//
// Структура передается в драйвер(смотреть DriverIOControl)
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
//Смотреть DriverAddInjectModule
//
typedef struct __TNOTIFY_LOAD_MODULE{
	PSTR ProcessName;		// target process name
	PVOID Module;			// module to inject
	ULONG Size;
	ULONG AttachCount;		// number of attach attempts
	ULONG Flags	;			// 
}TNOTIFY_LOAD_MODULE,*PTNOTIFY_LOAD_MODULE;

//
//	Смотреть DriverRemoveInjectModule
//
typedef struct __TNOTIFY_REMOVE_MODULE{
	PVOID injDesc;
}TNOTIFY_REMOVE_MODULE,*PNOTIFY_REMOVE_MODULE;



//
//	Функция для общения с драйвером 
//
BOOL DriverIOControl(PUSER_INIT_NOTIFY uin,DWORD code,PVOID in_data,DWORD in_size,PVOID out_data,DWORD out_size,DWORD t_wait);



//
// Убирает модуль из списка длл для инжекта
//	Module	-	то что вернула DriverAddInjectModule
//	возвращает TRUE если все хорошо.
BOOL DriverRemoveInjectModule(PUSER_INIT_NOTIFY puin,PVOID Module);



//
//	Тестирует общение с драйвером.
//	Возвращает TRUE если общение возможно.
//
BOOL DriverIOTest(PUSER_INIT_NOTIFY puin,DWORD t_wait);



//
//	Тестирует общение с драйвером.
//	Возвращает TRUE если общение возможно.
//
BOOL DriverIOTest(PUSER_INIT_NOTIFY puin,DWORD t_wait){
	ULONG Result = 0;
	
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_NULL,NULL,0,&Result,sizeof(Result),t_wait) )
		return FALSE;

	return Result != 0;
};


//
// Убирает модуль из списка длл для инжекта
//	Module	-	то что вернула DriverAddInjectModule
//	возвращает TRUE если все хорошо.
//
BOOL DriverRemoveInjectModule(PUSER_INIT_NOTIFY puin,PVOID Module)
{
	TNOTIFY_REMOVE_MODULE nrm;
	ULONG Stat;

	nrm.injDesc = Module;

// ожидаем 2 мин. ответа
//
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_REMOVE_MODULE,&nrm,sizeof(nrm),&Stat,sizeof(Stat),120*1000) )
			return NULL;
	
	return Stat ;
};




//
//	puin - значение переданное в процедуру Notify 
//	Module - указатель длл в памяти
//  TargetProcess - имя процесса в который нужно внедрить, если NULL то во все процессы.
//  size	- размер длл в памяти
//  flags -  дополнительные флаги для загрузки, пока передавать 0. если нужно можно будет расширить.
PVOID DriverAddInjectModule(PUSER_INIT_NOTIFY puin, PVOID Module,PCHAR TargetProcess,ULONG size,ULONG Flags){
	TNOTIFY_LOAD_MODULE nlm;
	PVOID Stat = NULL;

	pOutputDebugStringW(L"[DriverAddInjectModule]		\n");


	nlm.AttachCount = -1;
	nlm.Flags		= Flags;
	nlm.Module		= Module;
	nlm.ProcessName = TargetProcess;
	nlm.Size        = size;

	// ожидаем 2 мин. ответа
	//
	if(! DriverIOControl(puin,IO_CONTROL_DRIVER_LOAD_MODULE,&nlm,sizeof(nlm),&Stat,sizeof(Stat),120*1000) )
			return NULL;

	return Stat ;
};




//
// посылает управляющие "коды" драйверу
// code - код команды
//	in_data - входные данные
//  in_size - размер входных данных
//	out_data - выходные данные
//  out_size - размер выходных данных
//  t_wait   - время(в мс) ожидание обработки данных(должно быть достаточно большое чтоб успел переключиться контекст потоков)
//  
//  возвращает TRUE если общение(евент hCompleat  установили в сиг. сост ) с драйвером возможно.

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