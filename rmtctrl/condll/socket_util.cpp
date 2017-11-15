#include <WS2tcpip.h>
#include <process.h>

#include "socket_util.h"


#pragma comment(lib, "Ws2_32.lib")

//必须先调用CSUInit
bool CSktUtil::CSUInit()
{
#ifdef _WINDOWS_
	WSADATA  Ws;
	
	//初始化windows socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0)
	{
		return false;
	}
#endif
	return true;
}
//连接指定的IP，端口的服务器
int CSktUtil::CSUConnect(SOCKET& s, char* addr, int port)
{
	int ret = 0;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s)
	{
		//创建socket失败
		return 1;
	}
	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	InetPtonA(AF_INET, addr, &sa.sin_addr.S_un.S_addr);

	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		CSUClose(s);
		return 2;
	}
	return 0;
}
//连接指定的IP，端口的服务器
int CSktUtil::CSUConnect(SOCKET& s, sockaddr_in& sa)
{
	int ret = 0;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s)
	{
		//创建socket失败
		return 1;
	}
	ret = connect(s, (sockaddr*)&sa, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		CSUClose(s);
		return 2;
	}
	return 0;
}

sockaddr_in CSktUtil::CSUSucConnect(SOCKET & s, char* addr, int port)
{
	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	InetPtonA(AF_INET, addr, &sa.sin_addr.S_un.S_addr);

	while (CSktUtil::CSUConnect(s, sa))
	{
		Sleep(5000);
	}
	return sa;
}
void CSktUtil::CSUSucConnect(SOCKET & s, sockaddr_in& sa)
{
	while (CSktUtil::CSUConnect(s, sa))
	{
		Sleep(5000);
	}
}

int CSktUtil::CSUClose(SOCKET s)
{
	return closesocket(s);
}

int CSktUtil::CSURecv(SOCKET s, char* buf, int len)
{
	int max = 0;
	int n = 0;
	int remain = len;
	while (remain > 0)
	{	
		n = recv(s, buf, remain, 0);
		if (n <= 0)
		{
			//EAGAIN

			return n;	//有错
		}
		buf += n;
		max += n;
		remain -= n;
	}
	return max;
}
int CSktUtil::CSUSend(SOCKET s, char* buf, int len)
{
	int max = 0;
	int n = 0;
	int remain = len;
	while (remain > 0)
	{
		n = send(s, buf, remain, 0);
		if (n <= 0)
		{
			return n;	//有错
		}
		buf += n;
		max += n;
		remain -= n;
	}
	return max;
}

int CSktUtil::CSUStartAndListen(SOCKET& s, int port)
{
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == s)
	{
		return 1;
	}

	sockaddr_in sa;
	memset(&sa, 0x00, sizeof(sockaddr_in));

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.S_un.S_addr = INADDR_ANY;

	if (SOCKET_ERROR == bind(s, (sockaddr*)&sa, sizeof(sockaddr)))
	{
		return 2;
	}

	if (SOCKET_ERROR == listen(s, 5))
	{
		return 3;
	}

	return 0;
}
int CSktUtil::CSUStartAndListen(SOCKET& s, sockaddr_in& sa)
{
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == s)
	{
		return 1;
	}

	if (SOCKET_ERROR == bind(s, (sockaddr*)&sa, sizeof(sockaddr)))
	{
		return 2;
	}

	if (SOCKET_ERROR == listen(s, 5))
	{
		return 3;
	}

	return 0;
}

int CSktUtil::CSUCleanup()
{
#ifdef _WINDOWS_
	//释放windows soket
	return WSACleanup();
#endif
}

int CSktUtil::CSUAccept(SOCKET s, LPAcceptProc fuc, void* data,bool& exit)
{
	SocketInfo* si;
	int cnt = 0;
	while (!exit /*|| cnt > 100*/)
	{
		si = new SocketInfo();
		si->data = data;
		si->sock = accept(s, (sockaddr*)&si->addr, &si->addrlen);
		if(INVALID_SOCKET != si->sock)
		{
			_beginthread(fuc, 0, si);	
		}
		else
		{
			cnt++;
		}

	}

	return 0;
}
