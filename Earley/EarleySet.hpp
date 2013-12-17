#ifndef EARLEY_SET_HPP
#define EARLEY_SET_HPP

#include <ostream>
#include <vector>

#include "EarleyItem.hpp"
#include "Rule.hpp"

class EarleySet {
public:
	void add(const EarleyItem &item);

	void predict(const std::vector<Rule> &grammar, int position, const std::set<char> &nullable);
	void scan(EarleySet *target, char token) const;
	void complete(const std::vector<EarleySet> &sets);

	const std::vector<EarleyItem> &active() const { return mActive; }
	const std::vector<EarleyItem> &completed() const { return mCompleted; }

private:
	std::vector<EarleyItem> mActive;
	std::vector<EarleyItem> mCompleted;
};

std::ostream &operator<<(std::ostream &o, const EarleySet &set);

#endif