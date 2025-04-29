#include "main.h"

void *Calloc(size_t nmemb, size_t size)
{
	void *alloc = calloc(nmemb, size);
	if(alloc == NULL)
	{
		err_sys("Calloc failed");
	}

	return alloc;
}
