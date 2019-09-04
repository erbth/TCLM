#ifndef __ACQUIRE_LOCK_REQUEST_H
#define __ACQUIRE_LOCK_REQUEST_H

/* A request to be issued to an access concentrator for acquiring a Lock.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_QUEUED
 *   * RESPONSE_STATUS_NO_SUCH_PROCESS
 *   * RESPONSE_STATUS_NO_SUCH_LOCK */

#include "request.h"
#include <string>

namespace tclm_client {

/* Prototypes */
class request;
class Access_Concentrator;

class acquire_lock_request : public request
{
protected:
	/* To be set when the request is answered successfully */
	const uint32_t pid;
	const std::string *path;
	const uint8_t mode;

public:
	acquire_lock_request (const uint32_t pid, const std::string *path, uint8_t mode);

	const uint32_t get_pid () const;
	const std::string *get_path () const;
	const uint8_t get_mode () const;
	uint16_t issue (Access_Concentrator *ac) override;
};

}

#endif /* __ACQUIRE_LOCK_REQUEST_H */
