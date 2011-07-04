#ifndef PRINT_H
#define PRINT_H

#include "earley.h"

void print_sets(const char *input, struct EarleySet sets[]);
void print_tree(struct Tree *tree, int indent);

#endif