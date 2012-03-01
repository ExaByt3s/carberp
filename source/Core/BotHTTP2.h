//---------------------------------------------------------------------------
//  Библиотека для работы с данными по HTTP протоколом
//
//  Версия: 2.0
//  Дата:  февраль 2012
//---------------------------------------------------------------------------

#ifndef BotHTTP2H
#define BotHTTP2H
//---------------------------------------------------------------------------


#include "GetApi.h"
#include "Strings.h"


const char HTTPProtocolDelimeter[] = "://";
const char HTTPSlash[] = "/";


class TURL : public TBotClass
{
public:
	string Protocol;
	string Host;
	string Path;
	string Document;
	string Params;

    TURL() {};
	TURL(const char *URL);

	void Clear();
	bool Parse(const char *aURL);
	string URL(); // Функция собирает полный адрес
};


//---------------------------------------------------------------------------
#endif
