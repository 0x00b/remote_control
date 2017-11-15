#include "fileop.h"
#include <stdio.h>
#include <Windows.h>


int MyReadFile(FILE* f, char* buf, int len)
{
	int max = 0;
	int n = 0;
	int remain = len;
	while (remain > 0)
	{
		n = fread(buf, 1, remain, f);
		if (n <= 0)
		{
			return n;	//сп╢М
		}
		buf += n;
		max += n;
		remain -= n;
	}
	return max;
}

int MyWriteFile(FILE* f, char* buf, int len)
{
	int max = 0;
	int n = 0;
	int remain = len;
	while (remain > 0)
	{
		n = fwrite(buf, 1, len, f);
		if (n <= 0)
		{
			return n;	//сп╢М
		}
		buf += n;
		max += n;
		remain -= n;
	}
	return max;
}
bool TraverFolder(pVisitFile visit, const char* lpPath, void* data, int findsubdir)
{
	char szFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];
	bool bret = true;
	WIN32_FIND_DATAA FindFileData;
	strcpy_s(szFind, lpPath);

	strcat_s(szFind, "\\*.*");
	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return true;
	}
	while (TRUE)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				lstrcpyA(szFile, FindFileData.cFileName);
				//lstrcpyA(szFile, lpPath);
				//lstrcatA(szFile, "\\");
				//lstrcatA(szFile, FindFileData.cFileName);
				(!visit(szFile, FILE_ATTRIBUTE_DIRECTORY, data)) && (bret && (bret = false));
				findsubdir && (!TraverFolder(visit, szFile, data, findsubdir)) && (bret && (bret = false));
			}
		}
		else
		{
			lstrcpyA(szFile, FindFileData.cFileName);
			//lstrcpyA(szFile, lpPath);
			//lstrcatA(szFile, "\\");
			//lstrcatA(szFile, FindFileData.cFileName);
			(!visit(szFile, 0, data)) && (bret && (bret = false));
		}

		if (!FindNextFileA(hFind, &FindFileData))
		{
			break;
		}
	}
	FindClose(hFind);
	return bret;
}

