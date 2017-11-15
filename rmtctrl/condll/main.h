#ifndef _MY_MAIN_H_
#define _MY_MAIN_H_
#include <WS2tcpip.h>
#include <Windows.h>

#include <queue>
#include "MyMutex.h"
#include "mymalloc.h"

#ifdef __cplusplus
#define MEXPORT extern "C" __declspec (dllexport)
#else
#define MEXPORT __declspec (dllexport)
#endif

#define BUFFSIZE (1024*1024)
#define SEVER_IP "127.0.0.1"
//#define SEVER_IP "10.14.114.146"

#define SEVER_FPORT 8888
#define SEVER_PORT 8889

typedef unsigned int _uint32;

typedef struct _MOrder
{
	_uint32 cmd;
	char data[MAX_PATH];
}MOrder;

void DoEveryThingYouWant(void*);

void MySucConnect();

bool MyVisitFile(const char* lpPath, int kind, void* data);

#endif