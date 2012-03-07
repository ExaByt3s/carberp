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

	// Функция разбирает адрес на состовляющие

	if (URL == NULL)
		return false;

	int Pos = STR::Pos(URL, HTTPProtocolDelimeter);
	if (Pos >= 0)
	{
		// Разделитель найден
		Protocol.Copy(URL, 0, Pos);
		URL += Pos + STRA::Length(HTTPProtocolDelimeter);
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

	// Следующим этапом получаем пост данные
	string DocAndPath;

	Pos = STR::Pos(URL, HTTPParamsDelimeter);
	if (Pos >= 0)
	{
		// Сохраняем параметры
		Params = URL + Pos + 1;
	}
	else
        Pos = AnsiStr::Length(URL);


	// Разделяем путь и докумет
	const char* DocPtr = URL + Pos;

	// Переходим к началу имени документа
	while (DocPtr > URL && *DocPtr != *HTTPSlash) DocPtr--;
	DocPtr++;

	// Копируем имя документа
    DWORD DocLen = Pos - (DocPtr - URL);
	Document.Copy(DocPtr, 0, DocLen);

	// копируем
	Path.Copy(URL, 0, DocPtr - URL);

	return true;
}
