#include "EarleyParser.hpp"

#include "Tree.hpp"

EarleyParser::EarleyParser(const std::vector<Rule> &grammar)
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
			for(int j=0; j<rule.rhs().length(); j++) {
				if(mNullable.find(rule.rhs()[j]) == mNullable.end()) {
					isNullable = false;
					break;
				}
			}

			if(isNullable && mNullable.find(rule.lhs()) == mNullable.end()) {
				mNullable.insert(rule.lhs());
				changed = true;
			}
		}
	} while(changed);
}

std::vector<EarleySet> EarleyParser::parse(const std::string &input)
{
	std::vector<EarleySet> sets(input.size() + 1);

	sets[0].add(EarleyItem(0, 0, &mGrammar[0]));
	for(int i=0; i<input.size(); i++) {
		sets[i].predict(mGrammar, i, mNullable);
		sets[i].scan(&sets[i+1], input[i]);
		sets[i+1].complete(sets);
	}

	sets[sets.size() - 1].predict(mGrammar, sets.size() - 1, mNullable);

	return sets;
}

static bool findMemo(std::vector<Tree*> memo, const Rule *rule, int start, int end, std::vector<Tree*> &vector)
{
	bool found = false;
	for(int i=0; i<memo.size(); i++) {
		Tree *tree = memo[i];
		if(tree->rule() == rule && tree->start() == start && tree->end() == end) {
			vector.push_back(tree);
			found = true;
		}
	}

	return found;
}

static void createTree(const std::vector<EarleySet> &sets, const std::string &input, int end, char token, Tree *parent, std::vector<Tree*> &vector, std::vector<Tree*> &memo)
{
	std::vector<Tree*> trees;
	std::vector<Tree*> children;

	// Search for an item X -> aBc. (n) in current set, where token = X
	for(int i=0; i<sets[end].completed().size(); i++) {
		const EarleyItem &item = sets[end].completed()[i];
		if(item.rule()->lhs() != token) {
			continue;
		}

		// If item's start is before parent start, it can't possibly lead to a
		// valid parse tree
		int start = item.start();
		if(parent != 0 && start < parent->start()) {
			continue;
		}
		
		// If the current item is identical to one that is in the parent of this node,
		// we are looping, and need to stop
		bool found = false;
		for(Tree *tree = parent; tree != 0; tree = tree->parent()) {
			if(tree->rule() == item.rule() && tree->start() == start && tree->end() == end) {
				found = true;
				break;
			}
		}

		if(found) {
			continue;
		}

		// Check if the parse tree has already been memoized
		if(findMemo(memo, item.rule(), start, end, vector)) {
			continue;
		}

		// All early-out checks are done.  Create a new tree node and begin
		// filling it
		Tree *tree = new Tree(parent, std::vector<Tree*>(), item.rule(), start, end, 0);
		trees.push_back(tree);

		// Iterate through the list of trees.  This begins with solely the tree
		// created above, but more may be created by the duplication process below,
		// and each must be processed separately
		for(int j=0; j<trees.size(); j++) {
			tree = trees[j];
			int len = tree->rule()->rhs().size();
			// Populate the children of the tree in sequence, starting
			// from the end
			while(tree->children().size() < len) {
				int idx = len - tree->children().size() - 1;

				// If the current child of the rule matches the corresponding
				// input token, then it is a terminal and no child processing
				// is necessary
				if(tree->rule()->rhs()[idx] == input[end - tree->span() - 1]) {
					tree->children().push_back(0);
					tree->span()++;
					continue;
				}

				// Recursively create all parse trees for the child token
				createTree(sets, input, end - tree->span(), tree->rule()->rhs()[idx], tree, children, memo);

				if(children.size() == 0) {
					// No valid parsings for the child token.  This rule can't
					// possibly be valid, so stop processing it
					break;
				} else {
					// Put the first parse tree in place as a child of the
					// current tree
					Tree *child = children[0];
					int span = tree->span();
					tree->span() += child->span();
					tree->children().push_back(child);

					// If the child query produced more than one parse tree,
					// we must duplicate the current tree and generate new ones
					// for each additional child.  These must be added on to the
					// queue so that their other children may be processed as well
					for(int k=1; k < children.size(); k++) {
						child = children[k];
						Tree *newTree = new Tree(tree->parent(), tree->children(), tree->rule(), tree->start(), tree->end(), span + child->span());
						newTree->children()[newTree->children().size() - 1] = child;
						trees.push_back(newTree);
					}

					children.clear();
				}
			}

			// If the tree was completely processed, and exactly spanned the number of tokens that
			// the Earley item reported this rule to span, then accept it as a valid parse tree,
			// and add it to the output list
			if(tree->children().size() == len && tree->span() == tree->end() - tree->start()) {
				vector.push_back(tree);
				memo.push_back(tree);
			}
		}

		trees.clear();
	}
}

std::vector<Tree*> EarleyParser::parseTrees(const std::vector<EarleySet> &sets, const std::string &input)
{
	std::vector<Tree*> memo;
	std::vector<Tree*> trees;
	createTree(sets, input, sets.size() - 1, mGrammar[0].lhs(), 0, trees, memo);

	return trees;
}
