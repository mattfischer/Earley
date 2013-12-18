#ifndef RULE_HPP
#define RULE_HPP

#include <string>

struct Rule {
	char lhs;
	std::string rhs;

	Rule(const char &_lhs, const std::string &_rhs)
		: lhs(_lhs), rhs(_rhs)
	{}
};

#endif