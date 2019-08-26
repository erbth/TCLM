#ifndef __TCLM_CLIENT_EXCEPTIONS_HPP
#define __TCLM_CLIENT_EXCEPTIONS_HPP

#include <exception>
#include <string>

namespace tclm_client {

class cannot_connect_exception : public std::exception
{
protected:
	std::string *msg;

public:
	cannot_connect_exception (const std::string &servername, const uint16_t port,
			const std::string &protocol) noexcept;
	~cannot_connect_exception () override;

	const char *what () const noexcept override;
};

}

#endif /* __TCLM_CLIENT_EXCEPTIONS_HPP */
