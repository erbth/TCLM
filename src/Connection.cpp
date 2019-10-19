/* This file belongs to the communications unit. */

#include "Connection.h"
#include <cstdio>

extern "C" {
#include <errno.h>
#include <unistd.h>
}

using namespace std;

Connection::Connection (int fd) :
	polled_fd (fd, close_fd=true)
{
}

Connection::~Connection ()
{
	/* Free left over send streams */
	while (send_queue.size() > 0)
	{
		stream_free (send_queue.front());
		send_queue.pop();
	}
}


void Connection::set_receive_callback (receive_callback_t cb, void *data)
{
	lock_guard lk(m_receive);

	receive_callback = cb;
	receive_callback_data = data;
}


bool Connection::data_in ()
{
	uint8_t buf[10000];

	/* Read from the fd. This requires a lock on the fd. */
	ssize_t count;
	{
		lock_guard glk(m);
		count = read (fd, buf, sizeof(buf) / sizeof(*buf));
	}

	/* Afterwards, lock the receive part of the Connection */
	unique_lock rlk(m_receive);

	if (count > 0)
	{
		/* Append the data to the ringbuffer */
		receive_buffer.write (buf, count);

		/* See if a whole message came in yet and maybe call the receive
		 * callback. */
		if (wanted_ib_size == 0 && receive_buffer.size() >= 5)
		{
			/* Extract the message header */
			struct stream *s = stream_new ();

			if (s)
			{
				if (stream_ensure_remaining_capacity (s, 5) == 0)
				{
					receive_buffer.peek (stream_pointer (s), 5);
					stream_set_length (s, 5);

					stream_read_uint8_t (s);
					wanted_ib_size = 5 + stream_read_uint32_t (s);

					if (receive_buffer.size() >= wanted_ib_size)
					{
						if (receive_callback)
						{
							if (stream_ensure_remaining_capacity (s, wanted_ib_size - 5) == 0)
							{
								receive_buffer.remove(5);
								receive_buffer.read (stream_pointer(s), wanted_ib_size - 5);
								stream_seek(s,0);
								stream_set_length(s,wanted_ib_size);

								auto cb = receive_callback;
								auto cbd = receive_callback_data;

								rlk.unlock();
								cb (static_pointer_cast<Connection>(shared_from_this()), s, cbd);
								rlk.lock();

								s = nullptr;
							}
						}
						else
							receive_buffer.remove (wanted_ib_size);

						// Reset the wanted size to read the next message
						wanted_ib_size = 0;
					}
				}

				if (s)
					stream_free (s);
			}
		}
		else if (receive_buffer.size() >= wanted_ib_size)
		{
			if (receive_callback)
			{
				struct stream *s = stream_new ();

				if (s)
				{
					if (stream_ensure_remaining_capacity (s, wanted_ib_size) == 0)
					{
						receive_buffer.read (stream_pointer(s), wanted_ib_size);
						stream_set_length (s, wanted_ib_size);

						auto cb = receive_callback;
						auto cbd = receive_callback_data;

						rlk.unlock();
						cb (static_pointer_cast<Connection>(shared_from_this()), s, cbd);
						rlk.lock();
					}
					else
						stream_free (s);
				}
			}
			else
				receive_buffer.remove (wanted_ib_size);

			// Reset the wanted size to read the next message
			wanted_ib_size = 0;
		}
	}
	else if (count == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
	{
		/* Close the connection */
		return false;
	}

	return true;
}

bool Connection::data_out ()
{
	lock_guard slk(m_send);
	auto s = send_queue.front();

	ssize_t count;
	{
		lock_guard glk(m);
		count = write (fd, stream_pointer (s), stream_remaining_length (s));
	}

	if (count > 0)
	{
		stream_seek (s, stream_tell(s) + count);

		if (stream_remaining_length(s) == 0)
		{
			send_queue.pop();
			stream_free (s);

			if (send_queue.size() == 0)
			{
				lock_guard glk(m);
				mgr->fd_disable_out (this);
			}
		}
	}
	else if (count == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
	{
		/* Close the connection */
		return false;
	}

	return true;
}


void Connection::send (struct stream *s)
{
	stream_seek (s, 0);

	{
		lock_guard slk(m_send);
		send_queue.push (s);
	}

	lock_guard glk(m);

	if (!out_enabled)
	{
		/* If a Communications Manager is available, ask it else do it yourself.
		 * The Communications Manager will pick it up once you are added to one. */
		if (mgr)
			mgr->fd_enable_out (this);
		else
			out_enabled = true;
	}
}
