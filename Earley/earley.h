#ifndef EARLEY_H
#define EARLEY_H

#include "list.h"
#include "vector.h"

struct Rule {
	char lhs;
	char *rhs;
};

struct Tree {
	struct Rule *rule;
	int start;
	int end;
	int span;
	int num_children;
	struct ListHead list;
	struct ListHead memo;
	struct Tree *parent;
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
void earley_tree(struct EarleySet sets[], int num_sets, const char *input, struct Rule *start_rule, struct Vector *vector);

void token_set_init(struct TokenSet *set, int start, int stop);
void token_set_add(struct TokenSet *set, char token);
int token_set_test(struct TokenSet *set, char token);
#endif
