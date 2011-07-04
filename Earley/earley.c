#include <stdlib.h>

struct Rule {
	char lhs;
	char *rhs;
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

struct Pool {
	char *memory;
	int start;
	int size;
};

#define KEEP_SIZE 1 * 1024
char keep_memory[KEEP_SIZE];
struct Pool keep_pool = { keep_memory, 0, KEEP_SIZE };

#define TEMP_SIZE 5 * 1024
char temp_memory[KEEP_SIZE];
struct Pool temp_pool = { temp_memory, 0, TEMP_SIZE };

void *allocate(int size, struct Pool *pool)
{
	void *ret = pool->memory + pool->start;
	pool->start += size;
	
	if(pool->start >= pool->size) {
		printf("Error: Memory overflow\n");
	}

	return ret;
}

void fill_item(struct Item *item, struct Rule *rule, int scanned, struct Set *start)
{
	item->rule = rule;
	item->scanned = scanned;
	item->start = start;
	item->next = NULL;
}

struct Item *allocate_item(struct Item *copy, struct Pool *pool)
{
	struct Item *new_item;

	new_item = allocate(sizeof(struct Item), pool);
	fill_item(new_item, copy->rule, copy->scanned, copy->start);

	return new_item;
}

void add_tail(struct Item *tail, struct Item *new_item)
{
	new_item->next = tail->next;
	tail->next = new_item;
}

void add_head(struct Item **head, struct Item *new_item)
{
	new_item->next = *head;
	*head = new_item;
}

struct Item *find_item(struct Item *head, struct Item *item)
{
	struct Item *cursor;

	for(cursor = head; cursor != NULL; cursor = cursor->next) {
		if(cursor->rule == item->rule && cursor->scanned == item->scanned && cursor->start == item->start) {
			return cursor;
		}
	}

	return NULL;
}

void predict(struct Set *set, struct Rule *grammar)
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

void scan(struct Set *set, struct Set *next_set, struct Item *completed_tail, struct Rule *grammar, char token)
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

void complete(struct Set *set, struct Rule *grammar)
{
	struct Item *cursor;

	for(cursor = set->completed; cursor != NULL; cursor = cursor->next) {
		// For each item X -> y. (n) in set.completed, scan sets[n].active with
		// token X, placing the results into set
		scan(cursor->start, set, cursor, grammar, cursor->rule->lhs);
	}
}
				
void parse(const char *input, struct Rule *grammar, struct Set *sets)
{
	int i;
	struct Item start_item;

	fill_item(&start_item, &grammar[0], 0, &sets[0]);
	sets[0].active = allocate_item(&start_item, &temp_pool);

	for(i=0; i<strlen(input); i++) {
		predict(&sets[i], grammar);
		scan(&sets[i], &sets[i+1], NULL, grammar, input[i]);
		complete(&sets[i+1], grammar);
	}
}

void printdot(const char *str, int pos)
{
	int i;

	for(i=0; i<pos; i++) {
		printf("%c", str[i]);
	}

	printf(" . ");

	for(i=pos; i<strlen(str); i++) {
		printf("%c", str[i]);
	}
}

void print(const char *input, struct Rule *grammar, struct Set sets[] )
{
	int i;
	struct Item *cursor;

	for(i=0; i<strlen(input) + 1; i++) {
		printf("Set %i [", i);
		printdot(input, i);
		printf("]:\n");

		printf("Active:\n");
		for(cursor = sets[i].active; cursor != NULL; cursor = cursor->next) {
			int j;

			printf("  %c -> ", cursor->rule->lhs);
			printdot(cursor->rule->rhs, cursor->scanned);
			printf(" (%i)\n", cursor->start - sets);
		}
		printf("Completed:\n");
		for(cursor = sets[i].completed; cursor != NULL; cursor = cursor->next) {
			int j;

			printf("  %c -> ", cursor->rule->lhs);
			printdot(cursor->rule->rhs, cursor->scanned);
			printf(" (%i)\n", cursor->start - sets);
		}

		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	char *input = "n+n+n";
	struct Rule grammar[] = {
		{ 'S', "E" },
		{ 'E', "T" },
		{ 'E', "E+T" },
		{ 'T', "F" },
		{ 'T', "T*F" },
		{ 'F', "n" },
		{ 'F', "(E)" },
		{ 0, 0 }
	};

	struct Set *sets = allocate(sizeof(struct Set) * (strlen(input) + 1), &keep_pool);
	memset(sets, 0, sizeof(struct Set) * (strlen(input) + 1));

	parse(input, grammar, sets);
	print(input, grammar, sets);
}