#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>
#include "ringbuffer.h"
#include "errno_exception.h"

using namespace std;

ringbuffer::~ringbuffer()
{
	if (buffer)
		free (buffer);
}

unsigned long ringbuffer::size() const
{
	if (readpos <= writepos)
		return writepos - readpos;
	else
		return capacity - readpos + writepos;
}

unsigned long ringbuffer::get_capacity() const
{
	return capacity - 1;
}

void ringbuffer::write (const uint8_t *data, unsigned long length)
{
	auto s = size();

	if (!buffer)
	{
		// Make length a power of two
		unsigned long bc = 1024;

		while (bc - 1 < length && !(bc & (1UL << 63)))
			bc *= 2;

		buffer = (uint8_t*) malloc (bc);
		if (!buffer)
			throw errno_exception(errno);

		capacity = bc;
	}
	else if (capacity - 1 - s < length)
	{
		auto nc = capacity;

		while (nc - 1 - s < length && !(nc & (1UL << 63)))
			nc *= 2;

		uint8_t *nb = (uint8_t*) malloc (nc);
		if (!nb)
			throw errno_exception(errno);

		if (readpos <= writepos)
		{
			memcpy (nb, buffer + readpos, writepos - readpos);
			writepos -= readpos;
			readpos = 0;
		}
		else
		{
			memcpy (nb, buffer + readpos, capacity - readpos);
			memcpy (nb + (capacity - readpos), buffer, writepos);
			writepos = s;
			readpos = 0;
		}

		free (buffer);
		buffer = nb;
		capacity = nc;
	}

	if (capacity - 1 - s < length)
		throw ENOMEM;

	// Actually write to the buffer
	if (capacity - writepos >= length)
	{
		memcpy (buffer + writepos, data, length);
		writepos = (writepos + length) % capacity;
	}
	else
	{
		memcpy (buffer + writepos, data, capacity - writepos);
		memcpy (buffer, data + (capacity - writepos), length - (capacity - writepos));
		writepos = length - (capacity - writepos);
	}
}

unsigned long ringbuffer::read (uint8_t * buffer, unsigned long buffer_length)
{
	if (size () > 0)
	{
		auto to_readpos = size() >= buffer_length ? buffer_length : size();

		if (readpos <= writepos || to_readpos < capacity - readpos)
		{
			memcpy (buffer, this->buffer + readpos, to_readpos);
			readpos += to_readpos;
		}
		else
		{
			memcpy (buffer, this->buffer + readpos, capacity - readpos);
			memcpy (buffer + (capacity - readpos), this->buffer, to_readpos - (capacity - readpos));
			readpos = (readpos + to_readpos) % capacity;
		}

		return to_readpos;
	}
	else
	{
		return 0;
	}
}

uint32_t ringbuffer::read_uint32 ()
{
	uint32_t v;

	if (size () >= 4)
	{
		this->read ((uint8_t*) &v, 4);
		return be32toh (v);
	}
	else
	{
		return 0;
	}
}

uint16_t ringbuffer::read_uint16 ()
{
	uint16_t v;

	if (size () >= 2)
	{
		this->read ((uint8_t*) &v, 2);
		return be16toh (v);
	}
	else
	{
		return 0;
	}
}

uint8_t ringbuffer::read_uint8 ()
{
	uint8_t v;

	if (size () >= 1)
	{
		this->read (&v, 1);
		return v;
	}
	else
	{
		return 0;
	}
}

unsigned long ringbuffer::peek (uint8_t * buffer, unsigned long buffer_length) const
{
	if (size () > 0)
	{
		auto to_readpos = size() >= buffer_length ? buffer_length : size();

		if (readpos <= writepos || to_readpos < capacity - readpos)
		{
			memcpy (buffer, this->buffer + readpos, to_readpos);
		}
		else
		{
			memcpy (buffer, this->buffer + readpos, capacity - readpos);
			memcpy (buffer + (capacity - readpos), this->buffer, to_readpos - (capacity - readpos));
		}

		return to_readpos;
	}
	else
	{
		return 0;
	}
}

void ringbuffer::remove (unsigned long count)
{
	if (size () > 0)
	{
		auto to_readpos = size() >= count ? count : size();

		readpos = (readpos + to_readpos) % capacity;
	}
}
