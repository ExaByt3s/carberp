#include <windows.h>
#include <wincrypt.h>

#include "Crypt.h"
#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "Utils.h"
#include "BotHTTP.h"
#include "Config.h"


//----------------------------------------------------------------------------

// ����� �������� ��� BASE64 ���������
static const char Base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


// �������������� ������ ���������� ��� UIDCrypt �������
char UIDCryptDefaultIV[] = {'l', 'f', 'g', 'r', 'f', 'J', 'D', '6',  0};

//-----------------------------------------------------------------------------

char szTempBuf[100];

PCHAR WINAPI Decrypt(PCHAR String, PCHAR OutString)
{
	if (STR::IsEmpty(String))
		return NULL;

	PCHAR Res;
	if (OutString != NULL)
		Res = OutString;
	else
	{
		m_memset( szTempBuf, 0, 100);
		Res = szTempBuf;
    }

	__asm
	{
		pushad
		mov		esi,[String]
		//mov		edi,offset szTempBuf
        mov		edi, Res
	__again:
			lodsb
			test	al,al
			jz		__to_exit
			sub		al, 10h
			xor		al, 5h
			add		al, 10h
			stosb
			jmp		__again

	__to_exit:
		stosb
		popad
	} 

	return Res;
}

//-----------------------------------------------------------------------------

PCHAR BASE64::Encode(LPBYTE Buf, DWORD BufSize)
{
	if (Buf == NULL || BufSize == 0)
		return NULL;



  PCHAR Result = STR::Alloc((BufSize + 2) / 3 * 4 + 1);
  if (Result == NULL)
	return NULL;


    PCHAR P = Result;
    BYTE Cur[3];

    while(BufSize > 0)
    {
      DWORD Len = 0;
      for(DWORD i = 0; i < 3; i++)
      {
        if(BufSize > 0)
        {
          BufSize--;
          Len++;
          Cur[i] = Buf[i];
        }
        else Cur[i] = 0;
      }

      Buf += 3;

	  P[0] = Base64Chars[Cur[0] >> 2];
	  P[1] = Base64Chars[((Cur[0] & 0x03) << 4) | ((Cur[1] & 0xF0) >> 4)];
	  P[2] = (BYTE)(Len > 1 ? Base64Chars[((Cur[1] & 0x0F) << 2) | ((Cur[2] & 0xC0) >> 6) ] : '=');
	  P[3] = (BYTE)(Len > 2 ? Base64Chars[Cur[2] & 0x3F] : '=');

      P += 4;
    }

	*P = 0;
	STR::UpdateLength(Result, P - Result);
    return Result;
}


PCHAR BASE64::Decode(PCHAR Buf, DWORD *ResultSize)
{
	// ��c��������� ������ �� ������� BASE64
	// ������ ��������� ������ ����� ������ �������� STR::Length,
	// ���� �������� ��������� �� ����������, ���� ��� ���������
	if (ResultSize != NULL)
		*ResultSize = 0;
	if (STR::IsEmpty(Buf))
	return NULL;

	char C;
	short int x;
	WORD c4;
	short int StoredC4[4];

	PCHAR Result = STR::Alloc(StrCalcLength(Buf));
	DWORD Len = 0;
	PCHAR R = Result;
	c4 = 0;

	while (*Buf != 0)
	{
		while (*Buf != 0 && c4 < 4)
		{
			C = *Buf;
			if (C == '=') x = -1;
			else
			if (C == '+') x = 62;
			else
			if (C == '/') x = 63;
			else
			if (C >= '0' && C <= '9') x = C - ('0' - 52);
			else
			if (C >= 'A' && C <= 'Z') x = C - 'A';
			else
			if (C >= 'a' && C <= 'z') x = C - ('a' - 26);
			else
				return Result;

			StoredC4[c4] = x;
			c4++;
			Buf++;
		}

		if (c4 == 4)
		{
			c4 = 0;
			*R = (StoredC4[0] << 2) | (StoredC4[1] >> 4);
			R++;
			Len++;

			if (StoredC4[2] == -1) break;
			*R = (StoredC4[1] << 4) | (StoredC4[2] >> 2);
			R++;
			Len++;

			if (StoredC4[3] == -1) break;
			*R = (StoredC4[2] << 6) | StoredC4[3];
			R++;
			Len++;
		}
	}
	STR::UpdateLength(Result, Len);
	if (ResultSize != NULL)
		*ResultSize = Len;
	return Result;
}


//----------------------------------------------------------------------------
DWORD XORCrypt::Crypt(PCHAR Password, LPBYTE Buffer, DWORD Size)
{
	DWORD a, b;
    a = 0;

	while (a < Size)
    {
		b = 0;

		while (Password[b])
		{
			Buffer[a] ^= (Password[b] + (a * b));
			b++;
		}
		a++;
    }

	return a;
}

// ��� ������������ TCryptMethod
bool XORCrypt::Crypt(LPVOID Password, LPBYTE Buffer, DWORD Size, PDWORD OutSize)
{
	if (Buffer == NULL || Size == 0)
		return false;
   *OutSize = Crypt((PCHAR)Password, Buffer, Size);
   return *OutSize != 0;
}



LPBYTE XORCrypt::DecodeBuffer(PCHAR  Signature, LPVOID Buffer, DWORD &Size)
{
	// ������� �������������� ����� ���������� ������

	if (Buffer == NULL || Size == 0)
		return NULL;

	PCHAR Buf = (PCHAR)Buffer;
	PCHAR Start = Buf;

	// ���������� ���������
	if (!STR::IsEmpty(Signature))
	{
		DWORD SignLen = StrCalcLength(Signature);
		if (SignLen >= Size || !StrSame(Buf, Signature, true, SignLen))
			return NULL;
		Buf += SignLen;
    }

	// Jghtltkztv gfhjkm
	DWORD PassSize = *(PDWORD)Buf;
	Buf += sizeof(DWORD);

	PCHAR Password = STR::New(Buf, PassSize);
	if (Password == NULL)
		return NULL;
	Buf += PassSize;


	// ���������� ����� ������ ������
	int NewSize = Buf - Start;
	if ((DWORD)NewSize > Size)
		return NULL;
	Size -= NewSize;
	// �������������� ������
	NewSize = XORCrypt::Crypt(Password, (LPBYTE)Buf, Size);
	STR::Free(Password);
	if (NewSize <= 0)
		return NULL;

	Size = NewSize;
    return (LPBYTE)Buf;
}



//****************************************************************************
//  ������ ��� ���������� �������� WIN Crypt
//  ����� ������ �� ���������� ��������� (� - ����)
//
//  (4� ������� �������� IV)(BASE64 ������)(4� ������ �������� IV)(��������� BASE64 ==)
//****************************************************************************

PCHAR RC2Crypt::ExtractIV(PCHAR Buf, DWORD StrLen)
{
	// ������� ��������� ������ ������������� ����� �� ������
	if (STR::IsEmpty(Buf))
		return NULL;

	if (StrLen == 0)
		StrLen = StrCalcLength(Buf);
	if (StrLen < 8) return NULL;

	PCHAR V = STR::Alloc(8);

	// �������� ������ ����� �������
	STR::Copy(Buf, V, 0, 4);
	STR::Delete(Buf, 0, 4);
	StrLen -= 4;

	// �������� ������ �����
	PCHAR Ptr = Buf + StrLen - 1;
	while (Ptr > Buf && *Ptr == '=') Ptr--;
    Ptr -= 3;
    STR::Copy(Ptr, V + 4, 0, 4);
	STR::Delete(Ptr, 0, 4);

	return V;
}

// ������������ ��������� ������
PCHAR RC2Crypt::GenerateIV()
{
	PCHAR V = STR::Alloc(8);

	PCHAR T = V;
	DWORD Max = sizeof(Base64Chars) - 2;
	for (DWORD i = 0; i < 8; i++)
    {
		DWORD Index = Random::Generate(0, Max);
		*T = Base64Chars[Index];
		T++;
	}
	return V;
}

PCHAR RC2Crypt_PackEncodedBuffer(PCHAR Buf, DWORD BufSize, PCHAR IV)
{
	// ������������ � ����� ������������� ������ � ������ �����
	PCHAR Result = STR::Alloc(BufSize + 8);
	if (Result == NULL)
		return NULL;

    PCHAR P = Result;
	// ��������� ������ 4 ����� �����
	STR::Copy(IV, P, 0, 4);
    P += 4;
	// ���������� ������� �� ������� ����� ���������� ���� Base64
	PCHAR End = Buf + BufSize;
	while (End > Buf && *(End - 1) == '=') End--;

	STR::Copy(Buf, P, 0, End - Buf);
	P += End - Buf;

	// �������� ������ ����� �������
	STR::Copy(IV, P, 4, 4);
	P += 4;

	// �������� ���������� ����� ������
	STR::Copy(End, P, 0, BufSize - (End - Buf));

    return Result;
}

PCHAR RC2Crypt::Encode(LPBYTE Buf, DWORD BufSize,  PCHAR Password)
{
	// ���������� �����
	if (Buf == NULL || BufSize == 0 || STR::IsEmpty(Password))
		return NULL;

	PCHAR IV = GenerateIV();
	if (IV == NULL)
		return NULL;


	DWORD EncBufSize = BufSize;
	LPBYTE NewBuf = WinEncode(Buf, EncBufSize, Password, IV);


    PCHAR Result = NULL;

	if (NewBuf != NULL)
	{
		// �������� � BASE64
		PCHAR B64 = BASE64::Encode(NewBuf, EncBufSize);
		// ������������ ����� � ��������
        Result = RC2Crypt_PackEncodedBuffer(B64, STR::Length(B64), IV);

        STR::Free(B64);
        MemFree(NewBuf);
	}

	STR::Free(IV);
    return Result;

}


bool RC2Crypt::Decode(PCHAR Password, PCHAR Buf, DWORD &BufSize)
{
	// ������������ �����
	// �� ����� ������ BASE64 �������� � ������ � ��� �������� IV
	// BufSize - ������ ������������� ������.
	// ���� �������� ������� ��������, �� ������ ������
	// ����� ��������� �� �������� �������.
	// ����� ��������� ������ BufSize ����� ��������� �� ������
	// ��������������� ������

	if (STR::IsEmpty(Buf))
		return false;

	if (BufSize == 0)
		BufSize = StrCalcLength(Buf);

    DWORD OriginalSize = BufSize;

	PCHAR V = ExtractIV(Buf, BufSize);
	if (V == NULL) 	return false;
	BufSize -= 8; // ��������� �� ������ �������

	// BASE64 �����������
	PCHAR B64 = BASE64::Decode(Buf);

	BufSize = STR::Length(B64);
	STR::Copy(B64, Buf, 0, BufSize);
	if (BufSize < OriginalSize)
		*(Buf + BufSize) = 0;
	STR::Free(B64);

	// �������������� ������
	WinDecode(Buf, BufSize, Password, V);
	STR::Free(V);

    return true;
}


bool RC2Crypt::DecodeStr(PCHAR Password, PCHAR Str, DWORD Size)
{
	// ������� ���������� ������ � ��������� � ������
	if (STR::IsEmpty(Str) || STR::IsEmpty(Password))
		return false;
	if (Size == 0)
		Size = STR::Length(Str);

	if (Decode(Password, Str, Size))
	{
		STR::UpdateLength(Str, Size);
		return true;
	}

	return false;
}


LPBYTE RC2Crypt::WinEncode(LPBYTE Buf, DWORD &BufSize, PCHAR Password, PCHAR IV)
{
	// ��������� ����� ��������� Win Crypto API
	// ������� �������� ����������� ����� ������ ������� MemAlloc()
	if (Buf == NULL || BufSize == 0 || STR::IsEmpty(Password) || STR::IsEmpty(IV))
		return NULL;

	HCRYPTPROV Provider;
	HCRYPTKEY Key;
	LPBYTE NewBuf = NULL;
	if (GenerateKey(Password, IV, Provider, Key))
	{
		// ���������� ������ ������ ������
		DWORD NewBufSize = BufSize;
		if (pCryptEncrypt(Key, 0, TRUE, 0, NULL, &NewBufSize, 0))
		{
			NewBuf = (LPBYTE)MemAlloc(NewBufSize);
			if (NewBuf != NULL)
			{
				m_memcpy(NewBuf, Buf, BufSize);
				pCryptEncrypt(Key, 0, TRUE, 0, NewBuf, &BufSize, NewBufSize);
            }
        }

		pCryptDestroyKey(Key);
		pCryptReleaseContext(Provider, 0);
	}

	return NewBuf;
}


PCHAR RC2Crypt::WinDecode(PCHAR Buf, DWORD &BufSize, PCHAR Password, PCHAR IV)
{
	// ������������ ����� ��������� Win Crypto API
	HCRYPTPROV Provider;
	HCRYPTKEY Key;
	if (GenerateKey(Password, IV, Provider, Key))
	{
		if (pCryptDecrypt(Key, 0, TRUE, 0, (BYTE*)Buf, &BufSize))
			*(Buf + BufSize) = 0;

		pCryptDestroyKey(Key);
		pCryptReleaseContext(Provider, 0);
	}
	return Buf;
}


typedef struct TKeyBlob
{
	BLOBHEADER KeyHeader;
    DWORD KeySize;
	BYTE KeyData[64];
} *PKeyBlob;


bool RC2Crypt::GenerateKey(PCHAR Password, PCHAR IV, HCRYPTPROV &Provider, HCRYPTKEY &Key)
{
	// ������������ ���� ����������� �� ������ ������ � ������� ������������� IV

	Provider = NULL;
	Key = NULL;

	if (STR::IsEmpty(Password) || STR::IsEmpty(IV))
		return false;

	TKeyBlob KeyBlob;

	DWORD CryptMode;
	DWORD PadMode;

	// ����������� � ����������
	if (!pCryptAcquireContextA(&Provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return false;

    // ����������� ����
	ClearStruct(KeyBlob);
	KeyBlob.KeyHeader.bType    = PLAINTEXTKEYBLOB;
	KeyBlob.KeyHeader.bVersion = CUR_BLOB_VERSION;
	KeyBlob.KeyHeader.aiKeyAlg = CALG_RC2;

	KeyBlob.KeySize = StrCalcLength(Password);
    STR::Copy(Password, (PCHAR)KeyBlob.KeyData, 0, KeyBlob.KeySize);


	if (!pCryptImportKey(Provider, (LPBYTE)&KeyBlob, sizeof(KeyBlob), 0, 0, &Key))
	{
		pCryptReleaseContext(Provider, 0);
		Provider = NULL;
        return false;
    }

	// ����������� ����
	bool ValidKey = true;
	if (!pCryptSetKeyParam(Key, KP_IV, (LPBYTE)IV, 0))
		ValidKey = false;

    CryptMode = CRYPT_MODE_CBC;

	if (!pCryptSetKeyParam(Key, KP_MODE, (LPBYTE)&CryptMode, 0))
        ValidKey = false;

	PadMode = PKCS5_PADDING;
	if (!pCryptSetKeyParam(Key, KP_PADDING, (LPBYTE)&PadMode, 0))
	   ValidKey = false;

	if (!ValidKey)
	{
		pCryptDestroyKey(Key);
		pCryptReleaseContext(Provider, 0);
		Key = NULL;
		Provider = NULL;
	}
    return ValidKey;
}


//****************************************************************************
//  ������ ��� ���������� ������ ������
//****************************************************************************

DWORD CryptFile::WriteFromBuffer(PCHAR FileName, LPVOID Buffer, DWORD BufferSize, PCHAR Password)
{
	//  ������� ������������� ����������
	//  ������ � ���������� ��� � �����
	if (STR::IsEmpty(FileName) || Buffer == NULL || BufferSize == 0)
    	return 0;

	bool FreePass = STR::IsEmpty(Password);
	if (FreePass)
    	Password = GetMainPassword(true);

	PCHAR Buf = RC2Crypt::Encode((LPBYTE)Buffer, BufferSize, Password);

	DWORD Result = 0;
	if (Buf != NULL)
	{
		Result = File::WriteBufferA(FileName, Buf, STR::Length(Buf));
        STR::Free(Buf);
    }

	if (FreePass)
		STR::Free(Password);

    return Result;
}
//-----------------------------------------------------------------------------

LPVOID CryptFile::ReadToBuffer(PCHAR FileName, LPDWORD BufferSize, PCHAR Password)
{
	//  ������� ������ ���������� �����,
	//	�������������� ��� � ���������� ���������
	if (BufferSize != NULL)
		*BufferSize = 0;

	if (STR::IsEmpty(FileName))
    	return NULL;


	bool FreePass = STR::IsEmpty(Password);
	if (FreePass)
    	Password = GetMainPassword(true);


	// ��������� ����
	DWORD Size = 0;
	LPBYTE Buf = File::ReadToBufferA(FileName, Size);

	LPVOID Result = NULL;

	// �������������� ����
	if (Buf != NULL)
	{
		if (RC2Crypt::Decode(Password, (PCHAR)Buf, Size))
		{
			Result = MemAlloc(Size);
			m_memcpy(Result, Buf, Size);
			if (BufferSize != NULL)
				*BufferSize = Size;
        }
        MemFree(Buf);
    }



	if (FreePass)
		STR::Free(Password);

	return Result;
}

//****************************************************************************
//  ������ ��� ���������� ������ ��������������� ���� (UID) ������ ������
//  ���������� ������������ ��������  RC2Crypt
//****************************************************************************

PCHAR UIDCrypt::GeneratePassword()
{
	//������� ���������� ������ �� ������ UID
	PCHAR UID = MakeMachineID();
	if (UID == NULL)
		return NULL;

	DWORD UIDSize = StrCalcLength(UID);

	PCHAR Password = STR::Alloc(RC2_DEFAULT_PASSWORD_SIZE);

	if (Password != NULL)
	{
		// ��������� ������
		DWORD Size = Min(UIDSize, RC2_DEFAULT_PASSWORD_SIZE);

		if (Size < RC2_DEFAULT_PASSWORD_SIZE)
		{
			// � ������ ���� UID �������� ������ ������������
			// ������� �� ��������� ������ �� �����

			m_memset(Password, '-', RC2_DEFAULT_PASSWORD_SIZE);
		}

		// �������� UID
		DWORD CopySize = Min(RC2_DEFAULT_PASSWORD_SIZE, UIDSize);
		STR::Copy(UID, Password, 0, CopySize);

		// ��� ������� ������������ ���������� ������
		// ��������� ������� ������� ����
		PCHAR MPass = GetMainPassword(true);

        XORCrypt::Crypt(MPass, (LPBYTE)Password, RC2_DEFAULT_PASSWORD_SIZE);

		STR::Free(MPass);
    }


	STR::Free(UID);

	return Password;
}
//-----------------------------------------------------------------------------

PCHAR UIDCrypt::Crypt(LPVOID Data, DWORD DataSize, PCHAR Vector)
{
	// ������� ������� ������ ������ �� ������ UID
	if (Data == NULL || DataSize == 0)
		return NULL;

    // �������� ������
	PCHAR Password = GeneratePassword();
	if (Password == NULL)
		return NULL;


	PCHAR Result = NULL;


	// ������� ������

	if (Vector == NULL)
    	Vector = UIDCryptDefaultIV;

	LPBYTE Buf = RC2Crypt::WinEncode((LPBYTE)Data, DataSize, Password, Vector);

	// ��������������� � BASE64
	if (Buf != NULL)
	{
		Result = BASE64::Encode(Buf, DataSize);

		MemFree(Buf);
    }


	STR::Free(Password);

	return Result;
}
//-----------------------------------------------------------------------------

PCHAR UIDCrypt::CryptFileName(PCHAR FileName, bool CryptExt)
{
	//  ������� �������� ��� ����� ����� ��� ���������� ��� �������
	//  ����������. ���� � ����� �� ���������

	if (STR::IsEmpty(FileName))
    	return NULL;

	// �������� ��� �����
	PCHAR File = File::ExtractFileNameA(FileName, true);
	if (File == NULL)
    	return STR::New(FileName);

	// ��������� ������� ����
	PCHAR Path = File::ExtractFilePathA(FileName);

	// �������� ���������� �����
	PCHAR Ext = STR::ScanEnd(File, '.');
	if (Ext != NULL)
	{
		*Ext = 0;
		Ext++;
    }


	// ������� ������
	PCHAR NewFile = Crypt((LPVOID)File, StrCalcLength(File), NULL);


	// ����������� �����
	ConvertFileNameChars(NewFile);

	// ��������� ���������

    PCHAR ExtDelimeter = (Ext)? (PCHAR)"." : NULL;

	PCHAR Result = STR::New(4, Path, NewFile, ExtDelimeter, Ext);


	// ����������� ������
	STR::Free(Path);
	STR::Free(File);
	STR::Free(NewFile);

    return Result;
}
//-----------------------------------------------------------------------------

void UIDCrypt::ConvertFileNameChars(PCHAR Name)
{
	// ������� ����������� ������������ � ������������������ �������
	// ����� ����� ����������� ���� ����������
	if (STR::IsEmpty(Name))
		return;

	PCHAR Tmp = STR::End(Name);
	Tmp--;
	while (Tmp > Name && *Tmp == '=')
	{
		*Tmp =0;
		Tmp--;
	}

	for (Tmp = Name; *Tmp != 0; Tmp++)
	{
		if (*Tmp == '/')
			*Tmp = 'z';  // ������������ ������
		else
		if (*Tmp == '+')
			*Tmp = 'v';  // ����� ����������� � ���������, �� ������� ����� :))
	}
}



//*****************************************************************************
//  							TWinCrypt
//*****************************************************************************


TWinCrypt::TWinCrypt(const char *Container, DWORD Flags)
{
	// � ������������ ������������� �������������� ����������
    InitializeProvider(0, Container, Flags);
}

TWinCrypt::TWinCrypt(HCRYPTPROV Provider)
{
	InitializeProvider(Provider, 0, 0);
}

TWinCrypt::~TWinCrypt()
{
	// ����������� ������

	if (FKey)
    	pCryptDestroyKey(FKey);

	if (!FProviderAssigned && FProvider)
		pCryptReleaseContext(FProvider,0);
}


void TWinCrypt::DestroyKey()
{
	if (FKey)
	{
		CryptDestroyKey(FKey);
        FKey = 0;
    }
}


void TWinCrypt::InitializeProvider(HCRYPTPROV Provider, const char* Container, DWORD Flags)
{
	FProvider         = Provider;
	FProviderAssigned = FProvider != 0;
	FKey              = NULL;
	if (!FProvider)
	{
		if (!Flags)
			Flags = CRYPT_VERIFYCONTEXT;
		CryptAcquireContextA(&FProvider, Container, MS_ENHANCED_PROV_A, PROV_RSA_FULL, Flags);
    }
}


//--------------------------------------------------------
//  GenerateKey - ������� ���������� ����� � ��������
//                ����������
//--------------------------------------------------------
bool TWinCrypt::GenerateKey(DWORD AlgId, DWORD Flags)
{
	DestroyKey();

	//DWORD Sz = 1024 << 16;
	return CryptGenKey(FProvider, AlgId, Flags, &FKey) == TRUE;
}


//--------------------------------------------------------
//  ������� ������ ���� RC4 �� ������ ������.
//  �������� ����������� � FKey;
//--------------------------------------------------------
bool TWinCrypt::CreateRC4Key(const char *Password)
{
	DestroyKey();

	if (FProvider && !STRA::IsEmpty(Password))
	{
		// ������ ��� ������
        HCRYPTHASH Hash = HashData(CALG_MD5, Password, STRA::Length(Password));
		CryptDeriveKey(FProvider, CALG_RC4, Hash, 0, &FKey);
        CryptDestroyHash(Hash);
	}

	return FKey != 0;
}

//--------------------------------------------------------
//  ������� ���������� ��� ��������� ������
//--------------------------------------------------------
HCRYPTHASH TWinCrypt::HashData(DWORD Algoritm, const void*  Data, DWORD DataLen)
{
	HCRYPTHASH Hash = 0;
	if (FProvider)
	{
		CryptCreateHash(FProvider, Algoritm, NULL, 0, &Hash);
		if (Hash)
			CryptHashData(Hash, (LPBYTE)Data, DataLen, 0);
	}

    return Hash;
}

//--------------------------------------------------------
//  DoExportKey - ������� ������������ ��������� ����
//--------------------------------------------------------
LPBYTE TWinCrypt::DoExportKey(HCRYPTKEY ExpKey, DWORD BlobType, DWORD *BufSize)
{
	DWORD  Size = 0;
	LPBYTE Buf = NULL;

	if (FKey)
	{
		// ���������� ������ ������
		bool Ready = CryptExportKey(FKey, ExpKey, BlobType, 0, NULL, &Size) == TRUE;
		if (Ready)
		{
			Buf = (LPBYTE)MemAlloc(Size);
			CryptExportKey(FKey, ExpKey, BlobType, 0, Buf, &Size);
        }
	}

	if (BufSize) *BufSize = Size;
    return Buf;
}

//--------------------------------------------------------
//  DoImportKey - ������� ����������� ����
//--------------------------------------------------------
bool TWinCrypt::DoImportKey(HCRYPTKEY ExpKey, DWORD BlobType, LPBYTE Buf, DWORD BufSize)
{
	if (!FProvider || !Buf || !BufSize)
		return false;

	DestroyKey();

	return CryptImportKey(FProvider, Buf, BufSize, ExpKey, 0, &FKey) == TRUE;
}


//--------------------------------------------------------
//  ExportPrivateKey - ������� ������������ ��������� ����
//--------------------------------------------------------
LPBYTE TWinCrypt::ExportPrivateKey(const char *ExpPassword, DWORD *BufSize)
{
	if (!FKey) return NULL;

	TWinCrypt ExpKey(FProvider);
	ExpKey.CreateRC4Key(ExpPassword);

	return DoExportKey(ExpKey.Key(), PRIVATEKEYBLOB, BufSize);
}

//--------------------------------------------------------
//  ExportPublicKey - ������� ������������ ��������� ����
//--------------------------------------------------------
LPBYTE TWinCrypt::ExportPublicKey(DWORD *BufSize)
{
	return DoExportKey(0, PUBLICKEYBLOB, BufSize);
}

//--------------------------------------------------------
//  ImportPrivateKey - ������� ����������� ��������� ����
//--------------------------------------------------------
bool TWinCrypt::ImportPrivateKey(const char *ExpPassword, LPBYTE Buf, DWORD BufSize)
{
	TWinCrypt ExpKey(FProvider);
	ExpKey.CreateRC4Key(ExpPassword);

	return DoImportKey(ExpKey.Key(), 0, Buf, BufSize);
}

//--------------------------------------------------------
//  ImportPublicKey - ������� ����������� ��������� ����
//--------------------------------------------------------
bool TWinCrypt::ImportPublicKey(LPBYTE Buf, DWORD BufSize)
{
	return DoImportKey(0, 0, Buf, BufSize);
}


//--------------------------------------------------------
// Encode - ������� ������� ������
// Data - ������ ��� ����������
// DataSize - ������ ������.
// � ������ ������ ������� ������� ������ ��� �����������
// ����� � ����� �� ���� ���������. ������ ������
// ��������� � ���������� DataSize
//--------------------------------------------------------
LPBYTE TWinCrypt::Encode(LPBYTE Data, DWORD &DataSize)
{
	if (!FKey || !Data || !DataSize)
		return NULL;

	// ���������� ������ ��������������� ������
	DWORD NewSize = DataSize;
	LPBYTE NewBuf = NULL;
	BOOL Ready = CryptEncrypt(FKey, NULL, TRUE, 0, NULL, &NewSize, NULL);
	if (Ready)
	{
		NewBuf = (LPBYTE)MemAlloc(NewSize);
		if (NewBuf)
		{
			m_memcpy(NewBuf, Data, DataSize);
			Ready = CryptEncrypt(FKey, NULL, TRUE, 0, NewBuf, &DataSize, NewSize);
			if (!Ready)
			{
				MemFree(NewBuf);
				NewBuf = NULL;
            }
        }
	}

	DataSize = NewSize;
    return NewBuf;
}


//--------------------------------------------------------
//  Decode - ������� �������������� ������.
//  Data - ��������� �� ���� ����������� ������
//  DataSize - ������ ������.
//  � ������ ������ ������� ����� ����� ������ ������ �
//  ���������� DataSize
//--------------------------------------------------------
bool TWinCrypt::Decode(LPBYTE Data, DWORD &DataSize)
{
	if (!FKey || !Data || !DataSize)
		return false;
    DWORD NewSize = DataSize;
	bool Result = CryptDecrypt(FKey, NULL, TRUE, 0, Data, &NewSize) == TRUE;

	if (Result)
	{
		if (NewSize < DataSize)
            *(Data + NewSize) = 0;
		DataSize = NewSize;

    }

    return Result;
}


