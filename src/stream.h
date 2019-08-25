#ifndef _STREAM_H
#define _STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

struct stream
{
	uint8_t *buffer;
	unsigned long pos;
	unsigned long length;
	unsigned long capacity;
};

struct stream *stream_new (void);
void stream_free (struct stream *s);

#define stream_length(S) (S->length)
#define stream_remaining_length(S) (S->length - S->pos)
#define stream_capacity(S) (S->capacity)

#define stream_tell(S) (S->pos)
#define stream_seek(S, p) (S->pos = p)
#define stream_pointer(S) (S->buffer + S->pos)
#define stream_set_length(s,L) (s->length = L)

/* Returns 0 on success, -1 on error. If the latter happens the stream is not
 * altered. */
int stream_ensure_remaining_capacity(struct stream *s, unsigned long l);

/* These functions serialize to big-endian. They do NOT care for extending the
 * stream's capacity! This MUST be done using stream_ensure_remaining_capacity
 * BEFORE calling one of these! */
void stream_write_uint8_t(struct stream *s, uint8_t d);
void stream_write_uint16_t(struct stream *s, uint16_t d);
void stream_write_uint32_t(struct stream *s, uint32_t d);
void stream_write_uint64_t(struct stream *s, uint64_t d);

/* If the stream's end is reached, these functions return 0. */
uint8_t  stream_read_uint8_t (struct stream *s);
uint16_t stream_read_uint16_t(struct stream *s);
uint32_t stream_read_uint32_t(struct stream *s);
uint64_t stream_read_uint64_t(struct stream *s);

#ifdef __cplusplus
}
#endif

#endif
