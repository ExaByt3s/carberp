#ifndef BotClassesH
#define BotClassesH


//---------------------------------------------------------------------------

#include <windows.h>
#include "Crypt.h"
#include "Strings.h"



/*----------------  ������ ��� ������ �� �������� ��������� -----------------*/

typedef LPVOID PList;

typedef void (*TFreeItemMethod)(LPVOID);

namespace List
{
	// ������� �������� ������
	PList Create();

	// ������� ����������� ������
	void Free(PList List);

	// ������� ������� ������
	void Clear(PList List);

	// ������� ���������� ���������� ��������� ������
	DWORD Count(PList List);

	// ������� ��������� ����� �������
	int Add(PList List, LPVOID Item);

	// ������� ������� ������� �� ������
	void Delete(PList List, DWORD Index);

	// ������� �� ������ �������
	int Remove(PList List, LPVOID Item);

	// ������� �������
	 LPVOID Extract(PList List, DWORD Index);

	// ������� ���������� ������� �������� � ������
	int IndexOf(PList List, LPVOID Item);

	// ������� ������������� �������� �������� � ��������� ������� Index
	void SetItem(PList List, DWORD Index, LPVOID Item);

	// ������� ���������� �������� �������� �� ������� Index
	LPVOID GetItem(PList List, DWORD Index);

	// �������� ������ ������������������ ���������
	bool SetCapacity(PList List, DWORD Value);

	// ���������� ����� ��������������� �������� ���������
	void SetFreeItemMehod(PList List, TFreeItemMethod Method);

	// ������� ���������� ��������� ������� ������
	LPVOID Last(PList List);

	// ������� �������� �������� �� ������ Source � ������ Destination
    void CopyFrom(PList Destination, PList Source);
}


//---------------------------------------------------------------------------
//  MEMBLOCKS - ������� ��� ������ � ������� ������, �������� ������
//---------------------------------------------------------------------------
typedef struct TMemBlock
{
	DWORD ID;    // ������������� �����, ��������������� ��� ������ � ����
	DWORD Size;
	LPVOID Data;
}*PMemBlock;

typedef LPVOID PMemBlockList;

namespace MEMBLOCK
{
	// ������� ������ ������
	PMemBlockList CreateList();

	// ���������� ������ ������
	void FreeList(PMemBlockList List);

	// �������� ����� ���� � ������
    PMemBlock AddBlock(PMemBlockList List, LPVOID Data, DWORD DataSize, bool Copy = true);

	// ���������� ���� ������. �����!!!! ������ ������������ ������ �����, �� ��
	// ������� ���� �� ������ (���� �� � ��� ���������);
	void FreeBlock(PMemBlock Block);

	// ������� ������������ ����� ������ ������ ���� ������ ������
	DWORD Size(PMemBlockList List);

	// ������� �������� ������ ���� ������ � ����� Buf.
	// ���������������, ��� Buf ����� �������� � ���� ��� �����
    void BuildToBuf(PMemBlockList List, LPVOID Buf);

	// ������� �������� ��� ����� � ������
	PCHAR BuildAsStr(PMemBlockList List, DWORD *TotalSize);

	// ������� �������� ��� ����� � ������
	LPVOID BuildAsMem(PMemBlockList List, DWORD *TotalSize);

	// ������� ���������� ����� � ����
	PMemBlock Pack(PMemBlockList List);

	// ������� ������ ������ ���� � ������ ���� ���� � ��������� ���������������
	bool ContainBlock(PMemBlockList List, DWORD ID);

	// �������� ���� �� ��������������
	PMemBlock GetBlockByID(PMemBlockList List, DWORD ID);
	PCHAR GetBlockByAsStr(PMemBlockList List, DWORD ID);
}


//------------------------------------------------------
//  ������� ��������� �����
//------------------------------------------------------
#pragma pack(push, 1)
struct TFileHeader
{
	DWORD Signature;  // ��������� �����
	DWORD Version;    // ������ �����
};
#pragma pack(pop)


//---------------------------------------------------------------------------
//  DataFile - ������ ��� ������ � ������� �������
//---------------------------------------------------------------------------

#pragma pack(push, 1)
typedef struct TDataFileHead
{
	DWORD Signature;  // ��������� �����
	DWORD Version;    // ������ �����
	DWORD FileID;     // ������������� �����
	DWORD Flags;      // ����� �����
	DWORD FlagsEx;    // �������������� �����
} *PDataFileHead;     // ��� �����
#pragma pack(pop)


enum TFileCryptMode {fcNone, fcBlock, fcFile};

typedef struct TDataFileRec
{
	HANDLE Handle;              // ������������� �����
	TDataFileHead Head;         // ��������� �����
	TFileCryptMode CryptMode;   // ����� ���������� ������ �����
	TCryptMethod CryptMethod;   // ����� �����������
	TCryptMethod DecryptMethod; // ����� ����������� ������
	LPVOID CryptKey;            // ���� ����������
	PMemBlockList Blocks;       // ������ ������
	PCHAR FileName;
} *PDataFile;


#define DATA_FILE_VERSION 12

// ������� ���������� DWORD �� ������ ���������
#define SIGNATURE(Pointer) (*(DWORD*)Pointer)

namespace DataFile
{
	// ������� ������ ���� � ������ FileName, ������ ��������� ��������
	// ����� �� ��� ���� ������ � ���� �� ����������
    PDataFile CreateDataFile(PCHAR FileName, DWORD Signature);

	// ������� ��������� ���� ��� ������ � ������ �� ���� ���������
	PDataFile OpenFile(PCHAR FileName, DWORD Signature);

	// ���������� ������ �� ��������� File � ���������� ����
	bool WriteDataFile(PDataFile File);

	// ������ ������ �� ����������� ����� � ��������� File
	bool ReadDataFile(PDataFile File);

	// ������� ��������� ���� � ���������� ��������� File
    void CloseFile(PDataFile File);

	// �������� ���� ������
	void AddDataBlock(PDataFile File, DWORD BlockID, LPVOID Data, DWORD DataSize, bool Copy = true);
	void AddDataBlock(PDataFile File, DWORD BlockID, PCHAR StrData);

	// �������� ����� ������ � ����
	bool WriteBlocksToFile(PDataFile File);

	// �������� ����� ������ � ����
	bool ReadBlocksFromFile(PDataFile File);

	// ����������� ����� ������
    bool CryptBlocks(PMemBlockList Blocks, LPVOID Key, TCryptMethod Method);
}


//--------------------------------------------------
//  TEventContainer - �����, ��������� �������
//--------------------------------------------------

typedef void (*TBotEvent)(LPVOID Sender, int EventId, DWORD WParam, DWORD LParam);

class TEventContainer : public TBotObject
{
private:
	PList FEvents;
public:
    TEventContainer() { FEvents = NULL; }
	~TEventContainer();

	int AttachEvent(int EventId, TBotEvent Event);
	void DetachEvent(int Index);
	void CallEvent(int EventId, DWORD WParam, DWORD LParam);
	void CallEvent(int EventId);
};





//**********************************************************
//  TLock - ������ ����� � ���������� ������. ��������
//          ���� � ���, ��� ��� ������������� �� ������ �
//			����������� ������ ������� ����������� ������.
//          ��� ������ �� ���� ��������� ���������� ������
//			����� ������������� ��������� � � ����������� ��
//			������ �� ����������� ������
//
//  ������������ ������ ����������� ���������� �������
//**********************************************************
class TLock
{
private:
	PRTL_CRITICAL_SECTION FSection;
public:

	TLock(PRTL_CRITICAL_SECTION Section);
	~TLock();

	TLock operator=(const TLock &Locker);
	TLock operator=(PRTL_CRITICAL_SECTION Section);
};




//----------------------------------------------------
//  ������ �������� ������� ������� � ������
//  SEEK_ORIGIN...
//----------------------------------------------------
#define SO_BEGIN     FILE_BEGIN
#define SO_CURRENT   FILE_CURRENT
#define SO_END       FILE_END

//**********************************************************
//  TBotStream - ������� ����� ������\������
//**********************************************************
class TBotStream : public TBotObject
{
public:
	DWORD virtual Size();
	void  virtual SetSize(DWORD NewSize);

	DWORD virtual Seek(int Count, DWORD SeekMethod);
	DWORD Position();
	void  SetPosition(DWORD NewPosition);

	DWORD virtual Write(const void* Buf, DWORD Count);
	DWORD virtual Read(void* Buf, DWORD Count);

	DWORD WriteString(const char* Str);
	DWORD WriteString(const string &Str);
	DWORD WriteSizedString(const char* Str);
	DWORD WriteSizedString(const string &Str);

	LPVOID ReadToBuf(DWORD *Size);
	string ReadToString();
	int    ReadInt();
	BYTE   ReadByte();
	string ReadString(DWORD Size);
	string ReadSizedString();  // ������ ������ ������� [DWORD: ������][������]
};


//**********************************************************
// TBotMemoryStream - ����� ������/������ ������ ��
//                    ����������� ����� ������
//**********************************************************
class TBotMemoryStream : public TBotStream
{
private:
	LPBYTE FMemory;
	DWORD  FSize;
	DWORD  FPosition;
public:
	TBotMemoryStream(LPVOID Mem, DWORD MemSize);

	DWORD Read(void* Buf, DWORD Count);
	DWORD Write(const void* Buf, DWORD Count);

	DWORD Size();

	DWORD Seek(int Count, DWORD SeekMethod);
};





//**********************************************************
//  ������ �������� ������.
//**********************************************************
static const BYTE fcmRead      = 0;
static const BYTE fcmWrite     = 1 ;
static const BYTE fcmReadWrite = 2;
static const WORD fcmCreate    = 0xFF00;


//**********************************************************
//  TBotFileStream - ����� ������\������ ������ �����
//**********************************************************
class TBotFileStream : public TBotStream
{
private:
	HANDLE FHandle;
public:
	TBotFileStream(const char* FileName, WORD Mode);
	~TBotFileStream();
	void Close();
	DWORD Size();
	DWORD Seek(int Count, DWORD MoveMode);
	DWORD Write(const void* Buf, DWORD Count);
	DWORD Read(void* Buf, DWORD Count);
	HANDLE inline Handle() { return FHandle; }
};


//**********************************************************
//  TBotCollection - �������� ���������
//**********************************************************
class TBotCollectionItem;

class TBotCollection : public TBotObject
{
private:
	PList FItems;
    PRTL_CRITICAL_SECTION FLock;
	void InsertItem(TBotCollectionItem* Item);
	void RemoveItem(TBotCollectionItem* Item);

	friend class TBotCollectionItem;
protected:
	void Lock();
	void Unlock();
public:
	TBotCollection();
	~TBotCollection();


	void virtual Clear();
	void  SetThreadSafe();
	int   Count();
	TLock GetLocker();
    TBotCollectionItem* Items(int Index);
};


//**********************************************************
//  TBotCollectionItem - ������� ���������
//**********************************************************
class TBotCollectionItem : public TBotObject
{
private:
	TBotCollection* FOwner;

	friend class TBotCollection;
protected:
	void Lock();
	void Unlock();
public:
	TBotCollectionItem(TBotCollection* aOwner);
	~TBotCollectionItem();

	TBotCollection* Owner();
	void            SetOwner(TBotCollection* aOwner);
};



//**********************************************************
//  TValues - ����� ����������� ��������
//**********************************************************

class TValue : public TBotCollectionItem
{
public:
	TValue(TBotCollection* aOwner) : TBotCollectionItem(aOwner) {};
	string Name;
	string Value;
};


class TValues : public TBotCollection
{
protected:
	  TValue* GetItemByName(const char* Name);
public:
	TValues();

	void AddValue(const string &Name, const string &Value);
	void SetValue(int Index, const string &Value);
	void SetValue(const char* Name, const string &Value);

	string GetValue(int Index);
	string GetValue(const char *Name);

    inline TValue* Items(int Index) { return (TValue*)((TBotCollection*)this)->Items(Index); }

};



//------------------------------------------------------
//  ���������� ������
//------------------------------------------------------

#pragma pack(push, 1)
struct TDataHeader
{
	DWORD Type;      // ��� ������
	DWORD Flags;     // ����� ������
	DWORD FlagsEx;   // �������������� �����
	BYTE  Encrypted; // ������ �����������
    BYTE  Signed;    // ������� ����, ��� ������ ��������� �������� ��������
};
#pragma pack(pop)


#define  DATA_FILE_SIGNATURE 0x3878C167 /* BOT_DATA_FILE */
#define  DATA_FILE_VERSION   0x00010000 /* 1.0 */


//************************************************************
//
//************************************************************
//class TDataBlock : public TBotObject
//{
//public:
//    TDataBlock();
//};


//************************************************************
//  TDataFile - ���� �������� ������ � ����� �  �������
//              �������
//  ��������� �����:
//  [��������� �����: TFileHead][��������� ������: TDataHead]
//  [��������� �����][������ �����]...{������ �����}
//************************************************************
class TDataFile : public TBotObject
{
private:
	TBotStream *FStream;
	bool FStreamAssigned;
	bool WriteHeaders();
protected:
	bool Write(const void* Buf, DWORD BufSize, bool Encrypt = true, bool Hash = true);
	bool Read(void* Buf, DWORD BufSize, bool Decrypt = true, bool Hash = true);
	DWORD GetVarID();
public:
	DWORD  Signature; // ��������� �����
	DWORD  Version;   // ������ �����
	DWORD  Type;      // ��� ������
	DWORD  Flags;     // ����� (���������������� ��� ���. ����)
	DWORD  FlagsEx;   // ����� (���������������� ��� ���. ����)
	string Name;      // ��� ������ ������

	TDataFile();
	~TDataFile();

	bool Create(const char* FileName);
	bool Create(TBotStream *Stream);

	bool Open(const char* FileName);
	bool Open(const TBotStream *Stream);

    bool AddBlock(DWORD Type, const char *Name, DWORD NameLen, LPVOID Data, DWORD DataSize);

    void Close();
};



//---------------------------------------------------------------------------
#endif
