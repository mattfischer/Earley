#include <stdlib.h>

#include "print.h"

static void print_dot(const char *str, int pos)
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

void print_item(struct Item *item, struct Set sets[])
{
	printf("%c: ", item->rule->lhs);
	print_dot(item->rule->rhs, item->scanned);
	printf(" [%i]\n", item->start - sets);
}

void print_sets(const char *input, struct Rule *grammar, struct Set sets[])
{
	int i;
	struct Item *cursor;

	for(i=0; i<strlen(input) + 1; i++) {
		printf("[", i);
		print_dot(input, i);
		printf("]:\n");

		printf("  Active:\n");
		for(cursor = sets[i].active; cursor != NULL; cursor = cursor->next) {
			printf("    ");
			print_item(cursor, sets);
		}

		printf("  Completed:\n");
		for(cursor = sets[i].completed; cursor != NULL; cursor = cursor->next) {
			printf("    ");
			print_item(cursor, sets);
		}

		printf("\n");
	}
}


static void print_indent(int indent)
{
	int i;

	for(i=0; i<indent; i++) {
		printf(" ");
	}
}

void print_tree(struct Tree *tree, int indent)
{
	int i;

	print_indent(indent);
	printf("%c: %s\n", tree->rule->lhs, tree->rule->rhs);
	for(i=0; i<strlen(tree->rule->rhs); i++) {
		if(tree->children[i] != NULL) {
			print_tree(tree->children[i], indent + 2);
		}
	}
}
