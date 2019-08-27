#include "tclmc_impl.h"
#include "tclm_client_exceptions.hpp"
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

uint32_t tclmc_impl::register_process ()
{
	auto r = make_unique<register_process_request>();

	auto status_code = r->issue (&ac);

	switch (status_code)
	{
		case RESPONSE_STATUS_SUCCESS:
			return r->get_id();

		case RESPONSE_STATUS_TOO_MANY_PROCESSES:
			throw too_many_processes_exception();
	}

	throw "???";
}

void tclmc_impl::unregister_process (uint32_t id)
{
}
