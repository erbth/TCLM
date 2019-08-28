#include "unregister_process_request.h"

using namespace std;
using namespace tclm_client;

unregister_process_request::unregister_process_request (uint32_t id) :
	id(id)
{
}

const uint32_t unregister_process_request::get_id () const
{
	lock_guard lk(m);
	return id;
}

uint16_t unregister_process_request::issue (Access_Concentrator *ac)
{
	ac->issue_unregister_process_request (this);

	{
		unique_lock lk(m);
		while (!answered)
			cv.wait (lk);

		return status_code;
	}
}
