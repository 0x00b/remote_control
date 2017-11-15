#ifndef _MY_SOCKET_UTIL_H_
#define _MY_SOCKET_UTIL_H_

#include "main.h"

typedef void (*LPAcceptProc)(void*);

struct SocketInfo
{
	SOCKET sock;
	sockaddr_in addr;
	int addrlen;
	void* data;

	SocketInfo()
	{
		addrlen = sizeof(sockaddr_in);
		data = NULL;
	}
	~SocketInfo()
	{
		delete data ;
	}
};

class CSktUtil
{
public:

	static bool CSUInit();
	static int CSUCleanup();

	static int CSUConnect(SOCKET& s, char* addr, int port);
	static int CSUConnect(SOCKET& s, sockaddr_in& sa);
	static void CSUSucConnect(SOCKET& s, sockaddr_in& sa);
	static sockaddr_in CSUSucConnect(SOCKET & s, char* addr, int port);

	static int CSUClose(SOCKET s);

	static int CSURecv(SOCKET s, char* buf, int len);
	static int CSUSend(SOCKET s, char* buf, int len);

	static int CSUStartAndListen(SOCKET& s, int port);
	static int CSUStartAndListen(SOCKET& s, sockaddr_in& sa);

	static int CSUAccept(SOCKET s, LPAcceptProc fuc, void* data, bool& exit);

private:

};


#endif