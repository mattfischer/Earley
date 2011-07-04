#ifndef EARLEY_H
#define EARLEY_H

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

struct Tree *parse(const char *input, struct Rule *grammar);

#endif
