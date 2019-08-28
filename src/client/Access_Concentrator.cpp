#include "Access_Concentrator.h"
#include "tclm_client_exceptions.hpp"
#include "TCP_Connection.h"
#include "stream.h"
#include "messages.h"
#include "message_utils.h"
#include <iostream>
#include <new>

extern "C" {
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace std;
using namespace tclm_client;

Access_Concentrator::Access_Concentrator (
		const std::string &servername,
		const uint16_t tcp_port,
		const uint16_t udp_port) :
	servername(servername), tcp_port(tcp_port), udp_port(udp_port)
{
	/* If the first attempt to connect to a server doesn't succeed, it's
	 * ridiculous. */
	if (!create_tcp_connection())
		throw cannot_connect_exception (servername, tcp_port, "tcp");

	/* Create a thread for the Communications Manager */
	cm_thread = thread (cm_thread_func, this);
}

Access_Concentrator::~Access_Concentrator ()
{
	/* Stop the Communications Manager's thread */
	cm.request_quit();
	cm_thread.join();
}

bool Access_Concentrator::create_tcp_connection() noexcept
{
	lock_guard clk(m_tcp_connection);
	lock_guard slk(m_servername);

	if (tcp_connection)
		return true;

	struct addrinfo gai_hints = { 0 };
	gai_hints.ai_family = AF_UNSPEC;
	gai_hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *ais = nullptr;

	if (getaddrinfo (servername.c_str(), nullptr, &gai_hints, &ais) == 0)
	{
		struct addrinfo *ai = ais;

		while (ai)
		{
			auto addr = ai->ai_addr;
			auto addrlen = ai->ai_addrlen;

			if (ai->ai_family == AF_INET || ai->ai_family == AF_INET6)
			{
				/* Add a port */
				if (ai->ai_family == AF_INET)
					((struct sockaddr_in*) addr)->sin_port = htons (tcp_port);
				else
					((struct sockaddr_in6*) addr)->sin6_port = htons (tcp_port);

				/* Try to connect */
				int fd = socket (ai->ai_family, ai->ai_socktype, 0);
				if (fd)
				{
					if (connect (fd, addr, addrlen) == 0)
					{
						if (ai->ai_family == AF_INET)
						{
							try {
								tcp_connection = make_shared<TCP_Connection>(fd, (const sockaddr_in*) addr);
								break;
							} catch (...) {
								close (fd);
							}
						}
						else
							close (fd);

						break;
					}
					else
						close (fd);
				}
			}

			ai = ai->ai_next;
		}

		freeaddrinfo (ais);
		ais = nullptr;
	}

	if (tcp_connection)
	{
		tcp_connection->set_receive_callback (receive_message_tcp, this);

		if (!cm.add_polled_fd (tcp_connection))
			tcp_connection.reset();
	}

	return tcp_connection ? true : false;
}

bool Access_Concentrator::send_message_tcp (struct stream *s)
{
	lock_guard lk(m_tcp_connection);
	if (tcp_connection)
	{
		try {
			tcp_connection->send(s);
		} catch(bad_alloc) {
			return false;
		} catch(...) {
			stream_free (s);
			throw;
		}

		return true;
	}

	return false;
}

bool Access_Concentrator::send_message_auto (struct stream *s)
{
	bool ret = send_message_tcp (s);

	if (!ret)
		stream_free (s);

	return ret;
}

void Access_Concentrator::receive_message_tcp (Connection *c, struct stream *s, void *data)
{
	auto pThis = (Access_Concentrator*) data;
	pThis->receive_message_tcp_internal (c, s);
}

void Access_Concentrator::receive_message_tcp_internal (Connection *c, struct stream *s)
{
	uint8_t id = stream_read_uint8_t (s);
	uint32_t length = stream_read_uint32_t (s);

	switch (id)
	{
		case MSG_ID_REG_PROC_RESPONSE:
			receive_register_process_response (c, s, length);
			break;

		case MSG_ID_UNREG_PROC_RESPONSE:
			receive_unregister_process_response (c, s, length);
			break;
	}

	stream_free (s);
}


/* A Communications Manager in a separate thread */
void Access_Concentrator::cm_thread_func_internal ()
{
	cm.main_loop ();
}

void Access_Concentrator::cm_thread_func (void *data)
{
	auto pThis = (Access_Concentrator*) data;
	pThis->cm_thread_func_internal ();
}


/* Issue requests */
void Access_Concentrator::issue_register_process_request (register_process_request *r)
{
	/* Find a nonce for the request */
	{
		lock_guard nlk(m_register_process_next_nonce);

		r->set_nonce (register_process_next_nonce);

		if (register_process_next_nonce == numeric_limits<uint32_t>::max())
			register_process_next_nonce = 0;
		else
			register_process_next_nonce++;
	}

	/* Add the request to the list */
	{
		lock_guard vlk(m_register_process_requests);
		register_process_requests.push_back (r);
	}

	/* Send the message. The function handles failures appropriately */
	send_register_process_request (r);
}

void Access_Concentrator::issue_unregister_process_request (unregister_process_request *r)
{
	/* Add the request to the list */
	{
		lock_guard vlk(m_unregister_process_requests);
		unregister_process_requests.push_back(r);
	}

	/* Send the message */
	send_unregister_process_request (r);
}


/* Send messages */
void Access_Concentrator::send_register_process_request (register_process_request *r)
{
	/* Construct a message */
	auto s = stream_new();
	if (!s)
		return;

	if (stream_ensure_remaining_capacity (s, 9) != 0)
	{
		stream_free (s);
		return;
	}

	/* Cannot fail because the stream has enough capacity. */
	write_message_header (s, MSG_ID_REG_PROC, 4);
	stream_write_uint32_t (s, r->get_nonce());

	send_message_auto (s);
}

void Access_Concentrator::send_unregister_process_request (unregister_process_request *r)
{
	/* Construct a message */
	auto s = stream_new();
	if (!s)
		return;

	if (stream_ensure_remaining_capacity (s,9) != 0)
	{
		stream_free (s);
		return;
	}

	/* Cannot fail because the stream has enough capacity. */
	write_message_header (s, MSG_ID_UNREG_PROC, 4);
	stream_write_uint32_t (s, r->get_id());

	send_message_auto (s);
}

/* Receive messages */
void Access_Concentrator::receive_register_process_response (Connection *c, struct stream *s, uint32_t length)
{
	uint32_t nonce = stream_read_uint32_t (s);
	uint16_t status_code = stream_read_uint16_t (s);

	{
		lock_guard lk(m_register_process_requests);

		/* Find the corresponding request */
		for (auto i = register_process_requests.begin(); i != register_process_requests.end(); i++)
		{
			auto r = *i;

			if (r->get_nonce() == nonce)
			{
				/* Answer it */
				register_process_requests.erase (i);

				if (status_code == RESPONSE_STATUS_SUCCESS && stream_remaining_length (s) >= 4)
					r->set_id(stream_read_uint32_t (s));

				r->answer (status_code);
				return;
			}
		}
	}
}

void Access_Concentrator::receive_unregister_process_response (Connection *c, struct stream *s, uint32_t length)
{
	uint32_t id = stream_read_uint32_t(s);
	uint16_t status_code = stream_read_uint16_t(s);

	{
		lock_guard lk(m_unregister_process_requests);

		/* Find the corresponding request */
		for (auto i = unregister_process_requests.begin(); i != unregister_process_requests.end(); i++)
		{
			auto r = *i;

			if (r->get_id() == id)
			{
				/* Answer it */
				unregister_process_requests.erase (i);
				r->answer (status_code);
				return;
			}
		}
	}
}
