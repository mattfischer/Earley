#include "earley.h"
#include "print.h"

int main(int argc, char *argv[])
{
	struct EarleySet *sets;
	struct Tree *tree;
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
	char *input = "x";
	struct Rule grammar[] = {
		{ 'S', "AAx" },
		{ 'A', "" },
		{ 0, 0 }
	};
	struct Rule *start_rule = &grammar[0];

	sets = earley_parse(input, grammar, start_rule);
	printf("Earley Sets:\n");
	print_sets(input, sets);

	tree = earley_tree(sets, strlen(input) + 1, start_rule);
	printf("Parse Tree:\n");
	print_tree(tree, 2);
}