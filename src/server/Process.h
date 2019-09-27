#ifndef __PROCESS_H
#define __PROCESS_H

#include "Lock_Request.h"
#include <memory>
#include <mutex>
#include <map>
#include <set>
#include <string>

/* Prototypes */
class TCP_Connection;

namespace server {

/* Prototypes */
class Lock_Request;

class Process
{
protected:
	mutable std::mutex m;

	// 4 billion concurrent processes should be enough.
	const uint32_t id;

	/* A list of lock requests to be notified when this process is destroyed. */
	std::map<Lock_Request*,std::weak_ptr<Lock_Request>> lock_requests;

	/* A set of locks currently held by this process. These are to be unlocked
	 * when this process is destroyed.
	 * The format is (path,mode) with mode being one out of LOCK_REQUEST_MODE_* */
	std::set<std::pair<const std::string,uint8_t>> held_locks;

	/* A weak pointer to the Connections behind which this Process was seen lastly. */
	std::weak_ptr<TCP_Connection> tcp_conn;

public:
	Process (const uint32_t id);
	~Process ();

	const uint32_t get_id () const;

	/* To be called by Lock Requests only */
	/* The add function must only be called while the Process is not destructed.
	 * However this is usually the case as ownerschip through the Process map is
	 * ensured. */
	void add_lock_request (std::shared_ptr<Lock_Request> r);
	void remove_lock_request (Lock_Request *r);

	std::weak_ptr<TCP_Connection> get_tcp_conn() const;
	void update_tcp_conn (std::weak_ptr<TCP_Connection> conn);

	/* The list of held locks is different from the list of lock requests. None
	 * of the logic shared common code / situations. */
	void add_held_lock (const std::string *path, uint8_t mode);
	void remove_held_lock (const std::string *path, uint8_t mode);
	const uint32_t get_lock_count() const;
	std::set<std::pair<const std::string,uint8_t>> get_held_locks () const;
};

}

#endif /* __PROCESS_H */
