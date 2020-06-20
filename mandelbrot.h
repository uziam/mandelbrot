#ifndef MANDELBROT_H
#define MANDELBROT_H

struct mandelbrot {
	unsigned rows;
	unsigned columns;
	unsigned max_iter;
	unsigned **buf;
};

struct mandelbrot *mandelbrot_new(unsigned rows, unsigned columns);
void mandelbrot_compute(struct mandelbrot *mb, unsigned max_iter, double xmin,
                        double xmax, double ymin, double ymax);

#endif /* MANDELBROT_H */
