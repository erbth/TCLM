#include "Communications_Manager.h"
#include "TCP_Connection.h"
#include "argument_parser.h"
#include "messages.h"
#include "stream.h"
#include "tclm_config.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>

extern "C" {
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}

using namespace std;
using namespace manager;

/* Handlers for the messages sent by the server */
void receive_message_list_conns_response (Connection *c, struct stream *s, argument_parser *ap, uint32_t length)
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

						printf ("%d.%d.%d.%d:%d\n", a,b,c,d, port);
					}
					break;
			}
		}

		c->get_mgr()->request_quit();
	}
}

/* Receive a message */
void receive_message (Connection *c, struct stream *s, void *data)
{
	auto length = stream_length (s);
	auto ap = (argument_parser*) data;

	if (length >= 5)
	{
		uint8_t id = stream_read_uint8_t (s);
		uint8_t p_length = stream_read_uint32_t (s);

		if (length == p_length + 5)
		{
			switch (id)
			{
				case MSG_ID_LIST_CONNS_RESPONSE:
					receive_message_list_conns_response (c, s, ap, length);
					break;
			}
		}
	}

	stream_free (s);
}


/* A help message for the user's pleasure. */
void print_help_message ()
{
	cout << "\ntclm ... <parameters> ... <action> ... <parameters> ... <names> ...\n"

		<< "\nActions:\n"
		<< "    list-connections:    List all connections to the deamon including this one.\n"

		<< "\nParameters:\n"
		<< "    -h, --help:          Do nothing but print a help message (this one).\n"

		<< endl;
}

int main (int argc, char** argv)
{
	printf ("TCLM Manager for tclmd version %d.%d\n", SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR);

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
	{
	}
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
		Connection *c = nullptr;

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
								c = new TCP_Connection (fd, (const sockaddr_in*) addr);
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
			delete c;
			return EXIT_FAILURE;
		}

		if (ap.action == "list-connections")
		{
			if (stream_ensure_remaining_capacity (s, 5) < 0)
			{
				cerr << "Out of memory." << endl;
				delete c;
				return EXIT_FAILURE;
			}

			stream_write_uint8_t (s, 7);
			stream_write_uint32_t (s, 0);

			c->send (s);
		}

		/* Add the Connection afterwards to the Connection Manager because from that
		 * point it it will determine the Connections lifetime (i.e. the Connection
		 * can be closed). */
		if (!m.add_polled_fd (c))
		{
			cerr << "Failed to add Connection to a Connection Manager." << endl;
			delete c;
			return EXIT_FAILURE;
		}
	}

	/* Mainloop */
	m.main_loop ();

	/* Exit */
	return EXIT_SUCCESS;
}
