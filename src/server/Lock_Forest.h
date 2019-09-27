#ifndef __LOCK_FOREST_H
#define __LOCK_FOREST_H

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

namespace server {

/* Prototypes */
class Lock_Request;
class Lock;
class Process;

class Lock_Forest
{
protected:
	mutable std::shared_mutex m;

	std::map<std::string, Lock *> roots;

	static std::shared_ptr<std::vector<std::string>> split_path (const std::string *path);

public:
	~Lock_Forest ();

	/* Returns one out of LOCK_CREATE_*. If acquire_X is true, the new lock will
	 * be acquired in X mode upon creation. Otherwise a parent lock must be
	 * held in X mode by the requesting Process or LOCK_CREATE_PARENT_NOT_HELD
	 * will be returned. */
	int create (Process *p, const std::string *path_str, const bool acquire_X);
	void for_each_lock (std::function<void(const Lock*l, const uint32_t level)> f) const;

	/* Returns one out of LOCK_ACQUIRE_*, mode is one out of LOCK_REQUEST_MODE_* */
	int acquire (Process *p, const std::string *path_str, uint8_t mode);

	/* Returns one out of LOCK_RELEASE_* and a set of Lock Requests that were
	 * answered as result of the release operation, mode is one out of
	 * LOCK_REQUEST_MODE_* */
	std::pair<int,std::set<std::shared_ptr<Lock_Request>>> release (
			Process *p, const std::string *path_str, uint8_t mode);

	/* Returns one out of LOCK_DESTROY_* */
	// int destroy (Process *p, std::string *path_str);
};

}

/* Carefully placed includes */
#include "Lock_Request.h"
#include "Lock.h"
#include "Process.h"

#endif /* __LOCK_FOREST_H */
