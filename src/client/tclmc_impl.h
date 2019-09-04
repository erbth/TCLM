#ifndef __TCLMC_IMPL_H
#define __TCLMC_IMPL_H

#include <string>
#include <memory>
#include "tclm_config.h"
#include "Access_Concentrator.h"
#include "tclm_client.hpp"

/* Policy first draft:
 *
 * Once a connection to a tclmd is made all is done to keep it alive. That is if
 * if is closed, a new one is established silently. The methods may only throw
 * an exception if that cannot be done at all (may also block indefenitely), or
 * if the server died and e.g. the process is not known to it anymore. */

namespace tclm_client {

/* Prototypes */
class Process_impl;
class Lock_impl;

class tclmc_impl : public tclmc, public std::enable_shared_from_this<tclmc_impl>
{
protected:
	Access_Concentrator ac;

	friend Process_impl;
	friend Lock_impl;

public:
	/* May throw a cannot_connect_exception */
	tclmc_impl (const std::string &servername,
			const uint16_t tcp_port = SERVER_TCP_PORT, const uint16_t udp_port = 0);
	~tclmc_impl () override;

	std::shared_ptr<Process> register_process () override;
	std::shared_ptr<Lock> define_lock (const std::string &path) override;
};

}

#endif /* __TCLMC_IMPL_H */
