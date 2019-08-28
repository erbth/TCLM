#include "TCP_Listener.h"
#include "TCP_Connection.h"
#include "daemon.h"
#include "messages.h"
#include "message_utils.h"
#include "backend_exceptions.h"
#include <iostream>
#include <new>

extern "C" {
#include <arpa/inet.h>
}

using namespace std;
using namespace server;

bool daemon::run ()
{
	/* Something like a main loop */

	/* Create a TCP Listener */
	try {
		auto tcpl = make_shared<TCP_Listener>();
		tcpl->set_daemon (this);

		if (!cm.add_polled_fd (tcpl))
			return false;

	} catch (exception &e) {
		cerr << "Failed to create TCP Listener: " << e.what() << endl;
		return false;
	}

	return cm.main_loop ();
}


/* Callback functions for use by Connections. */
void daemon::receive_message (Connection *c, struct stream *s, void *data)
{
	auto pThis = (daemon*) data;
	pThis->receive_message_internal (c, s);
}

/* The backends of callback functions for use by connections */
void daemon::receive_message_internal (Connection *c, struct stream *s)
{
	uint8_t id = stream_read_uint8_t (s);
	uint32_t  length = stream_read_uint32_t (s);

	switch (id)
	{
		case MSG_ID_LIST_CONNS:
			receive_message_list_connections (c, s, length);
			break;

		case MSG_ID_LIST_PROCS:
			receive_message_list_processes (c, s, length);
			break;

		case MSG_ID_REG_PROC:
			receive_message_register_process (c, s, length);
			break;

		case MSG_ID_UNREG_PROC:
			receive_message_unregister_process (c, s, length);
			break;
	}

	stream_free (s);
}

/* Message handlers */
void daemon::receive_message_list_connections (Connection *c, struct stream *input, uint32_t length)
{
	/* Create a new stream */
	auto s = stream_new();
	if (!s)
		throw bad_alloc();

	/* Send a list of connections back */
	if (!write_message_header (s, MSG_ID_LIST_CONNS_RESPONSE, 0))
	{
		stream_free(s);
		throw bad_alloc();
	}

	cm.for_each_pfd ([s](const polled_fd *pfd){
			auto tcp_conn = dynamic_cast<const TCP_Connection*>(pfd);
			if (tcp_conn)
			{
				auto addr = tcp_conn->get_sockaddr();

				if (stream_ensure_remaining_capacity (s, 7) < 0)
				{
					stream_free (s);
					throw bad_alloc();
				}

				stream_write_uint8_t (s,0);
				stream_write_uint32_t (s,ntohl(addr->sin_addr.s_addr));
				stream_write_uint16_t (s,ntohs(addr->sin_port));
			}
		});

	/* Update the length of the message */
	update_message_length (s, stream_tell(s));

	/* Send the stream */
	c->send(s);
}

void daemon::receive_message_list_processes (Connection *c, struct stream *input, uint32_t length)
{
	/* Create new stream */
	auto s = stream_new ();
	if (!s)
		throw bad_alloc();

	if (!write_message_header (s, MSG_ID_LIST_PROCS_RESPONSE, 0))
	{
		stream_free (s);
		throw bad_alloc();
	}

	/* Add all processes */
	b.for_each_process ([s](const Process *p){
			if (stream_ensure_remaining_capacity (s, 8) < 0)
			{
				stream_free (s);
				throw bad_alloc();
			}

			stream_write_uint32_t (s, p->get_id());
			stream_write_uint32_t (s, p->get_lock_count());
		});

	/* Update the message's length */
	update_message_length (s, stream_tell(s));

	/* Send the stream */
	c->send(s);
}

void daemon::receive_message_register_process (Connection *c, struct stream *input, uint32_t length)
{
	uint32_t nonce = stream_read_uint32_t (input);

	auto s = stream_new ();
	if (!s || stream_ensure_remaining_capacity (s, 15) != 0)
		throw bad_alloc();

	write_message_header (s, MSG_ID_REG_PROC_RESPONSE, 10);
	stream_write_uint32_t (s, nonce);

	/* Call the backend */
	try {
		auto id = b.register_process ();
		stream_write_uint16_t (s, RESPONSE_STATUS_SUCCESS);
		stream_write_uint32_t (s, id);

	} catch (too_many_processes_exception &e) {
		stream_write_uint16_t (s, RESPONSE_STATUS_TOO_MANY_PROCESSES);
		update_message_length (s, 11);
	}

	/* Send a message */
	c->send (s);
}

void daemon::receive_message_unregister_process (Connection *c, struct stream *input, uint32_t length)
{
	uint32_t id = stream_read_uint32_t (input);

	auto s = stream_new ();
	if (!s || stream_ensure_remaining_capacity (s, 11) != 0)
		throw bad_alloc();

	write_message_header (s, MSG_ID_UNREG_PROC_RESPONSE, 6);
	stream_write_uint32_t (s, id);

	/* Call the backend */
	switch (b.unregister_process (id))
	{
		case PROCESS_UNREGISTER_RESULT_SUCCESS:
			stream_write_uint16_t (s, RESPONSE_STATUS_SUCCESS);
			break;

		case PROCESS_UNREGISTER_RESULT_NON_EXISTENT:
			stream_write_uint16_t (s, RESPONSE_STATUS_NO_SUCH_PROCESS);
			break;

		case PROCESS_UNREGISTER_RESULT_HOLDS_LOCKS:
			stream_write_uint16_t (s, RESPONSE_STATUS_PROCESS_HOLDS_LOCKS);
			break;
	}

	/* Send a message */
	c->send (s);
}
