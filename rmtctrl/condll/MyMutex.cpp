#include "MyMutex.h"

void MyMutex::InitCS(CRITICAL_SECTION* cs)
{
	InitializeCriticalSection(cs);
}

MyMutex::MyMutex(CRITICAL_SECTION* cs)
{
	m_cs = cs;
	EnterCriticalSection(m_cs);
}


MyMutex::~MyMutex()
{
	LeaveCriticalSection(m_cs);
}
