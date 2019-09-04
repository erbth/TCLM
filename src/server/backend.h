#ifndef __BACKEND_H
#define __BACKEND_H

#include "backend_operation_defines.h"
#include "Lock_Forest.h"
#include "Process_map.h"
#include <functional>
#include <string>

namespace server {

class backend
{
private:
	Lock_Forest Forest;
	Process_map Processes;

public:
	/* May throw a too_many_processes_exception */
	const uint32_t register_process ();

	/* Returns one out of PROCESS_UNREGISTER_RESULT_* */
	int unregister_process (const uint32_t id);
	void for_each_process (std::function<void(const Process *p)> f) const;

	/* Returns on out of CREATE_LOCK_RESULT_* */
	int create_lock (const uint32_t pid, std::string *path);
	void for_each_lock (std::function<void(const Lock *l, const uint32_t level)> f) const;
};

}

#endif /* __BACKEND_H */
