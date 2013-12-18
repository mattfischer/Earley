#ifndef EARLEY_PARSER_HPP
#define EARLEY_PARSER_HPP

#include <vector>
#include <set>

#include "Rule.hpp"
#include "EarleySet.hpp"
#include "Tree.hpp"

class EarleyParser {
public:
	EarleyParser(const std::vector<Rule> &grammar);

	std::vector<EarleySet> parse(const std::string &input);
	std::vector<Tree*> parseTrees(const std::vector<EarleySet> &sets, const std::string &input);

private:
	void add(EarleySet &set, const EarleyItem &item);

	void predict(std::vector<EarleySet> &sets, int position);
	void scan(EarleySet &set, EarleySet &target, char token);
	void complete(std::vector<EarleySet> &sets, EarleySet &set);

	std::vector<Rule> mGrammar;
	std::set<char> mNullable;
};

#endif