#include "earley.h"
#include "print.h"

int main(int argc, char *argv[])
{
	struct Set *sets;
	struct Tree *tree;
	char *input = "n*(n+n)";
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

	sets = parse(input, grammar, &grammar[0]);
	printf("Earley Sets:\n");
	print_sets(input, grammar, sets);

	tree = parse_tree(sets, strlen(input) + 1, &grammar[0]);
	printf("Parse Tree:\n");
	print_tree(tree, 2);
}