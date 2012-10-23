#include <windows.h>

PVOID HookApi( DWORD DllNum, DWORD FuncHash, LPVOID ReplacementFunc );
PVOID HookApi( DWORD DllNum, DWORD FuncHash, LPVOID ReplacementFunc, PVOID FuncReal );
PVOID HookApi( const char* DllName, DWORD FuncHash, LPVOID ReplacementFunc, PVOID FuncReal );
PVOID HookApi2( DWORD Dll, DWORD FuncVA, DWORD ReplacementFunc );
//установка хука в нужной DllName модуля Module, т. е. модуль подгрузил DllName и прямо в таблице импорта
//делается подмена
bool PathIAT(PVOID Module,PCHAR DllName,PCHAR FuncName,PVOID NewHandler,PVOID *OldHandler);