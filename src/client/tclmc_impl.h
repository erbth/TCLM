#ifndef __TCLMC_IMPL_H
#define __TCLMC_IMPL_H

#include <string>
#include <mutex>
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

class tclmc_impl : public tclmc
{
protected:
	Access_Concentrator ac;

public:
	/* May throw a cannot_connect_exception */
	tclmc_impl (const std::string &servername,
			const uint16_t tcp_port = SERVER_TCP_PORT, const uint16_t udp_port = 0);
	~tclmc_impl () override;

	uint32_t register_process () override;
	void unregister_process (uint32_t id) override;
};

}

#endif /* __TCLMC_IMPL_H */
