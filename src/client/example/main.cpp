#include "tclm_client.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <memory>
#include <vector>

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

	/* Create a lock */
	shared_ptr<tclm_client::Lock> root1;
	bool c;

	try {
		root1 = tclmc->define_lock ("root1");
		c = root1->create (p1, true);
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	if (c)
		printf ("Created a lock with path %s.\n", root1->get_path().c_str());
	else
		printf ("Lock with path %s exists already, obtained only an X lock.\n", root1->get_path().c_str());

	cout << "Press return to continue." << endl;
	getchar();

	/* Release the lock */
	try {
		root1->release_X (p1);
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	printf ("Unlocked the lock I created above.\n");

	cout << "Press return to continue." << endl;
	getchar();

	/* Acquire it again, however this time is S mode */
	try {
		root1->acquire_S (p2);
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	printf ("Acquired the lock I created above in S mode.\n");

	cout << "Press return to continue." << endl;
	getchar();

	/* Release it again. */
	try {
		root1->release_S (p2);
	} catch (exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	printf ("Unlocked the lock I created above again.\n");

	cout << "Press return to continue." << endl;
	getchar();

	cout << "----------------------------------------------------------------------\n"
		"Demonstrating a lock hierarchy:" << endl;

	vector<shared_ptr<tclm_client::Lock>> locks;
	locks.push_back(tclmc->define_lock("a"));
	locks.push_back(tclmc->define_lock("a.b"));
	locks.push_back(tclmc->define_lock("a.b.c"));
	locks.push_back(tclmc->define_lock("a.b.c.d"));
	locks.push_back(tclmc->define_lock("a.b.c.e"));
	locks.push_back(tclmc->define_lock("a.f"));

	for (auto lk : locks)
	{
		lk->create(p1, true);
		lk->release_X(p1);

		cout << "  Created '" << lk->get_path() << "'." << endl;
	}

	cout << endl;

	for (auto lk : locks)
	{
		lk->acquire_S(p1);
		cout << "  " << lk->get_path() << " acquired S" << endl;
		getchar();

		lk->acquire_Splus(p1);
		cout << "  " << lk->get_path() << " acquired S+" << endl;
		getchar();

		lk->release_S(p1);
		cout << "  " << lk->get_path() << " released S" << endl;
		getchar();

		lk->acquire_X(p1);
		cout << "  " << lk->get_path() << " acquired X" << endl;
		getchar();

		lk->release_Splus(p1);
		cout << "  " << lk->get_path() << " released S+" << endl;
		getchar();

		lk->release_X(p1);
		cout << "  " << lk->get_path() << " released X" << endl;
		getchar();
	}

	locks[1]->acquire_S(p1);
	cout << "deadlock." << endl;
	locks[0]->acquire_X(p1);

	return EXIT_SUCCESS;
}
