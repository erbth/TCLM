#ifndef __BACKEND_H
#define __BACKEND_H

#include "backend_operation_defines.h"
// #include "Lock_Request_map.h"
// #include "Lock_forest.h"
#include "Process_map.h"
#include <functional>

namespace server {

class backend
{
private:
	// Lock_forest forest;
	Process_map Processes;
	// Lock_Request_map Lock_Requests;

public:
	/* May throw a too_many_processes_exception */
	const uint32_t register_process ();

	/* Returns one out of PROCESS_UNREGISTER_RESULT_* */
	int unregister_process (const uint32_t id);
	void for_each_process (std::function<void(const Process *p)> f) const;
};

}

#endif /* __BACKEND_H */
