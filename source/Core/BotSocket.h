//---------------------------------------------------------------------------
//  Модуль организации работы с сокетами
//
//  Верси: 0.1
//  Модифицирован: Май 2012
//---------------------------------------------------------------------------

#ifndef BotSocketH
#define BotSocketH
//---------------------------------------------------------------------------


#include "windows.h"
#include "GetApi.h"



class TBotSocket;


//--------------------------------------------------------
//  CreateSocket - Функция создаёт класс сокета в зависи-
//			       мости от глобальных настроек проекта
//  По умолчанию используется Windows сокеты и создаётся
//  класс TWinSocket
//--------------------------------------------------------
//TBotSocket* CreateSocket();





//***************************************************************
//  TBotSocket - Базовый класс для работы с TCP сокетом
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
//  TWinSocket - Класс для работы с TCP сокетом
//				 используя WinSocket
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
