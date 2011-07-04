#include <stdlib.h>

#include "earley.h"
#include "pool.h"
#include "print.h"

#define KEEP_SIZE 1 * 1024
char keep_memory[KEEP_SIZE];
struct Pool keep_pool = { keep_memory, 0, KEEP_SIZE };

#define TEMP_SIZE 5 * 1024
char temp_memory[KEEP_SIZE];
struct Pool temp_pool = { temp_memory, 0, TEMP_SIZE };

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

	new_item = allocate(sizeof(struct Item), pool);
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

static void predict(struct Set *set, struct Rule *grammar)
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

				new_item = allocate_item(&potential_item, &temp_pool);
				add_tail(cursor, new_item);
			}
		}
	}
}

static void scan(struct Set *set, struct Set *next_set, struct Item *completed_tail, struct Rule *grammar, char token)
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

				new_item = allocate_item(&potential_item, &keep_pool);

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

				new_item = allocate_item(&potential_item, &temp_pool);
				add_head(&next_set->active, new_item);
			}
		}
	}
}

static void complete(struct Set *set, struct Rule *grammar)
{
	struct Item *cursor;

	for(cursor = set->completed; cursor != NULL; cursor = cursor->next) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		scan(cursor->start, set, cursor, grammar, cursor->rule->lhs);
	}
}
				
static struct Tree *create_tree(struct Set sets[], int idx, char token, int *start_idx)
{
	struct Set *set;
	struct Item *cursor;
	struct Rule *rule;
	struct Tree *tree;
	int i;
	int current_idx;

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
	current_idx = idx;
	tree = allocate(sizeof(struct Tree), &keep_pool);
	tree->rule = rule;
	tree->children = allocate(sizeof(struct Three*) * strlen(rule->rhs), &keep_pool);
	for(i=strlen(rule->rhs) - 1; i>=0; i--) {
		tree->children[i] = create_tree(sets, current_idx, rule->rhs[i], &current_idx);
	}

	if(start_idx != NULL) {
		*start_idx = current_idx;
	}

	return tree;
}

struct Tree *parse(const char *input, struct Rule *grammar)
{
	int i;
	struct Item start_item;
	struct Tree *tree;
	struct Set *sets;
	
	sets = allocate(sizeof(struct Set) * (strlen(input) + 1), &keep_pool);
	memset(sets, 0, sizeof(struct Set) * (strlen(input) + 1));

	fill_item(&start_item, &grammar[0], 0, &sets[0]);
	sets[0].active = allocate_item(&start_item, &temp_pool);

	for(i=0; i<strlen(input); i++) {
		predict(&sets[i], grammar);
		scan(&sets[i], &sets[i+1], NULL, grammar, input[i]);
		complete(&sets[i+1], grammar);
	}

	free_pool(&temp_pool);

	tree = create_tree(sets, strlen(input), grammar[0].lhs, NULL);

	print_sets(input, grammar, sets);

	return tree;
}