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

void render_show(SDL_Renderer *rend, struct mandelbrot *mb)
{
	if (!mb || !mb->max_iter)
		return;

	for (unsigned x = 0; x < mb->width; ++x) {
		for (unsigned y = 0; y < mb->height; ++y) {
			unsigned i = y * mb->width + x;
			struct color c = to_color(mb->pixels[i], mb->max_iter);
			SDL_SetRenderDrawColor(rend, c.r, c.g, c.b, 255);
			SDL_RenderDrawPoint(rend, x, y);
		}
	}

	SDL_RenderPresent(rend);
}
