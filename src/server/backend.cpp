#include "backend.h"

using namespace std;
using namespace server;

const uint32_t backend::register_process ()
{
	return Processes.create ();
}

int backend::unregister_process (const uint32_t id)
{
	return Processes.try_destroy (id);
}

void backend::for_each_process(function<void(const Process *p)> f) const
{
	Processes.for_each_process(f);
}
