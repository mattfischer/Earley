#ifndef VECTOR_H
#define VECTOR_H

struct Vector {
	int size;
	int capacity;
	void **data;
};

void vector_init(struct Vector *vector);
void vector_destroy(struct Vector *vector);
void vector_append(struct Vector *vector, void *item);
void vector_remove(struct Vector *vector, int n);
void vector_clear(struct Vector *vector);

#endif