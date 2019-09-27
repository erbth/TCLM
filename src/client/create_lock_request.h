#ifndef __CREATE_LOCK_REQUEST_H
#define __CREATE_LOCK_REQUEST_H

/* A request to be issued to an access concentrator for creating a new
 * lock.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_LOCK_EXISTS  (meaning the lock has not been created but
 *     acquired in X mode)
 *   * RESPONSE_STATUS_NO_SUCH_PROCES
 *   * RESPONSE_STATUS_PARENT_NOT_HELD  (only if acquire_X is false) */

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
	const bool acquire_X;

public:
	create_lock_request (const uint32_t pid, const std::string *path, const bool acquire_X);

	const uint32_t get_pid () const;
	const std::string *get_path () const;
	const bool get_acquire_X () const;
	uint16_t issue (Access_Concentrator *ac) override;
};

}

#endif /* __CREATE_LOCK_REQUEST_H */
