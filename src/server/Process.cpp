#include "Process.h"
#include "backend_exceptions.h"

using namespace std;
using namespace server;

Process::Process (const uint32_t id) : id(id)
{
	lock_count = 0;
}

const uint32_t Process::get_id () const
{
	// It may be required for memory synchronization between different cores
	lock_guard lk(m);
	return id;
}

const uint32_t Process::get_lock_count () const
{
	lock_guard lk(m);
	return lock_count;
}

void Process::increase_lock_count ()
{
	lock_guard lk(m);

	/* Are we on the safe side? */
	if (lock_count == numeric_limits<decltype(lock_count)>::max())
		throw process_too_many_locks_exception (id);

	lock_count++;
}

void Process::decrease_lock_count ()
{
	/* This must not happen, hence it will cause a program crash and the
	 * exception does not need to be machine readable. */
	if (lock_count == 0)
		throw "Process: decrease_lock_count of 0 requested.";

	lock_count--;
}
