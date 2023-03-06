#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>
#include <time.h>

#include "cassette.h"
#include "queue.h"

/*
 * cell_size		The size of each cell in bytes
 * tape_size		The length of the memory tape
 *
 * current_cell		The current cell index
 * tape				A pointer to the memory block for the memory tape.
 *
 * stop_after		How many instructions to automatically pause after. 0 halts
 * 					the interpreter. -1 executes indefinitely.
 * tick_delay		Timespec specifying how long between ticks to sleep the
 * 					interpreter thread.
 * die				If true, the interpreter dies at its soonest convenience
 *
 * instructionQueue	A queue containing the program's instructions
 *
 * output			The StringCassette to which program output will be written
 */
struct BrainfuckVM {
	thrd_t interpreter_thread;

	size_t cell_size;
	size_t tape_size;
	size_t output_tape_size;

	size_t current_cell;
	uint8_t *tape;

	_Atomic int stop_after;
	struct timespec tick_delay;
	_Atomic bool die;

	Queue instructionQueue;

	StringCassette output;
};

#ifndef _NOEXTERN
extern struct BrainfuckVM bfvm;
#endif  // _NOEXTERN

int interpreter_thread(void *arg);

#endif  // _INTERPRETER_H_

