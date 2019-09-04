/* A public header for the C++ TCLM client library.
 *
 * Every function may throw a stl exception, i.e. bad_alloc if no memory is
 * left. */

#ifndef __TCLM_CLIENT_HPP
#define __TCLM_CLIENT_HPP

#include "tclm_client_exceptions.hpp"
#include <string>
#include <memory>

namespace tclm_client {

/* Prototypes */
class Process;
class Lock;

class tclmc
{
protected:
	/* Make the class abstract */
	tclmc () {};

public:
	virtual ~tclmc () {};

	/* May throw a cannot_connect_exception. Does never return nullptr but throw
	 * an exception. Specifying a port number of 0 means using the default port. */
	static std::shared_ptr<tclmc> create (const std::string servername,
			uint16_t tcp_port = 0, uint16_t udp_port = 0);

	/* May throw one of the following exceptions (and a stl exception):
	 *   * too_many_processes_exception */
	virtual std::shared_ptr<Process> register_process () = 0;
	virtual std::shared_ptr<Lock> define_lock (const std::string &path) = 0;
};

class Process
{
protected:
	/* Make the class abstract */
	Process () {};

public:
	virtual ~Process () {};

	virtual const uint32_t get_id () const = 0;
};

class Lock
{
protected:
	/* Make the class abstract */
	Lock () {};

public:
	virtual ~Lock () {};

	virtual const std::string get_path () const = 0;
	virtual bool create (std::shared_ptr<Process> p) = 0;
	virtual void destroy(std::shared_ptr<Process> p) = 0;
};

}

#endif /* __TCLM_CLIENT_HPP */
