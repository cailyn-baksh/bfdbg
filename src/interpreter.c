#include <stdint.h>
#include <stdio.h>
#include <threads.h>

#include "interpreter.h"

int interpreter_thread(void *arg) {
	const uintmax_t cell_mask = ~(~((uintmax_t)0) << (bfvm.cell_size * 8));

	for (;;) {
		// If vm is not halted and there are instructions to execute
		if ((bfvm.stop_after != 0) && bfvm.instructionQueue.length > 0) {
			uintmax_t tmp;

			// Get the next instruction
			char instruction = Queue_dequeue(&bfvm.instructionQueue);

			// get pointer to cell
			uintmax_t *cell = (uintmax_t *)(((ptrdiff_t)bfvm.tape) + (bfvm.current_cell * bfvm.cell_size));

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
					if (bfvm.current_cell < bfvm.tape_size-1)
						++bfvm.current_cell;
					else
						bfvm.current_cell = 0;
					break;
				case '<':
					if (bfvm.current_cell > 0)
						--bfvm.current_cell;
					else
						bfvm.current_cell = bfvm.tape_size - 1;
					break;
				case '.':
					// TODO: figure out how to print multibyte chars
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

