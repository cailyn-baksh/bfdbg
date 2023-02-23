#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"
#include "ui.h"

Pane *create_pane(uint32_t id, int h, int w, int y, int x, char *title, PaneRendererCb *renderer) {
	Pane *pane = malloc(sizeof(Pane));

	pane->id = id;
	pane->window = newwin(h, w, y, x);

	pane->x = x;
	pane->y = y;
	pane->w = w;
	pane->h = h;

	if (title != NULL) {
		pane->title = malloc(strlen(title));
		strcpy(pane->title, title);
	} else {
		pane->title = NULL;
	}

	pane->renderer = renderer;

	return pane;
}

void delete_pane(Pane *pane) {
	delwin(pane->window);
	free(pane->title);
	free(pane);
}

void render_pane(Pane *pane) {
	// Call pane renderer
	if (pane->renderer != NULL) {
		pane->renderer(pane);
	}

	// Draw pane border + title
	box(pane->window, 0, 0);

	if (pane->title != NULL) {
		mvwaddch(pane->window, 0, 2, ACS_RTEE);
		wprintw(pane->window, " %s ", pane->title);
		waddch(pane->window, ACS_LTEE);
	}

	wrefresh(pane->window);
}


/* Specific pane renderers */
void MemPaneRenderer(Pane *pane) {
	int x = 1, y = 1;

	// calculate this once so that two cells dont get selected in one draw
	const size_t current_cell_base = bfvm.current_cell * bfvm.cell_size;

	for (size_t i=0; i < bfvm.tape_size * bfvm.cell_size; ++i) {
		uint8_t byte = *(((uint8_t *)bfvm.tape) + i);

		if (i >= current_cell_base && i < (current_cell_base + bfvm.cell_size))
			wattron(pane->window, A_REVERSE);
		else
			wattroff(pane->window, A_REVERSE);

		// TODO: draw on a cell-basis instead of a byte basis so that there isnt
		// TODO: a blank gap between active cell highlighting
		mvwprintw(pane->window, y, x, "%02X", byte);
		
		x += 3;

		if (x > pane->w) {
			x = 1;
			++y;
		}
	}
}

void OutPaneRenderer(Pane *pane) {
	const size_t bufsize = (bfvm.output.length < 4096) ? bfvm.output.length : 4096;
	char buffer[bufsize];

	wclear(pane->window);
	wmove(pane->window, 1, 1);

	for (size_t i=StringCassette_getHead(&bfvm.output), b=0; b < bfvm.output.length; b += bufsize) {
		i = StringCassette_read(&bfvm.output, bufsize, buffer, i);
		//memcpy(buffer, bfvm.output._data, bufsize);

		for (size_t j=0; j < bufsize; ++j) {
			if (buffer[j] == '\0') {
				continue;
			} else if (buffer[j] == '\n') {
				int x, y;
				getyx(pane->window, y, x);
				wmove(pane->window, y+1, 1);
			} else {
				int x, y;
				getyx(pane->window, y, x);

				if (x == pane->w-1) {
					wmove(pane->window, ++y, 1);
				}

				if (y == pane->h-1) {
					scroll(pane->window);
					wmove(pane->window, y-1, 1);
					wclrtoeol(pane->window);
				}

				waddch(pane->window, buffer[j]);
			}
		}
	}
}

