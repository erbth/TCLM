#include "TCP_Listener.h"
#include "TCP_Connection.h"
#include "daemon.h"
#include "messages.h"
#include "message_utils.h"
#include <iostream>

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
		auto tcpl = new TCP_Listener();
		tcpl->set_daemon (this);

		if (!m.add_polled_fd (tcpl))
		{
			delete tcpl;
			return false;
		}
	} catch (exception &e) {
		cerr << "Failed to create TCP Listener: " << e.what() << endl;
		return false;
	}

	return m.main_loop ();
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
	}

	stream_free (s);
}

/* Message handlers */
void daemon::receive_message_list_connections (Connection *c, struct stream *input, uint32_t length)
{
	/* Create a new stream */
	auto s = stream_new();
	if (!s)
	{
		cerr << "Failed to create a new stream." << endl;
		return;
	}

	/* Send a list of connections back */
	write_message_header (s, MSG_ID_LIST_CONNS_RESPONSE, 0);

	m.for_each_pfd ([s](const polled_fd *pfd){
			auto tcp_conn = dynamic_cast<const TCP_Connection*>(pfd);
			if (tcp_conn)
			{
				auto addr = tcp_conn->get_sockaddr();

				stream_write_uint8_t (s,0);
				stream_write_uint32_t (s,ntohl(addr->sin_addr.s_addr));
				stream_write_uint16_t (s,ntohs(addr->sin_port));
			}
		});

	/* Update the length of the stream */
	update_message_length (s, stream_tell(s));

	/* Send the stream */
	c->send(s);
}
