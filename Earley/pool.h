#ifndef POOL_H
#define POOL_H

struct Pool {
	char *memory;
	int start;
	int size;
};

void pool_create(struct Pool *pool, int size);
void pool_free(struct Pool *pool);

void *pool_alloc(struct Pool *pool, int size);

#endif