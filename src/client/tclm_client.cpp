#include "tclm_client.hpp"
#include "tclmc_impl.h"

using namespace std;
using namespace tclm_client;

shared_ptr<tclmc> tclmc::create (const std::string servername, uint16_t tcp_port, uint16_t udp_port)
{
	if (tcp_port != 0)
	{
		if (udp_port != 0)
			return make_shared<tclmc_impl> (servername, tcp_port, udp_port);
		else
			return make_shared<tclmc_impl> (servername, tcp_port);
	}
	else
		return make_shared<tclmc_impl> (servername);
}
