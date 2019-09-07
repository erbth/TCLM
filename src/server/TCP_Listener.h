#ifndef __TCP_LISTENER_H
#define __TCP_LISTENER_H

/* This file belongs to the communications unit. */
#include "Communications_Manager.h"
#include "polled_fd.h"
#include "daemon.h"

/* Prototypes for cyclic include dependencies */
class Communications_Manager;

namespace server {

/* Prototypes against cyclic-include-dependency-problems */
class daemon;

class TCP_Listener : public polled_fd
{
protected:
	daemon *d;

	/* This may throw an errno_exception. */
	TCP_Listener ();

public:
	/* This may throw an errno_exception. */
	static std::shared_ptr<TCP_Listener> create ();
	~TCP_Listener () override;

	void set_daemon (daemon *d);

	bool data_in () override;
	bool data_out () override;
};

}

#endif /* __TCP_LISTENER_H */
