#include "Process_map.h"
#include "backend_exceptions.h"

using namespace server;
using namespace std;

Process_map::Process_map() : next_id(0)
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

int Process_map::try_destroy (const uint32_t id)
{
	unique_lock lk(m);

	/* Try to find it */
	auto t = Processes.find (id);
	if (t == Processes.end())
		return PROCESS_UNREGISTER_RESULT_NON_EXISTENT;

	auto p = t->second;

	/* Look if it can be destroyed */
	if (p->get_lock_count() > 0)
		return PROCESS_UNREGISTER_RESULT_HOLDS_LOCKS;

	/* Eventually destroy it */
	Processes.erase(p->get_id());
	delete p;

	return PROCESS_UNREGISTER_RESULT_SUCCESS;
}

std::pair<Process*, shared_lock<shared_mutex>> Process_map::find (const uint32_t id)
{
	shared_lock lk(m);

	auto p = Processes.find (id);
	auto proc = p != Processes.end() ? p->second : nullptr;
	return pair (proc, move(lk));
}
