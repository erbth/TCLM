#ifndef __CONNECTION_H
#define __CONNECTION_H

/* This file belongs to the communications unit. */

#include "polled_fd.h"
#include "stream.h"
#include "ringbuffer.h"
#include <mutex>
#include <queue>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
}

class Connection : public polled_fd
{
public:
	typedef void (*receive_callback_t) (Connection *c, struct stream *s, void *data);

protected:
	/* Sending data */
	std::mutex m_send;
	std::queue<struct stream*> send_queue;

	/* Receiving data */
	std::mutex m_receive;
	ringbuffer receive_buffer;

	/* 0 means unknown because each message has a header of at least 5 bytes. */
	uint32_t wanted_ib_size = 0;

	/* Callback functions for various events. */
	/* The receive callback is expected to take ownership of the given stream.
	 * Hence it is responsible for destroying it. */
	receive_callback_t receive_callback = nullptr;
	void *receive_callback_data = nullptr;

public:
	/* Connections swallow the given fd. */
	Connection (int fd);

	/* There is a destructor; this is just to make the class pure virtual. */
	virtual ~Connection () = 0;

	void set_receive_callback (receive_callback_t cb, void *data);

	/* For use by a Communications Manager or other epoll-like backend */
	bool data_in () override;
	bool data_out () override;

	/* For use by the application which wants to communicate. */
	/* Swallows the stream that is takes ownership of it and cares for destroying
	 * it only if no exception is thrown. */
	void send (struct stream *s);
};

#endif /* __CONNECTION_H */
