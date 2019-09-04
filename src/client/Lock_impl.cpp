#include "Lock_impl.h"
#include "create_lock_request.h"
#include "messages.h"
#include "tclm_client_exceptions.hpp"

using namespace std;
using namespace tclm_client;

Lock_impl::Lock_impl (shared_ptr<tclmc_impl> tclmc, const string path) :
	tclmc(tclmc), path(path)
{}

const string Lock_impl::get_path () const
{
	return path;
}

bool Lock_impl::create (std::shared_ptr<Process> p)
{
	auto r = make_unique<create_lock_request> (p->get_id(), &path);
	auto status_code = r->issue (&(tclmc->ac));

	switch (status_code)
	{
		case RESPONSE_STATUS_SUCCESS:
			return true;

		case RESPONSE_STATUS_LOCK_EXISTS:
			return false;

		case RESPONSE_STATUS_NO_SUCH_PROCESS:
		default:
			throw no_such_process_exception ();
	}
}

void Lock_impl::destroy (std::shared_ptr<Process> p)
{
}
