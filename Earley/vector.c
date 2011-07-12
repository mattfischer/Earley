#include "vector.h"

static void check_resize(struct Vector *vector)
{
	if(vector->size > vector->capacity) {
		vector->capacity *= 2;
		vector->data = (void**)realloc(vector->data, vector->capacity * sizeof(void*));
	} else if(vector->size < vector->capacity / 2) {
		vector->capacity /= 2;
		vector->data = (void**)realloc(vector->data, vector->capacity * sizeof(void*));
	}
}

void vector_init(struct Vector *vector)
{
	vector->size = 0;
	vector->capacity = 1;
	vector->data = (void**)malloc(vector->capacity * sizeof(void*));
}

void vector_destroy(struct Vector *vector)
{
	free(vector->data);
}

void vector_append(struct Vector *vector, void *item)
{
	vector->size++;
	check_resize(vector);
	vector->data[vector->size - 1] = item;
}

void vector_remove(struct Vector *vector, int n)
{
	memcpy(vector->data + n, vector->data + n + 1, (vector->size - n - 1 ) * sizeof(void*));
	vector->size--;
	check_resize(vector);
}

void vector_clear(struct Vector *vector)
{
	vector->size = 0;
}