#include "polled_eventfd.h"
#include "errno_exception.h"

extern "C" {
#include <sys/eventfd.h>
#include <unistd.h>
}

using namespace std;

polled_eventfd::polled_eventfd ()
	: polled_fd (-1, true)
{
	fd = eventfd (0, 0);
	if (fd < 0)
		throw errno_exception (errno);
}

bool polled_eventfd::data_in ()
{
	uint64_t val;
	read_callback_t cb;
	void *cb_data;

	{
		lock_guard lk(m);
		if (read (fd, (void*) &val, 8) != 8)
			throw errno_exception (errno);

		cb = read_callback;
		cb_data = read_callback_data;
	}

	/* Call the specified callback */
	if (cb)
		cb (this, val, cb_data);

	return true;
}

bool polled_eventfd::data_out ()
{
	throw "polled_eventfd::data_out must not be used.";
}

void polled_eventfd::write (uint64_t val)
{
	lock_guard lk(m);

	if (::write (fd, (void*) &val, 8) != 8)
		throw errno_exception (errno);
}

void polled_eventfd::set_read_callback (read_callback_t cb, void *data)
{
	lock_guard lk(m);
	read_callback = cb;
	read_callback_data = data;
}
