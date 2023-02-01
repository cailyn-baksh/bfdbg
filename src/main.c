#include <stdio.h>
#include <threads.h>

#include <ncurses.h>
#include <unistd.h>

#include "ui.h"

#define PANE_PROG	0x1
#define PANE_OUT	0x2
#define PANE_MEM	0x3

#define THRD_NANOSLEEP(ns) thrd_sleep(&(struct timespec){.tv_nsec=ns}, NULL);
#define THRD_SLEEP(s) thrd_sleep(&(struct timespec){.tv_sec=s}, NULL);

void print_help(char *prgname) {
	printf("Usage: %s [-hR] [-c SIZE] [-m SIZE] [FILE]\n\n", prgname);

	printf("Options:\n");
	printf("  -c SIZE\tSet the cell size in bytes. Default is 1.\n");
	printf("  -h     \tDisplay this help message.\n");
	printf("  -m SIZE\tSet the length of the memory tape. Default is 256.\n");
	printf("  -R     \tRecord program input to FILE.\n");
}

int main(int argc, char *argv[]) {
	int ch;

	/* Parse command line args */
	while ((ch = getopt(argc, argv, "c:hm:R:")) != -1) {
		switch (ch) {
			case 'c':
				// Set cell size
				break;
			case 'h':
				// Print help
				print_help(argv[0]);
				break;
			case 'm':
				// Set memory tape length
				break;
			case 'R':
				break;
			default:
				// Unrecognised option
				return 1;
		}
	}

	if (optind < argc) {
		// Execute file
	} else {
		// Interpreter mode
	}

	/* Create ncurses ui */
	ESCDELAY = 10;

	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	refresh();

	// Create UI panes
	Pane *panes[] = {
		create_pane(PANE_PROG, LINES / 2, COLS / 2, 0, 0, "Program", NULL),
		create_pane(PANE_OUT, LINES / 2, COLS / 2, LINES / 2, 0, "Output", NULL),
		create_pane(PANE_MEM, LINES, COLS / 2, 0, COLS / 2, "Memory", NULL),
		NULL
	};	

	/* Mainloop */
	for (;;) {
		/* Handle Key Events */

		// handle a maximum of 16 key events per loop to prevent lots of key
		// input from lagging the ui
		for (int i=0; ((ch = getch()) != ERR) && i < 16; ++i) {
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
			}
		}

		/* Render */
		for (size_t i=0; panes[i] != NULL; ++i) {
			render_pane(panes[i]);
		}

		// Sleep until next loop
		// TODO: can we process key events or idle if there are none in between draws?
		THRD_NANOSLEEP(1000000000/24);
	}
end_mainloop:

	for (size_t i=0; panes[i] != NULL; ++i) {
		delete_pane(panes[i]);
	}
	endwin();

	return 0;
}

