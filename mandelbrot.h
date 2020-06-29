#ifndef MANDELBROT_H
#define MANDELBROT_H

struct mandelbrot {
	unsigned width;
	unsigned height;
	unsigned max_iter;
	unsigned *pixels;
};

struct mandelbrot *mandelbrot_new(unsigned width, unsigned height);
void mandelbrot_compute(struct mandelbrot *mb, unsigned max_iter, double xmin,
                        double xmax, double ymin, double ymax);

#endif /* MANDELBROT_H */
