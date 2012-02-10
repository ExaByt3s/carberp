#include <windows.h>

PVOID HookApi( DWORD DllNum, DWORD FuncHash, LPVOID ReplacementFunc );
PVOID HookApi( DWORD DllNum, DWORD FuncHash, LPVOID ReplacementFunc, PVOID FuncReal );
PVOID HookApi2( DWORD Dll, DWORD FuncVA, DWORD ReplacementFunc );
