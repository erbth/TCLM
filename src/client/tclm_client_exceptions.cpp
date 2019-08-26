#include "tclm_client_exceptions.hpp"
#include <memory>

using namespace std;
using namespace tclm_client;

/* cannot_connect_exception */
cannot_connect_exception::cannot_connect_exception (
		const string &servername,
		const uint16_t port,
		const string &protocol) noexcept
{
	try
	{
		msg = new string("Cannot `connect' to server \"");
		(*msg) += servername + ":" + to_string(port) + "\" using protocol " + protocol + ".";
	}
	catch (...) {
		msg = nullptr;
	}
}

cannot_connect_exception::~cannot_connect_exception () noexcept
{
	if (msg)
		delete msg;
}

const char *cannot_connect_exception::what () const noexcept
{
	if (msg)
		return msg->c_str();
	else
		return "Cannot connect to server.";
}
