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
		return CREATE_LOCK_RESULT_NO_SUCH_PROCESS;

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

int backend::release_lock (const uint32_t pid, string *path, uint8_t mode)
{
	/* Find the process object */
	auto pt = Processes.find (pid);
	auto p = pt.first;

	if (!p)
		return RELEASE_LOCK_RESULT_NO_SUCH_PROCESS;

	/* Release the lock */
	switch (Forest.release (p, path, mode))
	{
		case LOCK_RELEASE_SUCCESS:
			return RELEASE_LOCK_RESULT_RELEASED;

		case LOCK_RELEASE_NOT_HELD:
		default:
			return RELEASE_LOCK_RESULT_NOT_HELD;
	}
}

void backend::for_each_lock (function<void(const Lock *l, const uint32_t level)> f) const
{
	Forest.for_each_lock (f);
}
