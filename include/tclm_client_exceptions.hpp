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

class too_many_processes_exception : public std::exception
{
	const char *what() const noexcept override;
};

class process_holds_locks_exception : public std::exception
{
	const char *what() const noexcept override;
};

class no_such_process_exception : public std::exception
{
	const char *what() const noexcept override;
};

class lock_not_held_exception : public std::exception
{
	const char *what() const noexcept override;
};

}

#endif /* __TCLM_CLIENT_EXCEPTIONS_HPP */
