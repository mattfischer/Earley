#ifndef RULE_HPP
#define RULE_HPP

#include <string>

class Rule {
public:
	Rule(const char &lhs, const std::string &rhs)
		: mLhs(lhs), mRhs(rhs)
	{}
	Rule(const Rule &other)
		: mLhs(other.mLhs), mRhs(other.mRhs)
	{}

	Rule &operator=(const Rule &other)
	{
		mLhs = other.mLhs;
		mRhs = other.mRhs;
		return *this;
	}

	const char &lhs() const { return mLhs; }
	const std::string &rhs() const { return mRhs; }

private:
	char mLhs;
	std::string mRhs;
};

#endif