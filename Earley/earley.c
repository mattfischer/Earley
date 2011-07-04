#include "earley.h"

#include <stdlib.h>

#include "pool.h"
#include "print.h"

struct Allocator {
	struct Pool *active_pool;
	struct Pool *completed_pool;
};

static void fill_item(struct Item *item, struct Rule *rule, int scanned, struct Set *start)
{
	item->rule = rule;
	item->scanned = scanned;
	item->start = start;
	item->next = NULL;
}

static struct Item *allocate_item(struct Item *copy, struct Pool *pool)
{
	struct Item *new_item;

	new_item = pool_alloc(pool, sizeof(struct Item));
	fill_item(new_item, copy->rule, copy->scanned, copy->start);

	return new_item;
}

static void add_tail(struct Item *tail, struct Item *new_item)
{
	new_item->next = tail->next;
	tail->next = new_item;
}

static void add_head(struct Item **head, struct Item *new_item)
{
	new_item->next = *head;
	*head = new_item;
}

static struct Item *find_item(struct Item *head, struct Item *item)
{
	struct Item *cursor;

	for(cursor = head; cursor != NULL; cursor = cursor->next) {
		if(cursor->rule == item->rule && cursor->scanned == item->scanned && cursor->start == item->start) {
			return cursor;
		}
	}

	return NULL;
}

static void predict(struct Set *set, struct Rule *grammar, struct Allocator *allocator)
{
	struct Item *cursor;
	struct Item *new_item;
	struct Item potential_item;
	char token;
	int i;

	for(cursor = set->active; cursor != NULL; cursor = cursor->next) {
		// For each item A -> x.By in set.active, add all grammar rules B -> .X (idx) to set.active
		token = cursor->rule->rhs[cursor->scanned];
		for(i = 0; grammar[i].lhs != 0; i++) {
			if(grammar[i].lhs == token) {
				fill_item(&potential_item, &grammar[i], 0, set);
				if(find_item(set->active, &potential_item) != NULL) {
					continue;
				}

				new_item = allocate_item(&potential_item, allocator->active_pool);
				add_tail(cursor, new_item);
			}
		}
	}
}

static void scan(struct Set *set, struct Set *next_set, struct Item *completed_tail, char token, struct Allocator *allocator)
{
	struct Item *cursor;
	struct Item *new_item;
	struct Item potential_item;

	for(cursor = set->active; cursor != NULL; cursor = cursor->next) {
		// Input = c
		// (1) For each item A -> x.c (n) in set.active, add A -> xc. (n) to next_set.completed
		// (2) For each item A -> x.cy (n) in set.active, add A -> xc.y (n) to next_set.active
		if(cursor->rule->rhs[cursor->scanned] == token) {
			if(cursor->scanned == strlen(cursor->rule->rhs) - 1) {
				// (1)
				fill_item(&potential_item, cursor->rule, cursor->scanned + 1, cursor->start);

				if(find_item(next_set->completed, &potential_item) != NULL) {
					continue;
				}

				new_item = allocate_item(&potential_item, allocator->completed_pool);

				if(completed_tail != NULL) {
					add_tail(completed_tail, new_item);
				} else {
					add_head(&next_set->completed, new_item);
				}
			} else {
				// (2)
				fill_item(&potential_item, cursor->rule, cursor->scanned + 1, cursor->start);

				if(find_item(next_set->active, &potential_item) != NULL) {
					continue;
				}

				new_item = allocate_item(&potential_item, allocator->active_pool);
				add_head(&next_set->active, new_item);
			}
		}
	}
}

static void complete(struct Set *set, struct Allocator *allocator)
{
	struct Item *cursor;

	for(cursor = set->completed; cursor != NULL; cursor = cursor->next) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		scan(cursor->start, set, cursor, cursor->rule->lhs, allocator);
	}
}
				
static struct Tree *create_tree(struct Set sets[], int idx, char token, int *start_idx, struct Pool *pool)
{
	struct Set *set;
	struct Item *cursor;
	struct Rule *rule;
	struct Tree *tree;
	int i;
	int current_idx;
	int len;

	set = &sets[idx];
	for(cursor = set->completed; cursor != NULL; cursor = cursor->next) {
		if(cursor->rule->lhs == token) {
			break;
		}
	}

	if(cursor == NULL) {
		if(start_idx != NULL) {
			*start_idx = idx - 1;
		}

		return NULL;
	}

	rule = cursor->rule;
	len = strlen(rule->rhs);
	current_idx = idx;
	tree = pool_alloc(pool, sizeof(struct Tree));
	tree->rule = rule;
	tree->children = pool_alloc(pool, sizeof(struct Tree*) * len);
	for(i=len - 1; i>=0; i--) {
		tree->children[i] = create_tree(sets, current_idx, rule->rhs[i], &current_idx, pool);
	}

	if(start_idx != NULL) {
		*start_idx = current_idx;
	}

	return tree;
}

struct Set *parse(const char *input, struct Rule *grammar, struct Rule *start_rule, struct Pool *active_pool, struct Pool *completed_pool)
{
	int i;
	struct Item start_item;
	struct Tree *tree;
	struct Set *sets;
	struct Allocator allocator;
	int len;

	allocator.active_pool = active_pool;
	allocator.completed_pool = completed_pool;

	len = strlen(input);
	sets = pool_alloc(completed_pool, sizeof(struct Set) * (len + 1));
	memset(sets, 0, sizeof(struct Set) * (len + 1));

	fill_item(&start_item, start_rule, 0, &sets[0]);
	sets[0].active = allocate_item(&start_item, active_pool);

	for(i=0; i<len; i++) {
		predict(&sets[i], grammar, &allocator);
		scan(&sets[i], &sets[i+1], NULL, input[i], &allocator);
		complete(&sets[i+1], &allocator);
	}

	return sets;
}

struct Tree *parse_tree(struct Set sets[], int num_sets, struct Rule *start_rule, struct Pool *pool)
{
	return create_tree(sets, num_sets - 1, start_rule->lhs, NULL, pool);
}

