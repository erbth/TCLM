#include "Lock_impl.h"
#include "create_lock_request.h"
#include "acquire_lock_request.h"
#include "release_lock_request.h"
#include "messages.h"
#include "tclm_client_exceptions.hpp"

using namespace std;
using namespace tclm_client;

Lock_impl::Lock_impl (shared_ptr<tclmc_impl> tclmc, const string path) :
	path(path), tclmc(tclmc)
{}

const string Lock_impl::get_path () const
{
	return path;
}

bool Lock_impl::create (std::shared_ptr<Process> p, const bool acquire_X)
{
	auto r = make_unique<create_lock_request> (p->get_id(), &path, acquire_X);
	auto status_code = r->issue (&(tclmc->ac));

	switch (status_code)
	{
		case RESPONSE_STATUS_SUCCESS:
			return true;

		case RESPONSE_STATUS_LOCK_EXISTS:
			return false;

		case RESPONSE_STATUS_PARENT_NOT_HELD:
			throw parent_not_held_exception ();

		case RESPONSE_STATUS_NO_SUCH_PROCESS:
		default:
			throw no_such_process_exception ();
	}
}

void Lock_impl::destroy (std::shared_ptr<Process> p)
{
}


void Lock_impl::acquire_S (shared_ptr<Process> p)
{
	acquire (p, MSG_LOCK_MODE_S);
}

void Lock_impl::acquire_Splus (shared_ptr<Process> p)
{
	acquire (p, MSG_LOCK_MODE_Splus);
}

void Lock_impl::acquire_X (shared_ptr<Process> p)
{
	acquire (p, MSG_LOCK_MODE_X);
}

void Lock_impl::acquire (shared_ptr<Process> p, uint8_t mode)
{
	auto r = make_unique<acquire_lock_request> (p->get_id(), &path, mode);

	switch (r->issue (&(tclmc->ac)))
	{
		case RESPONSE_STATUS_SUCCESS:
			return;

		case RESPONSE_STATUS_NO_SUCH_PROCESS:
			throw no_such_process_exception ();

		case RESPONSE_STATUS_NO_SUCH_LOCK:
		default:
			throw no_such_lock_exception ();
	}
}

void Lock_impl::release_S (shared_ptr<Process> p)
{
	release (p, MSG_LOCK_MODE_S);
}

void Lock_impl::release_Splus (shared_ptr<Process> p)
{
	release (p, MSG_LOCK_MODE_Splus);
}

void Lock_impl::release_X (shared_ptr<Process> p)
{
	release (p, MSG_LOCK_MODE_X);
}

void Lock_impl::release (shared_ptr<Process> p, uint8_t mode)
{
	auto r = make_unique<release_lock_request> (p->get_id(), &path, mode);

	switch (r->issue (&(tclmc->ac)))
	{
		case RESPONSE_STATUS_SUCCESS:
			return;

		case RESPONSE_STATUS_NO_SUCH_PROCESS:
			throw no_such_process_exception ();

		case RESPONSE_STATUS_LOCK_NOT_HELD:
		default:
			throw lock_not_held_exception ();
	}
}
