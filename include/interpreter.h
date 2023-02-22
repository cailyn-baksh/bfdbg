#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <stdbool.h>
#include <stddef.h>
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
 * exec_mutex		Mutex that locks execution. Acquiring this mutex from a
 *					non-interpreter thread will pause the interpreter.
 * pause			If true, the interpreter stops running.
 * stop_after		How many instructions to automatically pause after
 * tick_delay		Timespec specifying how long between ticks to sleep the
 * 					interpreter thread.
 *
 * instructionQueue	A queue containing the program's instructions
 *
 * output			The StringCassette to which program output will be written
 */
struct BrainfuckVM {
	size_t cell_size;
	size_t tape_size;

	size_t current_cell;
	void **tape;

	mtx_t exec_mutex;

	_Atomic bool pause;
	int stop_after;
	struct timespec tick_delay;

	Queue instructionQueue;

	StringCassette output;
};

#ifndef _NOEXTERN
extern struct BrainfuckVM bfvm;
#endif  // _NOEXTERN

int interpreter_thread(void *arg);

#endif  // _INTERPRETER_H_

