#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

#include <stdio.h>

struct stream *stream_new (void)
{
	return calloc (1, sizeof (struct stream));
}

void stream_free (struct stream *s)
{
	if (s->buffer)
		free (s->buffer);

	free (s);
}

int stream_ensure_remaining_capacity (struct stream *s, unsigned long l)
{
	if (!s->buffer)
	{
		// Make l a power of 2
		int i = __builtin_clzl (l);

		if (l ^ (1 << (64 - i)) && i > 0)
			l = 1 << (64 - (i - 1));

		s->buffer = malloc (l);
		if (!s->buffer)
			return -1;
	}
	else if (s->capacity - s->pos < l)
	{
		unsigned long nc = s->capacity;

		while (nc - s->pos < l && !(nc & (1UL << 63)))
			nc *= 2;

		if (nc - s->pos < l)
			return -1;

		uint8_t *n = realloc (s->buffer, nc);
		if (!n)
			return -1;

		s->buffer = n;
		s->capacity = nc;
	}

	return 0;
}


inline void stream_write_uint8_t(struct stream *s, uint8_t d)
{
	s->buffer[s->pos++] = d;

	if (s->pos > s->length)
		s->length = s->pos;
}

inline void stream_write_uint16_t(struct stream *s, uint16_t d)
{
	d = htobe16 (d);

	s->buffer[s->pos++] = d & 0xff;
	s->buffer[s->pos++] = (d >> 8) & 0xff;

	if (s->pos > s->length)
		s->length = s->pos;
}

inline void stream_write_uint32_t(struct stream *s, uint32_t d)
{
	d = htobe32 (d);

	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;

	if (s->pos > s->length)
		s->length = s->pos;
}

inline void stream_write_uint64_t(struct stream *s, uint64_t d)
{
	d = htobe64 (d);

	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;
	d >>= 8;
	s->buffer[s->pos++] = d & 0xff;

	if (s->pos > s->length)
		s->length = s->pos;
}


inline uint8_t stream_read_uint8_t(struct stream *s)
{
	if (s->pos < s->length)
		return s->buffer[s->pos++];
	else
		return 0;
}

inline uint16_t stream_read_uint16_t(struct stream *s)
{
	if (s->pos+2 <= s->length)
	{
		uint16_t d;

		d = s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];

		return le16toh(d);
	}
	else
		return 0;
}

inline uint32_t stream_read_uint32_t(struct stream *s)
{
	if (s->pos+4 <= s->length)
	{
		uint32_t d;

		d = s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];

		return le32toh(d);
	}
	else
		return 0;
}

inline uint64_t stream_read_uint64_t(struct stream *s)
{
	if (s->pos+8 <= s->length)
	{
		uint64_t d;

		d = s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];
		d <<= 8;
		d |= s->buffer[s->pos++];

		return le64toh(d);
	}
	else
		return 0;
}
