#ifndef MemoryH
#define MemoryH

#include <windows.h>
//-----------------------------------------------------------------------------

void *m_memset(void *szBuffer, DWORD dwSym, DWORD dwLen);
void *m_memcpy(void *szBuf, const void *szStr, int nLen);
int   m_memcmp(const void *buf1, const void *buf2, size_t count);


/*---------------------------------------------------------------------------*/
// ��������� ������� ��� ��������� ������ ���������� VirtualAlloc
// ������� ����� ������ ������� � ���, ��� ������ ���������� �����������,
// ��� �������� � �������� �������� ������ ��� ������������� ���������
// ��������, ���� ��������� ������ ������.
//
// ��� ������������ ���������� ������� ������ ����������� ������� ����������
// ������ �� ���� ��������. ������� ���������� � MemHeap...
/*---------------------------------------------------------------------------*/

// ������ ������ ���������� ������
DWORD  GetMemSize(LPVOID lpAddr);

// ���������� ������
VOID   MemFree(LPVOID lpAddr);

// �������� ������ ���������� �������
LPVOID MemAlloc(DWORD dwSize);

// �������� � �������� ������ ���������� �������
LPVOID MemAllocAndClear(DWORD Size);

// ������������ ������
LPVOID MemRealloc(LPVOID lpAddr, DWORD dwSize );


/*---------------------------------------------------------------------------*/
//  ������� ��� ������ � ������� �� ���� ��������
//  ������ ������� �������� ��������� ��� WINAPI ��������� ��� ������
//  � ������� ����.
//  ������� ���������, ������������� ������ �������� � ������ ��������������
//  ������� ���������� ������
/*---------------------------------------------------------------------------*/

namespace HEAP
{
	LPVOID Alloc(DWORD Size);

	LPVOID ReAlloc(LPVOID Buf, DWORD Size);
	bool ReAlloc2(LPVOID &Buf, DWORD Size);

	void Free(LPVOID Buf);
	void Free2(LPVOID &Buf);

	DWORD Size(LPVOID Buf);
}

/*---------------------------------------------------------------------------*/
//  ����� �������� ����������� �����
/*---------------------------------------------------------------------------*/

// ��������� ���������� ������
#define ClearStruct(S) m_memset(&S, 0, sizeof(S))

// �������� ������ ��� ���������� ���� � ���������� ���������
#define CreateStruct(T) (T*)HEAP::Alloc(sizeof(T))

// �������� ������ ��� ���������� ���� � ���������� ���������
#define FreeStruct(S) HEAP::Free(S)

// ������� �������� ��������� Source � ��������� Destination
#define CopyStruct(Source, Destination) m_memcpy(Destination, Source, sizeof(*Destination))

//�������������� ��������� � ������������ ������ 
template<int SZ>
class MemPtr
{
		void* ptr;

	public:
		
		MemPtr()
		{
			ptr = MemAlloc(SZ);
		}
		~MemPtr()
		{
			MemFree(ptr);
		}
		operator char*()
		{
			return (char*)ptr;
		}

		char* str()
		{
			return (char*)ptr;
		}

		int size() const
		{
			return SZ;
		}
};


// ������������� �� ���������� ������� ���� ���
// ���������� �������� ������ ����� ������� �� �������� ������
// ���������� ��� ������������ ������
class TMemory
{
private:
	void* FBuf;
	DWORD FSize;
public:
	TMemory(DWORD Size)
	{
		FSize = Size;
        FBuf  = MemAlloc(Size);
	}

	~TMemory()
	{
        MemFree(FBuf);
	}

	inline LPVOID Buf()  { return FBuf; }
	inline DWORD  Size() { return FSize; }

    inline PCHAR AsStr() { return (PCHAR)FBuf; }

	inline operator char*()  { return (char*)FBuf; }
	inline operator LPBYTE() { return (LPBYTE)FBuf; }
};

//-----------------------------------------------------------------------------
#endif //MemoryH

