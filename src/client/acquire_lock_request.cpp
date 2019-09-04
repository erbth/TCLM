#include "acquire_lock_request.h"

using namespace std;
using namespace tclm_client;

acquire_lock_request::acquire_lock_request (const uint32_t pid, const string *path, const uint8_t mode) :
	pid(pid), path(path), mode(mode)
{}

const uint32_t acquire_lock_request::get_pid () const
{
	scoped_lock lk(m);
	return pid;
}

const string *acquire_lock_request::get_path () const
{
	scoped_lock lk(m);
	return path;
}

const uint8_t acquire_lock_request::get_mode () const
{
	scoped_lock lk(m);
	return mode;
}

uint16_t acquire_lock_request::issue (Access_Concentrator *ac)
{
	ac->issue_acquire_lock_request (this);

	{
		unique_lock lk(m);
		while (!answered)
			cv.wait (lk);

		return status_code;
	}
}
