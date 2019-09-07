#ifndef __LOCK_REQUEST_H
#define __LOCK_REQUEST_H

#include "messages.h"
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

/* No mutex required since the Lock Request will be protected by the lock it
 * is enqueued at. */

/* Definitions of lock modes */
#define LOCK_REQUEST_MODE_S			MSG_LOCK_MODE_S
#define LOCK_REQUEST_MODE_Splus		MSG_LOCK_MODE_Splus
#define LOCK_REQUEST_MODE_X			MSG_LOCK_MODE_X

namespace server {

/* Prototypes */
class Process;

class Lock_Request : public std::enable_shared_from_this<Lock_Request>
{
protected:
	/* A Process may be destroyed while a Lock Request referencing it still
	 * exists. This is caused by asynchronous notifications. */
	mutable std::mutex m_requester_exists;
	bool requester_exists = true;

	/* May only be instantiated as shared_ptr */
	Lock_Request (const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing);

public:
	/* Lock mode */
	const uint8_t mode;

	/* Who ? */
	Process * const requester;

	/* What ? */
	std::shared_ptr<std::vector<std::string>> path;
	uint32_t current_level = 0;
	const uint32_t level;

	bool create_missing;

	/* Set to true by the Lock implementation if a lock has been created to
	 * distinguish between acquiring an existing lock and creating a new one
	 * when create_missing is true. */
	bool lock_created = false;

	/* To tell if a request was acquired successfully without the return value
	 * of lock::acquire (useful with release returning answered requests) */
	int acquire_status = 0;

	static std::shared_ptr<Lock_Request> create (
			const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing = false);
	~Lock_Request ();

	/* The returned lock ensures that the information - existence of the Process
	 * - is valid. */
	std::pair<bool,std::unique_lock<std::mutex>> get_requester_exists() const;
	void set_requester_destroyed();
};

}

/* Carefully placed includes */
#include "Process.h"

#endif /* __LOCK_REQUEST_H */
