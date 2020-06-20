#include <SDL2/SDL.h>
#include <stdio.h>

#include "mandelbrot.h"
#include "render.h"
#include "setting.h"

double zoom  = 1;
double xcenter = -1;
double ycenter = 0;

static SDL_Window *create_window(void)
{
	printf("Creating window (%ux%u)\n", WINDOW_WIDTH, WINDOW_HEIGHT);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE,
	                                      SDL_WINDOWPOS_CENTERED,
	                                      SDL_WINDOWPOS_CENTERED,
	                                      WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (!window) {
		fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	return window;
}

static void cleanup_window(SDL_Window *window)
{
	if (!window)
		return;

	SDL_DestroyWindow(window);
	SDL_Quit();
}

static SDL_Renderer *create_renderer(SDL_Window *window)
{
	Uint32 flags = 0;

	if (REND_VSYNC)
		flags |= SDL_RENDERER_PRESENTVSYNC;

	if (REND_GPU)
		flags |= SDL_RENDERER_ACCELERATED;

	SDL_Renderer *rend = SDL_CreateRenderer(window, -1, flags);
	if (!rend) {
		fprintf(stderr, "Error creating renderer: %s\n",
		        SDL_GetError());
		return NULL;
	}

	return rend;
}

static void cleanup_renderer(SDL_Renderer *rend)
{
	if (!rend)
		return;

	SDL_DestroyRenderer(rend);
}

static int sdl_event_iteration(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
				return 1;

		case SDL_KEYDOWN:
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
			    event.key.keysym.scancode == SDL_SCANCODE_Q)
				return 1;
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE)
				zoom *= 0.5;
			if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT)
				zoom /= 0.5;
			if (event.key.keysym.scancode == SDL_SCANCODE_R) {
				zoom = 1;
				xcenter = -1;
				ycenter = 0;
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_D)
				xcenter += zoom;
			if (event.key.keysym.scancode == SDL_SCANCODE_A)
				xcenter -= zoom;
			if (event.key.keysym.scancode == SDL_SCANCODE_W)
				ycenter -= zoom;
			if (event.key.keysym.scancode == SDL_SCANCODE_S)
				ycenter += zoom;
                        break;

		case SDL_KEYUP:
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
			    event.key.keysym.scancode == SDL_SCANCODE_Q)
				return 1;
			break;

		default:
			break;
		}
	}

	return 0;
}

static void print_usage(const char *prog)
{
	fprintf(stderr, "usage: %s [max_iter]\n", prog);
}

int main(int argc, char *argv[])
{
	if (argc > 2) {
		print_usage(argv[0]);
		return -1;
	}

	unsigned max_iter = 100;
	if (argc == 2) {
		max_iter = atoi(argv[1]);
		if (max_iter <= 0) {
			fprintf(stderr, "Error: max_iter must be greater than 0\n");
			return -1;
		}
	}
	printf("Max iterations %d\n", max_iter);

	SDL_Window *window = create_window();
	if (!window)
		return -1;

	SDL_Renderer *rend = create_renderer(window);
	if (!rend)
		goto out_cleanup_err;

	struct mandelbrot *mb = mandelbrot_new(WINDOW_HEIGHT, WINDOW_WIDTH);
	if (!mb)
		goto out_cleanup_err;

	while (!sdl_event_iteration()) {
		double xmin = xcenter - 1.5 * zoom;
		double xmax = xcenter + 1.5 * zoom;
		double ymin = ycenter - 0.9 * zoom; 
		double ymax = ycenter + 0.9 * zoom;

		mandelbrot_compute(mb, max_iter, xmin, xmax, ymin, ymax);
		render(rend, mb);
	}

	cleanup_renderer(rend);
	cleanup_window(window);

	return 0;

out_cleanup_err:
	cleanup_renderer(rend);
	cleanup_window(window);

	return -1;
}
