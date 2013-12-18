#include "EarleySet.hpp"
#include "Rule.hpp"

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
