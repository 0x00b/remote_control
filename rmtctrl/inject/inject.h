#ifndef _INJECT_MAIN_
#define _INJECT_MAIN_

#include <Windows.h>
#include <TlHelp32.h>


#define MY_DLL_NAME "\\condll.dll"

/*
 * Զ��ע��dll�������У����е�dll������һ�飬ֱ��ע��ɹ�Ϊֹ
 */
int InjectProcess(PROCESSENTRY32& pe, char* dllPath);

/* 
 * ����Ȩ��
 */
bool ImprovePriv();

//дע���������
bool AutoStart(char* path);

#endif
