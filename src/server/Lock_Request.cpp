#include "Lock_Request.h"

using namespace server;
using namespace std;

Lock_Request::Lock_Request (const uint32_t id, const bool release,
		const bool write, Process *requester) :
	id(id), release(release), write(write), requester(requester)
{
}

const uint32_t Lock_Request::get_id ()
{
	/* This may be needed for memory synchronization */
	lock_guard lk(m);
	return id;
}
