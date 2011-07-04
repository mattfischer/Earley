#ifndef EARLEY_H
#define EARLEY_H

#include "pool.h"

struct Rule {
	char lhs;
	char *rhs;
};

struct Tree {
	struct Rule *rule;
	struct Tree **children;
};

struct EarleySet;

struct EarleyItem {
	struct EarleyItem *next;
	struct Rule *rule;
	int scanned;
	struct EarleySet *start;
};

struct EarleySet {
	struct EarleyItem *active;
	struct EarleyItem *completed;
};

struct EarleySet *earley_parse(const char *input, struct Rule *grammar, struct Rule *start_rule, struct Pool *active_pool, struct Pool *completed_pool);
struct Tree *earley_tree(struct EarleySet sets[], int num_sets, struct Rule *start_rule, struct Pool *pool);

#endif
