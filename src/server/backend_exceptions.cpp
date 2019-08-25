#include "backend_exceptions.h"

using namespace std;
using namespace server;

const char *too_many_processes_exception::what () const noexcept
{
	return "Too many processes, no free ids left.";
}


process_too_many_locks_exception::process_too_many_locks_exception (
		const uint32_t proc_id) :
	proc_id(proc_id), msg(nullptr)
{
	try
	{
		msg = new string();
		*msg = "Process " + to_string(proc_id) + " holds too many locks.";
	}
	catch (...)
	{
		if (msg)
		{
			delete msg;
			msg = nullptr;
		}
	}
}

process_too_many_locks_exception::~process_too_many_locks_exception()
{
	if (msg)
		delete msg;
}

const char *process_too_many_locks_exception::what() const noexcept
{
	if (msg)
		return msg->c_str();
	else
		return "A Process holds too many locks. Its id cannot be specified because we're out of memory.";
}

const uint32_t process_too_many_locks_exception::get_proc_id() const noexcept
{
	return proc_id;
}
