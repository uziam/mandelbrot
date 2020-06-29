#include <stdio.h>
#include <stdlib.h>

#include "render.h"

typedef struct renderer {
	SDL_Renderer *rend;
	SDL_Texture  *tex;
} renderer;

static unsigned to_color(unsigned val, unsigned max_iter)
{
	unsigned char r = 255 - val * 255.0 / max_iter;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 255;

	return (b << 24 | g << 16 | r << 8 | a);
}

renderer *render_init(SDL_Renderer *rend, unsigned width, unsigned height)
{
	if (!rend)
		return NULL;

	renderer *r = malloc(sizeof(struct renderer));
	if (!r) {
		perror("Error: failed malloc: ");
		return NULL;
	}

	r->tex = SDL_CreateTexture(rend,
	                           SDL_PIXELFORMAT_BGRA8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           width, height);
	if (!r->tex) {
		fprintf(stderr, "Error: failed to create texture: %s",
		        SDL_GetError());
		goto out_free_err;
	}

	r->rend = rend;

	return r;

out_free_err:
	free(r);
	return NULL;
}

void render_show(renderer *r, struct mandelbrot *mb)
{
	if (!mb || !mb->max_iter)
		return;

	int pitch;
	int *pixels;

	if (SDL_LockTexture(r->tex, NULL, (void **) &pixels, &pitch)) {
		fprintf(stderr, "Error: failed to lock texture: %s",
		        SDL_GetError());
		return;
	}

	#pragma omp parallel for
	for (unsigned x = 0; x < mb->width; ++x) {
		for (unsigned y = 0; y < mb->height; ++y) {
			unsigned i = y * mb->width + x;
			pixels[i] = to_color(mb->pixels[i], mb->max_iter);
		}
	}

	SDL_UnlockTexture(r->tex);

	if (SDL_RenderCopy(r->rend, r->tex, NULL, NULL))
		fprintf(stderr, "Error: failed to render: %s", SDL_GetError());

	SDL_RenderPresent(r->rend);
}
