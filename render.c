#include "render.h"

struct color {
	Uint8 r;
	Uint8 g;
	Uint8 b;
};

static struct color to_color(unsigned val, unsigned max_iter)
{
	struct color c;

	c.r = 255 - val * 255.0 / max_iter;
	c.g = 0;
	c.b = 0;

	return c;
}

void render(SDL_Renderer *rend, struct mandelbrot *mb)
{
	if (!mb || !mb->max_iter)
		return;

	for (int i = 0; i < mb->rows; ++i) {
		for (int j = 0; j < mb->columns; ++j) {
			struct color c = to_color(mb->buf[i][j], mb->max_iter);
			SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, 255);
			SDL_RenderDrawPoint(rend, j, i);
		}
	}

	SDL_RenderPresent(rend);
}
