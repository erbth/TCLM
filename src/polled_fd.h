#ifndef __POLLED_FD
#define __POLLED_FD

/* This file belongs to the communications unit. */
#include <memory>
#include <mutex>

/* Prototypes for cyclic include dependencies */
class Communications_Manager;

class polled_fd : public std::enable_shared_from_this<polled_fd>
{
protected:
	std::recursive_mutex m;

	int fd;
	bool close_fd;
	bool out_enabled;
	Communications_Manager *mgr;

	/* If close_fd is true, the class closes the fd on destruction. */
	polled_fd(int fd, bool close_fd = true);

public:
	virtual ~polled_fd();

	const int get_fd ();
	bool get_out_enabled ();
	Communications_Manager *get_mgr ();

	void set_out_enabled (bool enabled);
	void set_mgr (Communications_Manager *mgr);

	/* Return false if the connection is to be closed. */
	virtual bool data_in () = 0;
	virtual bool data_out () = 0;
};

/* Carefully placed includes */
#include "Communications_Manager.h"

#endif /* __POLLED_FD */
