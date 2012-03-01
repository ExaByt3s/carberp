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

//������� � ������ � ����� ������� c, ���������� ��������� �� s, ������ ������ ��� ������������ ������
char* trimall( char* s, char c = ' ' );


// ----------------------------------------------------------------------------
//  ����� ������� ��� ������ �� ��������
//
//  ���� ������������ ������ ������� ��� ������ �� ��������, ��? � ���������
//  �������, ����� ������� �������� ��������������. ��������, �������
//  StrLength �����������  ��������� �� �������������� ������� �������
//  ������������ ����� ������ �������� � �� �������� �������, � ��� ������
//  ������ ��� ������ ������� � ��������������
// ----------------------------------------------------------------------------

// �������� ������ ������ ������
// �����!!! ������� �������� �� �������� ������� ������� �������� StrNew
//void StrSetLength(PCHAR &Str, DWORD NewLength);


// ������� ����������� ����� ������ �������� �� � ������� ������������ ����
DWORD StrCalcLength(const char* Buf);
DWORD StrCalcLengthW(const wchar_t* Buf);


//  ������� StrConcat ���������� ��� � ����� ������.
//  ���� ������ ������ ������ Str1 ������ ������������,
//  �� ������ Str1 ����� ��������������
//  Count ���������� ������� ����� ����� ��������� � Str1
void StrConcat(PCHAR &Str1, PCHAR Str2);
void StrConcat(PCHAR &Str1, DWORD Count, PCHAR Str2...);

// ������� �������� ������ Source � ������ Dest
// ���� UpdateLen ������������ ������ ���� Dest ������� ���������
// StrAlloc, StrNew
// ���������� ������ ���� ���������� ����-�� ���� ����
bool StrCopy(PCHAR Dest, PCHAR Source, bool UpdateLen = false);

// �������� ��� ������
int StrCompare(const char* Str1, const char* Str2);

// ������� �� �������������� ����� � ����� � ��������
int StrToInt(PCHAR Str);
PCHAR StrLongToString(DWORD num);

// ������� �������� ������ � ������� ��������
void StrLowerCase(PCHAR Str);

// ������� ���� ������� ������ � ������� ����� �mo
int StrIndexOf(PCHAR Str, bool CaseSensetive, DWORD Count, PCHAR Cmp...);

// ������� ���������� ������ Str �� ������� ToSame.
// CaseSensetive ���������� ���������������� � ��������
// ���� StrEndPosrition �� ����� ����, �� ���������
// �������������� StrEndPosrition ���������
bool StrSame(PCHAR Str, PCHAR ToSame, bool CaseSensetive = true, DWORD StrEndPosition = 0);

// ������� ���������� ����� ������������  ����� ������� Before � After
// ����� ��������� ��������� Buffer  ����� ��������� �� ����� ������ After
// ��������� ���������� �������� StrFree
PCHAR GetTextBetween(PCHAR &Buffer, PCHAR Before, PCHAR After );

//----------------------------------------------------------------------------
//  STR - ������ ��� ������ �� ��������
//----------------------------------------------------------------------------
namespace STR
{
	// ������ � ������ ������ ������ ������� ��� �� ����� �������� StrLen + 1
	PCHAR Alloc(DWORD StrLen);

	// ������� ������ ������, ����� Source.
	// Len - ���������� ���������� ��������
	//       ���� ����� 0, �� ����� ����������� ��� ������
	PCHAR New(PCHAR Source, DWORD Len = 0);

	// ������� ������ ������ ��������� ��������� �����
	PCHAR New(DWORD Count, PCHAR Str...);

	// ���������� ������.
	// �����!!! ������ ������ ���� ������� � ������� ������� StrNew
	void Free(PCHAR Str);
	void Free2(PCHAR &Str);

	// ������� ���������� ����� ������.
	DWORD Length(PCHAR Str);

	// ��������� �������� ����� ������ � � ����������.
	// !!!!������ ��� ����� ��������� ����� STR::Alloc (STR::New)
	// ���� �������� RealLength �� ������� �� ����� �����
	// ������������� �� �������� �������
	void UpdateLength(PCHAR Str, DWORD RealLength = 0);

	// ������� ������� ������ SubStr  � ������ Str
	// ���� �������� StrLen �� ����� ����� ������� ��
	// �������� �������
	int Pos(const char* Str, const char* SubStr, DWORD StrLen = 0, bool CaseSensetive = true);

	// �������� � ������� Position ������ Source Count �������� �
	// ������ Destination
    void Copy(PCHAR Source, PCHAR Destination, DWORD Position, DWORD Count);

	// ������� Count �������� � ������� Position ������ Str
	void Delete(PCHAR Str, DWORD Position, DWORD Count, DWORD StrLength = 0);

	//  Replace - ������� �������� � ������ Str ������ SrcStr ��
	//  ������ DstStr.
	//  � ������ ������ ������� ���������� ����� ������
	PCHAR Replace(PCHAR Str, PCHAR SrcStr, PCHAR DstStr, DWORD StrLen = 0);


	// ������� ���������� ��������� �� ������ �
	PCHAR Scan(const char* Str, char C);

	// ������� ���������� ��������� �� ��������� ������ �
	PCHAR ScanEnd(PCHAR Str, char C);

	// ������� ���������� ����� �����, ������ ��������������
	// ������������ ������ SubStr
	PCHAR GetLeftStr(PCHAR Str, PCHAR SubStr, bool IncludeSubStr = false);
	PCHAR GetRightStr(PCHAR Str, PCHAR SubStr, bool IncludeSubStr = false);


	// ������� ���������� �������. ������� �� �������� ������, ���
	// ���������� ��������� �� ������ ������, ���� �� ����� ������
	PCHAR IgnoreSpaces( const PCHAR Str );

	// ���������� ��������� �� ����� ������
	PCHAR End(PCHAR Str);

	// ������� ����������� ������ �� ������ �������
//	PCHAR Format(PCHAR Str, ...);
	PCHAR Format(PCHAR Str, va_list Arguments);

	// ������� ���������� ������� ����� � ������.
	// ����� �������������� ��������� 10, 13 ���� ������ ������
	// ����� ������ �������������� ������� �������� ���� �������� �������
	PCHAR GetLine(PCHAR Str, DWORD StrSize = 0);

	// ������� ���������� ��������� �� ������ ������ ��������� ����� � ������
	// ���� �� ����� ������
	PCHAR GotoNextLine(PCHAR Str, DWORD StrSize = 0);

    // ������� ��������������� ����������������� ����� � ������������
	DWORD HexToDWORD(PCHAR Str);
	//�������������� ������ � ��������������� �����
	DWORD ToDWORD( const char* s );


	// ������� ���������� ������ ���� �� ����� ������� ��������� ����
	// ������ ������
	bool IsEmpty( const PCHAR Str);

	// ������� ����������� ��� ������
	// Str - �������� ������
	// Len - ����� ������, ���� ����� 0, �� ������������� �� ��������� ����
	// LowerCase - ��������� ������� � ������ ������� ����� ��������� ����
	DWORD GetHash(PCHAR Str, DWORD Len, bool LowerCase);

	// ������� ����������� ����� ������ ��������� �� ���������� �����
	// ��������� ���� �� ����� ������� �������� � ����� ������� �����
	// ������� ������� ������
	// ��������: str0str0str00
	DWORD CalcDoubleZeroStrLength(PCHAR Str);

	// ������� �������� ������� � ������ �������. ������� �� �������������
	// � ��������� ��������
	void AnsiLowerCase(PCHAR Str);

    // ������� ���������� ������ ��� ������ � ������ �������
    bool IsLinuxStr(PCHAR S);

	// ������� �������� ������ � ������� Linux
	PCHAR ConvertToLinuxFormat(PCHAR S);
}

namespace WSTR
{
	// �������� ������ �� ������ ������ StrLen
	PWCHAR Alloc(DWORD StrLen);

	// ������� ������ ������, ����� Source.
	// Len - ���������� ���������� ��������
	//       ���� ����� 0, �� ����� ����������� ��� ������
    PWCHAR New(PWCHAR Source, DWORD Len = 0);

	// ���������� ������ ���������� ��� ������
	void Free(PWCHAR Str);

	// ������� ����������� ����� ������
	DWORD CalcLength(PWCHAR Str);

	// ������� ���������� ������ ���� ������ ������
	bool IsEmpty(PWCHAR Str);

	// ��������� ����� ������
	DWORD CalcLength(PWCHAR Str);

	// ������� ��������������� WideString � ANSI String
	PCHAR ToAnsi(LPCWSTR Str, DWORD Len);

	// ���������� ��������� �� ����� ������
	PWCHAR End(PWCHAR Str);

	// ������� ���������� ��������� �� ��������� ������ �
	PWCHAR ScanEnd(PWCHAR Str, WCHAR C);

	// ������� ����������� ��� ������
	// Str - �������� ������
	// Len - ����� ������, ���� ����� 0, �� ������������� �� ��������� ����
	// LowerCase - ��������� ������� � ������ ������� ����� ��������� ����
	DWORD GetHash(const PWCHAR Str, DWORD Len, bool LowerCase);
}


#define CalcHash(Str) (STR::GetHash(Str, 0, false))
#define CalcHashA(Str) (STR::GetHash(Str, 0, false))
#define CalcHashW(Str) (WSTR::GetHash(Str, 0, false))


// ----------------------------------------------------------------------------
//  Strings - ����� ������� ��� ������ �� �������� �����
// ----------------------------------------------------------------------------

typedef LPVOID PStrings;

namespace Strings
{
	// ������� ����� �����
	PStrings Create();

	// ���������� ����� �����
	void Free(PStrings Strings);

	// �������� ����� �����
	void Clear(PStrings Strings);

	// �������� ���������� �����
	DWORD Count(PStrings Strings);

	// �������� ����� ������/
	// �� ��������� ������ �����������. ����� ��������� ��� �����
	// ���������� Duplicate = false. ������ Str ������ ���� �������
	// ���������� StrNew ���� StrAlloc
	// � ���� ������ ������ ����� �������� � ������
	// � ���������� ������ ������ ����� �������� ������ �����!!!
	int Add(PStrings Strings, PCHAR Str, bool Duplicate = true);

	// ������� ������ �� �������
	PCHAR GetItem(PStrings Strings, DWORD Index, bool DuplicateStr = true);

	// ���������� ��� ������ � ����. ���� �� ������
	// ����������� �� ����������� ����������� �������� � ����������
	PCHAR GetText(PStrings Strings, PCHAR LineDelimeter = NULL);

	// ������� ��������� ����� �� ������ ���������� ��������� �����
	// ������ � �������� �������
	void SetText(PStrings Strings, PCHAR Text);

	// ������� ������ � ������� Index
	void Delete(PStrings Strings, DWORD Index);

	// ������� ������� ������ Str �� ������
	int Remove(PStrings Strings, PCHAR Str);

	// ������� ���������� ������� ������ Str � ������
	int IndexOf(PStrings Strings, PCHAR Str);

	// �������� � ������ ���� ���=��������.
	// ���� �� ������ ����������� �� ����� ���� �� �������� ������ (�� ��������� =)
	int AddValue(PStrings Strings, PCHAR Name, PCHAR Value, PCHAR Delimeter = NULL);
}
//----------------------------------------------------------------------------------------------------


#define CharIsDigit(C)  ((C >= '0') && (C <= '9'))
#define LowerChar(C) if (C >= 'A' && C <= 'Z') {C = C + ('a'-'A');}

// �����, ��������� �����
template <class TCharType>
class TStrBuf : public TBotClass
{
private:
	DWORD FSize;      // ������ ���������� ������
	DWORD FLength;    // �������� ����� ������
	int   FRefCount;  // ���������� ������ �� ������
	TCharType* FData; // ����� ������

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

	// �������� ��� ������� ����
	DWORD static Hash(const TCharType* Str, DWORD Len = 0, bool LowerCase = false);
	DWORD inline Hash(DWORD Len = 0, bool LowerCase = false);

	// ������� ��������� �����
	int static Compare(const TCharType* Str1, const TCharType* Str2);
	int inline Compare(const TCharType* Str);

	bool static IsEqual(TStrBuf* Str1, const TCharType* Str2);
	bool static IsEqual(TStrBuf* Str1, TStrBuf* Str2);
};





//***************************************************************
//  ������ ������
//
// �����!!!
// ���� ������ ������������ � �������� ������ ��� ������
// � WinApi � ����� ������ ���� ����������� �������������
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



// ���������� ���������� ����� �����

typedef TStrBuf<char> AnsiBuf, StrBufA, STRBUFA;
typedef TStrBuf<wchar_t> UnicodeBuf, StrBufW, STRBUFW;


typedef TCustomString<char>  string;
typedef TCustomString<wchar_t>  wstring;


//----------------------------------------------------------------------------
#endif
