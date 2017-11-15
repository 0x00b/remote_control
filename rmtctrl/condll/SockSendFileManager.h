#ifndef _MY_SOCK_SEND_MANAGER_H_
#define _MY_SOCK_SEND_MANAGER_H_

#include "main.h"

typedef struct _CFileTransInfo
{
	_uint32 file_size;	//文件大小					自动赋值
	_uint32 part_count;	//文件分为多少部分传输		自动赋值
	_uint32 recved_size;//已接受的大小				自动赋值
	int zip;			//压缩方式

	char file_name[MAX_PATH];		//文件名,		必须赋值
	char file_path[MAX_PATH];		//文件路径		必须赋值
	char remote_file_path[MAX_PATH];//远程文件路径  必须赋值

	int result;
	//to add para

	_CFileTransInfo()
	{
		zip = 0;
		file_size = 0;
		part_count = 0;
		recved_size = 0;
		result = 0;
		memset(file_name, 0x00, sizeof(file_name));
		memset(file_path, 0x00, sizeof(file_path));
		memset(remote_file_path, 0x00, sizeof(remote_file_path));
	}
}CFileTransInfo;

class SockSendFileManager
{
public:
	enum 
	{
		MAX_BUFF = 1024 * 1024 * 2,
	};
	SockSendFileManager();
	~SockSendFileManager();

	int Init(char* severIP, int port);
	static int InitListen(int port, LPAcceptProc func = Accept);
	static void Accept(void*);
	const sockaddr_in* getsaddr();
	void SetEnd();
	bool End();
	int ReqSendFile(CFileTransInfo* pfti);

	static SOCKET m_listen_sock;
	static sockaddr_in m_listen_sa;
	static bool m_exit;

private:
	SOCKET m_sock;
	sockaddr_in m_sa;

	static void DoSendFile(void* data);
	static void DoRecvFile(void* data);

	int DoSendFile(CFileTransInfo* pfti);
	int DoRecvFile(CFileTransInfo* pfti);

protected:
	std::queue<CFileTransInfo*> m_recvqueue;
	std::queue<CFileTransInfo*> m_sendqueue;


	CRITICAL_SECTION m_recvcs;
	CRITICAL_SECTION m_sendcs;

	char* m_sendbuf;
	size_t m_sendbufsize;

	char* m_recvbuf;
	size_t m_recvbufsize;


	HANDLE m_thread[2];
};


#endif 
