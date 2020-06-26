#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

#include "mandelbrot.h"

/*
 * let z = a + bi and c = x + yi
 *
 * f(z) = z^2 + c
 *      = (a + bi) * (a + bi) + (x + yi)
 *      = a^2 - b^2 + 2abi + x + yi
 *
 * Re(f(z)) = a^2 - b^2 + x = a^2 - (b^2 - x)
 * Im(f(z)) = (2ab + y)i
 *
 * Returns a^2 + b^2 to compute modulus
 *
 * Note: the modulus square returned is for previous iteration's a and b
 */
static __m256d mandelbrot_iter(__m256d *a, __m256d *b, __m256d x, __m256d y)
{
	__m256d a2 = _mm256_mul_pd(*a, *a);
	__m256d b2 = _mm256_mul_pd(*b, *b);

	*b = _mm256_fmadd_pd(_mm256_add_pd(*a, *a), *b, y);
	*a = _mm256_sub_pd(a2, _mm256_sub_pd(b2, x));

	return _mm256_add_pd(a2, b2);
}

/*
 * Performs mandelbrot computation on a 2x2 block of coordinates and calculates
 * the number of iterations needed to reach conclusion. The order of results
 * written to buf is based on following coordinates:
 *     buf[0]: (x0, y0)
 *     buf[1]: (x0, y1)
 *     buf[2]: (x1, y0)
 *     buf[3]: (x1, y1)
 */
static void mandelbrot(double x0, double x1, double y0, double y1,
                       unsigned buf[4], unsigned max_iter)
{
	const __m128i k1 = _mm_set1_epi32(1);
	const __m256d k4 = _mm256_set_pd(4, 4, 4, 4);
	const __m256d x  = _mm256_set_pd(x1, x1, x0, x0);
	const __m256d y  = _mm256_set_pd(y1, y0, y1, y0);

	__m256d a     = _mm256_setzero_pd();
	__m256d b     = _mm256_setzero_pd();
	__m128i count = _mm_setzero_si128();

	for (unsigned i = 0; i <= max_iter; i++) {
		/* TODO - wasted calculations here for a an b at max_iter */
		__m256d mod2 = mandelbrot_iter(&a, &b, x, y);

		/* With every iteration we get mod^2 for the last iteration,
		 * since both a and b are 0 in the first iteration skip this
		 * test on the first iteration */
		if (i) {
			/* compare mod^2 < 4, if true increase count by 1 */
			__m256d test     = _mm256_cmp_pd(mod2, k4, _CMP_LT_OQ);
			__m256i testi256 = _mm256_castpd_si256(test);
			__m128i testi    = _mm256_castsi256_si128(testi256);

			count = _mm_sub_epi32(count, testi);

			/* early break if all points fail test */
			if (_mm_test_all_zeros(testi, k1))
				break;
		}
	}

	_mm_storeu_si128((__m128i *) buf, count);
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

	double dx = (xmax - xmin) / mb->columns;;
	double dy = (ymax - ymin) / mb->rows;

	/* running computing in 4x4 chunks */
	#pragma omp parallel for
	for (int i = 0; i < mb->rows; i += 2) {
		for (int j = 0; j < mb->columns; j += 2) {
			double x0 = xmin + j * dx;
			double x1 = xmin + (j + 1) * dx;
			double y0 = ymin + i * dy;
			double y1 = ymin + (i + 1) * dy;

			unsigned buf[4] = { 0 };

			mandelbrot(x0, x1, y0, y1, buf, max_iter);

			mb->buf[i][j] = buf[0];

			if (i + 1 < mb->rows) {
				mb->buf[i + 1][j] = buf[1];
				if (j + 1 < mb->columns)
					mb->buf[i + 1][j + 1] = buf[3];
			}

			if (j + 1 < mb->columns)
				mb->buf[i][j + 1] = buf[2];
		}
	}
}
