#ifndef ARENA_H
#define ARENA_H

#include <assert.h>
#include <stdint.h>

typedef struct Arena {
	struct Arena *next;
	char *stack_memory;
	uint64_t stack_pos;
	void *extra;
} Arena;

typedef struct {
	Arena *pages;
	Arena *first_free_page;
} Pool;

Arena *ArenaAlloc(Pool *pool);
void ArenaRelease(Arena *arena, Pool *pool);
void *ArenaPush(Arena *arena, uint64_t size);
void *ArenaZeroPush(Arena *arena, uint64_t size);

#define PushArray(arena, type, count) (type *)ArenaPush((arena), sizeof(type)*(count));
#define PushArrayZero(arena, type, count) (type *)ArenaPushZero((arena), sizeof(type)*(count));
#define PushStruct(arena, type) PushArray((arena), (type), 1);
#define PushStructZero(arena, type) PushArrayZero((arena), (type), 1);

uint64_t ArenaMaxSize();
void ArenaPop(Arena *arena, uint64_t size);
uint64_t ArenaGetPos(Arena *arena);
void ArenaClear(Arena *arena);
void ArenaSetPosBack(Arena *arena, uint64_t pos);
void ArenaSetExtra(Arena *arena, void *data);

#endif
