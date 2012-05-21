//---------------------------------------------------------------------------
//
//	Модуль содержащий утилиты для отладки бота.
//
//  Важно!!!
//
//	Функции модуля будут доступны при включенной директиве препроцессора
//
//	DEBUGBOT или DEBUGCONFIG
//
//
//---------------------------------------------------------------------------

#ifndef DebugUtilsModule
#define DebugUtilsModule


// Временная опция, разрешающая использование отладочных
// строк в любых конфигурацях сборки
//#define ALWAYSDEBUG


#if defined(ALWAYSDEBUG) || defined(DEBUGBOT) || defined(DEBUGCONFIG)

	// Объявляем для включения функций и шаблонов вывода отладочных строк
	#define DebugUtils
#endif

// Макросы для показа сообщений в окне компиляции.
#define PP_TOSTRING2(x) #x
#define PP_TOSTRING(x) PP_TOSTRING2(x)

#define PP_COMPILER_MESSAGE(msg)  /*__pragma(message("COMPILE-TIME MESSAGE: [" msg "]:(" __FILE__ ":" PP_TOSTRING(__LINE__) ":'"__FUNCTION__"')")) */

#include <Windows.h>


//*************************************************************
//  Имя OUTPUT_LINES_IN_FILE включает опцию вывода отладочных
//  строк в файл
//*************************************************************

//#define OUTPUT_LINES_IN_F+ILE


#ifdef OUTPUT_LINES_IN_FILE
	const char DebugLinesFileName[] = "c:\\BotDbgLines.log";
#endif



// Инициализировать данные для отладки бота и выдать сообщение
#ifdef DEBUGBOT
bool StartInDebugingMode(bool ShMSG);
#endif

// Вывести сообщение
//void DebugMessage(PCHAR Msg);


// Метод обработки отладочных строк
typedef void (WINAPI *TDebugMessageHandlerMethod)(LPVOID Handler, PCHAR Module,
	DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str);


// Установить обработчик отладочных строк
void SetDebugMessageHandler(LPVOID Handler, TDebugMessageHandlerMethod Method);

// информация об отладочном сообщении
typedef struct TDebugMessage
{
	PCHAR Module;
	DWORD Line;
	PCHAR Section;
	PCHAR Data;
} *PDebugMessage;

//----------------------------------------------------------------------------
//  Debug - методы для работы с отладочными данными
//----------------------------------------------------------------------------
namespace Debug
{
	void Message(PCHAR Module, PCHAR Str);
	void MessageEx(PCHAR Module, DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str, ...);
}

//---------------------------------------------------------------------------
#endif


