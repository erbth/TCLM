#include "tclm_config.h"
#include "daemon.h"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace std;
using namespace server;

/* The deamon; defined globally to be reached by the signal handler */
shared_ptr<daemon> d;

/* Signal handler */
void signal_handler (int signum)
{
	printf ("Exiting gracefully due to %s.\n", signum == SIGINT ? "SIGINT" : "SIGTERM");
	if (d)
		d->cm.request_quit();
	else
		exit (EXIT_SUCCESS);
}

int main (int argc, char** argv)
{
	printf ("TCLM Server version %d.%d\n", SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);

	/* Create a daemon */
	d = make_shared<daemon>();

	/* Register a signal handler */
	struct sigaction sa = { 0 };
	sa.sa_handler = signal_handler;

	if (sigaction (SIGTERM, &sa, NULL) < 0)
	{
		perror ("sigaction (SIGTERM) failed");
		return EXIT_FAILURE;
	}

	if (sigaction (SIGINT, &sa, NULL) < 0)
	{
		perror ("sigaction (SIGINT) failed");
		return EXIT_FAILURE;
	}

	/* Run main loop */
	auto ret = d->run();

	if (ret)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
