#ifndef POOL_H
#define POOL_H

struct Pool {
	char *memory;
	int start;
	int size;
};

void *allocate(int size, struct Pool *pool);
void free_pool(struct Pool *pool);

#endif