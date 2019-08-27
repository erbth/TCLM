#include "tclm_client.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace std;

int main (int argc, char** argv)
{
	cout << "Example for use of the tclm_client C++ library." << endl;

	shared_ptr<tclm_client::tclmc> tclmc;
		
	try {
		tclmc = shared_ptr<tclm_client::tclmc>(tclm_client::tclmc::create("localhost"));
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	cout << "Created a tclmc instance." << endl;
	cout << "Press return to continue." << endl;
	getchar ();

	uint32_t pid;

	try {
		pid = tclmc->register_process ();
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	cout << "Registered a process: id = " << to_string(pid) << "." << endl;
	cout << "Press return to continue." << endl;
	getchar();

	return EXIT_SUCCESS;
}
