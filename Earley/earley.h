#ifndef EARLEY_H
#define EARLEY_H

#include "list.h"

struct Rule {
	char lhs;
	char *rhs;
};

struct Tree {
	struct Rule *rule;
	struct Tree *children[1];
};

struct EarleySet;

struct EarleyItem {
	struct Rule *rule;
	int scanned;
	struct EarleySet *start;
	struct ListHead list;
};

struct EarleySet {
	struct ListHead active;
	struct ListHead completed;
};

struct TokenSet {
	char *memory;
	int start;
	int stop;
};

struct EarleySet *earley_parse(const char *input, struct Rule *grammar, struct Rule *start_rule);
struct Tree *earley_tree(struct EarleySet sets[], int num_sets, struct Rule *start_rule);

void token_set_init(struct TokenSet *set, int start, int stop);
void token_set_add(struct TokenSet *set, char token);
int token_set_test(struct TokenSet *set, char token);
#endif
