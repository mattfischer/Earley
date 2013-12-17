#include "Tree.hpp"

static void printTree(std::ostream &o, const Tree *tree, int indent)
{
	for(int i=0; i<indent; i++) {
		o << " ";
	}

	o << tree->rule()->lhs() << ": " << tree->rule()->rhs() << std::endl;
	for(int i=0; i<tree->children().size(); i++) {
		if(tree->children()[i]) {
			printTree(o, tree->children()[i], indent + 2);
		}
	}
}

std::ostream &operator<<(std::ostream &o, const Tree &tree)
{
	printTree(o, &tree, 0);
	return o;
}
