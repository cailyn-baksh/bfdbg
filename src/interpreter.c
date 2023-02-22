#include <threads.h>

#include "interpreter.h"

int interpreter_thread(void *arg) {
	for (;;) {

		if (bfvm.instructionQueue.length > 0) {

		}

		thrd_sleep(&bfvm.tick_delay, NULL);
		
	}
	
	return 0;
}

