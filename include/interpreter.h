#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stddef.h>

/*
 * cell_size		The size of each cell in bytes
 * tape_size		The length of the memory tape
 * current_cell		The current cell index
 *
 * tape				A pointer to the memory block for the memory tape.
 */
struct InterpreterConfig {
	size_t cell_size;
	size_t tape_size;

	size_t current_cell;

	void **tape;
};

#ifndef _NOEXTERN
extern struct InterpreterConfig interpreter_config;
#endif  // _NOEXTERN

#endif  // _INTERPRETER_H_

