#include "earley.h"

#include <stdlib.h>

static int find_memo(struct Vector *memo, struct Rule *rule, int start, int end, struct Vector *vector)
{
	struct Tree *tree;
	int found;
	int i;

	found = 0;
	for(i=0; i<memo->size; i++) {
		tree = memo->data[i];
		if(tree->rule == rule && tree->start == start && tree->end == end) {
			vector_append(vector, tree);
			found = 1;
		}
	}

	return found;
}

static void create_tree(struct EarleySet sets[], const char *input, int end, char token, struct Tree *parent, struct Vector *vector, struct Vector *memo)
{
	struct Vector tree_vector;
	struct Vector child_vector;
	struct Tree *tree;
	int i;
	int len;
	int idx;
	int size;

	vector_init(&tree_vector);
	vector_init(&child_vector);
	// Search for an item X -> aBc. (n) in current set, where token = X
	for(i=0; i<sets[end].completed.size; i++) {
		struct EarleyItem *item;
		int found;
		int start;
		int j;

		item = sets[end].completed.data[i];
		if(item->rule->lhs != token) {
			continue;
		}

		// If item's start is before parent start, it can't possibly lead to a
		// valid parse tree
		start = item->start - sets;
		if(parent != NULL && start < parent->start) {
			continue;
		}

		// If the current item is identical to one that is in the parent of this node,
		// we are looping, and need to stop
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

		// Check if the parse tree has already been memoized
		if(find_memo(memo, item->rule, start, end, vector)) {
			continue;
		}

		// All early-out checks are done.  Create a new tree node and begin
		// filling it
		len = strlen(item->rule->rhs);
		tree = malloc(sizeof(struct Tree) + sizeof(struct Tree*) * (len - 1));
		tree->rule = item->rule;
		tree->start = start;
		tree->end = end;
		tree->span = 0;
		tree->num_children = 0;
		tree->parent = parent;
		vector_append(&tree_vector, tree);

		// Iterate through the list of trees.  This begins with solely the tree
		// created above, but more may be created by the duplication process below,
		// and each must be processed separately
		for(j=0; j<tree_vector.size; j++) {
			tree = (struct Tree*)tree_vector.data[j];
			len = strlen(tree->rule->rhs);
			// Populate the children of the tree in sequence, starting
			// from the end
			while(tree->num_children < len) {
				idx = len - tree->num_children - 1;

				// If the current child of the rule matches the corresponding
				// input token, then it is a terminal and no child processing
				// is necessary
				if(tree->rule->rhs[idx] == input[end - tree->span - 1]) {
					tree->children[idx] = NULL;
					tree->num_children++;
					tree->span++;
					continue;
				}

				// Recursively create all parse trees for the child token
				create_tree(sets, input, end - tree->span, tree->rule->rhs[idx], tree, &child_vector, memo);

				if(child_vector.size == 0) {
					// No valid parsings for the child token.  This rule can't
					// possibly be valid, so stop processing it
					break;
				} else {
					int span;
					struct Tree *child;
					int k;

					// Put the first parse tree in place as a child of the
					// current tree
					child = (struct Tree*)child_vector.data[0];
					span = tree->span;
					tree->span = span + child->span;
					tree->children[idx] = child;
					tree->num_children++;

					// If the child query produced more than one parse tree,
					// we must duplicate the current tree and generate new ones
					// for each additional child.  These must be added on to the
					// queue so that their other children may be processed as well
					for(k=1; k < child_vector.size; k++) {
						struct Tree *new_tree;

						child = (struct Tree*)child_vector.data[k];
						size = sizeof(struct Tree) + sizeof(struct Tree*) * (len - 1);
						new_tree = malloc(size);
						memcpy(new_tree, tree, size);
						new_tree->span = span + child->span;
						new_tree->children[idx] = child;

						vector_append(&tree_vector, new_tree);
					}

					vector_clear(&child_vector);
				}
			}

			// If the tree was completely processed, and exactly spanned the number of tokens that
			// the Earley item reported this rule to span, then accept it as a valid parse tree,
			// and add it to the output list
			if(tree->num_children == strlen(tree->rule->rhs) && tree->span == tree->end - tree->start) {
				vector_append(vector, tree);
				vector_append(memo, tree);
			}
		}

		vector_clear(&tree_vector);
	}

	vector_destroy(&tree_vector);
	vector_destroy(&child_vector);
}

void earley_tree(struct EarleySet sets[], int num_sets, const char *input, struct Rule *start_rule, struct Vector *vector)
{
	struct Vector memo;

	vector_init(&memo);
	create_tree(sets, input, num_sets - 1, start_rule->lhs, NULL, vector, &memo);
}