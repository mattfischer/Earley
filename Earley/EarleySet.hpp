#ifndef EARLEY_SET_HPP
#define EARLEY_SET_HPP

#include <ostream>
#include <vector>

#include "EarleyItem.hpp"
#include "Rule.hpp"

class EarleySet {
public:
	std::vector<EarleyItem> &active() { return mActive; }
	std::vector<EarleyItem> &completed() { return mCompleted; }

	const std::vector<EarleyItem> &active() const { return mActive; }
	const std::vector<EarleyItem> &completed() const { return mCompleted; }

private:
	std::vector<EarleyItem> mActive;
	std::vector<EarleyItem> mCompleted;
};

std::ostream &operator<<(std::ostream &o, const EarleySet &set);

#endif