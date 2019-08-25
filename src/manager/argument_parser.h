#ifndef __ARGUMENT_PARSER_H
#define __ARGUMENT_PARSER_H

#include <string>
#include <vector>

namespace manager {

class argument_parser
{
	/* A commandline may look the following way:
	 * <program_name> ... --<long parameter> ... -<short (one letter) parameter>\
	 *     ... <action> ... <more parameters> ... <names> ... */

public:
	std::string program_name;
	std::string action;
	std::vector<std::string> long_parameters;
	std::vector<char> short_parameters;
	std::vector<std::string> names;

	/* Returns false if a syntactic error is encountered */
	bool parse (int argc, char** argv);
};

}

#endif /* __ARGUMENT_PARSER_H */
