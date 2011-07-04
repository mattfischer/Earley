#include "earley.h"
#include "print.h"

#define ACTIVE_SIZE 5 * 1024
#define COMPLETED_SIZE 1 * 1024
#define TREE_SIZE 1 * 1024

int main(int argc, char *argv[])
{
	struct Set *sets;
	struct Tree *tree;
	struct Pool active_pool, completed_pool, tree_pool;
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
	struct Rule *start_rule = &grammar[0];

	pool_create(&active_pool, ACTIVE_SIZE);
	pool_create(&completed_pool, COMPLETED_SIZE);

	sets = parse(input, grammar, start_rule, &active_pool, &completed_pool);
	printf("Earley Sets:\n");
	print_sets(input, grammar, sets);

	pool_free(&active_pool);
	pool_create(&tree_pool, TREE_SIZE);

	tree = parse_tree(sets, strlen(input) + 1, start_rule, &tree_pool);
	printf("Parse Tree:\n");
	print_tree(tree, 2);
}