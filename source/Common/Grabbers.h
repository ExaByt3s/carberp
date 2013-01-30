//---------------------------------------------------------------------------
//  ������ ������ ��� ������ ��������
//---------------------------------------------------------------------------

#ifndef GrabbersH
#define GrabbersH
//---------------------------------------------------------------------------

#include "GetApi.h"
#include "Strings.h"
#include "BotClasses.h"


// ��������� ����� �������
#define GRABBER_FILE_SIGNATURE 0x87AF1966

// ��������� ����� ������ ����� �������
#define GRABBER_FILE_BLOCK_SIGNATURE 0xC7BFB244

// ������ ����� �������
#define GRABBER_FILE_VERSION MAKELONG(1, 0)


// ���� ������ ��������
#define GRABBER_DATA_TEXT 1  /* ��������� ���� */


//-------------------------------------------
//  GetGrabbersPath - ������� ��������� ���
//                �������� �������� �������
//-------------------------------------------
string GetGrabbersPath();



class TGrabberFile;
class TGrabberBlock;



//------------------------------------------------------
//  TGrabberFile - ������� ����� ��� ������ � ������
//                 �������
//------------------------------------------------------
class TGrabberFile : public TBotObject
{
private:
	TBotCollection* FBlocks; // ��������� ������
	string FFileName;     // ��� �����
	string FGrabberName;  // ��� �������
	PCHAR  FPassword;     // ������ ����������
	DWORD  FType;         // ��� �����
	DWORD  FBlocksStart;  // �������� � ����� � ������ ������ ������
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
    bool Active();  // ������� ��������� ������ ���� � ������ ������ ��� ������
	bool Create();  // ������� ������ ���� �������
	bool Open();    // ������� ��������� ����� ��������� ����

	void CloseFile(); // ������� ��������� �������� ����

    string PackToCAB(); // ������� ������������ ������ � ��� �����

    bool Test();

	bool AddText(const string& Name, const string& Data); // ������� ��������� ��������� ������
};


//------------------------------------------------------
//  TGrabberBlock - ���� ������ �������
//------------------------------------------------------
class TGrabberBlock : public TBotCollectionItem
{
private:
	TGrabberFile* FFile;
	string FName;
	string FFileName;
	string FInternalFileName;
	DWORD  FDataSize;    // ������ ������ �����
	string FAsString;    // ��������� ������
	LPBYTE FAsBlob;      // ������ � �������� ����
	DWORD  FDataType;    // ��� ������ �����
	bool   FIgnoreBlock; // ������ ���� ���������� ���������������

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
