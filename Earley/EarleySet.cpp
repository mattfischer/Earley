#include "EarleySet.hpp"
#include "Rule.hpp"

#include <algorithm>

void EarleySet::add(const EarleyItem &item)
{
	if(item.scanned() == item.rule()->rhs().length()) {
		// If item is like X -> aBc. (n), add to set.completed
		if(std::find(mCompleted.begin(), mCompleted.end(), item) == mCompleted.end()) {
			mCompleted.push_back(item);
		}
	} else {
		// If item is like X -> aB.c (n), add to set.active
		if(std::find(mActive.begin(), mActive.end(), item) == mActive.end()) {
			mActive.push_back(item);
		}
	}
}

void EarleySet::predict(const std::vector<Rule> &grammar, int position, const std::set<char> &nullable)
{
	// For each item A -> x.By (n) in set.active...
	for(int i=0; i<mActive.size(); i++) {
		const EarleyItem &item = mActive[i];
		char token = item.rule()->rhs()[item.scanned()];

		// ...add all grammar rules B -> .X (idx) to set
		for(int j=0; j<grammar.size(); j++) {
			if(grammar[j].lhs() == token) {
				add(EarleyItem(position, 0, &grammar[j]));
			}
		}

		// ...if B is nullable, add A -> xB.y (n) to set
		if(nullable.find(token) != nullable.end()) {
			add(EarleyItem(item.start(), item.scanned() + 1, item.rule()));
		}
	}	
}

void EarleySet::scan(EarleySet *target, char token) const
{
	// For each item A -> x.cy (n) in set.active where input = c, add A -> xc.y (n) to next_set
	for(int i=0; i<mActive.size(); i++) {
		const EarleyItem &item = mActive[i];
		if(item.rule()->rhs()[item.scanned()] == token) {
			target->add(EarleyItem(item.start(), item.scanned() + 1, item.rule()));
		}
	}
}

void EarleySet::complete(const std::vector<EarleySet> &sets)
{
	// For each item X -> y. (n) in set.completed, scan sets[n].active with
	// token X, placing the results into set
	for(int i=0; i<mCompleted.size(); i++) {
		const EarleyItem &item = mCompleted[i];
		sets[item.start()].scan(this, item.rule()->lhs());
	}
}

std::ostream &operator<<(std::ostream &o, const EarleySet &set)
{
	o << "  Active:" << std::endl;
	for(int i=0; i<set.active().size(); i++) {
		o << "    " << set.active()[i] << std::endl;
	}
	o << "  Completed:" << std::endl;
	for(int i=0; i<set.completed().size(); i++) {
		o << "    " << set.completed()[i] << std::endl;
	}

	return o;
}
