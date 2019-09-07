#include "Communications_Manager.h"
#include "TCP_Connection.h"
#include "argument_parser.h"
#include "messages.h"
#include "message_utils.h"
#include "stream.h"
#include "tclm_config.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>
#include <iostream>
#include <memory>

extern "C" {
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace std;
using namespace manager;

/* Handlers for the messages sent by the server */
void receive_list_conns_response (shared_ptr<Connection> c, struct stream *s, argument_parser *ap, uint32_t length)
{
	if (ap->action == "list-connections")
	{
		printf ("Connections to tclmd:\n"
				"-------------------------------------------------------\n");

		while (stream_tell(s) < length)
		{
			uint8_t addr_type = stream_read_uint8_t(s);

			switch (addr_type)
			{
				/* TCP over IPv4 */
				case 0:
					if (stream_tell(s) + 6 <= length)
					{
						int a = stream_read_uint8_t(s);
						int b = stream_read_uint8_t(s);
						int c = stream_read_uint8_t(s);
						int d = stream_read_uint8_t(s);
						int port = stream_read_uint16_t(s);

						printf ("%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 ":%" PRIu16 "\n", a,b,c,d, port);
					}
					break;
			}
		}

		c->get_mgr()->request_quit();
	}
}

void receive_list_processes_response (shared_ptr<Connection> c, struct stream *s, argument_parser *ap, uint32_t length)
{
	if (ap->action == "list-processes")
	{
		printf ("Registered Processes:\n"
				"    id               lock_count\n"
				"-------------------------------------------------------\n");

		while (stream_tell(s) < length)
		{
			uint32_t id = stream_read_uint32_t(s);
			uint32_t lock_count = stream_read_uint32_t(s);

			printf ("%9" PRIu32 "             %9" PRIu32 "\n", id, lock_count);
		}

		c->get_mgr()->request_quit();
	}
}

void receive_list_locks_response (shared_ptr<Connection> c, struct stream *s, argument_parser *ap, uint32_t length)
{
	if (ap->action == "list-locks")
	{
		printf ("Created Locks:\n"
				"name:status\n"
				"-------------------------------------------------------\n");

		while (stream_tell(s) < length)
		{
			uint32_t level = stream_read_uint32_t (s);
			uint16_t strlength = stream_read_uint16_t (s);
			string name((char*) stream_pointer(s), strlength);
			stream_seek (s, stream_tell(s) + strlength);
			uint8_t status_bits = stream_read_uint8_t (s);

			string indent;
			for (uint32_t i = 0; i < level; i++)
				indent += "  ";

			string status =
					string(status_bits & 0x4 ? "S" : " ")
					+ (status_bits & 0x2 ? "S+" : " ")
					+ (status_bits & 0x1 ? "X" : " ");

			printf ("%s%s:%s\n", indent.c_str(), name.c_str(), status.c_str());
		}

		c->get_mgr()->request_quit();
	}
}

/* Receive a message */
void receive_message (shared_ptr<Connection> c, struct stream *s, void *data)
{
	auto length = stream_length (s);
	auto ap = (argument_parser*) data;

	uint8_t id = stream_read_uint8_t (s);
	stream_read_uint32_t (s);

	switch (id)
	{
		case MSG_ID_LIST_CONNS_RESPONSE:
			receive_list_conns_response (c, s, ap, length);
			break;

		case MSG_ID_LIST_PROCS_RESPONSE:
			receive_list_processes_response (c, s, ap, length);
			break;

		case MSG_ID_LIST_LOCKS_RESPONSE:
			receive_list_locks_response (c, s, ap, length);
			break;
	}

	stream_free (s);
}


/* A help message for the user's pleasure. */
void print_help_message ()
{
	cout << "\ntclm ... <parameters> ... <action> ... <parameters> ... <names> ...\n"

		<< "\nActions:\n"
		<< "    list-connections:    List all connections to the deamon including this one.\n"
		<< "    list-processes:      List the registered processes with their lock_counts.\n"
		<< "    list-locks:          List all locks and their status.\n"

		<< "\nParameters:\n"
		<< "    -h, --help:          Do nothing but print a help message (this one).\n"

		<< endl;
}

int main (int argc, char** argv)
{
	printf ("TCLM Manager for tclmd version %d.%d\n\n", SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);

	argument_parser ap;
	if (!ap.parse (argc, argv))
		return EXIT_FAILURE;

	if (find (ap.long_parameters.begin(), ap.long_parameters.end(), "help") != ap.long_parameters.end() ||
			find (ap.short_parameters.begin(), ap.short_parameters.end(), 'h') != ap.short_parameters.end())
	{
		print_help_message ();
		return EXIT_FAILURE;
	}

	string server_name = "127.0.0.1";

	if (ap.action == "list-connections")
	{}
	else if (ap.action == "list-processes")
	{}
	else if (ap.action == "list-locks")
	{}
	else if (ap.action.size() == 0)
	{
		cerr << "No action specified." << endl;
		return EXIT_FAILURE;
	}
	else
	{
		cerr << "Unknown action `" << ap.action << "'." << endl;
		return EXIT_FAILURE;
	}

	/* Open a connection */
	Communications_Manager m;

	{
		shared_ptr<Connection> c;

		struct addrinfo gai_hints = { 0 };
		gai_hints.ai_family = AF_UNSPEC;
		gai_hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *ais = nullptr;

		if (getaddrinfo (server_name.c_str(), nullptr, &gai_hints, &ais) == 0)
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
						((struct sockaddr_in*) addr)->sin_port = htons (SERVER_TCP_PORT);
					else
						((struct sockaddr_in6*) addr)->sin6_port = htons (SERVER_TCP_PORT);

					/* Try to connect */
					int fd = socket (ai->ai_family, ai->ai_socktype, 0);
					if (fd)
					{
						if (connect (fd, addr, addrlen) == 0)
						{
							if (ai->ai_family == AF_INET)
							{
								c = TCP_Connection::create (fd, (const sockaddr_in*) addr);
								break;
							}
							else
								close (fd);
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

		if (!c)
		{
			cerr << "Cannot connect to host \"" << server_name << "\"." << endl;
			return EXIT_FAILURE;
		}

		c->set_receive_callback (receive_message, (void*) &ap);

		/* Query thing */
		auto s = stream_new ();
		if (!s)
		{
			cerr << "Out of memory." << endl;
			return EXIT_FAILURE;
		}

		if (ap.action == "list-connections")
		{
			if (stream_ensure_remaining_capacity (s, 5) < 0)
			{
				cerr << "Out of memory." << endl;
				return EXIT_FAILURE;
			}

			stream_write_uint8_t (s, MSG_ID_LIST_CONNS);
			stream_write_uint32_t (s, 0);

			c->send (s);
		}
		else if (ap.action == "list-processes")
		{
			if (!write_message_header (s, MSG_ID_LIST_PROCS, 0))
			{
				cerr << "Out of memory." << endl;
				return EXIT_FAILURE;
			}
			c->send (s);
		}
		else if (ap.action == "list-locks")
		{
			if (!write_message_header (s, MSG_ID_LIST_LOCKS, 0))
			{
				cerr << "Out of memory." << endl;
				return EXIT_FAILURE;
			}
			c->send(s);
		}

		/* Add the Connection afterwards to the Connection Manager because from that
		 * point it it will determine the Connections lifetime (i.e. the Connection
		 * can be closed). */
		if (!m.add_polled_fd (c))
		{
			cerr << "Failed to add Connection to a Connection Manager." << endl;
			return EXIT_FAILURE;
		}
	}

	/* Mainloop */
	m.main_loop ();

	/* Exit */
	return EXIT_SUCCESS;
}
