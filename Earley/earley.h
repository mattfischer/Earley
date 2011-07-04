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

struct Set;

struct Item {
	struct Item *next;
	struct Rule *rule;
	int scanned;
	struct Set *start;
};

struct Set {
	struct Item *active;
	struct Item *completed;
};

struct Set *parse(const char *input, struct Rule *grammar, struct Rule *start_rule, struct Pool *active_pool, struct Pool *completed_pool);
struct Tree *parse_tree(struct Set sets[], int num_sets, struct Rule *start_rule, struct Pool *pool);

#endif
