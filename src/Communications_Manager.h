#ifndef __FD_MANAGER_H
#define __FD_MANAGER_H

/* This module belongs to the communications unit. It is responsible for
 * managing connections and especially polling them for data. */
#include "polled_fd.h"
#include <set>
#include <functional>

/* Prototypes for cyclic include-dependencies */
class polled_fd;

class Communications_Manager
{
protected:
	std::set<polled_fd*> polled_fds;
	int epfd = -1;

	/* For exiting the main loop */
	bool quit_requested = false;

public:
	~Communications_Manager ();

	void for_each_pfd (std::function<void(const polled_fd*)> f);

	/* Returns true if everything succeeded, otherwise false. */
	bool main_loop ();
	void request_quit ();

	/* Only needed if quit was requested before. The main loop does not reset
	 * the quit request switch on exit to avoid race conditions were multiple
	 * other threads (in a futural mt-safe version of this) request quit
	 * simultaniously.
	 * It is not needed to call this method initially. */
	void request_run ();

	/* Returns true if adding was successful. If it was not, the given
	 * connection was not destroyed. */
	bool add_polled_fd (polled_fd *pfd);

	/* This removes and destroys the given polled fd. */
	void destroy_polled_fd (polled_fd *pfd);

	bool fd_enable_out (polled_fd *pfd);
	bool fd_disable_out (polled_fd *pfd);
};

#endif /* __FD_MANAGER_H */
