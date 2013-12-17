#include "EarleyItem.hpp"

EarleyItem &EarleyItem::operator=(const EarleyItem &other)
{
	mStart = other.mStart;
	mScanned = other.mScanned;
	mRule = other.mRule;

	return *this;
}

bool EarleyItem::operator==(const EarleyItem &other) const
{
	return (mStart == other.mStart) && (mScanned == other.mScanned) && (mRule == other.mRule);
}

std::ostream &operator<<(std::ostream &o, const EarleyItem &item)
{
	o << item.rule()->lhs() << ": ";
	for(int i=0; i<item.rule()->rhs().size(); i++) {
		if(i == item.scanned()) {
			o << " . ";
		}
		o << item.rule()->rhs()[i];
	}

	o << " [" << item.start() << "]";

	return o;
}
