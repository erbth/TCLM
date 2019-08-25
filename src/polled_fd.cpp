/* This file belongs to the communications unit. */
#include "polled_fd.h"

extern "C" {
#include <unistd.h>
}

using namespace std;

polled_fd::polled_fd (int fd, bool close_fd) :
	fd(fd), close_fd(close_fd), out_enabled(false), mgr(nullptr)
{
}

polled_fd::~polled_fd ()
{
	if (close_fd && fd >= 0)
		close (fd);
}

const int polled_fd::get_fd ()
{
	return fd;
}

bool polled_fd::get_out_enabled ()
{
	return out_enabled;
}

Communications_Manager *polled_fd::get_mgr ()
{
	return mgr;
}

void polled_fd::set_out_enabled (bool enabled)
{
	this->out_enabled = enabled;
}

void polled_fd::set_mgr (Communications_Manager *mgr)
{
	this->mgr = mgr;
}
