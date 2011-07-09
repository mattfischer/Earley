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

static struct EarleyItem *find_item(struct ListHead *list, struct EarleyItem *target)
{
	struct EarleyItem *item;

	list_foreach(struct EarleyItem, list, item, *list) {
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
	struct EarleyItem *item;
	struct EarleyItem new_item;
	char token;
	int i;

	// For each item A -> x.By (n) in set.active...
	list_foreach(struct EarleyItem, list, item, set->active) {
		token = item->rule->rhs[item->scanned];

		// ...add all grammar rules B -> .X (idx) to set
		for(i = 0; grammar[i].lhs != 0; i++) {
			if(grammar[i].lhs == token) {
				fill_item(&new_item, &grammar[i], 0, set);
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

	list_foreach(struct EarleyItem, list, item, set->active) {
		// For each item A -> x.cy (n) in set.active where input = c, add A -> xc.y (n) to next_set
		if(item->rule->rhs[item->scanned] == token) {
			fill_item(&new_item, item->rule, item->scanned + 1, item->start);
			add_item(next_set, &new_item);
		}
	}
}

static void complete(struct EarleySet *set)
{
	struct EarleyItem *item;

	list_foreach(struct EarleyItem, list, item, set->completed) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		scan(item->start, set, item->rule->lhs);
	}
}
				
static void create_tree(struct EarleySet sets[], const char *input, int end, char token, struct Tree *parent, struct ListHead *list)
{
	struct EarleyItem *item;
	struct ListHead tree_list;
	struct Tree *tree;
	struct Tree *new_tree;
	struct Tree *child;
	struct Tree *extra;
	struct ListHead child_list;
	struct Rule *rule;
	int i;
	int len;
	int span;
	int start;
	int size;
	int found;

	list_init(tree_list);
	// Search for an item X -> aBc. (n) in current set, where token = X
	list_foreach(struct EarleyItem, list, item, sets[end].completed) {
		if(item->rule->lhs == token) {
			start = item->start - sets;
			found = 0;
			for(tree = parent; tree != NULL; tree = tree->parent) {
				if(tree->rule == item->rule && tree->start == start && tree->end == end) {
					found = 1;
					break;
				}
			}

			if(found) {
				continue;
			}
			len = strlen(item->rule->rhs);
			tree = malloc(sizeof(struct Tree) + sizeof(struct Tree*) * (len - 1));
			tree->rule = item->rule;
			tree->start = start;
			tree->end = end;
			tree->span = 0;
			tree->num_children = 0;
			tree->parent = parent;
			list_add_tail(tree_list, tree->list);
		}
	}

	list_foreach(struct Tree, list, tree, tree_list) {
		rule = tree->rule;
		len = strlen(rule->rhs);
		while(tree->num_children < len) {
			// For rule X -> ABC. (n), recursively create each subtree, recording the 
			// number of tokens consumed by each node
			i = len - tree->num_children - 1;

			if(rule->rhs[i] == input[end - tree->span - 1]) {
				tree->children[i] = NULL;
				tree->num_children++;
				tree->span++;
				continue;
			}

			list_init(child_list);
			create_tree(sets, input, end - tree->span, rule->rhs[i], tree, &child_list);

			child = list_head(struct Tree, list, child_list);
			if(child == NULL) {
				break;
			} else {
				span = tree->span;
				tree->span = span + child->span;
				tree->children[i] = child;
				tree->num_children++;

				list_remove(child->list);

				list_foreach_ex(struct Tree, list, child, extra, child_list) {
					size = sizeof(struct Tree) + sizeof(struct Tree*) * (len - 1);
					new_tree = malloc(size);
					memcpy(new_tree, tree, size);
					new_tree->span = span + child->span;
					new_tree->children[i] = child;

					list_remove(child->list);

					list_add_tail(tree_list, new_tree->list);
				}
			}
		}
	}

	list_foreach_ex(struct Tree, list, tree, extra, tree_list) {
		list_remove(tree->list);

		if(tree->num_children == strlen(tree->rule->rhs) && tree->span == tree->end - tree->start) {
			list_add_tail(*list, tree->list);
		}
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

	predict(&sets[len], grammar, &nullable);

	return sets;
}

void earley_tree(struct EarleySet sets[], int num_sets, const char *input, struct Rule *start_rule, struct ListHead *list)
{
	create_tree(sets, input, num_sets - 1, start_rule->lhs, NULL, list);
}

