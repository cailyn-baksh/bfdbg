#ifndef _CASSETTE_H_
#define _CASSETTE_H_

#include <stddef.h>

/*
 * A cassette tape that stores strings. When you read or write past the end of
 * the memory block, it just wraps back around to the beginning.
 *
 * length		The length of the cassette
 */
typedef struct {
	size_t length;

	char *_data;   // Pointer to the start of the memory block
				   // This pointer 'owns' the memory and should be freed
	size_t _tail;  // Offset from _data to the end of the tape. (Points to the end of the tape)
} StringCassette;

/*
 * Returns the index of the start of data in the cassette.
 */
#define StringCassette_getHead(c) (((c)->_tail+1 < (c)->length) ? (c)->_tail+1 : 0)

/*
 * Initializes a cassette
 *
 * c		The cassette to initialize
 * len		The length of the cassette
 */
void StringCassette_init(StringCassette *c, size_t len);

/*
 * Frees a cassette's memory. After this runs, the cassette is invalid unless
 * reinitialized.
 *
 * c		The cassette to free
 */
void StringCassette_free(StringCassette *c);

/*
 * Writes a null terminated string to the cassette.
 *
 * c		The cassette to write to
 * s		The string to write. This must be null terminated.
 */
void StringCassette_write(StringCassette *c, const char *s);

/*
 * Read data out of the cassette.
 *
 * c		The cassette to read from
 * n		The number of bytes to read.
 * buf		The buffer to read into
 * offset	The offset to start reading from
 *
 * Returns the offset of the next byte in the tape
 */
size_t StringCassette_read(StringCassette *c, size_t n, char *buf, size_t offset);

#endif  // _CASSETTE_H_

