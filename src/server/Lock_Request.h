#ifndef __LOCK_REQUEST_H
#define __LOCK_REQUEST_H

#include "Process.h"
#include <mutex>

namespace server {

class Lock_Request
{
protected:
	std::mutex m;

	/* To identify the request */
	const uint32_t id;

	/* Who and what? */
	Process *requester;

	const bool release;
	const bool write;

public:
	Lock_Request (const uint32_t id, const bool release, const bool write,
			Process *requester);

	const uint32_t get_id ();
};

}

#endif /* __LOCK_REQUEST_H */
