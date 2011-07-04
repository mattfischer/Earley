#include "pool.h"

void *allocate(int size, struct Pool *pool)
{
	void *ret = pool->memory + pool->start;
	pool->start += size;
	
	if(pool->start >= pool->size) {
		printf("Error: Memory overflow\n");
	}

	return ret;
}

void free_pool(struct Pool *pool)
{
	pool->start = 0;
}

