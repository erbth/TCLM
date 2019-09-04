#ifndef __LOCK_H
#define __LOCK_H

#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

/* Definitions for return values. Negative values are reserved for use by the
 * calling code to e.g. indicate no call to a child's function was made. */
#define LOCK_RELEASE_SUCCESS		0x00
#define LOCK_RELEASE_NOT_HELD		0x01

#define LOCK_ACQUIRE_ACQUIRED		0x00
#define LOCK_ACQUIRE_QUEUED			0x01
#define LOCK_ACQUIRE_NON_EXISTENT	0x02

#define LOCK_DESTROY_SUCCESS		0x00
#define LOCK_DESTROY_NON_EXCLUSIVE	0x01
#define LOCK_DESTROY_NON_EXISTENT	0x02

namespace server {

/* Prototypes */
class Process;
class Lock_Request;

class Lock
{
protected:
	mutable std::recursive_mutex m;

	const std::string name;
	
	// For the tree
	Lock *parent;
	std::vector<Lock*> children;

	// For tracking the Lock's state
	std::set<Process *> lockers_S;
	Process *locker_Splus;
	Process *locker_X;

	std::set<Process *> lockers_IS;
	std::set<Process *> lockers_ISplus;
	std::set<Process *> lockers_IX;

	/* Lock Requests waiting on this lock */
	std::deque<std::shared_ptr<Lock_Request>> lock_requests;

	Lock (Lock* parent, const std::string name);

public:
	/* Only creating root nodes is publicly allowed. */
	Lock (const std::string name);
	~Lock();

	const std::string get_name () const;

	/* Returns on of LOCK_ACQUIRE_*. LOCK_ACQUIRE_NON_EXISTENT will only by
	 * returned if create_missing is false and the lock does not exist. */
	int acquire (std::shared_ptr<Lock_Request> r);

	/* Returns one of LOCK_RELEASE_*. It doesn't distinguish between locks that
	 * are not held and those which do not exist, because non-existent locks are
	 * obviously not held. */
	int release (Process *p, uint8_t mode,
			std::shared_ptr<std::vector<std::string>> path,
			uint32_t current_level, uint32_t level);

	/* Returns one of LOCK_DESTROY_* and the object to be free'd. */
	std::pair<int, Lock*> destroy (Process *p,
			std::shared_ptr<std::vector<std::string>> path,
			uint32_t current_level, uint32_t level);

	/* Returns a bitmask of the following shape: X locked | Splus locked | S locked
	 * ... by the given process. */
	int query (Process *p, std::shared_ptr<std::vector<std::string>> path,
			uint32_t current_level, uint32_t level);

	/* For status reporting */
	void for_each_child (std::function<void(const Lock *l, const uint32_t level)> f, uint32_t level) const;
	bool is_S_locked () const;
	bool is_Splus_locked () const;
	bool is_X_locked () const;
};

}

/* Carefully placed includes */
#include "Process.h"
#include "Lock_Request.h"

#endif /* __LOCK_H */
