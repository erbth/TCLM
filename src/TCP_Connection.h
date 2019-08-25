#ifndef __TCP_CONNECTION_H
#define __TCP_CONNECTION_H

/* TCP over IPv4 connections */

#include "Connection.h"

extern "C" {
#include <netinet/in.h>
}

class TCP_Connection : public Connection
{
protected:
	struct sockaddr_in addr;

public:
	TCP_Connection(int fd, const struct sockaddr_in *addr);
	~TCP_Connection() override;

	const struct sockaddr_in *get_sockaddr () const;
};

#endif /* __TCP_CONNECTION_H */
