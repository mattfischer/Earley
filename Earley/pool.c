#include "pool.h"

#include <stdlib.h>
#include <malloc.h>

void pool_create(struct Pool *pool, int size)
{
	pool->memory = malloc(size);
	pool->size = size;
	pool->start = 0;
}

void pool_free(struct Pool *pool)
{
	free(pool->memory);
}

void *pool_alloc(struct Pool *pool, int size)
{
	void *ret = pool->memory + pool->start;
	pool->start += size;
	
	if(pool->start >= pool->size) {
		return NULL;
	}

	return ret;
}