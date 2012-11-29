#ifndef IFobsH
#define IFobsH


namespace IFobs
{

bool Init( const char* appName );
DWORD WINAPI FindIFobsClient(LPVOID);
void KillIFobs();

};


#endif //IFobsH
