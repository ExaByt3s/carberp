

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
	Copy(Source, Len);
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
void  TStrBuf<TCharType>::SetSize(DWORD NewSize)
{
	// Функция устанавливает новый размер буфера
	if (FSize == NewSize) return;

	TCharType *Temp = FData;

	FData = (TCharType *)HEAP::Alloc((NewSize + 1) * sizeof(TCharType));

	FSize = NewSize;
	if (Temp != NULL)
	{
		// Копируем старые данные
		DWORD ToCopy = Min(FSize, FLength);
		m_memcpy(FData, Temp, ToCopy * sizeof(TCharType));
		HEAP::Free(Temp);
		FLength = ToCopy;
    }

}


template <class TCharType>
 TStrBuf<TCharType>* TStrBuf<TCharType>::Unique(DWORD NewSize)
{
	// Функция уникализирует строку

	// В случае если на строку ссылается только один владелец
	// то возвращаем её-же
	if (FRefCount == 1)
	{
		if (NewSize != 0)
			SetSize(NewSize);
		return this;
    }

	// Дублируем строку
	DWORD Size = (NewSize != 0)? NewSize : FSize;
	DWORD Len  = Length();
	TStrBuf<TCharType>* Res = new TStrBuf<TCharType>(Size);
	Res->Copy(FData, Len);

	FRefCount--;
	return Res;
}


template <class TCharType>
void TStrBuf<TCharType>::Copy(const TCharType* Source,  DWORD SourceLen)
{
	if (Source == NULL)
	{
		m_memset(FData, 0, FSize * sizeof(TCharType));
		FLength = 0;
	}

	if (SourceLen == NULL)
		SourceLen  = CalcLength(Source);

    DWORD ToCopy = Min(SourceLen, FSize);

	m_memcpy(FData, Source, ToCopy * sizeof(TCharType));
	FLength = ToCopy;
    *(FData + FLength) = 0;
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
	FLength = Len + StrLen;
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



//*****************************************************************************
//
// 									 TCustomString
//
//*****************************************************************************


template <class TCharType>
TCustomString<TCharType>::TCustomString(const TCustomString<TCharType> &Source)
{
	FData = Source.FData->AddRef();
}

template <class TCharType>
TCustomString<TCharType>::TCustomString(DWORD Size)
{
    FData = new TStrBuf<TCharType>(Size);
}

template <class TCharType>
TCustomString<TCharType>::TCustomString(const TCharType* Source)
{
	FData = new TStrBuf<TCharType>(Source);
}

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator=(const TCharType* Source)
{

	TStrBuf<TCharType>::Release(FData);
	FData = new TStrBuf<TCharType>(Source);
	return *this;
}

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator=(const TCustomString<TCharType> &Source)
{

	TStrBuf<TCharType>::Release(FData);
	FData = Source.FData->AddRef();
	return *this;
}


template <class TCharType>
TCustomString<TCharType> TCustomString<TCharType>::operator+(const TCustomString<TCharType> &Source)
{
	TCustomString<TCharType> Temp(*this);
	Temp += Source;
	return Temp;
}


template <class TCharType>
TCustomString<TCharType> TCustomString<TCharType>::operator+(const TCharType* Source)
{
	TCustomString<TCharType> Temp(*this);
	Temp += Source;
	return Temp;
}
//

template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator +=(const TCustomString<TCharType> &Source)
{
	TStrBuf<TCharType>::Concat(FData, Source.FData->Data(),  Source.FData->Length());
	return *this;
}


template <class TCharType>
TCustomString<TCharType>& TCustomString<TCharType>::operator+=(const TCharType* Source)
{
	TStrBuf<TCharType>::Concat(FData, Source, 0);
    return *this;
}

template <class TCharType>
bool TCustomString<TCharType>::operator==(const TCustomString<TCharType> &Str)
{
	return TStrBuf<TCharType>::IsEqual(FData, Str);
}

template <class TCharType>
bool TCustomString<TCharType>::operator==(const TCharType* Str)
{
	return TStrBuf<TCharType>::IsEqual(FData, Str);
}
