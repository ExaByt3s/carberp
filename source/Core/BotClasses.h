#ifndef BotClassesH
#define BotClassesH


//---------------------------------------------------------------------------

#include <windows.h>
#include "Crypt.h"



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
	// ������� ���� �� ������ (���� �� � �� ���������);
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

	// ������� ����� ������ ���� � ������ ���� ���� � ��������� ���������������
	bool ContainBlock(PMemBlockList List, DWORD ID);

	// �������� ���� �� ��������������
	PMemBlock GetBlockByID(PMemBlockList List, DWORD ID);
	PCHAR GetBlockByAsStr(PMemBlockList List, DWORD ID);
}


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

typedef struct TDataFile
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

typedef void (*TBotEvent)(TBotObject* Sender, int EventId, DWORD WParam, DWORD LParam);

class TEventContainer : public TBotObject
{
public:
    TEventContainer() : FEvents(0) {};
	~TEventContainer();

	int AttachEvent(int EventId, TBotEvent Event);
	void DetachEvent(int Index);
protected:
	void CallEvent(int EventId, DWORD WParam, DWORD LParam);
	void CallEvent(int EventId);
private:
	PList FEvents;
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



//**********************************************************
// TMemReader - ����� ������/������ ������ ��
//                    ����������� ����� ������
//**********************************************************
class TMemReader : public TBotObject
{
private:
	LPBYTE FMemory;
	DWORD  FSize;
	DWORD  FPosition;
public:
	TMemReader(LPVOID Mem, DWORD MemSize);

	DWORD   Read(LPVOID Buf, DWORD Size);
	int     ReadInt();
	BYTE    ReadByte();
	string  ReadString(DWORD Size);
	string  ReadSizedString();  // ������ ������ ������� [DWORD: ������][������]

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


	void  Clear();
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


//---------------------------------------------------------------------------
#endif
