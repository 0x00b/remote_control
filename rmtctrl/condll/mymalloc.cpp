#include "mymalloc.h"
#include <malloc.h>

void* MyMalloc(size_t& size)
{
	char* buf = NULL;
	//���仺������ֱ���ɹ�Ϊֹ
	size_t max = size;
	while (!((size >= 512) && (buf = (char*)malloc(size))))
	{
		size /= 2;//����Сһ���ڴ�
		if (size < 512)
		{
			size = max;
		}
	}
	return buf;
}