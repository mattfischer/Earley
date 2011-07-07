#include "earley.h"

#include <stdlib.h>

#include "print.h"
#include "list.h"

void token_set_init(struct TokenSet *set, int start, int stop)
{
	int size = (stop - start + 7) / 8;

	set->memory = malloc(size);
	memset(set->memory, 0, size);
	set->start = start;
	set->stop = stop;
}

void token_set_add(struct TokenSet *set, char token)
{
	int idx = (token - set->start) / 8;
	int bit = (token - set->start) % 8;

	set->memory[idx] |= (1 << bit);
}

int token_set_test(struct TokenSet *set, char token)
{
	int idx = (token - set->start) / 8;
	int bit = (token - set->start) % 8;

	return set->memory[idx] & (1 << bit);
}

static void fill_item(struct EarleyItem *item, struct Rule *rule, int scanned, struct EarleySet *start)
{
	item->rule = rule;
	item->scanned = scanned;
	item->start = start;
	list_init(item->list);
}

static struct EarleyItem *allocate_item(struct EarleyItem *copy)
{
	struct EarleyItem *new_item;

	new_item = malloc(sizeof(struct EarleyItem));
	fill_item(new_item, copy->rule, copy->scanned, copy->start);

	return new_item;
}

static struct EarleyItem *find_item(struct ListHead *list, struct EarleyItem *item)
{
	struct EarleyItem *cursor;

	list_foreach(struct EarleyItem, cursor, list, *list) {
		if(cursor->rule == item->rule && cursor->scanned == item->scanned && cursor->start == item->start) {
			return cursor;
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
		list_add_tail(set->completed, new_item->list);
	} else {
		// If item is like X -> aB.c (n), add to set.active
		if(find_item(&set->active, item) != NULL) {
			return;
		}

		new_item = allocate_item(item);
		list_add_tail(set->active, new_item->list);
	}
}

static void predict(struct EarleySet *set, struct Rule *grammar, struct TokenSet *nullable)
{
	struct EarleyItem *cursor;
	struct EarleyItem new_item;
	char token;
	int i;

	// For each item A -> x.By (n) in set.active...
	list_foreach(struct EarleyItem, cursor, list, set->active) {
		token = cursor->rule->rhs[cursor->scanned];

		// ...add all grammar rules B -> .X (idx) to set
		for(i = 0; grammar[i].lhs != 0; i++) {
			if(grammar[i].lhs == token) {
				fill_item(&new_item, &grammar[i], 0, set);
				add_item(set, &new_item);
			}
		}

		// ...if B is nullable, add A -> xB.y (n) to set
		if(token_set_test(nullable, token) != 0) {
			fill_item(&new_item, cursor->rule, cursor->scanned + 1, cursor->start);
			add_item(set, &new_item);
		}
	}
}

static void scan(struct EarleySet *set, struct EarleySet *next_set, char token)
{
	struct EarleyItem *cursor;
	struct EarleyItem new_item;

	list_foreach(struct EarleyItem, cursor, list, set->active) {
		// For each item A -> x.cy (n) in set.active where input = c, add A -> xc.y (n) to next_set
		if(cursor->rule->rhs[cursor->scanned] == token) {
			fill_item(&new_item, cursor->rule, cursor->scanned + 1, cursor->start);
			add_item(next_set, &new_item);
		}
	}
}

static void complete(struct EarleySet *set)
{
	struct EarleyItem *cursor;

	list_foreach(struct EarleyItem, cursor, list, set->completed) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		scan(cursor->start, set, cursor->rule->lhs);
	}
}
				
static struct Tree *create_tree(struct EarleySet sets[], int idx, char token, int *start_idx)
{
	struct EarleySet *set;
	struct EarleyItem *cursor;
	struct EarleyItem *item;
	struct Rule *rule;
	struct Tree *tree;
	int i;
	int current_idx;
	int len;

	set = &sets[idx];
	item = NULL;
	// Search for an item X -> aBc. (n) in current set, where token = X
	list_foreach(struct EarleyItem, cursor, list, set->completed) {
		if(cursor->rule->lhs == token) {
			item = cursor;
			break;
		}
	}

	if(item == NULL) {
		if(start_idx != NULL) {
			*start_idx = idx - 1;
		}

		return NULL;
	}

	rule = item->rule;
	len = strlen(rule->rhs);
	current_idx = idx;
	tree = malloc(sizeof(struct Tree) + sizeof(struct Tree*) * (len - 1));
	tree->rule = rule;
	for(i=len - 1; i>=0; i--) {
		// For rule X -> ABC. (n), recursively create each subtree, recording the 
		// number of tokens consumed by each node
		tree->children[i] = create_tree(sets, current_idx, rule->rhs[i], &current_idx);
	}

	if(start_idx != NULL) {
		*start_idx = current_idx;
	}

	return tree;
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
				if(token_set_test(nullable, rule->rhs[j]) != 0) {
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
		list_init(sets[i].active);
		list_init(sets[i].completed);
	}

	fill_item(&start_item, start_rule, 0, &sets[0]);
	add_item(&sets[0], &start_item);

	for(i=0; i<len; i++) {
		predict(&sets[i], grammar, &nullable);
		scan(&sets[i], &sets[i+1], input[i]);
		complete(&sets[i+1]);
	}

	return sets;
}

struct Tree *earley_tree(struct EarleySet sets[], int num_sets, struct Rule *start_rule)
{
	return create_tree(sets, num_sets - 1, start_rule->lhs, NULL);
}

