#include "inject.h"

#include <windows.h>
#include <stdio.h>


int 
#if 1
WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPreInst,LPWSTR lpCmdLine, int nCmdShow)
#else
main()
#endif
{
	//提升进程访问权限
	printf("%d\n", ImprovePriv());
	PROCESSENTRY32 pe;
	char path[MAX_PATH] = {0};
	char exe[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, path);

	snprintf(exe, MAX_PATH, "%s\\inject.exe", path);
	//printf("%d",AutoStart(exe));
	
	strcat(path, MY_DLL_NAME);
	printf("%s %d\n",pe.szExeFile ,InjectProcess(pe, path));
	FILE* fp = fopen("D:\\PN.txt","w");
	fprintf(fp, pe.szExeFile);
	fclose(fp);

	//LoadLibrary("condll.dll");
	//printf("%d", LoadLibrary);

	//system("pause");
	return 0;
}
