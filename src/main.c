#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#include <ncurses.h>
#include <unistd.h>

#include "ui.h"
#include "queue.h"

#define _NOEXTERN
#include "interpreter.h"
#undef _NOEXTERN

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#error "C11 thread support is required for this program!"
#endif  // thread support

// number of frames to render each second
#define FRAMERATE 24

// pane IDs
#define PANE_PROG	0x1
#define PANE_OUT	0x2
#define PANE_MEM	0x3

// shorthand for thread sleep functions
#define THRD_NANOSLEEP(ns) thrd_sleep(&(struct timespec){.tv_nsec=ns}, NULL)
#define THRD_SLEEP(s) thrd_sleep(&(struct timespec){.tv_sec=s}, NULL)

// TODO: should this be atomic?
struct BrainfuckVM bfvm = {
	.cell_size = 1,
	.tape_size = 1024,

	.current_cell = 0,
	.tape = NULL,

	.stop_after = -1,

	.tick_delay = { .tv_sec = 0, .tv_nsec = 2500000000 }
};

void print_help(char *prgname) {
	printf("Usage: %s [-hR] [-c SIZE] [-m SIZE] [FILE]\n\n", prgname);

	printf("Options:\n");
	printf("  -c SIZE\tSet the cell size in bytes. This must be an integer \n"
		   "         \tbetween 1 and %zd. Default is 1.\n", sizeof(uintmax_t));
	printf("  -d TIME\tSet the interpreter tick delay. This determines how "
		   "long the interpreter pauses between executing each instruction. Default is 250ms.\n");
	printf("  -h     \tDisplay this help message.\n");
	printf("  -M     \tRun the program in ");
	printf("  -m SIZE\tSet the length of the memory tape. Default is 1024.\n");
	printf("  -O SIZE\tSet the max length of the output buffer. Default is 4096.\n");
	printf("  -P     \tAutomatically pause the interpreter.\n");
	printf("  -R     \tRecord program input to FILE.\n");
}

/*
 * Gets an integer argument from getopt.
 *
 * max_val		The maximum value for the integer
 *
 * Returns the value in optarg.
 * If a value cannot be parsed, then errno is set to EINVAL.
 */
unsigned long long get_int_arg(unsigned long long max_val) {
	char *endptr = NULL;
	unsigned long long val = strtoull(optarg, &endptr, 10);

	if ((*optarg != '\0' && *endptr != '\0')  // extra characters in arg
	|| val > max_val  // too big
	|| errno == EINVAL  // invalid
	|| errno == ERANGE) {  // too big 2
		errno = EINVAL;
		return 0;
	}

	return val;
}

int main(int argc, char *argv[]) {
	/* Init */
	int ch;
	unsigned long long output_tape_len = 4096;

	/* Parse command line args */
	while ((ch = getopt(argc, argv, "c:d:hm:O:PR")) != -1) {
		switch (ch) {
			case 'h':
				// Print help
				print_help(argv[0]);
				return 0;
			case 'c':
				// Set cell size
				bfvm.cell_size = get_int_arg(sizeof(uintmax_t));

				if (errno == EINVAL) {
					fprintf(stderr, "Invalid argument for option -c: '%s'\n", optarg);
					return 1;
				}

				break;
			case 'd':
				// Set tick delay
				// TODO: decide on a format for this arg and write a parser
				// it should also accept some aliased values, such as 'slow',
				// 'normal', and 'fast'
				break;
			case 'm':
				// Set memory tape length
				bfvm.tape_size = get_int_arg(SIZE_MAX);

				if (errno == EINVAL) {
					fprintf(stderr, "Invalid argument for option -m: '%s'\n", optarg);
					return 1;
				}

				break;
			case 'O':
				// Set output tape length
				output_tape_len = get_int_arg(SIZE_MAX);

				if (errno == EINVAL) {
					fprintf(stderr, "Invalid argument for option -O: '%s'\n", optarg);
					return 1;
				}

				break;
			case 'P':
				// Start paused
				bfvm.stop_after = 0;
				break;
			case 'R':
				break;
			default:
				// Unrecognised option
				return 1;
		}
	}

	/* Start interpreter thread */
	StringCassette_init(&bfvm.output, output_tape_len);
	Queue_init(&bfvm.instructionQueue);


	bfvm.tape = calloc(bfvm.tape_size, bfvm.cell_size);

	// alloc tape (zero-initialized)
	if (optind < argc) {
		// FILE positional argument specified
	} else {
		// FILE not specified
	}

	// TODO: THIS IS A TEMP CALL dont forget to move it into the above if
	thrd_t thr;
	thrd_create(&thr, interpreter_thread, NULL);

	/* Create ncurses ui */
	ESCDELAY = 10;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	curs_set(0);

	refresh();

	// Create UI panes
	Pane *panes[] = {
		create_pane(PANE_PROG, LINES / 2, COLS / 2, 0, 0, "Program", NULL),
		create_pane(PANE_OUT, LINES / 2, COLS / 2, LINES / 2, 0, "Output", OutPaneRenderer),
		create_pane(PANE_MEM, LINES, COLS / 2, 0, COLS / 2, "Memory", MemPaneRenderer),
		NULL
	};	

	scrollok(panes[1]->window, TRUE);

	/* Mainloop */
	for (;;) {
		clock_t loop_start = clock();
		
		/* Render */
		for (size_t i=0; panes[i] != NULL; ++i) {
			render_pane(panes[i]);
		}

		/* Handle Key Events */
		do {  // do ensures that key events do eventually get processed even if
			  // rendering takes the full frame time
			if ((ch = getch()) != ERR) {
				switch (ch) {
					case 27:  // ALT or ESC
						if ((ch = getch()) != ERR) {
							// ALT
						} else {
							// ESC
							goto end_mainloop;
						}
						break;
					case '+':
					case '-':
					case '>':
					case '<':
					case '.':
					case ',':
					case '[':
					case ']':
						// dispatch instruction to interpreter
						Queue_enqueue(&bfvm.instructionQueue, ch);
					default:
						// if in record mode record input
						// enter key should insert a newline 
						break;
				}
			}
		} while (((double)(clock()-loop_start)/CLOCKS_PER_SEC) < (1.0/FRAMERATE));
	}
end_mainloop:

	/* Cleanup */
	for (size_t i=0; panes[i] != NULL; ++i) {
		delete_pane(panes[i]);
	}
	endwin();
	
	free(bfvm.tape);

	return 0;
}

