#ifndef __UNREGISTER_PROCESS_REQUEST_H
#define __UNREGISTER_PROCESS_REQUEST_H

/* A request to be issued to an access concentrator for unregistering an existing
 * process.
 * The returned status code is one of:
 *   * RESPONSE_STATUS_SUCESS
 *   * RESPONSE_STATUS_NO_SUCH_PROCESS
 *   * RESPONSE_STATUS_PROCESS_HOLDS_LOCKS */

#include "request.h"

namespace tclm_client {

/* Prototypes */
class request;
class Access_Concentrator;

class unregister_process_request : public request
{
protected:
	const uint32_t id;

public:
	unregister_process_request (const uint32_t id);

	const uint32_t get_id () const;

	/* For use by the client implementation */
	uint16_t issue (Access_Concentrator *ac) override;
};

}

#endif /* __UNREGISTER_PROCESS_REQUEST_H */
