#include <stdint.h>
#include <stdio.h>
#include <threads.h>

#include "interpreter.h"

int interpreter_thread(void *arg) {
	const uintmax_t cell_mask = ~(~((uintmax_t)0) << (bfvm.cell_size * 8));

	for (;;) {
		// If vm is not halted and there are instructions to execute
		if ((bfvm.stop_after != 0) && bfvm.instructionQueue.length > 0) {
			// Get the next instruction
			char instruction = Queue_dequeue(&bfvm.instructionQueue);

			// TODO: write algorithm to convert cell indices into addresses using
			// TODO: the set cell size. this algorithm should allow for aligned
			// TODO: reads and writes

			// Loading the correct byte with proper alignment
			// read uintmax_t from (&bfvm.tape) + (bfvm.current_cell * bfvm.cell_size)

			// get pointer to cell
			uintmax_t *cell = ((uintmax_t *)bfvm.tape) + (bfvm.current_cell * bfvm.cell_size);

			uintmax_t tmp;

			char buf[64] = { '\0' };
			sprintf(buf, "%p + (%zd * %zd) = %p\n", bfvm.tape, bfvm.current_cell, bfvm.cell_size, cell);
			StringCassette_write(&bfvm.output, buf);

			switch (instruction) {
				case '+':
					tmp = *cell;
					*cell &= ~cell_mask;  // clear out old value
					*cell |= (tmp + 1) & cell_mask; 
					break;
				case '-':
					tmp = *cell;
					*cell &= ~cell_mask;
					*cell |= (tmp - 1) & cell_mask;
					break;
				case '>':
					++bfvm.current_cell;
					break;
				case '<':
					--bfvm.current_cell;
					break;
				case '.':
				case ',':
				case '[':
				case ']':
					break;
			}

			// One instruction has been executed; decrement the stop_after count
			if (bfvm.stop_after > 0) --bfvm.stop_after;
		}

		thrd_sleep(&bfvm.tick_delay, NULL);
	}
	
	return 0;
}

