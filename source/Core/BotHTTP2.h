//---------------------------------------------------------------------------
//  ���������� ��� ������ � ������� �� HTTP ����������
//
//  ������: 2.0
//  ����:  ������� 2012
//---------------------------------------------------------------------------

#ifndef BotHTTP2H
#define BotHTTP2H
//---------------------------------------------------------------------------


#include "GetApi.h"
#include "Strings.h"


//const char HTTPProtocolDelimeter[] = "://";
const char HTTPParamsDelimeter[] = "?";
const char HTTPSlash[] = "/";
const DWORD HTTPDefaultPort = 80;


class TURL : public TBotClass
{
public:
	string Protocol;
	string Host;
	string Path;
	string Document;
	string Params;
	WORD Port;

    TURL() {};
	TURL(const char *URL);

	void Clear();
	bool Parse(const char *URL);
	string URL(); // ������� �������� ������ �����

private:
    bool DoParse(const char *URL);
};


//---------------------------------------------------------------------------
#endif
