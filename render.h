#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

#include "mandelbrot.h"

typedef struct renderer renderer;

renderer *render_init(SDL_Renderer *rend, unsigned width, unsigned height);
void render_show(renderer *rend, struct mandelbrot *mb);
int render_save(renderer *rend, const char *filename);

#endif /* RENDER_H */
