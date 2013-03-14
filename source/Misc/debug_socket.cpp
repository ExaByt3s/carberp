//#include <winsock2.h>
#include <windows.h>
#include "debug_socket.h"
#include "Splice.h"
#include "Utils.h"
#include "memory.h"
#include "BotDebug.h"

typedef int (WINAPI *type_send)( SOCKET s, const char FAR* buf, int len, int flags );
typedef int (WINAPI *type_recv)( SOCKET s, char FAR* buf, int len, int flags );

static int WINAPI handler_send( SOCKET s, const char FAR* buf, int len, int flags );
static int WINAPI handler_recv( SOCKET s, char FAR* buf, int len, int flags );

type_send real_send;
type_recv real_recv;

fwsprintfA pwsprintf;

void StartDebugSocket()
{
	HookApi( DLL_WINSOCK, 0xE797764, &handler_send, &real_send );
	HookApi( DLL_WINSOCK, 0xE5971F6, &handler_recv, &real_recv );
	pwsprintf = Get_wsprintfA();
}

static int BufToHex( const char* from, int len, char* to )
{
	int ret = 0;
	for( int i = 0; i < len; i++ )
		ret += pwsprintf( to + ret, "%02x ", int((BYTE)from[i]) );
	return ret;
}

static int WINAPI handler_send( SOCKET s, const char FAR* buf, int len, int flags )
{
	int size = len * 3 + 64;
	char* dbgBuf = (char*)HEAP::Alloc(size);
	int lenDbg = pwsprintf( dbgBuf, "SEND to socket %d, bytes: %d, data: ", s, len );
	lenDbg += BufToHex( buf, len, dbgBuf + lenDbg );
	pOutputDebugStringA(dbgBuf);
	HEAP::Free(dbgBuf);
	return real_send( s, buf, len, flags );
}

static int WINAPI handler_recv( SOCKET s, char FAR* buf, int len, int flags )
{
	len = real_recv( s, buf, len, flags );
	int size = len * 3 + 64;
	char* dbgBuf = (char*)HEAP::Alloc(size);
	int lenDbg = pwsprintf( dbgBuf, "RECV from socket %d, bytes: %d, data: ", s, len );
	lenDbg += BufToHex( buf, len, dbgBuf + lenDbg );
	pOutputDebugStringA(dbgBuf);
	HEAP::Free(dbgBuf);
	return len;
}

