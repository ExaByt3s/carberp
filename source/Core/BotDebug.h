//---------------------------------------------------------------------------
//
//	������ ���������� ������� ��� ������� ����.
//
//  �����!!!
//
//	������� ������ ����� �������� ��� ���������� ��������� �������������
//
//	DEBUGBOT ��� DEBUGCONFIG
//
//
//---------------------------------------------------------------------------

#ifndef DebugUtilsModule
#define DebugUtilsModule


// ��������� �����, ����������� ������������� ����������
// ����� � ����� ������������ ������
//#define ALWAYSDEBUG


#if defined(ALWAYSDEBUG) || defined(DEBUGBOT) || defined(DEBUGCONFIG)

	// ��������� ��� ��������� ������� � �������� ������ ���������� �����
	#define DebugUtils
#endif

// ������� ��� ������ ��������� � ���� ����������.
#define PP_TOSTRING2(x) #x
#define PP_TOSTRING(x) PP_TOSTRING2(x)

#define PP_COMPILER_MESSAGE(msg)  /*__pragma(message("COMPILE-TIME MESSAGE: [" msg "]:(" __FILE__ ":" PP_TOSTRING(__LINE__) ":'"__FUNCTION__"')")) */

#include <Windows.h>


//*************************************************************
//  ��� OUTPUT_LINES_IN_FILE �������� ����� ������ ����������
//  ����� � ����
//*************************************************************

//#define OUTPUT_LINES_IN_F+ILE


#ifdef OUTPUT_LINES_IN_FILE
	const char DebugLinesFileName[] = "c:\\BotDbgLines.log";
#endif



// ���������������� ������ ��� ������� ���� � ������ ���������
#ifdef DEBUGBOT
bool StartInDebugingMode(bool ShMSG);
#endif

// ������� ���������
//void DebugMessage(PCHAR Msg);


// ����� ��������� ���������� �����
typedef void (WINAPI *TDebugMessageHandlerMethod)(LPVOID Handler, PCHAR Module,
	DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str);


// ���������� ���������� ���������� �����
void SetDebugMessageHandler(LPVOID Handler, TDebugMessageHandlerMethod Method);

// ���������� �� ���������� ���������
typedef struct TDebugMessage
{
	PCHAR Module;
	DWORD Line;
	PCHAR Section;
	PCHAR Data;
} *PDebugMessage;

//----------------------------------------------------------------------------
//  Debug - ������ ��� ������ � ����������� �������
//----------------------------------------------------------------------------
namespace Debug
{
	void Message(PCHAR Module, PCHAR Str);
	void MessageEx(PCHAR Module, DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str, ...);
}

//---------------------------------------------------------------------------
#endif


