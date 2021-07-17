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

#define LOCK_CREATE_CREATED			0x00
#define LOCK_CREATE_QUEUED			0x01
#define LOCK_CREATE_EXISTS			0x02
#define LOCK_CREATE_PARENT_NOT_HELD 0x03

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
	std::multiset<Process *> lockers_S;
	Process *locker_Splus = nullptr;
	Process *locker_X = nullptr;

	std::multiset<Process *> lockers_IS;
	std::multiset<Process *> lockers_ISplus;
	std::multiset<Process *> lockers_IX;

	/* Lock Requests waiting on this lock */
	std::deque<std::shared_ptr<Lock_Request>> lock_requests;

	Lock (Lock* parent, const std::string name);

public:
	/* Only creating root nodes is publicly allowed. */
	Lock (const std::string name);
	~Lock();

	const std::string get_name () const;

	/* Creates a lock that is not initially held. This means one of its parents
	 * must be held in X mode by the requesting process. If that is not the
	 * case, LOCK_CREATE_PARENT_NOT_HELD is returned. To create a lock that is
	 * owned initially, use acquire with create_missing set to true in the
	 * request. This method returns one of LOCK_CREATE_*, anyway. */
	int create_not_held (Process *p, std::shared_ptr<std::vector<std::string>> path);

protected:
	int create_not_held (
			Process *p,
			std::shared_ptr<std::vector<std::string>> path,
			uint32_t current_level, uint32_t level, bool ownership_ensured);

public:
	/* Returns one of LOCK_ACQUIRE_*. LOCK_ACQUIRE_NON_EXISTENT will only by
	 * returned if create_missing is false and the lock does not exist.
	 *
	 * If insert_in_current_queue is false, the request will only be queued if
	 * it advanced one level. This is useful when advancing already queued lock
	 * request on lower levels. */
	int acquire (std::shared_ptr<Lock_Request> r, bool insert_in_current_queue = true);

	/* Returns one of LOCK_RELEASE_*, and a set of Lock Requests that were
	 * answered as result of the release operation. It doesn't distinguish
	 * between locks that are not held and those which do not exist, because
	 * non-existent locks are obviously not held. */
	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> release (
			Process *p, uint8_t mode,
			std::shared_ptr<std::vector<std::string>> path);

	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> release (
			Process *p, uint8_t mode,
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
	bool is_IS_locked () const;
	bool is_ISplus_locked () const;
	bool is_IX_locked () const;
	bool is_S_locked () const;
	bool is_Splus_locked () const;
	bool is_X_locked () const;
};

}

/* Carefully placed includes */
#include "Process.h"
#include "Lock_Request.h"

#endif /* __LOCK_H */
