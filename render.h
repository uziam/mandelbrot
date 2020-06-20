#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

#include "mandelbrot.h"

void render(SDL_Renderer *rend, struct mandelbrot *mb);

#endif /* RENDER_H */