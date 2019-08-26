#include "Access_Concentrator.h"
#include "tclm_client_exceptions.hpp"
#include "TCP_Connection.h"

extern "C" {
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace std;
using namespace tclm_client;

Access_Concentrator::Access_Concentrator (
		const std::string &servername,
		const uint16_t tcp_port,
		const uint16_t udp_port) :
	servername(servername), tcp_port(tcp_port), udp_port(udp_port)
{
	/* If the first attempt to connect to a server doesn't succeed, it's
	 * ridiculous. */
	if (!create_tcp_connection())
		throw cannot_connect_exception (servername, tcp_port, "tcp");
}

bool Access_Concentrator::create_tcp_connection() noexcept
{
	if (tcp_connection)
		return true;

	struct addrinfo gai_hints = { 0 };
	gai_hints.ai_family = AF_UNSPEC;
	gai_hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *ais = nullptr;

	if (getaddrinfo (servername.c_str(), nullptr, &gai_hints, &ais) == 0)
	{
		struct addrinfo *ai = ais;

		while (ai)
		{
			auto addr = ai->ai_addr;
			auto addrlen = ai->ai_addrlen;

			if (ai->ai_family == AF_INET || ai->ai_family == AF_INET6)
			{
				/* Add a port */
				if (ai->ai_family == AF_INET)
					((struct sockaddr_in*) addr)->sin_port = htons (tcp_port);
				else
					((struct sockaddr_in6*) addr)->sin6_port = htons (tcp_port);

				/* Try to connect */
				int fd = socket (ai->ai_family, ai->ai_socktype, 0);
				if (fd)
				{
					if (connect (fd, addr, addrlen) == 0)
					{
						if (ai->ai_family == AF_INET)
						{
							try {
								tcp_connection = make_unique<TCP_Connection>(fd, (const sockaddr_in*) addr);
								break;
							} catch (...) {
								close (fd);
							}
						}
						else
							close (fd);

						break;
					}
					else
						close (fd);
				}
			}

			ai = ai->ai_next;
		}

		freeaddrinfo (ais);
		ais = nullptr;
	}

	return tcp_connection ? true : false;
}
