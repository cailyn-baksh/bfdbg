#ifndef _UI_H_
#define _UI_H_

#include <stddef.h>
#include <stdint.h>

#include <ncurses.h>

typedef struct Pane Pane;

typedef void PaneRendererCb(Pane *);

struct Pane {
	uint32_t id;

	WINDOW *window;

	int x;
	int y;
	int w;
	int h;

	char *title;

	PaneRendererCb *renderer;
};

/*
 * Create a panel with a given title
 *
 * id		The ID of the pane
 * h		The height of the pane
 * w		The width of the pane
 * y		The Y coordinate of the top left corner of the pane
 * x		The X coordinate of the top left corner of the pane
 * title	The title of the pane. This may be NULL
 * renderer	The renderer callback for this pane. This may be NULL.
 */
Pane *create_pane(uint32_t id, int h, int w, int y, int x, char *title, PaneRendererCb renderer);

void render_pane(Pane *pane);
void delete_pane(Pane *pane);

/* Specific pane renderers */
void MemPaneRenderer(Pane *pane);

#endif  // _UI_H_

