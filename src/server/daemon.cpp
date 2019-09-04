#include "TCP_Connection.h"
#include "TCP_Listener.h"
#include "backend_exceptions.h"
#include "daemon.h"
#include "message_utils.h"
#include "messages.h"
#include <cstring>
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

		case MSG_ID_LIST_LOCKS:
			receive_message_list_locks (c, s, length);
			break;

		case MSG_ID_REG_PROC:
			receive_message_register_process (c, s, length);
			break;

		case MSG_ID_UNREG_PROC:
			receive_message_unregister_process (c, s, length);
			break;

		case MSG_ID_CREATE_LOCK:
			receive_message_create_lock (c, s, length);
			break;

		case MSG_ID_RELEASE_LOCK:
			receive_message_release_lock (c, s, length);
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

void daemon::receive_message_list_locks (Connection *c, struct stream *input, uint32_t length)
{
	auto s = stream_new ();
	if (!s || !write_message_header (s, MSG_ID_LIST_LOCKS_RESPONSE, 0))
		throw bad_alloc ();

	/* Call the backend */
	b.for_each_lock ([s](const Lock *l, const uint32_t level)
			{
				string name = l->get_name();
				auto total_length = name.size() + 4 + 2 + 1;

				if (stream_ensure_remaining_capacity (s, total_length) != 0)
					throw bad_alloc();

				stream_write_uint32_t (s, level);
				stream_write_uint16_t (s, name.size());
				memcpy (stream_pointer(s), name.c_str(), name.size());
				stream_set_length (s, stream_length (s) + name.size());
				stream_seek(s, stream_tell(s) + name.size());

				uint8_t status = 0;

				if (l->is_S_locked())
					status |= 0x4;

				if (l->is_Splus_locked())
					status |= 0x2;

				if (l->is_X_locked())
					status |= 0x1;

				stream_write_uint8_t (s, status);
			});

	/* Update the message's length */
	update_message_length (s, stream_tell(s));

	/* Send a message */
	c->send (s);
}

void daemon::receive_message_create_lock (Connection *c, struct stream *input, uint32_t length)
{
	uint32_t pid = stream_read_uint32_t (input);
	uint16_t path_length = stream_read_uint16_t (input);
	string path ((char*) stream_pointer (input), path_length);

	/* Create new stream */
	auto s = stream_new ();
	if (!s)
		return;

	if (stream_ensure_remaining_capacity (s, 5 + 4 + 2 + path_length + 2) != 0)
	{
		stream_free (s);
		return;
	}
	
	/* Cannot fail because the stream has enough capacity. */
	write_message_header (s, MSG_ID_CREATE_LOCK_UPDATE, 4 + 2 + path_length + 2);

	/* Query backend */
	auto ret = b.create_lock (pid, &path);

	uint16_t status_code = RESPONSE_STATUS_NO_SUCH_PROCESS;
	switch (ret)
	{
		case CREATE_LOCK_RESULT_CREATED:
			status_code = RESPONSE_STATUS_SUCCESS;
			break;

		case CREATE_LOCK_RESULT_QUEUED:
			status_code = RESPONSE_STATUS_QUEUED;
			break;

		case CREATE_LOCK_RESULT_EXISTS:
			status_code = RESPONSE_STATUS_LOCK_EXISTS;
			break;
	}

	stream_write_uint32_t (s, pid);
	stream_write_uint16_t (s, path_length);

	memcpy (stream_pointer (s), path.c_str(), path_length);
	stream_set_length (s, stream_tell(s) + path_length);
	stream_seek (s, stream_tell(s) + path_length);

	stream_write_uint16_t (s, status_code);

	/* Send the message */
	c->send (s);
}

void daemon::receive_message_release_lock (Connection *c, struct stream *input, uint32_t length)
{
	uint32_t pid = stream_read_uint32_t (input);
	uint16_t path_length = stream_read_uint16_t (input);
	string path ((char*) stream_pointer (input), path_length);
	stream_seek (input, stream_tell(input) + path_length);
	uint8_t mode = stream_read_uint8_t (input);

	/* Create new stream for sending something */
	auto s = stream_new ();
	if (!s)
		return;

	if (stream_ensure_remaining_capacity (s, stream_capacity (input) + 2) != 0)
	{
		stream_free (s);
		return;
	}

	/* Cannot fail because the stream has enough capacity */
	write_message_header (s, MSG_ID_RELEASE_LOCK_RESPONSE, stream_length (input) + 2 - 5);

	stream_seek (input, 5);
	memcpy (stream_pointer(s), stream_pointer(input), stream_remaining_length (input));
	stream_set_length (s, stream_length (input));
	stream_seek (s, stream_length (input));

	/* Call backend */
	stream_write_uint16_t (s, b.release_lock (pid, &path, mode));

	/* Send the message */
	c->send (s);
}
