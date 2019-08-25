#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <cstdint>

class ringbuffer
{
protected:
	uint8_t *buffer = nullptr;
	unsigned long capacity = 0;
	unsigned long readpos = 0, writepos = 0;

public:
	~ringbuffer ();

	unsigned long size() const;
	unsigned long get_capacity() const;

	void write (const uint8_t *data, unsigned long length);

	unsigned long read (uint8_t *buffer, unsigned long buffer_length);

	/* These methods will return 0 if the buffer is too small. Hence the user
	 * should always known if the buffer holds enough data before calling one
	 * of these. */
	uint32_t read_uint32 ();
	uint16_t read_uint16 ();
	uint8_t read_uint8 ();

	unsigned long peek (uint8_t *buffer, unsigned long buffer_length) const;
	void remove (unsigned long count);
};

#endif
