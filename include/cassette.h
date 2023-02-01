#ifndef _CASSETTE_H_
#define _CASSETTE_H_

#include <stddef.h>
#include <stdint.h>

/*
 * A fixed-size 1-dimensional array which behaves like a cassette tape,
 * i.e. once the capacity is reached new data overwrites the old data
 * without moving any data.
 *
 * _data	A pointer to the start of the data block. This pointer 'owns' the
 * 			memory, and so it should not be reassigned and should be freed when
 * 			the Cassette is destroyed.
 * size		The size of the data block in bytes.
 * _head	A pointer to the head of the data
 */
typedef struct {
	uint8_t *_data;
	size_t size;

	uint8_t *_head;
} Cassette;

/*
 * Initialize a Cassette with the given size
 *
 * c	A pointer to the Cassette to initialize
 * size	The size of the new Cassette in bytes
 */
void CassetteInit(Cassette *c, size_t size);

/*
 * Write data to the Cassette
 *
 * c	A pointer to the Cassette to write to
 * n	The number of bytes to write
 * d	The data to write to the Cassette
 */
void CassetteWrite(Cassette *c, size_t n, void *d);

/*
 * Read data from the Cassette
 *
 * c	A pointer to the Cassette to read from
 * n	The number of bytes to read
 * d	A pointer to write the read data to
 */
void CassetteRead(Cassette *c, size_t n, void *d);

/*
 * Free the memory of a Cassette
 *
 * c	A pointer to the Cassette to free
 */
void CassetteFree(Cassette *c);

#endif  // _CASSETTE_H_

