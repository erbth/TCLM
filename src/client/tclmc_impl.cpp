#include "tclmc_impl.h"
#include "tclm_client_exceptions.hpp"

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
	return 0;
}

void tclmc_impl::unregister_process (uint32_t id)
{
}
