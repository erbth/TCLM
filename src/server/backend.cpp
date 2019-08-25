#include "backend.h"

using namespace std;
using namespace server;

const uint32_t backend::register_process ()
{
	return Processes.create ();
}

int backend::unregister_process (const uint32_t id)
{
	auto t = Processes.find (id);
	auto p = t.first;

	if (!p)
		return PROCESS_UNREGISTER_RESULT_NON_EXISTENT;

	if (Processes.try_destroy (p))
		return PROCESS_UNREGISTER_RESULT_SUCCESS;
	else
		return PROCESS_UNREGISTER_RESULT_HOLDS_LOCKS;
}
