#include "create_lock_request.h"
#include <iostream>

using namespace std;
using namespace tclm_client;

create_lock_request::create_lock_request (const uint32_t pid, const string *path, const bool acquire_X) :
	pid(pid), path(path), acquire_X(acquire_X)
{}

const uint32_t create_lock_request::get_pid () const
{
	scoped_lock lk(m);
	return pid;
}

const string *create_lock_request::get_path () const
{
	scoped_lock lk(m);
	return path;
}

const bool create_lock_request::get_acquire_X () const
{
	scoped_lock lk(m);
	return acquire_X;
}

uint16_t create_lock_request::issue (Access_Concentrator *ac)
{
	ac->issue_create_lock_request (this);

	{
		unique_lock lk(m);
		while (!answered)
			cv.wait (lk);

		return status_code;
	}
}
