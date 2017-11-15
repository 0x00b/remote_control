#pragma once

#include "resource.h"

void MyAccept(void*);
void AcceptProc(void*);

void MyFileAccept(void*);
void FileAcceptProc(void* data);

bool MyVisitFile(const char* lpPath, int kind, void* data);