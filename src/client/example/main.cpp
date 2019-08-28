#include "tclm_client.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <memory>

using namespace std;

int main (int argc, char** argv)
{
	cout << "Example for use of the tclm_client C++ library." << endl;

	shared_ptr<tclm_client::tclmc> tclmc;
		
	try {
		tclmc = tclm_client::tclmc::create("localhost");
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	cout << "Created a tclmc instance." << endl;
	cout << "Press return to continue." << endl;
	getchar ();

	shared_ptr<tclm_client::Process> p1, p2;

	try {
		p1 = tclmc->register_process ();
		p2 = tclmc->register_process ();
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	printf ("Registered processes p1 and p2 with ids %d and %d.\n",
			p1->get_id (), p2->get_id());

	cout << "Press return to continue." << endl;
	getchar();

	return EXIT_SUCCESS;
}
