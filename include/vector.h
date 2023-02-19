#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stddef.h>

typedef struct {
	size_t length;
	size_t _reserve;

	char *data;
} Vec;

/*
 * Initializes a vector
 *
 * vec		The vector to initialize
 */
void Vec_init(Vec *vec);

/*
 * Frees a vector.
 *
 * vec		The vector to free
 */
void Vec_free(Vec *vec);

/*
 * Reserve n bytes of space in the vector
 *
 * vec		The vector to reserve memory in
 * n		The number of bytes to reserve
 */
void Vec_reserve(Vec *vec, size_t n);

/*
 * Inserts a value into a vector
 *
 * vec		The vector to insert into
 * index	The index to insert at
 * c		The value to insert
 */
void Vec_insert(Vec *vec, size_t index, char c);

/*
 * Gets a value from a vector
 *
 * vec		The vector to get the value from
 * index	The index to get from
 */
char Vec_get(Vec *vec, size_t index);

#endif  // _VECTOR_H_

