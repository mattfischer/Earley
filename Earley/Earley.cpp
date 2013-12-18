#include "Earley.hpp"

#include "Tree.hpp"

#include <algorithm>

namespace Earley {

void Parser::add(std::vector<Set> &sets, int position, const Item &item)
{
	Set &set = sets[position];

	if(item.scanned == item.rule->rhs.length()) {
		// If item is like X -> aBc. (n), add to set.completed
		if(std::find(set.completed.begin(), set.completed.end(), item) == set.completed.end()) {
			set.completed.push_back(item);
		}
	} else {
		// If item is like X -> aB.c (n), add to set.active
		if(std::find(set.active.begin(), set.active.end(), item) == set.active.end()) {
			set.active.push_back(item);
		}
	}
}

void Parser::predict(std::vector<Set> &sets, int position)
{
	Set &set = sets[position];

	// For each item A -> x.By (n) in set.active...
	for(int i=0; i<set.active.size(); i++) {
		const Item &item = set.active[i];
		char token = item.rule->rhs[item.scanned];

		// ...add all grammar rules B -> .X (idx) to set
		for(int j=0; j<mGrammar.size(); j++) {
			if(mGrammar[j].lhs == token) {
				add(sets, position, Item(position, 0, &mGrammar[j]));
			}
		}

		// ...if B is nullable, add A -> xB.y (n) to set
		if(mNullable.find(token) != mNullable.end()) {
			add(sets, position, Item(item.start, item.scanned + 1, item.rule));
		}
	}	
}

void Parser::scan(std::vector<Set> &sets, int position, int target, char token)
{
	Set &set = sets[position];

	// For each item A -> x.cy (n) in set.active where input = c, add A -> xc.y (n) to next_set
	for(int i=0; i<set.active.size(); i++) {
		const Item &item = set.active[i];
		if(item.rule->rhs[item.scanned] == token) {
			add(sets, target, Item(item.start, item.scanned + 1, item.rule));
		}
	}
}

void Parser::complete(std::vector<Set> &sets, int position)
{
	Set &set = sets[position];

	// For each item X -> y. (n) in set.completed, scan sets[n].active with
	// token X, placing the results into set
	for(int i=0; i<set.completed.size(); i++) {
		const Item &item = set.completed[i];
		scan(sets, item.start, position, item.rule->lhs);
	}
}

Parser::Parser(const std::vector<Rule> &grammar)
: mGrammar(grammar)
{
	bool changed;

	do {
		changed = false;
		for(int i=0; i<mGrammar.size(); i++) {
			const Rule &rule = mGrammar[i];
			bool isNullable = true;

			// A is nullable iff there is a rule producing A for which
			// all (zero or more) rhs tokens are nullable
			for(int j=0; j<rule.rhs.length(); j++) {
				if(mNullable.find(rule.rhs[j]) == mNullable.end()) {
					isNullable = false;
					break;
				}
			}

			if(isNullable && mNullable.find(rule.lhs) == mNullable.end()) {
				mNullable.insert(rule.lhs);
				changed = true;

			}
		}
	} while(changed);
}

std::vector<Set> Parser::parse(const std::string &input)
{
	std::vector<Set> sets(input.size() + 1);

	add(sets, 0, Item(0, 0, &mGrammar[0]));
	for(int i=0; i<input.size(); i++) {
		predict(sets, i);
		scan(sets, i, i+1, input[i]);
		complete(sets, i+1);
	}

	predict(sets, sets.size() - 1);

	return sets;
}

static std::vector<Tree*> createTrees(const std::vector<Set> &sets, const std::string &input, int end, char token, Tree *parent, std::vector<Tree*> &memo)
{
	std::vector<Tree*> ret;

	// Search for an item X -> aBc. (n) in current set, where token = X
	for(int i=0; i<sets[end].completed.size(); i++) {
		std::vector<Tree*> trees;
		const Item &item = sets[end].completed[i];
		if(item.rule->lhs != token) {
			continue;
		}

		// If item's start is before parent start, it can't possibly lead to a
		// valid parse tree
		int start = item.start;
		if(parent != 0 && start < parent->start) {
			continue;
		}
		
		// If the current item is identical to one that is in the parent of this node,
		// we are looping, and need to stop
		bool found = false;
		for(Tree *tree = parent; tree != 0; tree = tree->parent) {
			if(tree->rule == item.rule && tree->start == start && tree->end == end) {
				found = true;
				break;
			}
		}

		if(found) {
			continue;
		}

		// Check if the parse tree has already been memoized
		found = false;
		for(int j=0; j<memo.size(); j++) {
			Tree *tree = memo[j];
			if(tree->rule == item.rule && tree->start == start && tree->end == end) {
				ret.push_back(tree);
				found = true;
			}
		}

		if(found) {
			continue;
		}

		// All early-out checks are done.  Create a new tree node and begin
		// filling it
		Tree *tree = new Tree(parent, std::vector<Tree*>(), item.rule, start, end, 0);
		trees.push_back(tree);

		// Iterate through the list of trees.  This begins with solely the tree
		// created above, but more may be created by the duplication process below,
		// and each must be processed separately
		for(int j=0; j<trees.size(); j++) {
			tree = trees[j];
			int len = tree->rule->rhs.size();
			// Populate the children of the tree in sequence, starting
			// from the end
			while(tree->children.size() < len) {
				int idx = len - tree->children.size() - 1;

				// If the current child of the rule matches the corresponding
				// input token, then it is a terminal and no child processing
				// is necessary
				if(tree->rule->rhs[idx] == input[end - tree->span - 1]) {
					tree->children.push_back(0);
					tree->span++;
					continue;
				}

				// Recursively create all parse trees for the child token
				std::vector<Tree*> children = createTrees(sets, input, end - tree->span, tree->rule->rhs[idx], tree, memo);

				if(children.size() == 0) {
					// No valid parsings for the child token.  This rule can't
					// possibly be valid, so stop processing it
					break;
				} else {
					// Put the first parse tree in place as a child of the
					// current tree
					Tree *child = children[0];
					int span = tree->span;
					tree->span += child->span;
					tree->children.push_back(child);

					// If the child query produced more than one parse tree,
					// we must duplicate the current tree and generate new ones
					// for each additional child.  These must be added on to the
					// queue so that their other children may be processed as well
					for(int k=1; k < children.size(); k++) {
						child = children[k];
						Tree *newTree = new Tree(tree->parent, tree->children, tree->rule, tree->start, tree->end, span + child->span);
						newTree->children[newTree->children.size() - 1] = child;
						trees.push_back(newTree);
					}
				}
			}

			// If the tree was completely processed, and exactly spanned the number of tokens that
			// the Earley item reported this rule to span, then accept it as a valid parse tree,
			// and add it to the output list
			if(tree->children.size() == len && tree->span == tree->end - tree->start) {
				ret.push_back(tree);
				memo.push_back(tree);
			}
		}
	}

	return ret;
}

std::vector<Tree*> Parser::parseTrees(const std::vector<Set> &sets, const std::string &input)
{
	std::vector<Tree*> memo;
	return createTrees(sets, input, sets.size() - 1, mGrammar[0].lhs, 0, memo);
}

std::ostream &operator<<(std::ostream &o, const Item &item)
{
	o << item.rule->lhs << ": ";
	for(int i=0; i<item.rule->rhs.size(); i++) {
		if(i == item.scanned) {
			o << " . ";
		}
		o << item.rule->rhs[i];
	}

	o << " [" << item.start << "]";
	return o;
}

std::ostream &operator<<(std::ostream &o, const Set &set)
{
	o << "  Active:" << std::endl;
	for(int i=0; i<set.active.size(); i++) {
		o << "    " << set.active[i] << std::endl;
	}
	o << "  Completed:" << std::endl;
	for(int i=0; i<set.completed.size(); i++) {
		o << "    " << set.completed[i] << std::endl;
	}

	return o;
}

}