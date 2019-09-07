#ifndef __PROCESS_MAP_H
#define __PROCESS_MAP_H

#include "Process.h"
#include "backend_operation_defines.h"
#include <functional>
#include <map>
#include <shared_mutex>

namespace server {

/* Prototypes */
class Lock_Request;
class Lock_Forest;

class Process_map
{
private:
	mutable std::shared_mutex m;

	std::map<const uint32_t, Process*> Processes;
	uint32_t next_id;

	Lock_Forest *Forest;

public:
	Process_map (Lock_Forest *Forest);
	~Process_map ();

	void for_each_process (std::function<void(const Process *p)> f) const;

	/* May throw a too_manu_processes_exception */
	const uint32_t create ();

	/* Returns one out of PROCESS_UNREGISTER_RESULT_* and a set of Lock Requests
	 * that where answered as result of the unregister operation. If
	 * release_locks is true, all held locks are released automatically before
	 * the process is destroyed. */
	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> try_destroy (
			const uint32_t id, bool release_locks = true);

	/* The return value is only valid as long as the returned lock is kept and
	 * the Process is not destroyed by the same thread.
	 *
	 * The Process* may be nullptr. However this information is only valid as
	 * long as the lock is kept. */
	std::pair<Process *, std::shared_lock<std::shared_mutex>> find (const uint32_t id);
};

}

#endif /* __PROCESS_MAP_H */
