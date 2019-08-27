#ifndef __MESSAGES_H
#define __MESSAGES_H

#include "backend_operation_defines.h"

#define MSG_ID_SERVER_ERROR						0x00
#define MSG_ID_REG_PROC							0x01
#define MSG_ID_REG_PROC_RESPONSE				0x02
#define MSG_ID_UNREG_PROC						0x03
#define MSG_ID_UNREG_PROC_RESPONSE				0x04
#define MSG_ID_LIST_PROCS						0x05
#define MSG_ID_LIST_PROCS_RESPONSE				0x06
#define MSG_ID_LIST_CONNS						0x07
#define MSG_ID_LIST_CONNS_RESPONSE				0x08

/* server_error codes */
#define SE_MSG_TOO_LONG							0x0001

/* Status codes for responses. For hints with which requests a certain status
 * code will be used see the reqeuests' documentation. */
#define RESPONSE_STATUS_SUCCESS					0x0000
#define RESPONSE_STATUS_TOO_MANY_PROCESSES		0x0001
#define RESPONSE_STATUS_PROCESS_HOLDS_LOCKS		0x0002
#define RESPONSE_STATUS_NO_SUCH_PROCESS			0x0003

#endif /* __MESSAGES_H */
