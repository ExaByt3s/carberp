
#ifndef StringsH
	Исключите файл из проекта
#endif


#include "Memory.h"


//*****************************************************************************
//                               STRUTILS
//*****************************************************************************

#define CHARAPI(Result_Type) template<class TChar> Result_Type STRUTILS<TChar>


// Функция возвращает истину если строка пустая
CHARAPI(bool)::IsEmpty(const TChar* Str)
{
	return Str == NULL || *Str == 0;
}
//-----------------------------------------------------------------------------


CHARAPI(DWORD)::Length(const TChar* Str)
{
	// Функция расчитывает длину строки
	DWORD Len = 0;
	if (Str != NULL)
		while (*Str++) Len++;

	return Len;
}
//-----------------------------------------------------------------------------


CHARAPI(bool)::Equal(const TChar* Str1, const TChar* Str2)
{
	// Функция возвращает истину если строки идентичны
	if (Str1 == Str2 || (IsEmpty(Str1) && IsEmpty(Str2)))
		return true;
	else
    	return Compare(Str1, Str2) == 0;
}
//-----------------------------------------------------------------------------

CHARAPI(int)::Compare(const TChar* Str1, const TChar* Str2)
{
	// Функция сравнивает две строки

	if (Str1 == NULL || Str2 == NULL)
		return -1;

	const TChar* S1 = Str1;
	const TChar* S2 = Str2;

	while(1)
	{
		if (*S1 != *S2)
		{
			if (*S1 > *S2)
				return 1;
			else
            	return -1;
		}

		if (*S1 == 0) return 0;

		S1++;
		S2++;
    }
}
//-----------------------------------------------------------------------------

// Функция расчитывает хэш строки
CHARAPI(DWORD)::Hash(const TChar* Str, DWORD Len, bool LowerCase)
{
	// Функция расчитывает хэш строки
	// Str - исходная строка
	// Len - Длина строки, если равно 0, то расчитывается до конечного нуля
	// LowerCase - приводить символы в нижний регистр перед созданием хэша

	if (Str == NULL) return (DWORD)-1; // Наследие Фрека

	DWORD H = 0;

	for (DWORD i = 0; *Str != 0 && (Len == 0 || i < Len); Str++, i++)
	{
		TChar Ch = *Str;
		if (LowerCase) LowerChar(Ch);
		H = (( H << 7 ) & (DWORD)( -1 ) ) | ( H >> ( 32 - 7 ));
		H = H ^ Ch;
	}

	return H;
}


CHARAPI(DWORD)::Hash(const TChar* Str)
{
    return Hash(Str, 0, false);
}
//-----------------------------------------------------------------------------



//*****************************************************************************
//                               STRBUF
//*****************************************************************************

STRBUFAPI(TChar*) STRBUF::Alloc(DWORD Size)
{
	// Функция создаёт строку размером Size символов
	// Длина строки устанавливается нулевой
	if (Size == 0) return NULL;

	#ifdef USE_BLOCKS_STRINGS
        Size = ((Size / STRING_BLOCK_SIZE) + 1) * STRING_BLOCK_SIZE;
	#endif

	DWORD BufSize = (Size + 1) * sizeof(TChar) + sizeof(TStrRec);
	TStrRec* Buf = (TStrRec*)HEAP::Alloc(BufSize);
	if (Buf == NULL) return NULL;

	Buf->Size = Size;
	Buf->Length = 0;
	Buf->RefCount = 1;

	Buf++;
	return (TChar*)Buf;
}
//----------------------------------------------------------------------------

STRBUFAPI(void) STRBUF::Release(TChar* &Str)
{
	// Функция уменьшает счётчик ссылок и, при необходимости, уничтожает строку
	if (Str)
	{
		TStrRec &R = GetRec(Str);
		R.RefCount--;
		if (R.RefCount == 0)
            HEAP::Free(&R);
        Str = NULL;
    }
}

//----------------------------------------------------------------------------

STRBUFAPI(TChar*) STRBUF::AddRef(TChar* Str)
{
	// Функция увеличивает счётчик ссылок строки и возвращает указатель на неё
	if (Str)
		GetRec(Str).RefCount++;
	return Str;
}
//----------------------------------------------------------------------------

STRBUFAPI(void) STRBUF::Unique(TChar* &Str)
{
	// Функция уникализирует строку
	if (Str)
	{
		TStrRec &R = GetRec<TChar>(Str);
		if (R.RefCount > 1)
		{
			R.RefCount--;
            Str = CreateFromStr(Str, R.Length, 0);
        }
    }
}
//----------------------------------------------------------------------------


STRBUFAPI(STRBUF::TStrRec&) STRBUF::GetRec(TChar* Str)
{
	// Функция возвращает переменную зоголовок строки
	TStrRec* R = (TStrRec*)Str;
    return R[-1];
}
//----------------------------------------------------------------------------

// Функция возвращает длину строки
STRBUFAPI(DWORD) STRBUF::Length(TChar *Str)
{
	return (Str)? GetRec(Str).Length : 0;
}
//----------------------------------------------------------------------------


STRBUFAPI(TChar*) STRBUF::CreateFromStr(const TChar* Str, DWORD StrLen, DWORD ResultStrSize)
{
	// Создаёт строку на основе исходной строки
	if (STRUTILS<TChar>::IsEmpty(Str))
		return NULL;

	if (StrLen == 0)
		StrLen = STRUTILS<TChar>::Length(Str);

    DWORD ResSize = Max(StrLen, ResultStrSize);
	TChar* Result = Alloc<TChar>(ResSize);
	if (Result)
	{
		// Копируем данные строки
		m_memcpy(Result, Str, StrLen * sizeof(TChar));
		GetRec(Result).Length = StrLen;
	}
	return Result;
}
//----------------------------------------------------------------------------


STRBUFAPI(void) STRBUF::Append(TChar* &Dst, const TChar* Src, DWORD SrcLen)
{
	// Функция добавляет к строке Dst строку Src
	if (STRUTILS<TChar>::IsEmpty(Src))
		return;


	if (SrcLen == 0)
		SrcLen = STRUTILS<TChar>::Length(Src);

	if (Dst == NULL)
	{
    	// Создаём строку на основе исходной строки
		Dst = CreateFromStr<TChar>(Src, SrcLen, 0);
	}
	else
	{
		// Объединяем две строки
		TStrRec &R = GetRec(Dst);
		TChar* Tmp = NULL;
		DWORD DstLen = R.Length;
		DWORD TotalLen = SrcLen + DstLen;
		if (R.Size < TotalLen || R.RefCount > 1)
		{
        	Tmp = Dst;
			Dst = CreateFromStr<TChar>(Tmp, DstLen, TotalLen);
        }
		m_memcpy(Dst + DstLen, Src, SrcLen);
		GetRec(Dst).Length = TotalLen;

		if (Tmp)
			Release<TChar>(Tmp);
    }
}
//----------------------------------------------------------------------------

STRBUFAPI(void) STRBUF::Copy(TChar* &Dst, const TChar* Src, DWORD Pos, DWORD Count)
{
	// Функция копирует количество символов Count с позиции Pos
	if (STRUTILS<TCHAR>::IsEmpty(Src) || Count == 0)
		return;

	if (Dst == NULL)
	{
		// Создаём новую строку
        Dst = CreateFromStr(Src + Pos, Count, 0);
	}
	else
	{
		TStrRec &R = GetRec(Dst);

		// В случае ели трока уже оздана проверяем два момента
		// - Количество сылок. Если больше единицы то создаём новую строку
		// - Длину строки. Если длина нулевая и размер памяти позволяет скопировать
		//                 нужное количество символов то читаем, что память строки
		//				   изначально выделялась с учётом данной операции

		if (R.RefCount == 1 && R.Length == 0 && R.Size >= Count)
		{
			m_memcpy(Dst, Src + Pos, Count * sizeof(TChar));
			R.Length = Count;
            *(Dst + Count) = 0;
		}
		else
		{
			// На случай если копирование происходит из самой себя
			// уничтожим строку после копирования
			TCHAR* Tmp = Dst;
			Dst = CreateFromStr(Src + Pos, Count, 0);
			Release<TChar>(Tmp);
		}
	}
}
//----------------------------------------------------------------------------

STRBUFAPI(void) _Insert_RePack(TChar* Buf, const TChar* Src, DWORD SrcLen, const TChar* Dst, DWORD DstLen, DWORD Position)
{
	// Функция упаковывает строку в контексте работы Insert
	m_memcpy(Buf, Src, Position * sizeof(TChar));
	Buf += Position;
	Src += Position;
	m_memcpy(Buf, Dst, DstLen * sizeof(TChar));
	Buf += DstLen;
	m_memcpy(Buf, Src, (SrcLen - Position) * sizeof(TChar));
}


STRBUFAPI(void) _Insert_Expand(TChar *Dst, DWORD DstLen,  const TChar *Src, DWORD SrcLen, DWORD Position)
{
	// Функция раздвигает строку и вставляет текст
	TChar *StartPtr = Dst + Position;
	TChar *EndPtr   = Dst + DstLen;
	TChar *ToCopy   = Dst + (DstLen + SrcLen);

	// Раздвигаем строку
	while (EndPtr >= StartPtr)
	{
		*ToCopy = *EndPtr;
		ToCopy--;
		EndPtr--;
    }

    m_memcpy(StartPtr, Src, SrcLen);

	// Закрываем строку
    *(Dst + DstLen + SrcLen) = 0;
}


STRBUFAPI(void) STRBUF::Insert(TChar* &Buf, const TChar* Str, DWORD Position, DWORD StrLen)
{
	// Функция вставляет строку Str в буфер
	if (STRUTILS<TChar>::IsEmpty(Str))
		return;


	if (Buf == NULL)
	{
		// Буфер пустой, если позиция вставки равна нулю, то создаём строку
		// на основе вставляемой строки
		if (Position == 0)
        	Buf = CreateFromStr(Str, 0, 0);
        return;
    }

	// Вставляем строку
	if (StrLen == 0)
		StrLen = STRUTILS<TChar>::Length(Str);

	TStrRec &R = GetRec<TChar>(Buf);

	// Проверяем позицию
	if (Position > R.Length)
		return;

	// Определяем размер буфера
	DWORD TotalLen = R.Length + StrLen;
	if (R.Size < TotalLen || R.RefCount > 1)
	{
		// Создаём новую строку
		TChar *Tmp = Alloc<TChar>(TotalLen);
		_Insert_RePack<TChar>(Tmp, Buf, R.Length, Str, StrLen, Position);
		Release<TChar>(Buf);
		Buf = Tmp;
	}
	else
    	_Insert_Expand(Buf, R.Length, Str, StrLen, Position);

	// Вставляем данные
	GetRec(Buf).Length = TotalLen;
}

//----------------------------------------------------------------------------



//*********************************************************************
//   							TString
//*********************************************************************

#define STRFUNC(Result_Type) template<class TChar> Result_Type TString<TChar>
#define STRCONSTRUCTOR() template<class TChar> TString<TChar>

STRCONSTRUCTOR()::TString(unsigned long StrBufSize)
{
	Data = STRBUF::Alloc<TChar>(StrBufSize);
}

STRCONSTRUCTOR()::TString(const TString& src)
{
	Data = STRBUF::AddRef<TChar>(src.Data);
}


STRCONSTRUCTOR()::TString(const TChar* Src)
{
	Data = STRBUF::CreateFromStr<TChar>(Src, 0, 0);
}

STRCONSTRUCTOR()::~TString()
{
	STRBUF::Release<TChar>(Data);
}


STRFUNC(DWORD)::Length() const
{
	return  STRBUF::Length<TChar>(Data);
}

STRFUNC(DWORD)::CalcLength()
{
	if (Data)
		STRBUF::GetRec<TChar>(Data).Length = STRUTILS<TChar>::Length(Data);
	return Length();
}

STRFUNC(bool)::IsEmpty() const
{
	return STRUTILS<TChar>::IsEmpty(Data);
}


STRFUNC(TChar*)::t_str() const
{
	// Функция возвращает указатель на данные
	// В случае отсутствия данных функция возвращает
	// указатель на пустую строку
	return (Data)? Data : (TChar*)L"";
}

STRFUNC(void)::Clear()
{
	STRBUF::Release<TChar>(Data);
}


STRFUNC(void)::Copy(const TChar* Source, DWORD Position, DWORD Count)
{
	STRBUF::Copy<TChar>(Data, Source, Position, Count);
}


STRFUNC(void)::Copy(const TString &Source, DWORD Position, DWORD Count)
{
	STRBUF::Copy<TChar>(Data, Source.Data, Position, Count);
}

STRFUNC(void)::Insert(const TChar* Str, DWORD Position)
{
    STRBUF::Insert<TChar>(Data, Str, Position, 0);
}


STRFUNC(void)::Insert(const TString &Str, DWORD Position)
{
	STRBUF::Insert<TChar>(Buf, Str.Data, Position, Str.Length());
}


STRFUNC(DWORD)::Hash()
{
	return STRUTILS<TChar>::Hash(Data);
}

STRFUNC(DWORD)::Hash(DWORD Len, bool LowerChar)
{
	return STRUTILS<TChar>::Hash(Data, Len, LowerChar);
}

STRFUNC(void)::Unique()
{
    STRBUF::Unique<TChar>(Data);
}


STRFUNC(TString<TChar>&)::operator=(const TString &Source)
{
	if (Data != Source.Data)
	{
		STRBUF::Release<TChar>(Data);
		Data = STRBUF::AddRef<TChar>(Source.Data);
    }
	return *this;
}

STRFUNC(TString<TChar>&)::operator=(const TChar* Source)
{
	STRBUF::Release<TChar>(Data);
	Data = STRBUF::CreateFromStr<TChar>(Source, 0, 0);
	return *this;
}

STRFUNC(TString<TChar>&)::operator+=(const TString &Source)
{
	if (Source.Data)
		STRBUF::Append<TChar>(Data, Source.Data, Source.Length());
	return *this;
}

STRFUNC(TString<TChar>&)::operator+=(const TChar* Source)
{
	if (Source)
    	STRBUF::Append<TChar>(Data, Source, 0);
	return *this;
}

STRFUNC(TString<TChar>)::operator+(const TString &Source)
{
	TString<TChar> Temp(*this);
	Temp += Source;
	return Temp;
}

STRFUNC(TString<TChar>)::operator+(const TChar* Source)
{
	TString<TChar> Temp(*this);
	Temp += Source;
	return Temp;
}

STRFUNC(bool)::operator ==(const TString &Str)
{
	return STRUTILS<TChar>::Equal(Data, Str.Data);
}

STRFUNC(bool)::operator ==(const TChar* Str)
{
	return STRUTILS<TChar>::Equal(Data, Str);
}

STRFUNC(bool)::operator !=(const TString &Str)
{
	return !STRUTILS<TChar>::Equal(Data, Str.Data);
}

STRFUNC(bool)::operator !=(const TChar* Str)
{
	return !STRUTILS<TChar>::Equal(Data, Str);
}

STRFUNC(TChar)::operator[](const DWORD Index) const
{
	if (Index < Length())
		return Data[Index];
	else
    	return 0;
}



STRFUNC(TChar&)::operator[](const DWORD Index)
{
	// В нормальном варианте  в случае ошибочного индекса
	// нужно сгенерировать исключение, но бот не использует
	// исключения. Есть потенциальная опастность в случае
	// если индекс выйдет за пределы строки
	if (Index < Length())
	{
    	Unique();
		return Data[Index];
	}
}


