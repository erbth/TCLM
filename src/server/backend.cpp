#include "backend.h"

using namespace std;
using namespace server;

backend::backend () :
	Forest(), Processes(&Forest)
{
}

const uint32_t backend::register_process ()
{
	return Processes.create ();
}

std::pair<int,std::set<std::shared_ptr<Lock_Request>>> backend::unregister_process (const uint32_t id)
{
	return Processes.try_destroy (id);
}

void backend::for_each_process(function<void(const Process *p)> f) const
{
	Processes.for_each_process(f);
}

pair<Process *, shared_lock<shared_mutex>> backend::find_process (const uint32_t id)
{
	return move(Processes.find(id));
}

int backend::create_lock (const uint32_t pid, string *path, const bool acquire_X)
{
	/* Find the process object. */
	auto t = Processes.find (pid);
	auto p = t.first;

	if (!p)
		return CREATE_LOCK_RESULT_NO_SUCH_PROCESS;

	/* Create the Lock */
	auto ret = Forest.create (p, path, acquire_X);

	switch (ret)
	{
		case LOCK_CREATE_CREATED:
			return CREATE_LOCK_RESULT_CREATED;

		case LOCK_CREATE_QUEUED:
			return CREATE_LOCK_RESULT_QUEUED;

		case LOCK_CREATE_EXISTS:
			return CREATE_LOCK_RESULT_EXISTS;

		case LOCK_CREATE_PARENT_NOT_HELD:
			return CREATE_LOCK_RESULT_PARENT_NOT_HELD;

		default:
			/* Cannot happen */
			return CREATE_LOCK_RESULT_NO_SUCH_PROCESS;
	}
}

int backend::acquire_lock (const uint32_t pid, string *path, uint8_t mode)
{
	/* Find the process object */
	auto pt = Processes.find (pid);
	auto p = pt.first;

	if (!p)
		return ACQUIRE_LOCK_RESULT_NO_SUCH_PROCESS;

	/* Acquire the lock */
	switch (Forest.acquire (p, path, mode))
	{
		case LOCK_ACQUIRE_ACQUIRED:
			return ACQUIRE_LOCK_RESULT_ACQUIRED;

		case LOCK_ACQUIRE_QUEUED:
			return ACQUIRE_LOCK_RESULT_QUEUED;

		case LOCK_ACQUIRE_NON_EXISTENT:
		default:
			return ACQUIRE_LOCK_RESULT_NO_SUCH_LOCK;
	}
}


std::pair<int,std::set<std::shared_ptr<Lock_Request>>> backend::release_lock (
		const uint32_t pid, string *path, uint8_t mode)
{

	/* Find the process object */
	auto pt = Processes.find (pid);
	auto p = pt.first;

	if (!p)
		return pair(RELEASE_LOCK_RESULT_NO_SUCH_PROCESS,set<shared_ptr<Lock_Request>>());

	/* Release the lock */
	auto rt = Forest.release (p, path, mode);
	switch (rt.first)
	{
		case LOCK_RELEASE_SUCCESS:
			return pair(RELEASE_LOCK_RESULT_RELEASED,rt.second);

		case LOCK_RELEASE_NOT_HELD:
		default:
			return pair(RELEASE_LOCK_RESULT_NOT_HELD,rt.second);
	}
}

void backend::for_each_lock (function<void(const Lock *l, const uint32_t level)> f) const
{
	Forest.for_each_lock (f);
}
