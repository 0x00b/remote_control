#ifndef _MY_SOCK_SEND_MANAGER_H_
#define _MY_SOCK_SEND_MANAGER_H_

#include "main.h"

typedef struct _CFileTransInfo
{
	_uint32 file_size;	//�ļ���С					�Զ���ֵ
	_uint32 part_count;	//�ļ���Ϊ���ٲ��ִ���		�Զ���ֵ
	_uint32 recved_size;//�ѽ��ܵĴ�С				�Զ���ֵ
	int zip;			//ѹ����ʽ

	char file_name[MAX_PATH];		//�ļ���,		���븳ֵ
	char file_path[MAX_PATH];		//�ļ�·��		���븳ֵ
	char remote_file_path[MAX_PATH];//Զ���ļ�·��  ���븳ֵ

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
