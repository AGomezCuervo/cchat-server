#include "arena.h"
#include <stdlib.h>
#define ALIGNMENT 64
#define HEAP_SIZE 4096

Arena *ArenaAlloc(Pool *pool)
{
	Arena *result = pool->first_free_page;
	if(result == NULL)
	{
		result = (Arena *)malloc(sizeof(Arena));
		if(result == NULL)
		{
			err_sys("Cannot alloc memory for arena");
		}
		result->next = NULL;
		result->stack_memory == NULL;
		result->stack_pos = 0;
		int r = posix_memalign((void**)&(result->stack_memory), ALIGNMENT, HEAP_SIZE);

		if(r != 0)
		{
			err_sys("Error allocating Arena");
		}

		if(pool->pages == NULL)
		{
			pool->pages = result;
		}
		else
		{
			Arena *i = pool->pages;
			while(i->next != NULL)
			{
				i = i->next;
			}
			i->next = result;
		}
		
	}
	else
	{
		pool->first_free_page = pool->first_free_page->next;
		memset(result, 0, sizeof(result));
	}
	
	return result;
}

void ArenaRelease(Arena *arena, Pool *pool)
{
	arena->stack_pos = 0;
	Arena *result = pool->first_free_page;
	if(result == NULL)
	{
		pool->first_free_page = arena;
	}
	else
	{
		while(result->next != NULL)
		{
			result = result->next;
		}
		result->next = arena;
	}
}

void *ArenaPush(Arena *arena, uint64_t size)
{
	void *ptr = arena->stack_memory + arena->stack_pos;
	arena->stack_pos += size;
	return ptr;
}

void *ArenaZeroPush(Arena *arena, uint64_t size)
{
	void *ptr = arena->stack_memory + arena->stack_pos;
	arena->stack_pos += size;
	memset(ptr, 0, size);
	
	return ptr;
}

uint64_t ArenaMaxSize()
{
	return HEAP_SIZE;
}

void ArenaPop(Arena *arena, uint64_t size)
{
	arena->stack_pos -= size;
}

uint64_t ArenaGetPos(Arena *arena)
{
	return arena->stack_pos;
}

void ArenaClear(Arena *arena)
{
	arena->stack_pos = 0;
}

void ArenaSetPosBack(Arena *arena, uint64_t pos)
{
	assert(pos <= arena->stack_pos);
	arena->stack_pos = pos;
}
