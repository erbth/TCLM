#ifndef __POLLED_EVENTFD_H
#define __POLLED_EVENTFD_H

#include "polled_fd.h"

class polled_eventfd : public polled_fd
{
	typedef void (*read_callback_t) (std::shared_ptr<polled_eventfd> pefd, uint64_t val, void *data);

protected:
	read_callback_t read_callback = nullptr;
	void *read_callback_data;

	polled_eventfd ();

public:
	static std::shared_ptr<polled_eventfd> create ();

	bool data_in () override;
	bool data_out () override;

	/* For the user */
	void write (uint64_t val);
	void set_read_callback (read_callback_t cb, void *data);
};

#endif /* __POLLED_EVENTFD_H */
