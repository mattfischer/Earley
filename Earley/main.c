#include "earley.h"
#include "print.h"

int main(int argc, char *argv[])
{
	struct TreeNode *tree;
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

	tree = parse(input, grammar);

	printf("Parse Tree:\n");
	print_tree(tree, 2);
}