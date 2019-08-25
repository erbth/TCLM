#ifndef __ERRNO_EXCEPTION_H
#define __ERRNO_EXCEPTION_H

#include <exception>

class errno_exception : public std::exception
{
protected:
	char msg[2000];

public:
	errno_exception (int code);

	const char *what () const noexcept override;
};

#endif /* __ERRNO_EXCEPTION_H */
