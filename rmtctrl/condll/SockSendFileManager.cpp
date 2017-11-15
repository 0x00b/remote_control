#include <WS2tcpip.h>
#include <process.h>
#include "socket_util.h"
#include "SockSendFileManager.h"
#include "fileop.h"


SOCKET SockSendFileManager::m_listen_sock;
sockaddr_in SockSendFileManager::m_listen_sa;
bool SockSendFileManager::m_exit;

SockSendFileManager::SockSendFileManager()
{
	MyMutex::InitCS(&m_recvcs);
	MyMutex::InitCS(&m_sendcs);

	m_sendbufsize = MAX_BUFF;
	m_sendbuf = (char*)MyMalloc(m_sendbufsize);

	m_recvbufsize = MAX_BUFF;
	m_recvbuf = (char*)MyMalloc(m_recvbufsize);

	m_thread[0] = NULL;
	m_thread[1] = NULL;

	m_exit = false;
}

SockSendFileManager::~SockSendFileManager()
{
	m_exit = true;
	WaitForMultipleObjects(2, m_thread, TRUE, INFINITE);

	CSktUtil::CSUClose(m_sock);

	free(m_sendbuf);
	free(m_recvbuf);
}

int SockSendFileManager::Init(char* severIP, int port)
{
	ZeroMemory(&m_sa, sizeof(sockaddr_in));
	m_sa.sin_family = AF_INET;
	m_sa.sin_port = htons(port);
	InetPtonA(AF_INET, severIP, &m_sa.sin_addr.S_un.S_addr);

	CSktUtil::CSUSucConnect(m_sock, m_sa);

	m_thread[0] = (HANDLE)_beginthread(DoSendFile, 0, this);
	m_thread[1] = (HANDLE)_beginthread(DoRecvFile, 0, this);

	return 0;
}

int SockSendFileManager::InitListen(int port,LPAcceptProc func)
{
	ZeroMemory(&m_listen_sa, sizeof(sockaddr_in));
	m_listen_sa.sin_family = AF_INET;
	m_listen_sa.sin_port = htons(port);
	m_listen_sa.sin_addr.S_un.S_addr = INADDR_ANY;
	//InetPton(AF_INET, severIP, &m_sa.sin_addr.S_un.S_addr);

	CSktUtil::CSUStartAndListen(m_listen_sock, m_listen_sa);

	CSktUtil::CSUAccept(m_listen_sock, func, NULL, m_exit);

	return 0;
}

void SockSendFileManager::Accept(void *data)
{
	SocketInfo* si = (SocketInfo*)data;

	SockSendFileManager* psssm = (SockSendFileManager*)si->data;
	
	psssm->m_sock = si->sock;
	//psssm->m_sa = si->addr;

	psssm->m_thread[0] = (HANDLE)_beginthread(DoSendFile, 0, psssm);
	psssm->m_thread[1] = (HANDLE)_beginthread(DoRecvFile, 0, psssm);

	WaitForMultipleObjects(2, psssm->m_thread, TRUE, INFINITE);
}

void SockSendFileManager::DoSendFile(void* data)
{
	SockSendFileManager* ssm = (SockSendFileManager*)data;

	CFileTransInfo* pfti;

	bool flag = false;
	while (!ssm->m_exit)
	{
		flag = false;
		{
			MyMutex mtx(&ssm->m_sendcs);
			if (ssm->m_sendqueue.empty())
			{
				flag = true;
			}
			else
			{
				pfti = ssm->m_sendqueue.front();
				ssm->m_sendqueue.pop();
			}
		}
		if (flag)
		{
			Sleep(10);
			continue;
		}
		//发送文件内容
		if (ssm->DoSendFile(pfti))
		{
			//ssm->m_exit = true;
		}
		delete (pfti);
	}
}
void SockSendFileManager::DoRecvFile(void* data)
{
	SockSendFileManager* ssm = (SockSendFileManager*)data;

	CFileTransInfo ofti;

	int ret = 0;
	int infolen = 0;
	char* buf;
	_uint32 temp = 0;

	while (!ssm->m_exit)
	{
		buf = ssm->m_recvbuf;
		if (CSktUtil::CSURecv(ssm->m_sock, buf, 4) <= 0)
		{
			//ssm->m_exit = true;
			CSktUtil::CSUClose(ssm->m_sock);
			CSktUtil::CSUSucConnect(ssm->m_sock, ssm->m_sa);
			continue;
			//break;	//err
		}
		infolen = ntohl(*(_uint32*)buf);
		if (CSktUtil::CSURecv(ssm->m_sock, buf, infolen) <= 0)
		{
			//ssm->m_exit = true;
			CSktUtil::CSUClose(ssm->m_sock);
			CSktUtil::CSUSucConnect(ssm->m_sock, ssm->m_sa);
			continue;
			//break;	//err
		}
		ofti.file_size = ntohl(*(_uint32*)buf);
		buf += 4;
		ofti.part_count = ntohl(*(_uint32*)buf);
		buf += 4;

		temp = ntohl(*(_uint32*)buf);
		buf += 4;
		strncpy_s(ofti.file_path, buf, temp);
		ofti.file_path[temp] = 0;
		buf += temp;

		temp = ntohl(*(_uint32*)buf);
		buf += 4;
		strncpy_s(ofti.file_name, buf , temp);
		ofti.file_name[temp] = 0;

		if (ssm->DoRecvFile(&ofti))
		{
			//ssm->m_exit = true;
		}
	}
}
int SockSendFileManager::DoSendFile(CFileTransInfo* pfti)
{
	char path[MAX_PATH];
	snprintf(path, MAX_PATH, "%s\\%s", pfti->file_path, pfti->file_name);

	FILE* f;

	fopen_s(&f, path, "rb");

	if (NULL == f)
	{
		return 1;
	}
	fseek(f, 0, SEEK_END);		//定位到文件末 
	pfti->file_size = ftell(f);	//文件长度
	pfti->part_count =1 + pfti->file_size / m_sendbufsize;

	char* buf = m_sendbuf;
	size_t infolen = 4;
	//先传输文件相关信息
	*(_uint32*)(buf + infolen) = htonl(pfti->file_size);	//文件总长度
	infolen += 4;
	*(_uint32*)(buf + infolen) = htonl(pfti->part_count);	//文件分为多少块
	infolen += 4;

	_uint32 filepathlen = strlen(pfti->remote_file_path);	//路径长度
	*(_uint32*)(buf + infolen) = htonl(filepathlen);		//
	infolen += 4;
	memcpy(buf + infolen, pfti->remote_file_path, filepathlen);//文件路径
	infolen += filepathlen;

	_uint32 filenamelen = strlen(pfti->file_name);			//文件名长度
	*(_uint32*)(buf + infolen) = htonl(filenamelen);		//
	infolen += 4;
	memcpy(buf + infolen, pfti->file_name, filenamelen);	//文件名
	infolen += filenamelen;

	*(_uint32*)buf = htonl(infolen - 4);						//文件信息头长度
	CSktUtil::CSUSend(m_sock, buf, infolen);				//这里infolen > m_sendbufsize,没处理，暂时不需要

	//传送文件内容
	fseek(f, 0, SEEK_SET);		//定位到文件头

	int partlen = 0;

	int remain = pfti->file_size;
	int ret = 0;
	for (size_t i = 0; i < pfti->part_count; i++)			//防止文件太大，不能一次加载到内存中
	{
		if (remain > m_sendbufsize)
		{
			remain -= m_sendbufsize;
			partlen = m_sendbufsize;
		}
		else
		{
			partlen = remain;
		}
		ret = MyReadFile(f, buf + 4, partlen);

		char* bbb = buf + 4;
		//在这里压缩buf

		*(int*)buf = htonl(partlen);
		ret = CSktUtil::CSUSend(m_sock, buf, partlen + 4);		 //本部分的内容
		if (ret <= 0)
		{
			CSktUtil::CSUClose(m_sock);
			CSktUtil::CSUSucConnect(m_sock, m_sa);
		}
	}


	fclose(f);

	return 0;
}

int SockSendFileManager::DoRecvFile(CFileTransInfo * pfti) 
{
	char path[MAX_PATH];
	snprintf(path, MAX_PATH, "%s\\%s", pfti->file_path, pfti->file_name);

	FILE* f;
	fopen_s(&f ,path, "wb");

	if (NULL == f)
	{
		//return 1;
	}

	int partlen = 0;
	int alllen = 0;
	int ret = 0;
	char* buf = m_recvbuf;

	for (size_t i = 0; i < pfti->part_count; i++)				//防止文件太大，不能一次加载到内存中
	{
		ret = CSktUtil::CSURecv(m_sock, buf, 4);
		partlen = ntohl(*(int*)buf);
		alllen += partlen;

		ret = CSktUtil::CSURecv(m_sock, buf, partlen);
		if (ret <= 0)
		{
			CSktUtil::CSUClose(m_sock);
			CSktUtil::CSUSucConnect(m_sock, m_sa);
			continue;
		}
		//在这里解压buf

		f && MyWriteFile(f, buf, partlen);
	}

	f&&fclose(f);

	if (alllen != pfti->file_size)
	{
		//err
	}


	return 0;
}

int SockSendFileManager::ReqSendFile(CFileTransInfo * pfti)
{
	MyMutex mtx(&m_sendcs);

	m_sendqueue.push(pfti);

	return 0;
}

const sockaddr_in * SockSendFileManager::getsaddr()
{
	return &m_sa;
}

void SockSendFileManager::SetEnd()
{
	m_exit = true;
}

bool SockSendFileManager::End()
{
	return m_exit;
}
