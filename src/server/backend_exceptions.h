#ifndef __BACKEND_EXCEPTIONS_H
#define __BACKEND_EXCEPTIONS_H

#include <exception>
#include <string>

namespace server {

class too_many_processes_exception : public std::exception
{
public:
	const char *what() const noexcept override;
};

class process_too_many_locks_exception : public std::exception
{
protected:
	const uint32_t proc_id;
	std::string *msg;

public:
	process_too_many_locks_exception (const uint32_t proc_id);
	~process_too_many_locks_exception () override;

	const char *what() const noexcept override;
	const uint32_t get_proc_id () const noexcept;
};

}

#endif /* __BACKEND_EXCEPTIONS_H */
