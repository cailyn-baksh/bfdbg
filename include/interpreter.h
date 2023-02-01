#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stddef.h>

/*
 *
 */
typedef struct {
	size_t cell_size;
	size_t tape_size;

	void **tape;
} InterpreterConfig;

#endif  // _INTERPRETER_H_

