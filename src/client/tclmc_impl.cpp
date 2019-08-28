#include "tclmc_impl.h"
#include "tclm_client_exceptions.hpp"
#include "Process_impl.h"
#include "messages.h"
#include <iostream>
#include <memory>

using namespace std;
using namespace tclm_client;

tclmc_impl::tclmc_impl (const string &servername, const uint16_t tcp_port, const uint16_t udp_port) :
	ac(servername, tcp_port, udp_port)
{
}

tclmc_impl::~tclmc_impl ()
{
}

shared_ptr<Process> tclmc_impl::register_process ()
{
	auto r = make_unique<register_process_request>();

	auto status_code = r->issue (&ac);

	switch (status_code)
	{
		case RESPONSE_STATUS_SUCCESS:
			return make_shared<Process_impl>(shared_from_this (), r->get_id());

		case RESPONSE_STATUS_TOO_MANY_PROCESSES:
			throw too_many_processes_exception();
	}

	throw "???";
}
