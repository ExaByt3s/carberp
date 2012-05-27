//---------------------------------------------------------------------------

#pragma hdrstop

#include "BotSocket.h"
//---------------------------------------------------------------------------



//****************************************************************************
//  								TBotSocket
//****************************************************************************

TBotSocket::~TBotSocket()
{
	// В деструкторе автоматически закрываем соединения
    Close();
}


//------------------------------------------------------
//  Connect - Функция сокет и подключается к указанному
//			  серверу
//  HostName - Имя сервера
//  Port     - Порт к которому будем подключаться
//------------------------------------------------------
bool TBotSocket::Connect(const char *HostName, WORD Port)
{
	return DoConnect(HostName, Port, 0);
}

//------------------------------------------------------
//  Connect - Функция сокет и подключается к указанному
//			  серверу
//  HostName - Имя сервера
//  Port     - Порт к которому будем подключаться
//  Timeout  - Интервал времени в течении которого будет
//             производиться попытка подключения
//------------------------------------------------------
bool TBotSocket::Connect(const char *HostName, WORD Port, DWORD Timeout)
{
	return DoConnect(HostName, Port, Timeout);
}



//****************************************************************************
//  								TWinSocket
//****************************************************************************

TWinSocket::TWinSocket()
{

}

bool TWinSocket::DoConnect(const char *HostName, WORD Port, DWORD Timeout)
{

}


void TWinSocket::DoClose()
{
	int I = 10;
    I = 10;
}

int TWinSocket::Write(const void* Buf, DWORD BufSize)
{

}

int TWinSocket::Read(void* Buf, DWORD BufSize)
{

}
