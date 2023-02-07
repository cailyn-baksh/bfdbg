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
	// Draw pane border + title
	box(pane->window, 0, 0);

	if (pane->title != NULL) {
		mvwaddch(pane->window, 0, 2, ACS_RTEE);
		wprintw(pane->window, " %s ", pane->title);
		waddch(pane->window, ACS_LTEE);
	}

	if (pane->renderer != NULL) {
		pane->renderer(pane);
	}

	wrefresh(pane->window);
}


/* Specific pane renderers */
void MemPaneRenderer(Pane *pane) {
	int x = 1, y = 1;

	for (size_t i=0; i < interpreter_config.tape_size * interpreter_config.cell_size; ++i) {
		uint8_t byte = *(((uint8_t *)interpreter_config.tape) + i);

		if (i == interpreter_config.current_cell) wattron(pane->window, A_REVERSE);

		mvwprintw(pane->window, y, x, "%02X", byte);
		
		if (i == interpreter_config.current_cell) wattroff(pane->window, A_REVERSE);

		x += 3;

		if (x > pane->w) {
			x = 1;
			++y;
		}
	}
}

