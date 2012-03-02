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

	if (URL == NULL)
		return false;

	int Pos = STR::Pos(URL, HTTPProtocolDelimeter);
	if (Pos >= 0)
	{
		// Разделитель найден
		Protocol.Copy(URL, 0, Pos);
		URL += Pos + StrBufA::CalcLength(HTTPProtocolDelimeter);
    }

	// Определяем позицию начала пути
	Pos = STR::Pos(URL, HTTPSlash);

	if (Pos < 0)
	{
		// Адрес не содержит путь
		if (STR::Scan(URL, '.') == NULL) return false;
		Host = URL;
		return true;
	}

	// Сохраняем хост и переводуим указатель на начало пути
	Host.Copy(URL, 0, Pos);
	URL += Pos;

	// Следующим этапом получаем параметры
	string DocAndPath;

	Pos = STR::Pos(URL, HTTPParamsDelimeter);
	if (Pos >= 0)
	{
    	// Сохраняем копию пути с документом
		DocAndPath.Copy(URL, 0, Pos);

		// Сохраняем параметры
		URL += Pos + 1;
		Params = URL;

		// Устанавливаем указатель обратно на путь
        URL = DocAndPath.t_str();
    }

	// Разделяем путь и докумет
	PCHAR DocPtr = STR::ScanEnd((PCHAR)URL, *HTTPSlash);
	if (DocPtr != NULL)
	{
		Path.Copy(URL, 0, (DocPtr - URL) + 1);

		DocPtr++;
		Document = DocPtr;
	}
	else
	{
		Path = HTTPSlash;
		Document = URL;
	}


  /*	// Определяем протокол
	Rec->Protocol = STR::GetLeftStr(URL, "://");


	// Определяем указатели на вадные блоки
	PCHAR PathPtr = STR::Scan(Buf, '/');
	IncStrEx(PathPtr);

	// Строка параметров
	PCHAR ArgsPtr = NULL;
	PCHAR DocPtr = NULL;
	if (FullPars)
	{
		ArgsPtr = STR::Scan(PathPtr, '?');
		IncStrEx(ArgsPtr);

		// Определяем документ
		DocPtr = STR::ScanEnd(PathPtr, '/'); // Ищем последний слеш
		if (DocPtr == NULL)
		{
			// Путь является документом
			DocPtr = PathPtr;
            PathPtr = NULL;
		}
		else
        	IncStrEx(DocPtr);
	}

	// Строка хоста и порта
	PCHAR HostPtr = Buf;
	PCHAR PortPtr = STR::Scan(HostPtr, ':');
	IncStrEx(PortPtr);


	STR::Free(Buf);
	return true;  */

	return true;
}
