
//---------------------------------------------------------------------------
//  Модуль хранения строковыъ констант бота
//
//  Модифицирован: Июль 2012
//---------------------------------------------------------------------------

#ifndef StrConstsH
#define StrConstsH
//---------------------------------------------------------------------------

#include "Strings.h"

//тип кодируемой строки, выравнивается в один байт
#ifdef _MSC_VER
#	define CSSTR __declspec(align(1)) char
#else
#	define CSSTR char
#endif

//*****************************************************
// GetStr - Функция расшифровывает строку EncryptedStr
//*****************************************************
string GetStr(const char* EncryptedStr);





//=====================================================
//  Публичное объявление строковых данных
//=====================================================
extern CSSTR StrBotWorkPath[];
extern CSSTR StrGrabberPath[];

extern CSSTR StrPrefixFileName[];


// Имена функций библиотеки видеозаписи.
extern CSSTR VideRecFuncRecordProcess[];
extern CSSTR VideRecFuncRecordWnd[];
extern CSSTR VideRecFuncStop[];
extern CSSTR VideRecFuncResetTimer[];
extern CSSTR VideRecFuncSendData[];
extern CSSTR VideRecFuncRunPortForward[];


//Не шифрованные строки и данные
extern CSSTR Slash[];
extern CSSTR SlashChar;



#endif
