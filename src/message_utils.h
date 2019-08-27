#ifndef __MESSAGE_UTILS_H
#define __MESSAGE_UTILS_H

/* Functions for writing parts of messages that are commonly used or creating
 * commonly used messages. */

#include "stream.h"

/* Takes care that the stream's remaining capacity is big enough. */
inline bool write_message_header (struct stream *s, uint8_t id, uint32_t length)
{
	if (stream_ensure_remaining_capacity (s, 5) != 0)
		return false;

	stream_write_uint8_t (s, id);
	stream_write_uint32_t (s, length);

	return true;
}

inline void update_message_length (struct stream *s, uint32_t total_length)
{
	stream_seek(s,1);
	stream_write_uint32_t(s, total_length - 5);
}

#endif /* __MESSAGE_UTILS_H */
