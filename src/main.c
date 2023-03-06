#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define KEY_CTRL(key) ((key) & 0x1F)

// TODO: should this be atomic?
struct BrainfuckVM bfvm = {
	.cell_size = 1,
	.tape_size = 1024,
	.output_tape_size = 4096,

	.current_cell = 0,
	.tape = NULL,

	.stop_after = -1,
	.tick_delay = { .tv_sec = 0, .tv_nsec = 250000000 },
	.die = false
};

void reset_vm() {
	// Reset the vm to startup settings
	// free resources
	free(bfvm.tape);
	Queue_free(&bfvm.instructionQueue);
	StringCassette_free(&bfvm.output);

	bfvm.current_cell = 0;
	bfvm.die = false;

	StringCassette_init(&bfvm.output, bfvm.output_tape_size);
	Queue_init(&bfvm.instructionQueue);

	bfvm.tape = calloc(bfvm.tape_size, bfvm.cell_size);
}

void print_help(char *prgname) {
	printf("Usage: %s [-hPR] [-c SIZE] [-d TIME] [-m SIZE] [-O SIZE] [FILE]\n\n", prgname);

	printf("Options:\n");
	printf("  -c SIZE\tSet the cell size in bytes. This must be an integer\n"
		   "         \tbetween 1 and %zd. Default is 1.\n", sizeof(uintmax_t));
	printf("  -d TIME\tSet the interpreter tick delay. This determines how\n"
		   "         \tlong the interpreter pauses between executing each\n"
		   "         \tinstruction. Default is 250ms.\n");
	printf("  -h     \tDisplay this help message.\n");
	printf("  -m SIZE\tSet the length of the memory tape. Default is 1024.\n");
	printf("  -O SIZE\tSet the max length of the output buffer. Default is 4096.\n");
	printf("  -P     \tAutomatically pause the interpreter.\n");
	printf("  -R     \tRecord program input to FILE.\n");
}

/*
 * Gets an unsigned integer argument from getopt.
 *
 * min_val		The minimum value for the integer
 * max_val		The maximum value for the integer
 *
 * Returns the value in optarg.
 * If a value cannot be parsed, then errno is set to EINVAL.
 */
unsigned long long get_uint_arg(unsigned long long min_val, unsigned long long max_val) {
	char *endptr = NULL;
	unsigned long long val = strtoull(optarg, &endptr, 10);

	if ((*optarg != '\0' && *endptr != '\0')  // extra characters in arg
	|| val < min_val  // too small
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

	/* Parse command line args */
	while ((ch = getopt(argc, argv, "c:d:hm:O:PR")) != -1) {
		switch (ch) {
			case 'h':
				// Print help
				print_help(argv[0]);
				return 0;
			case 'c':
				// Set cell size
				bfvm.cell_size = get_uint_arg(1, sizeof(uintmax_t));

				if (errno == EINVAL) {
					goto handle_invalid_arg;
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
				bfvm.tape_size = get_uint_arg(1, SIZE_MAX);

				if (errno == EINVAL) {
					goto handle_invalid_arg;
				}

				break;
			case 'O':
				// Set output tape length
				bfvm.output_tape_size = get_uint_arg(1, SIZE_MAX);

				if (errno == EINVAL) {
					goto handle_invalid_arg;
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
			handle_invalid_arg:
				fprintf(stderr, "Invalid argument for option -%c: '%s'\n", ch, optarg);
				fprintf(stderr, "To view the help message, use the -h option\n");
				return 1;
		}
	}

	/* Start interpreter thread */
	StringCassette_init(&bfvm.output, bfvm.output_tape_size);
	Queue_init(&bfvm.instructionQueue);

	bfvm.tape = calloc(bfvm.tape_size, bfvm.cell_size);

	// alloc tape (zero-initialized)
	if (optind < argc) {
		// FILE positional argument specified

		FILE *fp = fopen(argv[optind], "r");
		char buf[256];

		while (fgets(buf, 256, fp) != NULL) {
			Queue_enqueue_all(&bfvm.instructionQueue, strlen(buf), buf);
		}

		fclose(fp);
	} else {
		// FILE not specified
	}

	thrd_create(&bfvm.interpreter_thread, interpreter_thread, NULL);

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
					case KEY_F(2):
						// pause/resume interpreter
						bfvm.stop_after = bfvm.stop_after ? 0 : -1;
						break;
					case KEY_CTRL('R'):
						// reset the VM

						// kill the interpreter thread
						bfvm.die = true;
						thrd_join(bfvm.interpreter_thread, NULL);

						// reset the interpreter
						reset_vm();

						// start a new interpreter thread
						thrd_create(&bfvm.interpreter_thread, interpreter_thread, NULL);

						// notify the user that a reset has occured
						flash();
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

