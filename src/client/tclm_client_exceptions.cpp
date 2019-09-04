#include "tclm_client_exceptions.hpp"
#include <memory>

using namespace std;
using namespace tclm_client;

/* cannot_connect_exception */
cannot_connect_exception::cannot_connect_exception (
		const string &servername,
		const uint16_t port,
		const string &protocol) noexcept
{
	try
	{
		msg = new string("Cannot `connect' to server \"");
		(*msg) += servername + ":" + to_string(port) + "\" using protocol " + protocol + ".";
	}
	catch (...) {
		msg = nullptr;
	}
}

cannot_connect_exception::~cannot_connect_exception () noexcept
{
	if (msg)
		delete msg;
}

const char *cannot_connect_exception::what () const noexcept
{
	if (msg)
		return msg->c_str();
	else
		return "Cannot connect to server.";
}

/* too_many_processes_exception */
const char *too_many_processes_exception::what () const noexcept
{
	return "Too many processes.";
}

/* process_holds_locks_exception */
const char *process_holds_locks_exception::what () const noexcept
{
	return "Process holds locks.";
}

/* no_such_process_exception */
const char *no_such_process_exception::what () const noexcept
{
	return "No such process.";
}

/* lock_not_held_exception */
const char *lock_not_held_exception::what () const noexcept
{
	return "Lock not held in this mode.";
}

/* no_such_lock_exception */
const char *no_such_lock_exception::what () const noexcept
{
	return "No such lock.";
}
