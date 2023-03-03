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
	const uintmax_t cell_mask = ~(~((uintmax_t)0) << (bfvm.cell_size * 8));
	const size_t current_cell = bfvm.current_cell;  // obtain this once so it doesnt get changed by another thread
	const size_t cell_str_len = bfvm.cell_size * 2;  // length of the string representing the cell

	int x = 1, y = 1;

	for (size_t i=0; i < bfvm.tape_size; ++i) {
		// Obtain cell value
		uintmax_t cell = *((uintmax_t *)(bfvm.tape + (i * bfvm.cell_size))) & cell_mask;

		if (i == current_cell) wattron(pane->window, A_REVERSE);

		mvwprintw(pane->window, y, x, "%0*zX", (int)cell_str_len, cell);
		
		if (i == current_cell) wattroff(pane->window, A_REVERSE);

		x += cell_str_len + 1;

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

