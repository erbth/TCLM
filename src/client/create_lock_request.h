#ifndef __CREATE_LOCK_REQUEST_H
#define __CREATE_LOCK_REQUEST_H

/* A request to be issued to an access concentrator for registering a new
 * process.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_TOO_MANY_PROCESSES */

#include "request.h"
#include <string>

namespace tclm_client {

/* Prototypes */
class request;
class Access_Concentrator;

class create_lock_request : public request
{
protected:
	/* To be set when the request is answered successfully */
	const uint32_t pid;
	const std::string *path;

public:
	create_lock_request (const uint32_t pid, const std::string *path);

	const uint32_t get_pid () const;
	const std::string *get_path () const;
	uint16_t issue (Access_Concentrator *ac) override;
};

}

#endif /* __CREATE_LOCK_REQUEST_H */
