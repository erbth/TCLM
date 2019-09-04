#ifndef __LOCK_REQUEST_H
#define __LOCK_REQUEST_H

#include "messages.h"
#include <memory>
#include <string>
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

class Lock_Request
{
public:
	/* Lock mode */
	const uint8_t mode;

	/* Who ? */
	Process *requester;

	/* What ? */
	std::shared_ptr<std::vector<std::string>> path;
	uint32_t current_level = 0;
	const uint32_t level;

	bool create_missing;

	/* Set to true by the Lock implementation if a lock has been created to
	 * distinguish between acquiring an existing lock and creating a new one
	 * when create_missing is true. */
	bool lock_created = false;

	Lock_Request (const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing = false);
};

}

/* Carefully placed includes */
#include "Process.h"

#endif /* __LOCK_REQUEST_H */
