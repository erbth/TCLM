#ifndef __RELEASE_LOCK_REQUEST_H
#define __RELEASE_LOCK_REQUEST_H

/* A request to be issued to an access concentrator for releasing a Lock.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_NO_SUCH_PROCESS
 *   * RESPONSE_STATUS_NOT_HELD */

#include "request.h"
#include <string>

namespace tclm_client {

/* Prototypes */
class request;
class Access_Concentrator;

class release_lock_request : public request
{
protected:
	/* To be set when the request is answered successfully */
	const uint32_t pid;
	const std::string *path;
	const uint8_t mode;

public:
	release_lock_request (const uint32_t pid, const std::string *path, uint8_t mode);

	const uint32_t get_pid () const;
	const std::string *get_path () const;
	const uint8_t get_mode () const;
	uint16_t issue (Access_Concentrator *ac) override;
};

}

#endif /* __RELEASE_LOCK_REQUEST_H */
