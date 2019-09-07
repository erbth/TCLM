#ifndef __BACKEND_H
#define __BACKEND_H

#include "backend_operation_defines.h"
#include "Lock_Forest.h"
#include "Process_map.h"
#include <functional>
#include <string>

namespace server {

/* Prototypes */
class Lock_Request;

class backend
{
private:
	Lock_Forest Forest;
	Process_map Processes;

public:
	backend();

	/* May throw a too_many_processes_exception */
	const uint32_t register_process ();

	/* Returns one out of PROCESS_UNREGISTER_RESULT_* and a set of lock requests
	 * that were answered as result of releasing locks during unregistering that
	 * Process. */
	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> unregister_process (const uint32_t id);

	void for_each_process (std::function<void(const Process *p)> f) const;

	/* The return value is only valid as long as the returned lock is kept and
	 * the Process is not destroyed by the same thread.
	 *
	 * The Process* may be nullptr. However this information is only valid as
	 * long as the lock is kept. */
	std::pair<Process *, std::shared_lock<std::shared_mutex>> find_process (const uint32_t id);

	/* Returns one out of CREATE_LOCK_RESULT_* */
	int create_lock (const uint32_t pid, std::string *path);

	/* Returns one out of ACQUIRE_LOCK_RESULT_* and taked one out of LOCK_REQUEST_MODE_*
	 * for mode */
	int acquire_lock (const uint32_t pid, std::string *path, uint8_t mode);

	/* Returns one out of RELEASE_LOCK_RESULT_* and a set of Lock Requests that
	 * were answered as result of the release operation. Takes one out of
	 * LOCK_REQUEST_MODE_* for mode.
	 * Be aware that the process could have been destroyed already, therefore
	 * check get_requester_exists() first. */
	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> release_lock (
			const uint32_t pid, std::string *path, uint8_t mode);

	void for_each_lock (std::function<void(const Lock *l, const uint32_t level)> f) const;
};

}

#endif /* __BACKEND_H */
