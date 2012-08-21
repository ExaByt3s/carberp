//---------------------------------------------------------------------------
//
//   ������ ������� ������� ��������� ������������� �������� ��������
//   � �������� ������� �� ������ ����������
//
//---------------------------------------------------------------------------

#ifndef HistoryAnalizerH
#define HistoryAnalizerH

//---------------------------------------------------------------------------
#include <windows.h>
#include "Strings.h"


#define BOTPARAM_HISANALIZERLINKS "_HIS_ANALIZER_LINKS_\0"

#define BOTPARAM_SIZE_HISANALIZERLINKS 10000

#define BOTPARAM_HASH_HISANALIZERLINKS 0x8CBDB122 /* _HIS_ANALIZER_LINKS_ */


#ifdef DEBUGCONFIG
#	define BOTPARAM_ENCRYPTED_HISANALIZERLINKS false
#else
#	define BOTPARAM_ENCRYPTED_HISANALIZERLINKS true
#endif



typedef struct THistoryConfig
{
	PCHAR Command1;   // �������� �������. ���������� ���� � �������
					  // ����� ������� ��������� ����-�� ������ ����� �� ������.
					  // � ��������� ������ ���������� �������������� �������
	PCHAR Command2;   // �������������� �������
	PStrings Sites;   // ������ ������� ������
	PStrings Files;   // ������ ������� �� ������ ������������ ������
	bool ContainsURL; // ������� ����, ��� ���� �� ������ ������ � �������
} *PHistoryConfig;


namespace HisAnalizer
{
/*
	//  ��������� ���������������� ���� �����������
	bool Download(PCHAR URL, PHistoryConfig &Config);

	//  ���������� ������ ����������������� �����
	bool Parse(PCHAR Lines, PHistoryConfig Config);

	//  ���������� ������ ������� � ��������� ����������� �������
	void Execute(PHistoryConfig Config);

	//  ��������� � ��������� ������� �� ����������������� �����
    void DownloadAndExecute(PCHAR URL);

	//  �������� ������
	void ClearConfig(PHistoryConfig Config);

	// ���������� ������
	void FreeConfig(PHistoryConfig Config);

    // ��������� �������� � ���������� ����������� ������� � ��������� ������
	void StartAnalizerThread(PCHAR URL);
*/

	//------------------------------------------------------
	// Execute - ������� ��������� ������ ������� ���������
	//------------------------------------------------------
	void Execute();

}


//---------------------------------------------------------------------------
#endif
