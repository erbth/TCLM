#ifndef __POLLED_FD
#define __POLLED_FD

/* This file belongs to the communications unit. */
#include "Communications_Manager.h"

/* Prototypes for cyclic include dependencies */
class Communications_Manager;

class polled_fd
{
protected:
	int fd;
	bool close_fd;
	bool out_enabled;
	Communications_Manager *mgr;

public:
	/* If close_fd is true, the class closes the fd on destruction. */
	polled_fd(int fd, bool close_fd = true);
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

#endif /* __POLLED_FD */
