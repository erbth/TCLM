#include "tclm_config.h"
#include "daemon.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace server;

int main (int argc, char** argv)
{
	printf ("TCLM Server version %d.%d\n", SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);

	daemon d;
	auto ret = d.run();

	if (ret)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
