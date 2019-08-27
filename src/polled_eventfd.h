#ifndef __POLLED_EVENTFD_H
#define __POLLED_EVENTFD_H

#include "polled_fd.h"

class polled_eventfd : public polled_fd
{
	typedef void (*read_callback_t) (polled_eventfd *pefd, uint64_t val, void *data);

protected:
	read_callback_t read_callback = nullptr;
	void *read_callback_data;

public:
	polled_eventfd ();

	bool data_in () override;
	bool data_out () override;

	/* For the user */
	void write (uint64_t val);
	void set_read_callback (read_callback_t cb, void *data);
};

#endif /* __POLLED_EVENTFD_H */
