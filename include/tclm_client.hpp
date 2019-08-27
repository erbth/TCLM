/* A public header for the C++ TCLM client library.
 *
 * Every function may throw a stl exception, i.e. bad_alloc if no memory is
 * left. */

#ifndef __TCLM_CLIENT_HPP
#define __TCLM_CLIENT_HPP

#include "tclm_client_exceptions.hpp"
#include <string>

namespace tclm_client {

class tclmc
{
protected:
	/* Make the class abstract */
	tclmc () {};

public:
	virtual ~tclmc () {};

	/* May throw a cannot_connect_exception. Does never return nullptr but throw
	 * an exception. Specifying a port number of 0 means using the default port. */
	static tclmc *create (const std::string servername,
			uint16_t tcp_port = 0, uint16_t udp_port = 0);

	virtual uint32_t register_process () = 0;
	virtual void unregister_process (uint32_t id) = 0;
};

}

#endif /* __TCLM_CLIENT_HPP */
