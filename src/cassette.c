#include <stdlib.h>
#include <string.h>

#include "cassette.h"

void CassetteInit(Cassette *c, size_t size) {
	c->_data = malloc(size);
	c->size = size;
	c->_head = c->_data;

	memset(c->_data, 0, size);
}

void CassetteFree(Cassette *c) {
	free(c->_data);

	c->_data = NULL;
	c->_head = NULL;
	c->size = 0;
}

void CassetteWrite(Cassette *c, size_t n, void *d) {
	// Write maximum of c->size bytes
	// Anything more than that will just get overwritten
	if (n > c->size) {
		d = d + n - c->size;
		n = c->size;
	}

	// Get the number of bytes remaining between c->_head and the end 
	size_t remain = c->_data + c->size - c->_head;

	// If we dont have enough data to write to fill to the end then just write everything
	if (n < remain) remain = n;

	// Copy data to end of cassette
	memcpy(c->_head, d, remain);
	c->_head += remain;

	if (n > remain) {
		// We couldnt fit all the bytes at the end; write it back to the start
		memcpy(c->_data, d + (n - remain), n - remain);
		c->_head = c->_data + (n - remain);
	}
}

