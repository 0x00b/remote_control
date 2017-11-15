#ifndef _MY_MUTEX_H_
#define _MY_MUTEX_H_

#include <Windows.h>

class MyMutex
{
public:
	static void InitCS(CRITICAL_SECTION* cs);
	MyMutex(CRITICAL_SECTION* cs);
	~MyMutex();
private:
	CRITICAL_SECTION* m_cs;
};

#endif // !_MY_MUTEX_H_


