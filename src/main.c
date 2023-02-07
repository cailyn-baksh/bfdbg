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
#define THRD_NANOSLEEP(ns) thrd_sleep(&(struct timespec){.tv_nsec=ns}, NULL);
#define THRD_SLEEP(s) thrd_sleep(&(struct timespec){.tv_sec=s}, NULL);

// TODO: should this be atomic?
struct BrainfuckVM bfvm = {
	.cell_size = 1,
	.tape_size = 1024,

	.current_cell = 0,

	.tape = NULL,
};

void print_help(char *prgname) {
	printf("Usage: %s [-hR] [-c SIZE] [-m SIZE] [FILE]\n\n", prgname);

	printf("Options:\n");
	printf("  -c SIZE\tSet the cell size in bytes. This must be an integer \n"
		   "         \tbetween 1 and %zd. Default is 1.\n", sizeof(uintmax_t));
	printf("  -h     \tDisplay this help message.\n");
	printf("  -m SIZE\tSet the length of the memory tape. Default is 1024.\n");
	printf("  -R     \tRecord program input to FILE.\n");
}

int main(int argc, char *argv[]) {
	/* Init */
	int ch;

	/* Parse command line args */
	while ((ch = getopt(argc, argv, "c:hm:R")) != -1) {
		switch (ch) {
			case 'h':
				// Print help
				print_help(argv[0]);
				return 0;
			case 'c': {
				// Set cell size
				char *endptr = NULL;
				unsigned long long size = strtoull(optarg, &endptr, 10);

				if ((*optarg != '\0' && *endptr != '\0')  // extra characters in arg
				 || size > sizeof(uintmax_t)  // size is too long
				 || errno == EINVAL  // invalid
				 || errno == ERANGE) {  // too big
					fprintf(stderr, "Invalid argument for option -c: '%s'\n", optarg);
					return 1;
				}

				bfvm.cell_size = size;
				break;
			}
			case 'm': {
				// Set memory tape length
				char *endptr = NULL;
				unsigned long long size = strtoull(optarg, &endptr, 10);

				if ((*optarg != '\0' && *endptr != '\0')  // extra characters in arg
				 || size > SIZE_MAX  // too big
				 || errno == EINVAL  // invalid
				 || errno == ERANGE) {  // too big 2
					fprintf(stderr, "Invalid argument for option -m: '%s'\n", optarg);
					return 1;
				}

				bfvm.tape_size = size;
				break;
			}
			case 'R':
				break;
			default:
				// Unrecognised option
				return 1;
		}
	}

	/* Start interpreter thread */

	// alloc tape (zero-initialized)
	bfvm.tape = calloc(bfvm.tape_size, bfvm.cell_size);

	StringCassette_init(&bfvm.output, 16);

	if (optind < argc) {
		// FILE positional argument specified
	} else {
		// FILE not specified
	}

	/* Create ncurses ui */
	ESCDELAY = 10;

	initscr();
	cbreak();
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
					case '+':
					case '-':
					case '>':
					case '<':
					case '.':
					case ',':
					case '[':
					case ']':
					case KEY_ENTER:
						// if in record mode record input
						// enter key should insert a newline 
						break;
					case 27:  // ALT or ESC
						//THRD_NANOSLEEP(10000000);  // sleep for ~10ms to see if another code comes in
						if ((ch = getch()) != ERR) {
							// ALT
						} else {
							// ESC
							goto end_mainloop;
						}
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

	return 0;
}

