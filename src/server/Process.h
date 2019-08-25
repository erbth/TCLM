#ifndef __PROCESS_H
#define __PROCESS_H

#include <mutex>

namespace server {

class Process
{
protected:
	std::mutex m;

	// 2 billion concurrent processes should be enough.
	const uint32_t id;

	/* This counter helps to make sure the Process does not hold any lock when
	 * unregistered. */
	uint32_t lock_count;

public:
	Process (const uint32_t id);

	const uint32_t get_id ();

	const uint32_t get_lock_count();
	void increase_lock_count();
	void decrease_lock_count();
};

}

#endif /* __PROCESS_H */
