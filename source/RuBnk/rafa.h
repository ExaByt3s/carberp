#ifndef RafaH
#define RafaH

#define RAFAHOSTS_PARAM_SIZE      100
#define RAFAHOSTS_PARAM_NAME      "__RAFA_HOSTS__\0"
#define RAFAHOSTS_PARAM_ENCRYPTED true


namespace Rafa
{
	void Init();
	PCHAR Hosts();
};

#endif //RafaH
