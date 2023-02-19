#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stddef.h>

#include "cassette.h"

/*
 * cell_size		The size of each cell in bytes
 * tape_size		The length of the memory tape
 * current_cell		The current cell index
 *
 * tape				A pointer to the memory block for the memory tape.
 */
struct BrainfuckVM {
	size_t cell_size;
	size_t tape_size;

	size_t current_cell;

	void **tape;

	StringCassette output;
};

#ifndef _NOEXTERN
extern struct BrainfuckVM bfvm;
#endif  // _NOEXTERN

int interpreter_thread(void *arg);

#endif  // _INTERPRETER_H_

