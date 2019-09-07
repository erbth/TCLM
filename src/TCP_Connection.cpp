#include "TCP_Connection.h"
#include <cstring>

using namespace std;

class concrete_TCP_Connection : public TCP_Connection {
public:
	concrete_TCP_Connection (int fd, const struct sockaddr_in *addr) :
		TCP_Connection (fd, addr) {}
};

shared_ptr<TCP_Connection> TCP_Connection::create (int fd, const struct sockaddr_in *addr)
{
	return make_shared<concrete_TCP_Connection>(fd, addr);
}

TCP_Connection::TCP_Connection (int fd, const struct sockaddr_in *addr) :
	Connection(fd), addr(*addr)
{
}

TCP_Connection::~TCP_Connection ()
{
}

const struct sockaddr_in *TCP_Connection::get_sockaddr () const
{
	return &addr;
}
