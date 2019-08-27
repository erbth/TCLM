#ifndef __REQUEST_H
#define __REQUEST_H

#include <condition_variable>
#include <mutex>

namespace tclm_client {

/* Prototypes */
class Access_Concentrator;

class request
{
protected:
	mutable std::mutex m;
	mutable std::condition_variable cv;

	/* Set to true when an answer is received */
	bool answered = false;

	/* Contains a code indicating if registering was successful. */
	uint16_t status_code;

public:
	/* Make this class pure virtual and hence abstract */
	virtual ~request()  = 0;

	/* For use by the client implementation */
	/* Returns on of RESPONSE_STATUS_*. For a list of the codes returned by a
	 * specific request see its documentation. */
	virtual uint16_t issue (Access_Concentrator *ac) = 0;

	/* To be used by the Access Concentrator only */
	void answer (uint16_t status_code);
};

}

/* Carefully arranged includes ... */
#include "Access_Concentrator.h"

#endif /* __REQUEST_H */
