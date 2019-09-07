/* This file belongs to the communications unit */
#include "TCP_Connection.h"
#include "TCP_Listener.h"
#include "errno_exception.h"
#include "tclm_config.h"
#include <cerrno>
#include <iostream>
#include <memory>

extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace std;
using namespace server;

class concrete_TCP_Listener : public TCP_Listener {};

shared_ptr<TCP_Listener> TCP_Listener::create ()
{
	return make_shared<concrete_TCP_Listener>();
}

TCP_Listener::TCP_Listener () :
	polled_fd (-1, close_fd=true)
{
	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw errno_exception (errno);

	int reuseaddr = 1;
	if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) < 0)
	{
		int e = errno;
		close (fd);
		throw errno_exception (e);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons (SERVER_TCP_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind (fd, (struct sockaddr*) &addr, sizeof (addr)) < 0)
	{
		int e = errno;
		close (fd);
		throw errno_exception (e);
	}

	if (listen (fd, 100) < 0)
	{
		int e = errno;
		close (fd);
		throw errno_exception (e);
	}
}

TCP_Listener::~TCP_Listener ()
{
}

void TCP_Listener::set_daemon (daemon *d)
{
	this->d = d;
}

bool TCP_Listener::data_in ()
{
	/* One comment: We actually never want to close this socket (except for
	 * program termination). */

	struct sockaddr_in endpoint_addr;
	socklen_t addrlen = sizeof (endpoint_addr);

	int endpoint_fd = accept (fd, (struct sockaddr*) &endpoint_addr, &addrlen);
	if (endpoint_fd < 0)
		return true;

	try {
		auto c = TCP_Connection::create (endpoint_fd, &endpoint_addr);
		c->set_receive_callback (daemon::receive_message, (void*) d);
		mgr->add_polled_fd (c);
	} catch (exception &e) {
		cerr << "Failed to create Connection: " << e.what() << endl;
		close (endpoint_fd);
	}

	return true;
}

bool TCP_Listener::data_out ()
{
	/* We actually do not want to write data. */
	mgr->fd_disable_out (this);
	return true;
}
