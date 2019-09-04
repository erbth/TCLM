#include "backend.h"

using namespace std;
using namespace server;

const uint32_t backend::register_process ()
{
	return Processes.create ();
}

int backend::unregister_process (const uint32_t id)
{
	return Processes.try_destroy (id);
}

void backend::for_each_process(function<void(const Process *p)> f) const
{
	Processes.for_each_process(f);
}

int backend::create_lock (const uint32_t pid, string *path)
{
	/* Find the process object. */
	auto t = Processes.find (pid);
	auto p = t.first;

	if (!p)
		return CREATE_LOCK_NO_SUCH_PROCESS;

	/* Create the Lock */
	auto ret = Forest.create (p, path);

	switch (ret)
	{
		case LOCK_CREATE_CREATED:
			return CREATE_LOCK_RESULT_CREATED;

		case LOCK_CREATE_QUEUED:
			return CREATE_LOCK_RESULT_QUEUED;

		case LOCK_CREATE_EXISTS:
		default:
			return CREATE_LOCK_RESULT_EXISTS;
	}
}

void backend::for_each_lock (function<void(const Lock *l, const uint32_t level)> f) const
{
	Forest.for_each_lock (f);
}
