#ifndef __BACKEND_OPERATION_DEFINES
#define __BACKEND_OPERATION_DEFINES

#include "messages.h"

/* Result of a process-unregister operation */
#define PROCESS_UNREGISTER_RESULT_SUCCESS		0
#define PROCESS_UNREGISTER_RESULT_HOLDS_LOCKS	1
#define PROCESS_UNREGISTER_RESULT_NON_EXISTENT	2

/* Result of a create-lock operation */
#define CREATE_LOCK_RESULT_NO_SUCH_PROCESS		0
#define CREATE_LOCK_RESULT_CREATED				1
#define CREATE_LOCK_RESULT_QUEUED				2
#define CREATE_LOCK_RESULT_EXISTS				3

/* Result of acquire-lock operation */
#define ACQUIRE_LOCK_RESULT_NO_SUCH_PROCESS		RESPONSE_STATUS_NO_SUCH_PROCESS
#define ACQUIRE_LOCK_RESULT_ACQUIRED			RESPONSE_STATUS_SUCCESS
#define ACQUIRE_LOCK_RESULT_QUEUED				RESPONSE_STATUS_QUEUED
#define ACQUIRE_LOCK_RESULT_NO_SUCH_LOCK		RESPONSE_STATUS_NO_SUCH_LOCK

/* Result of release-lock operation */
#define RELEASE_LOCK_RESULT_NO_SUCH_PROCESS		RESPONSE_STATUS_NO_SUCH_PROCESS
#define RELEASE_LOCK_RESULT_RELEASED			RESPONSE_STATUS_SUCCESS
#define RELEASE_LOCK_RESULT_NOT_HELD			RESPONSE_STATUS_LOCK_NOT_HELD

#endif /* __BACKEND_OPERATION_DEFINES */
