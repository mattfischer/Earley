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
	std::vector<Rule> mGrammar;
	std::set<char> mNullable;
};

#endif