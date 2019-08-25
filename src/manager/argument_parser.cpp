#include "argument_parser.h"
#include <cstring>
#include <iostream>

using namespace std;
using namespace manager;

bool argument_parser::parse (int argc, char** argv)
{
	program_name = argv[0];

	for (int i = 1; i < argc; i++)
	{
		/* Interpret the argument */
		auto len = strlen (argv[i]);

		if (len == 0)
		{
			continue;
		}
		else if (len >= 1 && argv[i][0] == '-')
		{
			if (len >= 2 && argv[i][1] == '-')
			{
				/* Long parameter */
				if (len < 4)
				{
					cerr << "Parameter `" << argv[i]
						<< "' too short for a long parameter." << endl;

					return false;
				}

				if (argv[i][2] == '-')
				{
					cerr << "Regarding parameter `" << argv[i]
						<< "': Long parameters must not start with a `-'." << endl;

					return false;
				}

				long_parameters.push_back (string (argv[i] + 2));
			}
			else
			{
				/* Short parameter */
				if (len < 2)
				{
					cerr << "There is a `-' alone ..." << endl;
					return false;
				}

				if (len > 2)
				{
					cerr << "Parameter `" << argv[i]
						<< "' too long for a short parameter."<< endl;
					return false;
				}

				short_parameters.push_back (argv[i][1]);
			}
		}
		else
		{
			if (action.size() == 0)
			{
				/* Action */
				action = argv[i];
			}
			else
			{
				/* Name */
				names.push_back (argv[i]);
			}
		}
	}

	return true;
}
