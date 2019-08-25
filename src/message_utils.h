#ifndef __MESSAGE_UTILS_H
#define __MESSAGE_UTILS_H

/* Functions for writing parts of messages that are commonly used or creating
 * commonly used messages. */

#include "stream.h"

/* Takes care that the stream's remaining capacity is big enough. */
inline void write_message_header (struct stream *s, uint8_t id, uint32_t length)
{
	stream_ensure_remaining_capacity (s, 5);
	stream_write_uint8_t (s, id);
	stream_write_uint32_t (s, length);
}

inline void update_message_length (struct stream *s, uint32_t payload_length)
{
	stream_seek(s,1);
	stream_write_uint32_t(s, payload_length - 5);
}

#endif /* __MESSAGE_UTILS_H */
