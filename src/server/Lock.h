#ifndef __LOCK_H
#define __LOCK_H

#include "Process.h"
#include <dequeue>
#include <mutex>
#include <vector>

namespace server {

class Lock
{
protected:
	std::mutex m;
	
	// For the "tree"
	vector<Lock*> parents;
	vector<Lock*> children;

	// For tracking the Lock's state
	Process *w_locker;
	vector<Process*> r_lockers;

	/* One queue for lock requests to avoid starvation. It is a dequeue because
	 * it allows random access to insert Lock Requests from the same process one
	 * after another. */
	std::dequeue<> Lock_Requests;

public:
	add_parent (Lock *parent);
	add_child (Lock *child);
};

}

#endif /* __LOCK_H */
