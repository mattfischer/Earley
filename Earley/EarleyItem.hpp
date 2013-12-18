#ifndef EARLEY_ITEM_HPP
#define EARLEY_ITEM_HPP

#include <set>

#include "Rule.hpp"
#include "Tree.hpp"

class EarleySet;

class EarleyItem {
public:
	EarleyItem(int start, int scanned, const Rule *rule)
		: mStart(start), mScanned(scanned), mRule(rule)
	{}
	EarleyItem(const EarleyItem &other)
		: mStart(other.mStart), mScanned(other.mScanned), mRule(other.mRule)
	{}
	EarleyItem &operator=(const EarleyItem &other);

	int start() const { return mStart; }
	int scanned() const { return mScanned; }
	const Rule *rule() const { return mRule; }

	bool operator==(const EarleyItem &other) const;

private:
	int mStart;
	int mScanned;
	const Rule *mRule;
};

std::ostream &operator<<(std::ostream &o, const EarleyItem &item);

#endif