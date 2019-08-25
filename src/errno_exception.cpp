#include "errno_exception.h"
#include <cstring>

using namespace std;

errno_exception::errno_exception (int code)
{
	strerror_r (code, msg, sizeof(msg) / sizeof(*msg));
}

const char *errno_exception::what () const noexcept
{
	return msg;
}
