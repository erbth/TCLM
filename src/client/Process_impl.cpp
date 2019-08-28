#include "Process_impl.h"
#include "unregister_process_request.h"
#include "tclm_client_exceptions.hpp"

using namespace std;
using namespace tclm_client;

Process_impl::Process_impl (shared_ptr<tclmc_impl> tclmc, const uint32_t id)
	: tclmc(tclmc), id(id)
{
}

Process_impl::~Process_impl ()
{
	auto r = make_unique<unregister_process_request>(id);
	auto status_code = r->issue (&(tclmc->ac));
}

const uint32_t Process_impl::get_id () const
{
	return id;
}
