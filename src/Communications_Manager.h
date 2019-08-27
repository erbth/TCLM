#ifndef __COMMUNICATIONS_MANAGER_H
#define __COMMUNICATIONS_MANAGER_H

/* This module belongs to the communications unit. It is responsible for
 * managing connections and especially polling them for data. */
#include <map>
#include <functional>
#include <memory>
#include <mutex>

/* Prototypes for cyclic include-dependencies */
class polled_fd;
class polled_eventfd;

class Communications_Manager
{
protected:
	std::recursive_mutex m_epfd;
	int epfd = -1;

	/* This polled eventfd can wakeup the main loop's epoll_wait. */
	std::shared_ptr<polled_eventfd> pefd;

	static void pefd_read_callback (polled_eventfd *pefd, uint64_t val, void *data);

	std::recursive_mutex m_polled_fds;
	std::map<polled_fd*, std::shared_ptr<polled_fd>> polled_fds;

	/* For exiting the main loop */
	std::mutex m_flags;
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
	 * It is not necessary to call this method initially. */
	void request_run ();

	/* Returns true if adding was successful. If it was not, the given
	 * connection was not destroyed. */
	bool add_polled_fd (std::shared_ptr<polled_fd> pfd);

	/* This removes and destroys the given polled fd. */
	void destroy_polled_fd (polled_fd *pfd);

	bool fd_enable_out (polled_fd *pfd);
	bool fd_disable_out (polled_fd *pfd);
};

/* Carefully placed includes */
#include "polled_fd.h"
#include "polled_eventfd.h"

#endif /* __COMMUNICATIONS_MANAGER_H */
