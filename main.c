#include <SDL2/SDL.h>
#include <stdio.h>
#include <unistd.h>

#include "mandelbrot.h"
#include "render.h"
#include "setting.h"

double zoom  = 1;
double xcenter = -0.75;
double ycenter = 0;
double saveimg = 0;

static SDL_Window *create_window(void)
{
	printf("Creating window (%ux%u)\n", WINDOW_WIDTH, WINDOW_HEIGHT);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
		return NULL;
	}

	Uint32 flags = 0;

	if (WINDOW_FULLSCREEN)
		flags |= SDL_WINDOW_FULLSCREEN;

	SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE,
	                                      SDL_WINDOWPOS_CENTERED,
	                                      SDL_WINDOWPOS_CENTERED,
	                                      WINDOW_WIDTH, WINDOW_HEIGHT,
	                                      flags);
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

	SDL_Renderer *sdlrend = SDL_CreateRenderer(window, -1, flags);
	if (!sdlrend) {
		fprintf(stderr, "Error creating renderer: %s\n",
		        SDL_GetError());
		return NULL;
	}

	return sdlrend;
}

static void cleanup_renderer(SDL_Renderer *sdlrend)
{
	if (!sdlrend)
		return;

	SDL_DestroyRenderer(sdlrend);
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

			if (event.key.keysym.scancode == SDL_SCANCODE_P)
				saveimg = 1;
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

static int save_img(renderer *rend)
{
	static int index = 0;
	char filename[100];

	/* find a name not currently in use */
	do {
		snprintf(filename, 100, "capture%d.png", index++);
	} while (access(filename, F_OK) != -1);

	if (render_save(rend, filename))
		return -1;

	printf("Image saved to %s\n", filename);

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

	SDL_Renderer *sdlrend = create_renderer(window);
	if (!sdlrend)
		goto out_cleanup_err;

	renderer *rend = render_init(sdlrend, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (!rend)
		goto out_cleanup_err;

	struct mandelbrot *mb = mandelbrot_new(WINDOW_WIDTH, WINDOW_HEIGHT);
	if (!mb)
		goto out_cleanup_err;

	while (!sdl_event_iteration()) {
		double xmin = xcenter - 1.75 * zoom;
		double xmax = xcenter + 1.75 * zoom;
		double ymin = ycenter - 1 * zoom;
		double ymax = ycenter + 1 * zoom;

		mandelbrot_compute(mb, max_iter, xmin, xmax, ymin, ymax);
		render_show(rend, mb);

		if (saveimg) {
			if (save_img(rend))
				goto out_cleanup_err;
			saveimg = 0;
		}
	}

	cleanup_renderer(sdlrend);
	cleanup_window(window);

	return 0;

out_cleanup_err:
	cleanup_renderer(sdlrend);
	cleanup_window(window);

	return -1;
}
