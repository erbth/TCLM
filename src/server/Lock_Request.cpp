#include "Lock_Request.h"

using namespace server;
using namespace std;

class concrete_Lock_Request : public Lock_Request
{
public:
	concrete_Lock_Request (const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing)
		: Lock_Request(mode, requester, path, create_missing) {}
};

std::shared_ptr<Lock_Request> Lock_Request::create (const uint8_t mode,
		Process *requester, std::shared_ptr<std::vector<std::string>> path,
		bool create_missing)
{
	shared_ptr<Lock_Request> pThis = make_shared<concrete_Lock_Request>(
			mode, requester, path, create_missing);

	/* Register with the requester to be notified when it is destroyed.
	 * Needs to be here because shared_from_this() doesn't work in a
	 * constructor.*/
	pThis->requester->add_lock_request (pThis);
	return pThis;
}

Lock_Request::Lock_Request (const uint8_t mode, Process *requester,
			std::shared_ptr<std::vector<std::string>> path,
			bool create_missing) :
	mode(mode), requester(requester), path(path), level(path->size() - 1),
			create_missing(create_missing)
{
}

Lock_Request::~Lock_Request ()
{
	/* Unregister with the requester.
	 * Acquires two locks: m_requester_exists_flag and requester->m. */
	scoped_lock lk(m_requester_exists);
	if (requester_exists)
		requester->remove_lock_request (this);
}

pair<bool,unique_lock<mutex>> Lock_Request::get_requester_exists () const
{
	unique_lock lk(m_requester_exists);
	return pair(requester_exists, move(lk));
}

void Lock_Request::set_requester_destroyed ()
{
	scoped_lock lk(m_requester_exists);
	requester_exists = false;
}
