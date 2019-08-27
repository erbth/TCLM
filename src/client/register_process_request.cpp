#include "register_process_request.h"
#include <iostream>

using namespace std;
using namespace tclm_client;

uint32_t register_process_request::get_id () const
{
	lock_guard lk(m);
	return id;
}

uint16_t register_process_request::issue (Access_Concentrator *ac)
{
	ac->issue_register_process_request (this);

	{
		unique_lock lk(m);
		while (!answered)
			cv.wait (lk);

		return status_code;
	}
}

uint32_t register_process_request::get_nonce () const
{
	lock_guard lk(m);
	return nonce;
}

void register_process_request::set_nonce (uint32_t nonce)
{
	lock_guard lk(m);
	this->nonce = nonce;
}

void register_process_request::set_id (uint32_t id)
{
	lock_guard lk(m);
	this->id = id;
}
