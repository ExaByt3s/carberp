//---------------------------------------------------------------------------
//  ������� ��� �������� ������ ����
//  ��� �������� �� C++ Builder
//---------------------------------------------------------------------------

#ifndef BuilderUtilsH
#define BuilderUtilsH
//---------------------------------------------------------------------------
#include <windows.h>
#include <Classes.hpp>


namespace Builder
{

	//-------------------------------------------------------------------
	//  PackStringsToDoubleZeroEndLine - ������� ����������� ������ �
	//		���� ������. ������ ������ ����� �������� ������� ��������.
	//		� ����� ��������� ������ ����� ������ ��� ������� �������
	//		������ ������ ����� ���������������.
	//
	//  Lines - �������� ����� �����
	//
	//  CryptLines - ��������� � ������������� ��������� ������ ������
	//				 ����� ���������.
	//
	//  TrimLines - ��������� ������� �������� � ���� � ������ ������.
	//
	//
	//  OutBuf - �������������� �����. ������ ���������� �������� MemAlloc
	//
	//  OutBufSize - ������ ��������������� ������
	//
	//  ������� ���������� ������ ���� ������� ��������� ����-�� ����
	//			������
	//
	//  ����������: � �������� ����� ����� Lines ����� ���� �������
	//              ���������. � ����� ���� ���������� ��������� ������
	//				� �������� ��������� ����������� �� �����
	//-------------------------------------------------------------------
	bool PackStringsToDoubleZeroEndLine(TStrings *Lines,
										bool CryptLines, bool TrimLines,
										PCHAR &OutBuf, DWORD &OutBufSize);
}



class TBotBuilder;
class TBotModule;
class TBotParam;



enum TBotParamStatus {psOk, psWarning, psError};


//*************************************************************
//  TBotBuilder - �����, ������� ����
//*************************************************************
class TBotBuilder : public TComponent
{
private:
	UnicodeString FSourceFileName;
	UnicodeString FResultFileName;
	bool FResultFileNameChanged;
	TCollection* FParams;
	TList* FModules;
	TList* FActiveModules;
	TMemoryStream* FFile;   // ����������� ����
	TBotParam* FPrefix;     // ������� ����
	TBotParam* FPassword;   // ������ ����
	TBotParam* FDelay;      // ������ �������
	TBotParam* FHosts;      // �������� �����
	friend class TBotModule;
	friend class TBotParam;
    void __fastcall UpdateResultFileName(bool Reset);
protected:
    void __fastcall virtual ParamValueChanged(TBotParam* Sender);
public:
	__fastcall TBotBuilder(TComponent* AOwner);
	__fastcall ~TBotBuilder();


	void __fastcall LoadSourceFile(const UnicodeString &FileName);
	TBotParamStatus __fastcall CheckParams(TStrings* Errors);
	bool __fastcall Build();

	__property UnicodeString SourceFileName = {read=FSourceFileName};
	__property UnicodeString ResultFileName = {read=FResultFileName};

	// ������� ��������� ����
	__property TBotParam* Prefix   = {read = FPrefix};
	__property TBotParam* Password = {read = FPassword};
	__property TBotParam* Delay    = {read = FDelay};
	__property TBotParam* Hosts    = {read = FHosts};
};


//*************************************************************
//  TBotModule - ����� �������� ���������� ������
//*************************************************************
class TBotParam : public TCollectionItem
{
private:
	TBotBuilder* FOwner;
	AnsiString FName;
	bool  FNotNull;   // ������������ ��������
	DWORD FSize;      // ������ ������ ���������
	PCHAR FData;      // ������
	DWORD FDataSize;  // ������ ������
	UnicodeString FTitle;
	void __fastcall Changed();
	UnicodeString __fastcall GetAsUnicodeString();
	void __fastcall SetAsUnicodeString(const UnicodeString &Value);
	DWORD __fastcall GetAsInt();
	void __fastcall SetAsInt(DWORD Value);
protected:
	UnicodeString __fastcall GetDisplayName(void);
    void __fastcall virtual DoChanged();
public:
	__fastcall TBotParam(TBotBuilder* AOwner, bool NotNull, const char* Name, DWORD Size, const char* Title);
	__fastcall ~TBotParam();

	bool __fastcall IsEmpty();
	void __fastcall Clear();
	void __fastcall SetValue(PCHAR Value, DWORD ValueSize);
	TBotParamStatus __fastcall Status();

	__property UnicodeString Title = {read = FTitle, write = FTitle};
	__property DWORD AsInt = {read = GetAsInt, write = SetAsInt}
	__property UnicodeString AsUnicodeString = {read = GetAsUnicodeString, write = SetAsUnicodeString};

};


//*************************************************************
//  TBotModule - ����� �������� ���������� ������
//*************************************************************
class TBotModule : public TComponent
{
private:
	TBotBuilder* FBuilder;
public:
	TBotModule(TBotBuilder* AOwner);
	__property TBotBuilder* Builder = {read = FBuilder};
};




//---------------------------------------------------------------------------
#endif
