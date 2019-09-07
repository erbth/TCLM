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
	void receive_message_internal (std::shared_ptr<Connection> c, struct stream *s);

	void update_pid_connection (uint32_t pid, std::shared_ptr<Connection> c);
	void notify_answered_requests (std::set<std::shared_ptr<Lock_Request>> requests);

	/* Message handlers */
	void receive_message_list_connections (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_list_processes (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_list_locks (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_register_process (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_unregister_process (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_create_lock (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_release_lock (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);
	void receive_message_acquire_lock (std::shared_ptr<Connection> c, struct stream *input, uint32_t length);

public:
	Communications_Manager cm;
	backend b;

	/* Returns false if an error was encountered. Must not be called more than
	 * once during an invocation of tclmd. */
	bool run();

	/* Callback functions for use by Connections */
	static void receive_message (std::shared_ptr<Connection> c, struct stream *s, void *data);
};

}

#endif /* __DAEMON_H */
