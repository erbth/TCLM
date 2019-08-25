#ifndef __DAEMON_H
#define __DAEMON_H

/* Backend + front end (partly implemented here since daemons may be software
 * which makes resources available ...) */

#include "Communications_Manager.h"
#include "Connection.h"
#include "stream.h"
#include "backend.h"

namespace server {

/* Prototypes against cyclic-include-dependency-problems */
class TCP_Listener;

class daemon
{
protected:
	/* The backends of callback functions for use by Connections */
	void receive_message_internal (Connection *c, struct stream *s);

	/* Message handlers */
	void receive_message_list_connections (Connection *c, struct stream *input, uint32_t length);

public:
	Communications_Manager m;
	backend b;

	/* Returns false if an error was encountered. Must not be called more than
	 * once during an invocation of tclmd. */
	bool run();

	/* Callback functions for use by Connections */
	static void receive_message (Connection *c, struct stream *s, void *data);
};

}

#endif /* __DAEMON_H */
