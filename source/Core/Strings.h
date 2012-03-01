#include <windows.h>

#ifndef StringsH
#define StringsH
//-----------------------------------------------------------------------------

#include "Memory.h"
#include "GetApi.h"

//int WINAPI URLEncode( char *pszDestiny, char *pszSource );

//void DbgMsg(char *file, int line, char *msg, ...);
//void DbgMsgW(WCHAR *file, int line, WCHAR *msg, ...);

int m_atoi( const char *nptr );

void   WINAPI m_lstrcat( char *szBuf, const char *szStr );
void   WINAPI m_lstrcpy( char *szBuf, const char *szStr );
void   WINAPI m_lstrlwr( char *str );

void WINAPI m_lwcscat( WCHAR *szBuf, const WCHAR *szStr );

char * WINAPI m_strstr( const char * _Str, const char * _SubStr );
char * WINAPI m_strstrmask( const char * _Str, const char * _SubStr );
char * WINAPI m_strtok_s( char *String, const char *Control, char **Context );

//char * WINAPI ToAnsi( LPCWSTR String );
//char * WINAPI ToAnsiEx( LPCWSTR String, DWORD dwSize );

bool   WINAPI m_lstrncpy( char *pString_src, const char *pString_dst, DWORD len );
bool   WINAPI SearchByMask( char *mask, char *name );

int  WINAPI m_istrstr( const char * _Str, const char * _SubStr );
DWORD  WINAPI m_lstrcmp( const char *szStr1, const char *szStr2 );
DWORD  WINAPI m_lstrncmp( const char *szstr1, const char *szstr2, int nlen );
DWORD  WINAPI m_lstrlen(const char *szPointer );

DWORD WINAPI m_wcslen(const wchar_t *String );

wchar_t *m_wcsncpy( wchar_t *dest, wchar_t *src, unsigned long n );
wchar_t *m_wcslwr( wchar_t *Str );
bool m_wcsncmp( WCHAR *s1, WCHAR *s2, size_t iMaxLen );
const wchar_t * WINAPI m_wcsstr( const wchar_t * _Str, const wchar_t * _SubStr );

bool WildCmp(const char *Buffer, const char *Mask, LPDWORD Start, LPDWORD End, LPDWORD Len );
bool WildCmp(PCHAR Buffer, PCHAR Mask);

bool CompareUrl( char *MaskUrl, char *Url );

WCHAR * AnsiToUnicode( char *AnsiString, DWORD dwStrLen );

//void AlertError( LPTSTR lpszFunction ) ;

//убирает в начале и конце символы c, возвращает указатель на s, только внутри уже обработанная строка
char* trimall( char* s, char c = ' ' );


// ----------------------------------------------------------------------------
//  Набор функция для работы со строками
//
//  Если использовать данные функции для работы со строками, то? в некоторых
//  случаях, можно достить большего быстродействия. Например, функция
//  StrLength многократно  превышает по быстродействию функции которые
//  просчитывают длину строки сканируя её до нулевого символа, и чем больше
//  строка тем больше разница в быстродействии
// ----------------------------------------------------------------------------

// Изменить размер буфера строки
// Важно!!! Функция работает со строками которые созданы функцией StrNew
//void StrSetLength(PCHAR &Str, DWORD NewLength);


// Функция расчитывает длину строки сканируя ея в поисках завершаешего нуля
DWORD StrCalcLength(const char* Buf);
DWORD StrCalcLengthW(const wchar_t* Buf);


//  Функция StrConcat объеденяет две и более строки.
//  если размер буфера строки Str1 меньше необходимого,
//  то строка Str1 будет переопределена
//  Count определяет сколько строк будет добавлено к Str1
void StrConcat(PCHAR &Str1, PCHAR Str2);
void StrConcat(PCHAR &Str1, DWORD Count, PCHAR Str2...);

// функция копирует строку Source в строку Dest
// Флаг UpdateLen использовать только если Dest создана функциями
// StrAlloc, StrNew
// Возвращает истину если скопирован хотя-бы один байт
bool StrCopy(PCHAR Dest, PCHAR Source, bool UpdateLen = false);

// Сравнить две строки
int StrCompare(const char* Str1, const char* Str2);

// Функции дл преобразования строк в числи и наоборот
int StrToInt(PCHAR Str);
PCHAR StrLongToString(DWORD num);

// Функция приводит строку к нижнему регистру
void StrLowerCase(PCHAR Str);

// Функция ищет позицию строки в массиве строк Сmo
int StrIndexOf(PCHAR Str, bool CaseSensetive, DWORD Count, PCHAR Cmp...);

// Функция сравнивает строку Str со строкой ToSame.
// CaseSensetive определяет чувствительность к регистру
// Если StrEndPosrition не равен нулю, то сравнение
// ограничивается StrEndPosrition символами
bool StrSame(PCHAR Str, PCHAR ToSame, bool CaseSensetive = true, DWORD StrEndPosition = 0);

// Функция возвращает текст находящиийся  между текстом Before и After
// после обработки указатель Buffer  будет указывать на конец текста After
// результат уничтожить функцией StrFree
PCHAR GetTextBetween(PCHAR &Buffer, PCHAR Before, PCHAR After );

//----------------------------------------------------------------------------
//  STR - Методы для работы со строками
//----------------------------------------------------------------------------
namespace STR
{
	// Создаёт в памяти пустую строку выделяя под неё буфер размером StrLen + 1
	PCHAR Alloc(DWORD StrLen);

	// Функция создаёт строку, копию Source.
	// Len - Количество копируемых символов
	//       если равно 0, то будет скопирована вся строка
	PCHAR New(PCHAR Source, DWORD Len = 0);

	// Функция создаёт строку объеденив несколько строк
	PCHAR New(DWORD Count, PCHAR Str...);

	// Уничтожить строку.
	// Важно!!! Строка должна быть создана с помощью функции StrNew
	void Free(PCHAR Str);
	void Free2(PCHAR &Str);

	// Функция возвращает длину строки.
	DWORD Length(PCHAR Str);

	// Обновляет значение длины строки в её заголовоке.
	// !!!!Только для строк созданных через STR::Alloc (STR::New)
	// Если значение RealLength не указано то новая длина
	// расчитывается до нулевого символа
	void UpdateLength(PCHAR Str, DWORD RealLength = 0);

	// Находит позицию строки SubStr  в строке Str
	// Если значение StrLen то поиск будет вестись до
	// нулевого символа
	int Pos(const char* Str, const char* SubStr, DWORD StrLen = 0, bool CaseSensetive = true);

	// Копирует с позиции Position строки Source Count символов в
	// строку Destination
    void Copy(PCHAR Source, PCHAR Destination, DWORD Position, DWORD Count);

	// Удаляет Count символов с позоции Position строки Str
	void Delete(PCHAR Str, DWORD Position, DWORD Count, DWORD StrLength = 0);

	//  Replace - Функция заменяет в строке Str строку SrcStr на
	//  строку DstStr.
	//  В случае успеха функция возвращает новую строку
	PCHAR Replace(PCHAR Str, PCHAR SrcStr, PCHAR DstStr, DWORD StrLen = 0);


	// Функция возвращает указатель на символ С
	PCHAR Scan(const char* Str, char C);

	// Функция возвращает указатель на последний символ С
	PCHAR ScanEnd(PCHAR Str, char C);

	// Функции возвращают текст слева, справа соответственно
	// относительно строки SubStr
	PCHAR GetLeftStr(PCHAR Str, PCHAR SubStr, bool IncludeSubStr = false);
	PCHAR GetRightStr(PCHAR Str, PCHAR SubStr, bool IncludeSubStr = false);


	// Функция игнорирует пробелы. Функция НЕ изменяет строку, она
	// возвращает указатель на нужный символ, либо на конец строки
	PCHAR IgnoreSpaces( const PCHAR Str );

	// Возвращает указатель на конец строки
	PCHAR End(PCHAR Str);

	// Функция форматирует строку на основе шаблона
//	PCHAR Format(PCHAR Str, ...);
	PCHAR Format(PCHAR Str, va_list Arguments);

	// Функция возвращает текущую линию в строке.
	// Линия ограничивается символами 10, 13 либо концом строки
	// Конец строки ограничивается нулевым символом либо размером ЫекЫшяу
	PCHAR GetLine(PCHAR Str, DWORD StrSize = 0);

	// Функция возвращает указатель на первый символ следующей линии в строке
	// либо на конец строки
	PCHAR GotoNextLine(PCHAR Str, DWORD StrSize = 0);

    // Функция преобразовывает шестнадцатиричное число в десятиричное
	DWORD HexToDWORD(PCHAR Str);
	//преобрзовывает строку в неотрицательное число
	DWORD ToDWORD( const char* s );


	// Функция возвращает истину если на входе нулевой указатель либо
	// пустая строка
	bool IsEmpty( const PCHAR Str);

	// Функция расчитывает хэш строки
	// Str - исходная строка
	// Len - Длина строки, если равно 0, то расчитывается до конечного нуля
	// LowerCase - приводить символы в нижний регистр перед созданием хэша
	DWORD GetHash(PCHAR Str, DWORD Len, bool LowerCase);

	// Функция расчитывает длину строки состоящей из нескольких строк
	// отделённых друг от другу нулевым символом в конце которых стоит
	// двойной нулевой символ
	// Например: str0str0str00
	DWORD CalcDoubleZeroStrLength(PCHAR Str);

	// Функция приводит символы в нижний регистр. Функция не чувствительна
	// к кодировке символов
	void AnsiLowerCase(PCHAR Str);

    // Функция возвращает истину ели строка в Линукс формате
    bool IsLinuxStr(PCHAR S);

	// Функция приводит строку к формату Linux
	PCHAR ConvertToLinuxFormat(PCHAR S);
}

namespace WSTR
{
	// Выделить память по строку длиной StrLen
	PWCHAR Alloc(DWORD StrLen);

	// Функция создаёт строку, копию Source.
	// Len - Количество копируемых символов
	//       если равно 0, то будет скопирована вся строка
    PWCHAR New(PWCHAR Source, DWORD Len = 0);

	// Освободить память выделенную под строку
	void Free(PWCHAR Str);

	// Функция расчитывает длину строки
	DWORD CalcLength(PWCHAR Str);

	// Функция возвращает истину если строка пустая
	bool IsEmpty(PWCHAR Str);

	// Расчитать длину строки
	DWORD CalcLength(PWCHAR Str);

	// Функция преобразовывает WideString в ANSI String
	PCHAR ToAnsi(LPCWSTR Str, DWORD Len);

	// Возвращает указатель на конец строки
	PWCHAR End(PWCHAR Str);

	// Функция возвращает указатель на последний символ С
	PWCHAR ScanEnd(PWCHAR Str, WCHAR C);

	// Функция расчитывает хэш строки
	// Str - исходная строка
	// Len - Длина строки, если равно 0, то расчитывается до конечного нуля
	// LowerCase - приводить символы в нижний регистр перед созданием хэша
	DWORD GetHash(const PWCHAR Str, DWORD Len, bool LowerCase);
}


#define CalcHash(Str) (STR::GetHash(Str, 0, false))
#define CalcHashA(Str) (STR::GetHash(Str, 0, false))
#define CalcHashW(Str) (WSTR::GetHash(Str, 0, false))


// ----------------------------------------------------------------------------
//  Strings - Набор функция для работы со списками строк
// ----------------------------------------------------------------------------

typedef LPVOID PStrings;

namespace Strings
{
	// Создать набор строк
	PStrings Create();

	// Уничтожить набор строк
	void Free(PStrings Strings);

	// Очистить набор строк
	void Clear(PStrings Strings);

	// Получить количество строк
	DWORD Count(PStrings Strings);

	// Добавить новую строку/
	// По умолчанию строка дублируется. Чтобы отключить эту опцию
	// установите Duplicate = false. Строка Str должна быть создана
	// функцциями StrNew либо StrAlloc
	// В этом случае строка будет помещена в список
	// и управление жьзнью строки будет передану списку строк!!!
	int Add(PStrings Strings, PCHAR Str, bool Duplicate = true);

	// Получит строку из позиции
	PCHAR GetItem(PStrings Strings, DWORD Index, bool DuplicateStr = true);

	// Объеденяет все строки в одну. Если не указан
	// разделитель то использован разделитель заданный в настройках
	PCHAR GetText(PStrings Strings, PCHAR LineDelimeter = NULL);

	// Функция разбирает текст на строки разделённые символами новой
	// строки и перевода каретки
	void SetText(PStrings Strings, PCHAR Text);

	// Удалить строку в позиции Index
	void Delete(PStrings Strings, DWORD Index);

	// Функция удаляет строку Str из списка
	int Remove(PStrings Strings, PCHAR Str);

	// Функция возвращает позицию строки Str в списке
	int IndexOf(PStrings Strings, PCHAR Str);

	// Добавить в список пару Имя=Значение.
	// Если не указан разделитель то будет взят из настроек списка (по умолчанию =)
	int AddValue(PStrings Strings, PCHAR Name, PCHAR Value, PCHAR Delimeter = NULL);
}
//----------------------------------------------------------------------------------------------------


#define CharIsDigit(C)  ((C >= '0') && (C <= '9'))
#define LowerChar(C) if (C >= 'A' && C <= 'Z') {C = C + ('a'-'A');}

// Класс, текстовый буфер
template <class TCharType>
class TStrBuf : public TBotClass
{
private:
	DWORD FSize;      // Размер выделенной памяти
	DWORD FLength;    // Реальная длина строки
	int   FRefCount;  // Количество ссылок на строку
	TCharType* FData; // Буфкр данных

	void AllocMem(DWORD aSize);
	void SetSize(DWORD NewSize);
public:

	TStrBuf(DWORD aSize);
	TStrBuf(const TCharType* Source);

	~TStrBuf() { HEAP::Free(FData);	}

	TStrBuf*  AddRef();
	static void  Release(TStrBuf* &Buf);

	TStrBuf* Unique(int NewSize);
	TStrBuf* Unique() { return Unique(-1); }

	inline TCharType* t_str() const { return FData; };

	DWORD inline Length() { return FLength; }
	DWORD static CalcLength(const TCharType *Str);
	DWORD CalcLength();

	void Copy(const TCharType* Source, DWORD Position,  DWORD Count);
	void Copy(const TCharType* Source);
	void Concat(const TCharType* Str, DWORD StrLen = 0);
	void static Concat(TStrBuf* &Buf, const TCharType* Str, DWORD StrLen);

	// Функциии для расчёта хэша
	DWORD static Hash(const TCharType* Str, DWORD Len = 0, bool LowerCase = false);
	DWORD inline Hash(DWORD Len = 0, bool LowerCase = false);

	// Функции сравнения строк
	int static Compare(const TCharType* Str1, const TCharType* Str2);
	int inline Compare(const TCharType* Str);

	bool static IsEqual(TStrBuf* Str1, const TCharType* Str2);
	bool static IsEqual(TStrBuf* Str1, TStrBuf* Str2);
};





//***************************************************************
//  Шаблон строки
//
// Важно!!!
// Если строка исполузуется в качестве буфера для работы
// с WinApi её длина должна быть пересчитана принудительно
//
//	string Path(MAX_PATH);
//	SHGetSpecialFolderPathA(NULL, Path.c_str(), CSIDL_APPDATA, TRUE);
//	Path.CalcLength();
//  Path += "\\Data\\";
//
//***************************************************************
template <class TCharType>
class TCustomString : public TBotClass
{
private:
    TStrBuf<TCharType> *FData;
public:
    TCustomString();
	TCustomString(const TCustomString &Source);
    TCustomString(const TStrBuf<TCharType> &Source);
	TCustomString(const TCharType* Source);

	~TCustomString() { TStrBuf<TCharType>::Release(FData); }

	void Copy(const TCharType* Source, DWORD Position, DWORD Count);
    void Copy(const TCustomString<TCharType> &Source, DWORD Position, DWORD Count);

	inline DWORD Length()      { return FData->Length(); }
	inline DWORD CalcLength()  { return FData->CalcLength(); }

    DWORD inline Hash(DWORD Len = 0, bool LowerCase = false) {return FData->Hash(Len, LowerCase); }

	inline TCharType* t_str() const { return FData->t_str(); }

	TCustomString& operator =(const TCustomString &Source);
	TCustomString& operator =(const TCharType* Source);
	TCustomString operator +(const TCustomString &Source);
	TCustomString operator +(const TCharType* Source);
	TCustomString& operator +=(const TCustomString &Source);
	TCustomString& operator +=(const TCharType* Source);
	bool operator ==(const TCustomString &Str);
	bool operator ==(const TCharType* Str);

};


#include "StrTemplates.cpp"



// Реализации конкретных типов строк

typedef TStrBuf<char> AnsiBuf, StrBufA, STRBUFA;
typedef TStrBuf<wchar_t> UnicodeBuf, StrBufW, STRBUFW;


typedef TCustomString<char>  string;
typedef TCustomString<wchar_t>  wstring;


//----------------------------------------------------------------------------
#endif
