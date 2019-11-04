/* This file belongs to the communications unit. */
#include "Communications_Manager.h"
#include <cerrno>
#include <exception>
#include <iostream>
#include <memory>

extern "C" {
#include <sys/epoll.h>
#include <unistd.h>
}

using namespace std;

Communications_Manager::~Communications_Manager ()
{
	if (epfd >= 0)
		close (epfd);

	polled_fds.clear ();
}

void Communications_Manager::for_each_pfd (std::function<void(const polled_fd*)> f)
{
	lock_guard lk(m_polled_fds);

	for (auto i = polled_fds.cbegin(); i != polled_fds.cend(); i++)
		f (i->first);
}

void Communications_Manager::pefd_read_callback (shared_ptr<polled_eventfd> pefd, uint64_t val, void *data)
{
	/* Reset the eventfd */
	pefd->write (0);
}

bool Communications_Manager::main_loop ()
{
	unique_lock elk(m_epfd);
	bool return_state = true;

	/* Create an epoll instance */
	if (epfd < 0)
	{
		epfd = epoll_create (8);
		if (epfd < 0)
		{
			cerr << "Failed to create epoll instance." << endl;
			return false;
		}

		struct epoll_event event = { 0 };

		{
			lock_guard plk(m_polled_fds);

			for (auto i = polled_fds.begin(); i != polled_fds.end(); i++)
			{
				auto pfd = i->first;

				event.data.ptr = pfd;
				event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

				if (pfd->get_out_enabled ())
					event.events |= EPOLLOUT;

				if (epoll_ctl (epfd, EPOLL_CTL_ADD, pfd->get_fd(), &event) < 0)
				{
					cerr << "Failed to add a fd to the epoll instance." << endl;
					close (epfd);
					epfd = -1;
					return false;
				}
			}
		}
	}

	/* If it does not exist yet, create a polled eventfd. */
	if (!pefd)
	{
		pefd = polled_eventfd::create();
		pefd->set_read_callback (pefd_read_callback, this);
		add_polled_fd (pefd);
	}

	/* Poll the epoll instance in a loop */
	struct epoll_event event = { 0 };

	elk.unlock();

	for (;;)
	{
		{
			lock_guard flk(m_flags);
			if (quit_requested)
				break;
		}

		auto r = epoll_wait (epfd, &event, 1, -1);

		if (r < 0 && errno != EINTR)
		{
			break;
		}
		else if (r > 0)
		{
			auto pfd = (polled_fd*) event.data.ptr;

			/* The first cascade may destroy the polled fd already (e.g. if
			 * there is a read error). */
			if (event.events & EPOLLIN)
			{
				if (!pfd->data_in ())
				{
					destroy_polled_fd (pfd);
					pfd = nullptr;
				}
			}
			if (event.events & EPOLLOUT && pfd)
			{
				if (!pfd->data_out ())
				{
					destroy_polled_fd (pfd);
					pfd = nullptr;
				}
			}
			if (event.events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP) && pfd)
			{
				destroy_polled_fd (pfd);
				pfd = nullptr;
			}
		}
	}

	return return_state;
}

void Communications_Manager::request_quit ()
{
	lock_guard flk(m_flags);
	lock_guard elk(m_epfd);

	if (pefd)
		pefd->write(1);

	quit_requested = true;
}

void Communications_Manager::request_run ()
{
	lock_guard lk(m_flags);
	quit_requested = false;
}

bool Communications_Manager::add_polled_fd (shared_ptr<polled_fd> pfd)
{
	lock_guard elk(m_epfd);
	lock_guard plk(m_polled_fds);

	if (epfd >= 0)
	{
		struct epoll_event event = { 0 };
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;

		if (pfd->get_out_enabled ())
			event.events |= EPOLLOUT;

		event.data.ptr = (void*) pfd.get();

		if (epoll_ctl (epfd, EPOLL_CTL_ADD, pfd->get_fd(), &event) < 0)
		{
			cerr << "Failed to add a fd to the epoll instance." << endl;
			return false;
		}
	}

	polled_fds.insert (pair (pfd.get(), pfd));
	pfd->set_mgr (this);
	return true;
}

void Communications_Manager::destroy_polled_fd (polled_fd *pfd)
{
	lock_guard elk(m_epfd);
	lock_guard plk(m_polled_fds);

	if (epfd >= 0)
	{
		if (epoll_ctl (epfd, EPOLL_CTL_DEL, pfd->get_fd(), nullptr) < 0)
			cerr << "Failed to remove fd from the epoll instance.";
	}

	polled_fds.erase (pfd);
}

bool Communications_Manager::fd_enable_out (polled_fd *pfd)
{
	lock_guard elk(m_epfd);

	pfd->set_out_enabled (true);

	if (epfd >= 0)
	{
		struct epoll_event event = { 0 };
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR | EPOLLOUT;
		event.data.ptr = (void*) pfd;

		if (epoll_ctl (epfd, EPOLL_CTL_MOD, pfd->get_fd(), &event) < 0)
		{
			cerr << "Failed to add a fd to the epoll instance." << endl;
			return false;
		}
	}

	return true;
}

bool Communications_Manager::fd_disable_out (polled_fd *pfd)
{
	lock_guard elk(m_epfd);

	pfd->set_out_enabled (false);

	if (epfd >= 0)
	{
		struct epoll_event event = { 0 };
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
		event.data.ptr = (void*) pfd;

		if (epoll_ctl (epfd, EPOLL_CTL_MOD, pfd->get_fd(), &event) < 0)
		{
			cerr << "Failed to add a fd to the epoll instance." << endl;
			return false;
		}
	}

	return true;
}
