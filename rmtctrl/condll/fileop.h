#ifndef _MY_FILE_OP_H_
#define _MY_FILE_OP_H_

#include "main.h"

typedef bool(*pVisitFile)(const char* lpPath, int kind, void* data);

bool TraverFolder(pVisitFile visit, const char* lpPath, void* data,int findsubdir);

int MyWriteFile(FILE* f, char* buf, int len);
int MyReadFile(FILE* f, char* buf, int len);

#endif