#include "Lock_Request.h"

using namespace server;
using namespace std;

Lock_Request::Lock_Request (const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing) :
	mode(mode), requester(requester), path(path), level(path->size() - 1),
			create_missing(create_missing)
{
}
