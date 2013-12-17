#include <iostream>
#include <string>

#include <vector>

#include "Rule.hpp"
#include "EarleyParser.hpp"

int main(int argc, char *argv[])
{
	std::vector<Rule> grammar;
	grammar.push_back(Rule('S', "E"));
	grammar.push_back(Rule('E', "E+E"));
	grammar.push_back(Rule('E', "n"));

	EarleyParser parser(grammar);

	std::string input = "n+n+n";
	std::vector<EarleySet> sets = parser.parse(input);

	std::cout << "Earley Sets:" << std::endl;
	for(int i=0; i<sets.size(); i++) {
		std::cout << "[";
		for(int j=0; j<input.size(); j++) {
			if(j == i) {
				std::cout << " . ";
			}
			std::cout << input[j];
		}
		std::cout << "]:" << std::endl;
		std::cout << sets[i] << std::endl;
	}

	std::vector<Tree*> trees = parser.parseTrees(sets, input);
	for(int i=0; i<trees.size(); i++) {
		std::cout << *trees[i] << std::endl;
	}
	return 0;
}