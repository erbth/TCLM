#include "Lock_Request.h"
#include "Process.h"
#include "backend_exceptions.h"
#include <thread>

using namespace std;
using namespace server;

Process::Process (const uint32_t id) : id(id)
{
}

Process::~Process ()
{
	/* Notify the Lock Requests referencing us. */
	/* The requests may try to inform as right now. Hence they try to lock their
	 * lock and then our lock through remove_lock_request. We need to use a
	 * deadlock avoidence strategy ...
	 *
	 * However it is guarantied that no new Lock Requests are added now. */
	unique_lock lk(m);

	while (lock_requests.size() > 0)
	{
		auto w = lock_requests.begin()->second;
		lk.unlock();

		/* Lock requests may try to remove themselves from the list at any point. */
		auto r = w.lock();
		if (r)
		{
			/* If we come here, the Lock Requests does still exist, the
			 * destructor was not invoked yet and it won't try to remove itself
			 * from our list anymore as we tell it so. */
			r->set_requester_destroyed ();
		}
		else
		{
			/* If we get here, the Lock Request was destroyed and therefore
			 * removed itself from our list already or will do so in a moment.
			 * To ease that free the cpu. */
			std::this_thread::yield();
		}

		lk.lock();
	}

	/* Every other destruction operation must be done below, as a Lock Request
	 * could hold the set_process_destroyed call and therefore we must still
	 * fullfil all invariants. */
	lk.unlock();
}

const uint32_t Process::get_id () const
{
	// It may be required for memory synchronization between different cores
	lock_guard lk(m);
	return id;
}

void Process::add_lock_request (shared_ptr<Lock_Request> r)
{
	scoped_lock lk(m);
	lock_requests.insert(pair(r.get(),r));
}

void Process::remove_lock_request (Lock_Request *r)
{
	scoped_lock lk(m);
	lock_requests.erase(r);
}

weak_ptr<TCP_Connection> Process::get_tcp_conn() const
{
	scoped_lock lk(m);
	return tcp_conn;
}

void Process::update_tcp_conn (weak_ptr<TCP_Connection> conn)
{
	scoped_lock lk(m);
	tcp_conn = conn;
}

void Process::add_held_lock (const string *path, uint8_t mode)
{
	scoped_lock lk(m);
	held_locks.insert(pair(*path,mode));
}

void Process::remove_held_lock (const string *path, uint8_t mode)
{
	scoped_lock lk(m);
	held_locks.erase(pair(*path,mode));
}

const uint32_t Process::get_lock_count () const
{
	scoped_lock lk(m);
	return held_locks.size();
}

std::set<std::pair<const std::string,uint8_t>> Process::get_held_locks () const
{
	scoped_lock lk(m);
	return held_locks;
}
