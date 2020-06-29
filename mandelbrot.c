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
 * the number of iterations needed to reach conclusion.
 */
static void mandelbrot(__m256d x, __m256d y, unsigned buf[4], unsigned max_iter)
{
	const __m128i k1  = _mm_set1_epi32(1);
	const __m256d k4  = _mm256_set_pd(4, 4, 4, 4);

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
			__m256d test = _mm256_cmp_pd(mod2, k4, _CMP_LT_OQ);

			/* Optimized for amd */
			__m128 hi =
				_mm_castpd_ps(_mm256_extractf128_pd(test, 1));

			__m128 lo =
				_mm_castpd_ps(_mm256_castpd256_pd128(test));

			__m128 even =
				_mm_shuffle_ps(lo, hi, _MM_SHUFFLE(2, 0, 2, 0));

			__m128i testi = _mm_castps_si128(even);

			/* Optimized for intel
			__m256i testi256 = _mm256_castpd_si256(test);
			__m256i map = _mm256_set_epi32(7, 5, 3, 1, 6, 4, 2, 0);
			testi256 = _mm256_permutevar8x32_epi32(testi256, map);
			__m128i testi    = _mm256_castsi256_si128(testi256);
			*/

			count = _mm_sub_epi32(count, testi);

			/* early break if all points fail test */
			if (_mm_test_all_zeros(testi, k1))
				break;
		}
	}

	_mm_storeu_si128((__m128i *) buf, count);
}

struct mandelbrot *mandelbrot_new(unsigned width, unsigned height)
{
	if (!width || !height) {
		fprintf(stderr, "Error: bad dimensions, width: %u, height %u\n",
		        width, height);
		return NULL;
	}

	struct mandelbrot *mb = malloc(sizeof(struct mandelbrot));
	if (!mb) {
		perror("Error: failed malloc: ");
		return NULL;
	}

	if (!(mb->pixels = malloc(width * height * sizeof(unsigned)))) {
		perror("Error: failed malloc: ");
		free(mb);
		return NULL;
	}

	mb->width    = width;
	mb->height   = height;
	mb->max_iter = 0;

	return mb;
}

void mandelbrot_compute(struct mandelbrot *mb, unsigned max_iter, double xmin,
                        double xmax, double ymin, double ymax)
{
	if (!mb)
		return;

	mb->max_iter = max_iter;

	double dx = (xmax - xmin) / mb->width;;
	double dy = (ymax - ymin) / mb->height;

	#pragma omp parallel for
	for (unsigned i = 0; i < mb->width * mb->height; i += 4) {
		__m256d x = _mm256_set_pd(xmin + ((i + 3) % mb->width) * dx,
		                          xmin + ((i + 2) % mb->width) * dx,
		                          xmin + ((i + 1) % mb->width) * dx,
		                          xmin + ((i + 0) % mb->width) * dx);

		__m256d y = _mm256_set_pd(ymin + ((i + 3) / mb->width) * dy,
		                          ymin + ((i + 2) / mb->width) * dy,
		                          ymin + ((i + 1) / mb->width) * dy,
		                          ymin + ((i + 0) / mb->width) * dy);

		unsigned buf[4] = { 0 };

		mandelbrot(x, y, buf, max_iter);

		for (int j = 0; j < 4 && (i + j < mb->width * mb->height); j++)
			mb->pixels[i + j] = buf[j];
	}
}
