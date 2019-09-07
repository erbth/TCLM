#include "Lock_Forest.h"
#include "Lock_Request.h"
#include "Process_map.h"
#include "backend_exceptions.h"

using namespace server;
using namespace std;

Process_map::Process_map(Lock_Forest *Forest) :
	next_id(0), Forest(Forest)
{
}

Process_map::~Process_map()
{
	for (auto i = Processes.begin(); i != Processes.end(); i++)
		delete i->second;
}

void Process_map::for_each_process(function<void(const Process *p)> f) const
{
	shared_lock lk(m);

	for (auto i = Processes.begin(); i != Processes.end(); i++)
		f(i->second);
}

const uint32_t Process_map::create ()
{
	unique_lock lk(m);

	if (Processes.find (next_id) != Processes.end())
		throw too_many_processes_exception ();

	auto p = new Process (next_id);
	Processes.insert (pair<const uint32_t, Process*>(next_id, p));

	if (next_id == numeric_limits<decltype(next_id)>::max())
		next_id = 0;
	else
		next_id++;

	return p->get_id();
}

pair<int,set<shared_ptr<Lock_Request>>> Process_map::try_destroy (const uint32_t id, bool release_locks)
{
	unique_lock lk(m);
	set<shared_ptr<Lock_Request>> answered_requests;

	/* Try to find it */
	auto t = Processes.find (id);
	if (t == Processes.end())
		return pair(PROCESS_UNREGISTER_RESULT_NON_EXISTENT, answered_requests);

	auto p = t->second;

	/* Look if it can be destroyed and eventually release held locks */
	if (release_locks)
	{
		/* This should make it more robust, eventually catching us in an
		 * endless loop if something does not work. */
		while (p->get_lock_count() > 0)
		{
			auto locks = p->get_held_locks();
			for (auto i = locks.begin(); i != locks.end(); i++)
				answered_requests.merge((Forest->release (p, &(i->first), i->second)).second);
		}
	}
	else
	{
		if (p->get_lock_count() > 0)
			return pair(PROCESS_UNREGISTER_RESULT_HOLDS_LOCKS, answered_requests);
	}

	/* Eventually destroy it */
	Processes.erase(p->get_id());
	delete p;

	return pair(PROCESS_UNREGISTER_RESULT_SUCCESS, answered_requests);
}

std::pair<Process*, shared_lock<shared_mutex>> Process_map::find (const uint32_t id)
{
	shared_lock lk(m);

	auto p = Processes.find (id);
	auto proc = p != Processes.end() ? p->second : nullptr;
	return pair (proc, move(lk));
}
