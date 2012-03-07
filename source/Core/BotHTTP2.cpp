//---------------------------------------------------------------------------


#pragma hdrstop

#include "BotHTTP2.h"
#include "BotHTTP.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


TURL::TURL(const char * aURL)
{
    Parse(aURL);
}


string TURL::URL()
{
	string R = Protocol;
	R += HTTPProtocolDelimeter;
	R += Host;
	R += Path;
	R += Document;

	return R;
}


void TURL::Clear()
{
	Protocol = "";
	Host = "";
	Path = "";
	Document = "";
	Params = "";
	Port = HTTPDefaultPort;
}

bool TURL::Parse(const char *URL)
{
	Clear();
    return DoParse(URL);
}


bool TURL::DoParse(const char *URL)
{

	// ������� ��������� ����� �� ������������

	if (URL == NULL)
		return false;

	int Pos = STR::Pos(URL, HTTPProtocolDelimeter);
	if (Pos >= 0)
	{
		// ����������� ������
		Protocol.Copy(URL, 0, Pos);
		URL += Pos + STRA::Length(HTTPProtocolDelimeter);
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

	// ��������� ���� � ���������� ��������� �� ������ ����
	Host.Copy(URL, 0, Pos);
	URL += Pos;

	// ��������� ������ �������� ���� ������
	string DocAndPath;

	Pos = STR::Pos(URL, HTTPParamsDelimeter);
	if (Pos >= 0)
	{
		// ��������� ���������
		Params = URL + Pos + 1;
	}
	else
        Pos = AnsiStr::Length(URL);


	// ��������� ���� � �������
	const char* DocPtr = URL + Pos;

	// ��������� � ������ ����� ���������
	while (DocPtr > URL && *DocPtr != *HTTPSlash) DocPtr--;
	DocPtr++;

	// �������� ��� ���������
    DWORD DocLen = Pos - (DocPtr - URL);
	Document.Copy(DocPtr, 0, DocLen);

	// ��������
	Path.Copy(URL, 0, DocPtr - URL);

	return true;
}
