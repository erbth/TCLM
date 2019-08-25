#include "TCP_Connection.h"
#include <cstring>

using namespace std;

TCP_Connection::TCP_Connection (int fd, const struct sockaddr_in *addr) :
	Connection(fd)
{
	memcpy (&(this->addr), addr, sizeof (this->addr));
}

TCP_Connection::~TCP_Connection ()
{
}

const struct sockaddr_in *TCP_Connection::get_sockaddr () const
{
	return &addr;
}
