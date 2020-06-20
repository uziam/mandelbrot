#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "mandelbrot.h"

static unsigned mandelbrot(double x, double y, unsigned max_iter)
{
	double complex c = x + y * I;
	double complex z = 0 + 0 * I;

	for (unsigned i = 0; i < max_iter; i++) {
		z = z * z + c;
		if (creal(z) * creal(z) + cimag(z) * cimag(z) >= 4)
			return i;
	}

	return max_iter;
}

struct mandelbrot *mandelbrot_new(unsigned rows, unsigned columns)
{
	if (!rows || !columns) {
		fprintf(stderr, "Error: bad dimensions, rows: %u, columns %u\n",
		        rows, columns);
		return NULL;
	}

	struct mandelbrot *mb = malloc(sizeof(struct mandelbrot));
	if (!mb) {
		perror("Error: failed malloc: ");
		return NULL;
	}

	if (!(mb->buf = malloc(rows * sizeof(unsigned *)))) {
		perror("Error: failed malloc: ");
		free(mb);
		return NULL;
	}

	for (int i = 0; i < rows; ++i) {
		if (!(mb->buf[i] = malloc(columns * sizeof(unsigned)))) {
			perror("Error: failed malloc: ");
			free(mb);
			return NULL;
		}
	}

	mb->rows     = rows;
	mb->columns  = columns;
	mb->max_iter = 0;

	return mb;
}

void mandelbrot_compute(struct mandelbrot *mb, unsigned max_iter, double xmin,
                        double xmax, double ymin, double ymax)
{
	if (!mb)
		return;

	mb->max_iter = max_iter;

	double xdiff = xmax - xmin;
	double ydiff = ymax - ymin;

	#pragma omp parallel for
	for (int i = 0; i < mb->rows; ++i) {
		for (int j = 0; j < mb->columns; ++j) {
			double x = (double) j / mb->columns * xdiff + xmin;
			double y = (double) i / mb->rows * ydiff + ymin;
			mb->buf[i][j] = mandelbrot(x, y, max_iter);
		}
	}
}
