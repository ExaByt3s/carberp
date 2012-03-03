

#ifndef StringsH
    Исключите файл из проекта
#endif


#include "Strings.h"


//*****************************************************************************
//
// 									 TStrBuf
//
//*****************************************************************************


template <class TCharType>
void TStrBuf<TCharType>::AllocMem(DWORD aSize)
{
	// Функция выделяет память
	FData = (TCharType*)HEAP::Alloc((aSize + 1) * sizeof(TCharType));
	FSize = aSize;
	FLength = 0;
	FRefCount = 1;
}


template <class TCharType>
TStrBuf<TCharType>::TStrBuf(DWORD aSize)
{
	AllocMem(aSize);
};


template <class TCharType>
TStrBuf<TCharType>::TStrBuf(const TCharType* Source)
{
	DWORD Len = CalcLength(Source);
	AllocMem(Len);
	Copy(Source, 0, Len);
};


template <class TCharType>
DWORD TStrBuf<TCharType>::CalcLength(const TCharType *Str)
{
	DWORD Len = 0;
	if (Str != NULL)
		while (*Str++) Len ++;

	return Len;
}



template <class TCharType>
DWORD TStrBuf<TCharType>::CalcLength()
{
	FLength = CalcLength(FData);
	return FLength;
}

template <class TCharType>
TStrBuf<TCharType>*  TStrBuf<TCharType>::AddRef()
{
	FRefCount++;
	return this;
}


template <class TCharType>
void TStrBuf<TCharType>::Release(TStrBuf* &Buf)
{
	Buf->FRefCount--;
	if (Buf->FRefCount <= 0)
		delete Buf;
	Buf = NULL;
};


template <class TCharType>
void  TStrBuf<TCharType>::SetSize(DWORD NewSize, bool CopyData)
{
	// Функция устанавливает новый размер буфера
	if (FSize == NewSize) return;

	TCharType *Temp = FData;

	FData = (TCharType *)HEAP::Alloc((NewSize + 1) * sizeof(TCharType));
	FLength = 0;

	FSize = NewSize;
	if (Temp != NULL)
	{
		// Копируем старые данные
		if (CopyData)
		{
			DWORD ToCopy = Min(FSize, FLength);
			m_memcpy(FData, Temp, ToCopy * sizeof(TCharType));
            FLength = ToCopy;
        }
		HEAP::Free(Temp);
	}

}


template <class TCharType>
 TStrBuf<TCharType>* TStrBuf<TCharType>::Unique(int NewSize, bool CopyData)
{
	// Функция уникализирует строку

	// В случае если на строку ссылается только один владелец
	// то возвращаем её-же
	if (FRefCount == 1)
	{
		if (NewSize >= 0)
			SetSize(NewSize, CopyData);
		return this;
    }

	// Дублируем строку
	DWORD Size = (NewSize >= 0)? NewSize : FSize;
	DWORD Len  = Length();
	TStrBuf<TCharType>* Res = new TStrBuf<TCharType>(Size);
	if (CopyData)
		Res->Copy(FData, 0, Len);

	FRefCount--;
	return Res;
}


template <class TCharType>
void TStrBuf<TCharType>::Copy(const TCharType* Source, DWORD Position,  DWORD Count)
{
	if (Source == NULL || Count == 0)
		return;

	DWORD ToCopy = Min(Count, FSize);

	m_memcpy(FData, Source + Position, ToCopy * sizeof(TCharType));
	FLength = ToCopy;
    *(FData + FLength) = 0;
}

template <class TCharType>
void TStrBuf<TCharType>::Copy(const TCharType* Source)
{
	Copy(Source, 0, CalcLength(Source));
}



template <class TCharType>
void TStrBuf<TCharType>::Concat(const TCharType* Str, DWORD StrLen)
{
	// Функция добавляет к текущей строке строку Str
	// Функция не выделяет память, использует то, что есть

	if (StrLen == 0)
		StrLen = CalcLength(Str);
	if (Str == NULL || StrLen == 0) return;

	DWORD Len = Length();
	DWORD FreeSize = FSize - Len;
	DWORD ToCopy = Min(FreeSize, StrLen);

	TCharType* Temp = FData + Len;
	m_memcpy(Temp, Str, ToCopy * sizeof(TCharType));
	FLength = Len + ToCopy;
	*(FData + FLength) = 0;
}


template <class TCharType>
void TStrBuf<TCharType>::Concat(TStrBuf<TCharType>* &Buf, const TCharType* Str, DWORD StrLen)
{
   // Функция добавляет к буферу строку, при необходимости, выделяя память
	if (StrLen == NULL)
		StrLen = CalcLength(Str);
	if (StrLen == 0) return;

	if (Buf == NULL)
		Buf = new TStrBuf<TCharType>(StrLen);

	DWORD TotalSize = Buf->Length() + StrLen;

	Buf = Buf->Unique(TotalSize);
	Buf->Concat(Str, StrLen);
}



template <class TCharType>
int TStrBuf<TCharType>::Compare(const TCharType* Str1, const TCharType* Str2)
{
	// Функция сравнивает строки
	if (Str1 == NULL || Str2 == NULL)
		return -1;

	const TCharType* S1 = Str1;
	const TCharType* S2 = Str2;

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

template <class TCharType>
int TStrBuf<TCharType>::Compare(const TCharType* Str)
{
	return Compare(FData, Str);
}


template <class TCharType>
bool TStrBuf<TCharType>::IsEqual(TStrBuf<TCharType>* Str1, const TCharType* Str2)
{
	// Функция возвращает истину если строки идентичны
	bool Empty1 = (Str1 == NULL || Str1->FData == NULL || *Str1->FData == 0);
	bool Empty2 = (Str2 == NULL || *Str2 == 0);

	if (Empty1 && Empty2) return true;

    return Compare(Str1->FData, Str2) == 0;
}


template <class TCharType>
bool TStrBuf<TCharType>::IsEqual(TStrBuf<TCharType>* Str1, TStrBuf<TCharType>* Str2)
{
	// Функция возвращает истину если строки идентичны
	bool Empty1 = (Str1 == NULL || Str1->FData == NULL || *Str1->FData == 0);
	bool Empty2 = (Str2 == NULL || Str2->FData == NULL || *Str2->FData == 0);

	if (Empty1 && Empty2) return true;

    return Compare(Str1->FData, Str2->FData) == 0;
}


template <class TCharType>
DWORD TStrBuf<TCharType>::Hash(const TCharType* Str, DWORD Len, bool LowerCase)
{
	// Функция расчитывает хэш строки
	// Str - исходная строка
	// Len - Длина строки, если равно 0, то расчитывается до конечного нуля
	// LowerCase - приводить символы в нижний регистр перед созданием хэша

	if (Str == NULL) return (DWORD)-1; // Наследие Фрека

	DWORD H = 0;

	for (DWORD i = 0; *Str != 0 && (Len == 0 || i < Len); Str++, i++)
	{
		TCharType Ch = *Str;
		if (LowerCase) LowerChar(Ch);
		H = (( H << 7 ) & (DWORD)( -1 ) ) | ( H >> ( 32 - 7 ));
		H = H ^ Ch;
	}

	return H;
}


template <class TCharType>
DWORD TStrBuf<TCharType>::Hash(DWORD Len, bool LowerCase)
{
	return Hash(FData, Len, LowerCase);
}




//*****************************************************************************
//
// 									 TCustomString
//
//*****************************************************************************

template <class TCharType>
TCustomString<TCharType>::TCustomString()
{
    FData = new TStrBuf<TCharType>((DWORD)0);
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>::TCustomString(DWORD StrLen)
{
    FData = new TStrBuf<TCharType>(StrLen);
}
//----------------------------------------------------------------------------


template <class TCharType>
TCustomString<TCharType>::TCustomString(const TCustomString<TCharType> &Source)
{
	FData = Source.FData->AddRef();
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>::TCustomString(const TStrBuf<TCharType> &Source)
{
	// При создании строки из буфера обязательно пересчитываем
	// длину буфера
	FData = new TStrBuf<TCharType>(Source.t_str());
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>::TCustomString(const TCharType* Source)
{
	FData = new TStrBuf<TCharType>(Source);
}
//----------------------------------------------------------------------------

template <class TCharType>
void TCustomString<TCharType>::Copy(const TCharType* Source, DWORD Position, DWORD Count)
{
	FData = FData->Unique(Count);
	FData->Copy(Source, Position, Count);
}
//----------------------------------------------------------------------------

template <class TCharType>
void TCustomString<TCharType>::Copy(const TCustomString<TCharType> &Source, DWORD Position, DWORD Count)
{
	FData = FData->Unique(Count);
	FData->Copy(Source.FData, Position, Count);
}
//----------------------------------------------------------------------------


template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator=(const TCharType* Source)
{
	DWORD Len = TStrBuf<TCharType>::CalcLength(Source);
	FData = FData->Unique(Len, false);
	FData->Copy(Source, 0, Len);
	return *this;
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator=(const TCustomString<TCharType> &Source)
{

	TStrBuf<TCharType>::Release(FData);
	FData = Source.FData->AddRef();
	return *this;
}
//----------------------------------------------------------------------------


template <class TCharType>
TCustomString<TCharType> TCustomString<TCharType>::operator+(const TCustomString<TCharType> &Source)
{
	TCustomString<TCharType> Temp(*this);
	Temp += Source;
	return Temp;
}
//----------------------------------------------------------------------------


template <class TCharType>
TCustomString<TCharType> TCustomString<TCharType>::operator+(const TCharType* Source)
{
	TCustomString<TCharType> Temp(*this);
	Temp += Source;
	return Temp;
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator +=(const TCustomString<TCharType> &Source)
{
	TStrBuf<TCharType>::Concat(FData, Source.FData->t_str(),  Source.FData->Length());
	return *this;
}
//----------------------------------------------------------------------------

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator+=(const TCharType* Source)
{
	TStrBuf<TCharType>::Concat(FData, Source, 0);
    return *this;
}
//----------------------------------------------------------------------------

template <class TCharType>
bool TCustomString<TCharType>::operator==(const TCustomString<TCharType> &Str)
{
	return TStrBuf<TCharType>::IsEqual(FData, Str);
}
//----------------------------------------------------------------------------

template <class TCharType>
bool TCustomString<TCharType>::operator==(const TCharType* Str)
{
	return TStrBuf<TCharType>::IsEqual(FData, Str);
}
//----------------------------------------------------------------------------
