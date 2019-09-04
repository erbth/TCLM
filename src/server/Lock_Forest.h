#ifndef __LOCK_FOREST_H
#define __LOCK_FOREST_H

#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#define LOCK_CREATE_CREATED			0x00
#define LOCK_CREATE_QUEUED			0x01
#define LOCK_CREATE_EXISTS			0x02

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

	static std::shared_ptr<std::vector<std::string>> split_path (std::string *path);

public:
	~Lock_Forest ();

	/* Returns one out of LOCK_CREATE_* */
	int create (Process *p, std::string *path_str);
	void for_each_lock (std::function<void(const Lock*l, const uint32_t level)> f) const;

	/* Returns one out of LOCK_ACQUIRE_*, mode is one out of LOCK_REQUEST_MODE_* */
	int acquire (Process *p, std::string *path_str, uint8_t mode);

	/* Returns one out of LOCK_RELEASE_*, mode is one out of LOCK_REQUEST_MODE_* */
	int release (Process *p, std::string *path_str, uint8_t mode);

	/* Returns one out of LOCK_DESTROY_* */
	// int destroy (Process *p, std::string *path_str);
};

}

/* Carefully placed includes */
#include "Lock_Request.h"
#include "Lock.h"
#include "Process.h"

#endif /* __LOCK_FOREST_H */
