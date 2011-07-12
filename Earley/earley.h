#ifndef EARLEY_H
#define EARLEY_H

#include "vector.h"

struct Rule {
	char lhs;
	char *rhs;
};

struct EarleySet {
	struct Vector active;
	struct Vector completed;
};

struct EarleyItem {
	struct Rule *rule;
	int scanned;
	struct EarleySet *start;
};

struct Tree {
	struct Rule *rule;
	int start;
	int end;
	int span;
	int num_children;
	struct Tree *parent;
	struct Tree *children[1];
};

struct EarleySet *earley_parse(const char *input, struct Rule *grammar, struct Rule *start_rule);
void earley_tree(struct EarleySet sets[], int num_sets, const char *input, struct Rule *start_rule, struct Vector *vector);

#endif
