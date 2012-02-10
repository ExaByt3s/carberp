//---------------------------------------------------------------------------


#pragma hdrstop

#include "BuilderUtils.h"
#include "Memory.h"
#include "Crypt.h"
#include "Strings.h"

//---------------------------------------------------------------------------

char ERROR_MEMALLOC[] = "Ошибка выделения памяти!";



bool Builder::PackStringsToDoubleZeroEndLine(TStrings *Lines,
									bool CryptLines, bool TrimLines,
									PCHAR &OutBuf, DWORD &OutBufSize)
{
	//  функция упаковывает строки в одну строку. Каждая строка будет
	//  отделена нулевым символом. В конце последней строки будет стоять
	//  два нулевых символа. Пустые строки будут проигнорированы.

	if (Lines == NULL || Lines->Count == 0)
		return false;

	OutBuf = NULL;
	OutBufSize = 0;


	UnicodeString S;


	// Определяем длину буфера и удаляем пустые строки
	for (int i = Lines->Count - 1; i >= 0; i--)
    {
		S = Lines->Strings[i];
		DWORD OldLength = S.Length();

		if (TrimLines)
			S = S.Trim();

		if (S.Length() == 0)
			Lines->Delete(i);
		else
		{
			if (TrimLines && S.Length() != OldLength)
				Lines->Strings[i] = S;
			OutBufSize += S.Length() + 1;
        }
	}
	if (OutBufSize == 0) return false;

	OutBufSize++; // Резервируем место для дополнительного нуля

	// Собираем данные
	OutBuf = (PCHAR)MemAlloc(OutBufSize);
	if (OutBuf == NULL)
	{
		throw Exception(ERROR_MEMALLOC);
    }

    PCHAR Tmp = OutBuf;

	// Копируем строки
	for (int i = 0; i < Lines->Count; i++)
    {
		S = Lines->Strings[i];
		// Копируем строку
		DWORD SL = S.Length();
		PCHAR Line = NULL;

		if (CryptLines)
		{
			Line = STR::Alloc(SL);
			Decrypt(S.t_str(), Line);
		}
		else
			Line = S.t_str();

		m_memcpy(Tmp, Line, SL);

		if (CryptLines)
			STR::Free(Line);

		// Смещаем указатели
		Tmp += SL;
		*Tmp = 0;
		Tmp++;
	}

	// Добавляем дополнительный ноль
	*Tmp = 0;

	return true;
}


