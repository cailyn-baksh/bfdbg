#include <stdlib.h>

#include "cassette.h"

void StringCassette_init(StringCassette *c, size_t len) {
	c->length = len;
	c->_tail = 0;
	c->_data = calloc(len, sizeof(char));
}

void StringCassette_free(StringCassette *c) {
	free(c->_data);
}

void StringCassette_write(StringCassette *c, const char *s) {
	// loop until null terminator, moving the tail forwards each iteration
	for (; *s; ++s, ++c->_tail) {
		// if the tail is past the end of the tape; wrap it back around
		if (c->_tail >= c->length) c->_tail = 0;

		// write the current char to the tail
		c->_data[c->_tail] = *s;
	}
}

size_t StringCassette_read(StringCassette *c, size_t n, char *buf, size_t offset) {
	// ensure offset is within array bounds
	size_t index = (offset) % c->length;

	// loop until n bytes are copied
	for (size_t b=0; b < n; ++b, ++index) {
		// if the read index is past the end of the tape, wrap it back around
		if (index > c->length) index = 0;

		// write the current char to the buffer
		buf[b] = c->_data[index];
	}

	// write null terminator to end
	//buf[n] = '\0';
	if (index > c->length) index = 0;
	return index;
}

