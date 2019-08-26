/* The Access Concentrator is responsible for haveing a connection to a tclmd
 * and for handling any operation on that tclmd. */

#ifndef __ACCESS_CONCENTRATOR_H
#define __ACCESS_CONCENTRATOR_H

#include "Connection.h"
#include <memory>
#include <string>

namespace tclm_client {

class Access_Concentrator
{
protected:
	std::string servername;
	uint16_t tcp_port;
	uint16_t udp_port;

	std::unique_ptr<Connection> tcp_connection;

	/* Try to create a TCP connection (over IPv4 or IPv6) */
	bool create_tcp_connection () noexcept;

public:
	Access_Concentrator (const std::string &servername,
			const uint16_t tcp_port, const uint16_t udp_port);
};

}

#endif /* __ACCESS_CONCENTRATOR_H */
