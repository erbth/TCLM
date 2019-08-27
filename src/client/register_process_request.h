#ifndef __REGISTER_PROCESS_REQUEST_H
#define __REGISTER_PROCESS_REQUEST_H

/* A request to be issued to an access concentrator for registering a new
 * process.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_TOO_MANY_PROCESSES */

#include "request.h"

namespace tclm_client {

/* Prototypes */
class request;
class Access_Concentrator;

class register_process_request : public request
{
protected:
	/* To be set by the access concentrator when the request is issued */
	uint32_t nonce;

	/* To be set when the request is answered successfully */
	uint32_t id;

public:
	/* For use by the client implementation */
	uint32_t get_id () const;
	uint16_t issue (Access_Concentrator *ac) override;

	/* To be used by the Access Concentrator only */
	uint32_t get_nonce () const;
	void set_nonce (uint32_t nonce);
	void set_id (uint32_t id);
};

}

#endif /* __REGISTER_PROCESS_REQUEST_H */
