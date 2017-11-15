#include "mymalloc.h"
#include <malloc.h>

void* MyMalloc(size_t& size)
{
	char* buf = NULL;
	//分配缓冲区，直到成功为止
	size_t max = size;
	while (!((size >= 512) && (buf = (char*)malloc(size))))
	{
		size /= 2;//分配小一点内存
		if (size < 512)
		{
			size = max;
		}
	}
	return buf;
}