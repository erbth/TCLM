#include "request.h"
#include "messages.h"

using namespace std;
using namespace tclm_client;

request::~request ()
{
}

void request::answer (uint16_t status_code)
{
	lock_guard lk(m);

	answered = true;
	this->status_code = status_code;

	cv.notify_one();
}
