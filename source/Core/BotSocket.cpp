//---------------------------------------------------------------------------

#pragma hdrstop

#include "BotSocket.h"
//---------------------------------------------------------------------------



//****************************************************************************
//  								TBotSocket
//****************************************************************************

TBotSocket::~TBotSocket()
{
	// � ����������� ������������� ��������� ����������
    Close();
}


//------------------------------------------------------
//  Connect - ������� ����� � ������������ � ����������
//			  �������
//  HostName - ��� �������
//  Port     - ���� � �������� ����� ������������
//------------------------------------------------------
bool TBotSocket::Connect(const char *HostName, WORD Port)
{
	return DoConnect(HostName, Port, 0);
}

//------------------------------------------------------
//  Connect - ������� ����� � ������������ � ����������
//			  �������
//  HostName - ��� �������
//  Port     - ���� � �������� ����� ������������
//  Timeout  - �������� ������� � ������� �������� �����
//             ������������� ������� �����������
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
