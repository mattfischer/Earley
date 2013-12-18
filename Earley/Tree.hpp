#ifndef TREE_HPP
#define TREE_HPP

#include <ostream>
#include <vector>

#include "Rule.hpp"

class Tree;

struct Tree {
	Tree *parent;
	std::vector<Tree*> children;
	const Rule *rule;
	int start;
	int end;
	int span;

	Tree(Tree *_parent, const std::vector<Tree*> _children, const Rule *_rule, int _start, int _end, int _span)
		: parent(_parent), children(_children), rule(_rule), start(_start), end(_end), span(_span)
	{}

};

std::ostream &operator<<(std::ostream &o, const Tree &tree);

#endif