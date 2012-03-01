//---------------------------------------------------------------------------


#pragma hdrstop

#include "BotHTTP2.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


TURL::TURL(const char * aURL)
{
    Parse(aURL);
}


string TURL::URL()
{
	string R = Protocol;
	R += Host;
	R += Path;

	return R;
}

void TURL::Clear()
{

}


bool TURL::Parse(const char *URL)
{
	Clear();
	if (URL == NULL)
		return false;

	int Pos = STR::Pos(URL, HTTPProtocolDelimeter);
	if (Pos >= 0)
	{
		// ����������� ������
		Protocol.Copy(URL, 0, Pos);
		URL += Pos + StrBufA::CalcLength(HTTPProtocolDelimeter);
    }

	// ���������� ������� ������ ����
	Pos = STR::Pos(URL, HTTPSlash);

	if (Pos < 0)
	{
		// ����� �� �������� ����
		if (STR::Scan(URL, '.') == NULL) return false;
		Host = URL;
		return true;
	}

	Host.Copy(URL, 0, Pos);
	URL += Pos + 1;

  /*	// ���������� ��������
	Rec->Protocol = STR::GetLeftStr(URL, "://");


	// ���������� ��������� �� ������ �����
	PCHAR PathPtr = STR::Scan(Buf, '/');
	IncStrEx(PathPtr);

	// ������ ����������
	PCHAR ArgsPtr = NULL;
	PCHAR DocPtr = NULL;
	if (FullPars)
	{
		ArgsPtr = STR::Scan(PathPtr, '?');
		IncStrEx(ArgsPtr);

		// ���������� ��������
		DocPtr = STR::ScanEnd(PathPtr, '/'); // ���� ��������� ����
		if (DocPtr == NULL)
		{
			// ���� �������� ����������
			DocPtr = PathPtr;
            PathPtr = NULL;
		}
		else
        	IncStrEx(DocPtr);
	}

	// ������ ����� � �����
	PCHAR HostPtr = Buf;
	PCHAR PortPtr = STR::Scan(HostPtr, ':');
	IncStrEx(PortPtr);


	STR::Free(Buf);
	return true;  */

	return true;
}
