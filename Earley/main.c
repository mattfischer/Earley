#include "earley.h"
#include "print.h"

int main(int argc, char *argv[])
{
	struct EarleySet *sets;
	struct Tree *tree;
	struct Vector tree_vector;
	int i;

	/*char *input = "n*(n+n)";
	struct Rule grammar[] = {
		{ 'S', "E" },
		{ 'E', "T" },
		{ 'E', "E+T" },
		{ 'T', "F" },
		{ 'T', "T*F" },
		{ 'F', "n" },
		{ 'F', "(E)" },
		{ 0, 0 }
	};*/
	char *input = "n+n+n+n";
	struct Rule grammar[] = {
		{ 'S', "E" },
		{ 'E', "E+E" },
		{ 'E', "n" },
		{ 0, 0 }
	};
	/*
	char *input = "aa";
	struct Rule grammar[] = {
		{ 'S', "AAA" },
		{ 'A', "a" },
		{ 'A', "" },
		{ 0, 0 }
	};*/
	struct Rule *start_rule = &grammar[0];

	sets = earley_parse(input, grammar, start_rule);
	printf("Earley Sets:\n");
	print_sets(input, sets);

	vector_init(&tree_vector);
	earley_tree(sets, strlen(input) + 1, input, start_rule, &tree_vector);

	for(i=0; i<tree_vector.size; i++) {
		tree = (struct Tree*)tree_vector.data[i];
		printf("Parse Tree:\n");
		print_tree(tree, 2);
		printf("\n");
	}
}