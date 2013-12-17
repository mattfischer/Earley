#ifndef TREE_HPP
#define TREE_HPP

#include <ostream>
#include <vector>

#include "Rule.hpp"

class Tree;

class Tree {
public:
	Tree(Tree *parent, const std::vector<Tree*> children, const Rule *rule, int start, int end, int span)
		: mParent(parent), mChildren(children), mRule(rule), mStart(start), mEnd(end), mSpan(span)
	{}

	Tree *parent() const { return mParent; }
	const std::vector<Tree*> &children() const { return mChildren; }
	std::vector<Tree*> &children() { return mChildren; }

	const Rule *rule() const { return mRule; }
	int start() const { return mStart; }
	int end() const { return mEnd; }
	int &span() { return mSpan; }

private:
	Tree *mParent;
	std::vector<Tree*> mChildren;
	const Rule *mRule;
	int mStart;
	int mEnd;
	int mSpan;
};

std::ostream &operator<<(std::ostream &o, const Tree &tree);

#endif