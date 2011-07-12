#include "earley.h"

#include <stdlib.h>

struct TokenSet {
	char *memory;
	int start;
	int stop;
};

static void token_set_init(struct TokenSet *set, int start, int stop)
{
	int size = (stop - start + 7) / 8;

	set->memory = malloc(size);
	memset(set->memory, 0, size);
	set->start = start;
	set->stop = stop;
}

static void token_set_add(struct TokenSet *set, char token)
{
	int idx = (token - set->start) / 8;
	int bit = (token - set->start) % 8;

	set->memory[idx] |= (1 << bit);
}

static int token_set_test(struct TokenSet *set, char token)
{
	int idx = (token - set->start) / 8;
	int bit = (token - set->start) % 8;

	return set->memory[idx] & (1 << bit);
}

static void token_set_destroy(struct TokenSet *set)
{
	free(set->memory);
}

static void fill_item(struct EarleyItem *item, struct Rule *rule, int scanned, struct EarleySet *start)
{
	item->rule = rule;
	item->scanned = scanned;
	item->start = start;
}

static struct EarleyItem *allocate_item(struct EarleyItem *copy)
{
	struct EarleyItem *new_item;

	new_item = malloc(sizeof(struct EarleyItem));
	fill_item(new_item, copy->rule, copy->scanned, copy->start);

	return new_item;
}

static struct EarleyItem *find_item(struct Vector *vector, struct EarleyItem *target)
{
	struct EarleyItem *item;
	int i;

	for(i=0; i<vector->size; i++) {
		item = vector->data[i];
		if(item->rule == target->rule && item->scanned == target->scanned && item->start == target->start) {
			return item;
		}
	}

	return NULL;
}

static void add_item(struct EarleySet *set, struct EarleyItem *item)
{
	struct EarleyItem *new_item;

	if(item->scanned == strlen(item->rule->rhs)) {
		// If item is like X -> aBc. (n), add to set.completed
		if(find_item(&set->completed, item) != NULL) {
			return;
		}

		new_item = allocate_item(item);
		vector_append(&set->completed, new_item);
	} else {
		// If item is like X -> aB.c (n), add to set.active
		if(find_item(&set->active, item) != NULL) {
			return;
		}

		new_item = allocate_item(item);
		vector_append(&set->active, new_item);
	}
}

static void predict(struct EarleySet *set, struct Rule *grammar, struct TokenSet *nullable)
{
	struct EarleyItem *item;
	struct EarleyItem new_item;
	char token;
	int i, j;

	// For each item A -> x.By (n) in set.active...
	for(i=0; i<set->active.size; i++) {
		item = set->active.data[i];
		token = item->rule->rhs[item->scanned];

		// ...add all grammar rules B -> .X (idx) to set
		for(j = 0; grammar[j].lhs != 0; j++) {
			if(grammar[j].lhs == token) {
				fill_item(&new_item, &grammar[j], 0, set);
				add_item(set, &new_item);
			}
		}

		// ...if B is nullable, add A -> xB.y (n) to set
		if(token_set_test(nullable, token) != 0) {
			fill_item(&new_item, item->rule, item->scanned + 1, item->start);
			add_item(set, &new_item);
		}
	}
}

static void scan(struct EarleySet *set, struct EarleySet *next_set, char token)
{
	struct EarleyItem *item;
	struct EarleyItem new_item;
	int i;

	for(i=0; i<set->active.size; i++) {
		// For each item A -> x.cy (n) in set.active where input = c, add A -> xc.y (n) to next_set
		item = set->active.data[i];
		if(item->rule->rhs[item->scanned] == token) {
			fill_item(&new_item, item->rule, item->scanned + 1, item->start);
			add_item(next_set, &new_item);
		}
	}
}

static void complete(struct EarleySet *set)
{
	struct EarleyItem *item;
	int i;

	for(i=0; i<set->completed.size; i++) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		item = set->completed.data[i];
		scan(item->start, set, item->rule->lhs);
	}
}

static void compute_nullable(struct Rule *grammar, struct TokenSet *nullable)
{
	int i;
	int j;
	int len;
	int is_nullable;
	struct Rule *rule;
	int changed;

	do {
		changed = 0;
		for(i=0; grammar[i].lhs != 0; i++) {
			rule = &grammar[i];
			len = strlen(rule->rhs);
			is_nullable = 1;

			// A is nullable iff there is a rule producing A for which
			// all (zero or more) rhs tokens are nullable
			for(j=0; j<len; j++) {
				if(token_set_test(nullable, rule->rhs[j]) == 0) {
					is_nullable = 0;
					break;
				}
			}

			if(is_nullable && token_set_test(nullable, rule->lhs) == 0) {
				token_set_add(nullable, rule->lhs);
				changed = 1;
			}
		}
	} while(changed);
}

struct EarleySet *earley_parse(const char *input, struct Rule *grammar, struct Rule *start_rule)
{
	int i;
	struct EarleyItem start_item;
	struct Tree *tree;
	struct EarleySet *sets;
	struct TokenSet nullable;
	int len;

	token_set_init(&nullable, 0, 127);
	compute_nullable(grammar, &nullable);

	len = strlen(input);
	sets = malloc(sizeof(struct EarleySet) * (len + 1));
	for(i=0; i<len+1; i++) {
		vector_init(&sets[i].active);
		vector_init(&sets[i].completed);
	}

	fill_item(&start_item, start_rule, 0, &sets[0]);
	add_item(&sets[0], &start_item);

	for(i=0; i<len; i++) {
		predict(&sets[i], grammar, &nullable);
		scan(&sets[i], &sets[i+1], input[i]);
		complete(&sets[i+1]);
	}

	predict(&sets[len], grammar, &nullable);

	token_set_destroy(&nullable);

	return sets;
}