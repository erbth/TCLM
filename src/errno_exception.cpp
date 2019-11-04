#include "errno_exception.h"
#include <cstring>
#include <iostream>

using namespace std;

errno_exception::errno_exception (int code)
{
	strcpy(msg, strerror_r (code, msg, sizeof(msg) / sizeof(*msg)));
}

const char *errno_exception::what () const noexcept
{
	return msg;
}
