#ifndef EARLEY_PARSER_HPP
#define EARLEY_PARSER_HPP

#include <vector>
#include <set>

#include "Rule.hpp"
#include "Tree.hpp"

#include <ostream>

namespace Earley {

struct Item {
	int start;
	int scanned;
	const Rule *rule;

	Item(int _start, int _scanned, const Rule *_rule)
		: start(_start), scanned(_scanned), rule(_rule)
	{}

	bool operator==(const Item &other) const
	{
		return (start == other.start) && (scanned == other.scanned) && (rule == other.rule);
	}
};

std::ostream &operator<<(std::ostream &o, const Item &item);

struct Set {
	std::vector<Item> active;
	std::vector<Item> completed;
};

std::ostream &operator<<(std::ostream &o, const Set &set);

class Parser {
public:
	Parser(const std::vector<Rule> &grammar);

	std::vector<Set> parse(const std::string &input);
	std::vector<Tree*> parseTrees(const std::vector<Set> &sets, const std::string &input);

private:
	void add(Set &set, const Item &item);

	void predict(std::vector<Set> &sets, int position);
	void scan(Set &set, Set &target, char token);
	void complete(std::vector<Set> &sets, Set &set);

	std::vector<Rule> mGrammar;
	std::set<char> mNullable;
};

}

#endif