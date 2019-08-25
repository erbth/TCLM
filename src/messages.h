#ifndef __MESSAGES_H
#define __MESSAGES_H

#include "backend_operation_defines.h"

#define MSG_ID_SERVER_ERROR					0x00
#define MSG_ID_REG_PROC						0x01
#define MSG_ID_REG_PROC_RESPONSE			0x02
#define MSG_ID_UNREG_PROC					0x03
#define MSG_ID_UNREG_PROC_RESPONSE			0x04
#define MSG_ID_LIST_PROCS					0x05
#define MSG_ID_LIST_PROCS_RESPONSE			0x06
#define MSG_ID_LIST_CONNS					0x07
#define MSG_ID_LIST_CONNS_RESPONSE			0x08

/* server_error codes */
#define SE_MSG_TOO_LONG						0x0001

#endif /* __MESSAGES_H */
