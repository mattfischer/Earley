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

void print_item(struct EarleyItem *item, struct EarleySet sets[])
{
	printf("%c: ", item->rule->lhs);
	print_dot(item->rule->rhs, item->scanned);
	printf(" [%i]\n", item->start - sets);
}

void print_sets(const char *input, struct EarleySet sets[])
{
	int i;
	int len;
	struct EarleyItem *cursor;

	len = strlen(input);
	for(i=0; i<len + 1; i++) {
		printf("[", i);
		print_dot(input, i);
		printf("]:\n");

		printf("  Active:\n");
		list_foreach(struct EarleyItem, list, cursor, sets[i].active) {
			printf("    ");
			print_item(cursor, sets);
		}

		printf("  Completed:\n");
		list_foreach(struct EarleyItem, list, cursor, sets[i].completed) {
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
	printf("%c: %s (0x%08x)\n", tree->rule->lhs, tree->rule->rhs, tree);
	for(i=0; i<strlen(tree->rule->rhs); i++) {
		if(tree->children[i] != NULL) {
			print_tree(tree->children[i], indent + 2);
		}
	}
}
