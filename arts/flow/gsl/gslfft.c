#include "gslfft.h"
#include <math.h>

#define BUTTERFLY_XY(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,Wre,Wim) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = X2re * Wre;  \
  T1im = X2im * Wre;  \
  T2re = X2im * Wim;  \
  T2im = X2re * Wim;  \
  T1re -= T2re;       \
  T1im += T2im;       \
  T2re = X1re - T1re; \
  T2im = X1im - T1im; \
  Y1re = X1re + T1re; \
  Y1im = X1im + T1im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_Yx(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,Wre,Wim) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = X2re * Wim;  \
  T1im = X2im * Wim;  \
  T2re = X2im * Wre;  \
  T2im = X2re * Wre;  \
  T1re += T2re;       \
  T1im -= T2im;       \
  T2re = X1re + T1re; \
  T2im = X1im + T1im; \
  Y1re = X1re - T1re; \
  Y1im = X1im - T1im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_yX(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,Wre,Wim) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = X2re * Wim;  \
  T1im = X2im * Wim;  \
  T2re = X2im * Wre;  \
  T2im = X2re * Wre;  \
  T1re += T2re;       \
  T1im -= T2im;       \
  T2re = X1re - T1re; \
  T2im = X1im - T1im; \
  Y1re = X1re + T1re; \
  Y1im = X1im + T1im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_10(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,_1,_2) { \
  register double T2re, T2im; \
  T2re = X1re - X2re; \
  T2im = X1im - X2im; \
  Y1re = X1re + X2re; \
  Y1im = X1im + X2im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_01(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,_1,_2) { \
  register double T2re, T2im; \
  T2re = X1re + X2im; \
  T2im = X1im - X2re; \
  Y1re = X1re - X2im; \
  Y1im = X1im + X2re; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_0m(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,_1,_2) { \
  register double T2re, T2im; \
  T2re = X1re - X2im; \
  T2im = X1im + X2re; \
  Y1re = X1re + X2im; \
  Y1im = X1im - X2re; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_XX(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,Wre,_2) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = X2re * Wre;  \
  T1im = X2im * Wre;  \
  T2re = T1im; \
  T2im = T1re; \
  T1re -= T2re;       \
  T1im += T2im;       \
  T2re = X1re - T1re; \
  T2im = X1im - T1im; \
  Y1re = X1re + T1re; \
  Y1im = X1im + T1im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_yY(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,Wre,_2) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = X2re * Wre;  \
  T1im = X2im * Wre;  \
  T2re = T1im;  \
  T2im = T1re;  \
  T1re += T2re;       \
  T1im -= T2im;       \
  T2re = X1re - T1re; \
  T2im = X1im - T1im; \
  Y1re = X1re + T1re; \
  Y1im = X1im + T1im; \
  Y2re = T2re;        \
  Y2im = T2im;        \
}
#define BUTTERFLY_10scale(X1re,X1im,X2re,X2im,Y1re,Y1im,Y2re,Y2im,S) { \
  register double T2re, T2im; \
  T2re = X1re - X2re; \
  T2im = X1im - X2im; \
  Y1re = X1re + X2re; \
  Y1im = X1im + X2im; \
  Y2re = T2re * S;    \
  Y2im = T2im * S;    \
  Y1re *= S;          \
  Y1im *= S;          \
}

#define WMULTIPLY(Wre,Wim,Dre,Dim) { \
  register double T1re, T1im, T2re, T2im; \
  T1re = Wre * Dre;  \
  T1im = Wim * Dre;  \
  T2re = Wim * Dim;  \
  T2im = Wre * Dim;  \
  T1re -= T2re;      \
  T1im += T2im;      \
  Wre += T1re;       \
  Wim += T1im;       \
}
static inline void
bitreverse_fft2analysis (const unsigned int n,
                         const double        *X,
                         double              *Y)
{
  const unsigned int n2 = n >> 1, n1 = n + n2, max = n >> 2;
  unsigned int i, r;
  
  BUTTERFLY_10 (X[0], X[1],
		X[n], X[n + 1],
		Y[0], Y[1],
		Y[2], Y[3],
		__1, __0);
  BUTTERFLY_10 (X[n2], X[n2 + 1],
		X[n1], X[n1 + 1],
		Y[4], Y[5],
		Y[6], Y[7],
		__1, __0);
  for (i = 1, r = 0; i < max; i++)
    {
      unsigned int k, j = n >> 1;

      while (r >= j)
	{
	  r -= j;
	  j >>= 1;
	}
      r |= j;

      k = r >> 1;
      j = i << 3;
      BUTTERFLY_10 (X[k], X[k + 1],
		    X[k + n], X[k + n + 1],
		    Y[j], Y[j + 1],
		    Y[j + 2], Y[j + 3],
		    __1, __0);
      k += n2;
      j += 4;
      BUTTERFLY_10 (X[k], X[k + 1],
		    X[k + n], X[k + n + 1],
		    Y[j], Y[j + 1],
		    Y[j + 2], Y[j + 3],
		    __1, __0);
    }
}
static inline void
bitreverse_fft2synthesis (const unsigned int n,
                          const double        *X,
                          double              *Y)
{
  const unsigned int n2 = n >> 1, n1 = n + n2, max = n >> 2;
  unsigned int i, r;
  double scale = n;

  scale = 1.0 / scale;
  BUTTERFLY_10scale (X[0], X[1],
		     X[n], X[n + 1],
		     Y[0], Y[1],
		     Y[2], Y[3],
		     scale);
  BUTTERFLY_10scale (X[n2], X[n2 + 1],
		     X[n1], X[n1 + 1],
		     Y[4], Y[5],
		     Y[6], Y[7],
		     scale);
  for (i = 1, r = 0; i < max; i++)
    {
      unsigned int k, j = n >> 1;

      while (r >= j)
	{
	  r -= j;
	  j >>= 1;
	}
      r |= j;

      k = r >> 1;
      j = i << 3;
      BUTTERFLY_10scale (X[k], X[k + 1],
			 X[k + n], X[k + n + 1],
			 Y[j], Y[j + 1],
			 Y[j + 2], Y[j + 3],
			 scale);
      k += n2;
      j += 4;
      BUTTERFLY_10scale (X[k], X[k + 1],
			 X[k + n], X[k + n + 1],
			 Y[j], Y[j + 1],
			 Y[j + 2], Y[j + 3],
			 scale);
    }
}
/**
 ** Generated data (by gsl-genfft 2 F)
 **/
static void
gsl_power2_fft2analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 1 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[2], X[2 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4 S F)
 **/
static void
gsl_power2_fft4analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 2 times fft2 */

  /* perform 1 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_01 (Y[2],               /* W1 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4 F X)
 **/
static void
gsl_power2_fft4analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 2 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[4], X[4 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[2], X[2 + 1],
                  X[6], X[6 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  __1, __0);

  /* perform 1 times fft4 */
  gsl_power2_fft4analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8 S F F)
 **/
static void
gsl_power2_fft8analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 4 times fft2 */

  /* perform 2 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_01 (Y[2],               /* W2 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[10],               /* W2 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);

  /* perform 1 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XX (Y[2],               /* W1 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_01 (Y[4],               /* W2 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_yY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8 F S X)
 **/
static void
gsl_power2_fft8analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 4 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[8], X[8 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[4], X[4 + 1],
                  X[12], X[12 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[2], X[2 + 1],
                  X[10], X[10 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[6], X[6 + 1],
                  X[14], X[14 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  __1, __0);

  /* skipping 2 times fft4 */

  /* perform 1 times fft8 */
  gsl_power2_fft8analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 16 S F F F)
 **/
static void
gsl_power2_fft16analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 8 times fft2 */

  /* perform 4 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                Y[16],
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[24],               /* W0 */
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                Y[24],
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_01 (Y[2],               /* W4 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[10],               /* W4 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[18],               /* W4 */
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                Y[18],
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[26],               /* W4 */
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                Y[26],
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);

  /* perform 2 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                Y[16],
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XX (Y[2],               /* W2 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XX (Y[18],               /* W2 */
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                Y[18],
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_01 (Y[4],               /* W4 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[20],               /* W4 */
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                Y[20],
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_yY (Y[6],               /* W6 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_yY (Y[22],               /* W6 */
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                Y[22],
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);

  /* perform 1 times fft16 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                Y[0],
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                Y[2],
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XX (Y[4],               /* W2 */
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                Y[4],
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                Y[6],
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_01 (Y[8],               /* W4 */
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                Y[8],
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                Y[10],
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_yY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                Y[12],
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                Y[14],
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 16 F S S X)
 **/
static void
gsl_power2_fft16analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 8 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[16], X[16 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[8], X[8 + 1],
                  X[24], X[24 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[4], X[4 + 1],
                  X[20], X[20 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[12], X[12 + 1],
                  X[28], X[28 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[2], X[2 + 1],
                  X[18], X[18 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[10], X[10 + 1],
                  X[26], X[26 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[6], X[6 + 1],
                  X[22], X[22 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[14], X[14 + 1],
                  X[30], X[30 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  __1, __0);

  /* skipping 4 times fft4 */

  /* skipping 2 times fft8 */

  /* perform 1 times fft16 */
  gsl_power2_fft16analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 32 S F F F F)
 **/
static void
gsl_power2_fft32analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 16 times fft2 */

  /* perform 8 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                Y[16],
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[24],               /* W0 */
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                Y[24],
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[36],
                Y[36 + 1],
                Y[32],
                Y[32 + 1],
                Y[36],
                Y[36 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[40],               /* W0 */
                Y[40 + 1],
                Y[44],
                Y[44 + 1],
                Y[40],
                Y[40 + 1],
                Y[44],
                Y[44 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[48],               /* W0 */
                Y[48 + 1],
                Y[52],
                Y[52 + 1],
                Y[48],
                Y[48 + 1],
                Y[52],
                Y[52 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[56],               /* W0 */
                Y[56 + 1],
                Y[60],
                Y[60 + 1],
                Y[56],
                Y[56 + 1],
                Y[60],
                Y[60 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_01 (Y[2],               /* W8 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[10],               /* W8 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[18],               /* W8 */
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                Y[18],
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[26],               /* W8 */
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                Y[26],
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[34],               /* W8 */
                Y[34 + 1],
                Y[38],
                Y[38 + 1],
                Y[34],
                Y[34 + 1],
                Y[38],
                Y[38 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[42],               /* W8 */
                Y[42 + 1],
                Y[46],
                Y[46 + 1],
                Y[42],
                Y[42 + 1],
                Y[46],
                Y[46 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[50],               /* W8 */
                Y[50 + 1],
                Y[54],
                Y[54 + 1],
                Y[50],
                Y[50 + 1],
                Y[54],
                Y[54 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[58],               /* W8 */
                Y[58 + 1],
                Y[62],
                Y[62 + 1],
                Y[58],
                Y[58 + 1],
                Y[62],
                Y[62 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);

  /* perform 4 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                Y[16],
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[40],
                Y[40 + 1],
                Y[32],
                Y[32 + 1],
                Y[40],
                Y[40 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[48],               /* W0 */
                Y[48 + 1],
                Y[56],
                Y[56 + 1],
                Y[48],
                Y[48 + 1],
                Y[56],
                Y[56 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XX (Y[2],               /* W4 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XX (Y[18],               /* W4 */
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                Y[18],
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XX (Y[34],               /* W4 */
                Y[34 + 1],
                Y[42],
                Y[42 + 1],
                Y[34],
                Y[34 + 1],
                Y[42],
                Y[42 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XX (Y[50],               /* W4 */
                Y[50 + 1],
                Y[58],
                Y[58 + 1],
                Y[50],
                Y[50 + 1],
                Y[58],
                Y[58 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_01 (Y[4],               /* W8 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[20],               /* W8 */
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                Y[20],
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[36],               /* W8 */
                Y[36 + 1],
                Y[44],
                Y[44 + 1],
                Y[36],
                Y[36 + 1],
                Y[44],
                Y[44 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[52],               /* W8 */
                Y[52 + 1],
                Y[60],
                Y[60 + 1],
                Y[52],
                Y[52 + 1],
                Y[60],
                Y[60 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_yY (Y[6],               /* W12 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_yY (Y[22],               /* W12 */
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                Y[22],
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_yY (Y[38],               /* W12 */
                Y[38 + 1],
                Y[46],
                Y[46 + 1],
                Y[38],
                Y[38 + 1],
                Y[46],
                Y[46 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_yY (Y[54],               /* W12 */
                Y[54 + 1],
                Y[62],
                Y[62 + 1],
                Y[54],
                Y[54 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);

  /* perform 2 times fft16 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                Y[0],
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[48],
                Y[48 + 1],
                Y[32],
                Y[32 + 1],
                Y[48],
                Y[48 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W2 */
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                Y[2],
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[34],               /* W2 */
                Y[34 + 1],
                Y[50],
                Y[50 + 1],
                Y[34],
                Y[34 + 1],
                Y[50],
                Y[50 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XX (Y[4],               /* W4 */
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                Y[4],
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XX (Y[36],               /* W4 */
                Y[36 + 1],
                Y[52],
                Y[52 + 1],
                Y[36],
                Y[36 + 1],
                Y[52],
                Y[52 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XY (Y[6],               /* W6 */
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                Y[6],
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[38],               /* W6 */
                Y[38 + 1],
                Y[54],
                Y[54 + 1],
                Y[38],
                Y[38 + 1],
                Y[54],
                Y[54 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_01 (Y[8],               /* W8 */
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                Y[8],
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_01 (Y[40],               /* W8 */
                Y[40 + 1],
                Y[56],
                Y[56 + 1],
                Y[40],
                Y[40 + 1],
                Y[56],
                Y[56 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_XY (Y[10],               /* W10 */
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                Y[10],
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[42],               /* W10 */
                Y[42 + 1],
                Y[58],
                Y[58 + 1],
                Y[42],
                Y[42 + 1],
                Y[58],
                Y[58 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_yY (Y[12],               /* W12 */
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                Y[12],
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_yY (Y[44],               /* W12 */
                Y[44 + 1],
                Y[60],
                Y[60 + 1],
                Y[44],
                Y[44 + 1],
                Y[60],
                Y[60 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_XY (Y[14],               /* W14 */
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                Y[14],
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[46],               /* W14 */
                Y[46 + 1],
                Y[62],
                Y[62 + 1],
                Y[46],
                Y[46 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);

  /* perform 1 times fft32 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[32],
                Y[32 + 1],
                Y[0],
                Y[0 + 1],
                Y[32],
                Y[32 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[34],
                Y[34 + 1],
                Y[2],
                Y[2 + 1],
                Y[34],
                Y[34 + 1],
                (double) +0.980785280403230, (double) +0.195090322016128);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[36],
                Y[36 + 1],
                Y[4],
                Y[4 + 1],
                Y[36],
                Y[36 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[38],
                Y[38 + 1],
                Y[6],
                Y[6 + 1],
                Y[38],
                Y[38 + 1],
                (double) +0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XX (Y[8],               /* W4 */
                Y[8 + 1],
                Y[40],
                Y[40 + 1],
                Y[8],
                Y[8 + 1],
                Y[40],
                Y[40 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[42],
                Y[42 + 1],
                Y[10],
                Y[10 + 1],
                Y[42],
                Y[42 + 1],
                (double) +0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[44],
                Y[44 + 1],
                Y[12],
                Y[12 + 1],
                Y[44],
                Y[44 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[46],
                Y[46 + 1],
                Y[14],
                Y[14 + 1],
                Y[46],
                Y[46 + 1],
                (double) +0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_01 (Y[16],               /* W8 */
                Y[16 + 1],
                Y[48],
                Y[48 + 1],
                Y[16],
                Y[16 + 1],
                Y[48],
                Y[48 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[50],
                Y[50 + 1],
                Y[18],
                Y[18 + 1],
                Y[50],
                Y[50 + 1],
                (double) -0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[52],
                Y[52 + 1],
                Y[20],
                Y[20 + 1],
                Y[52],
                Y[52 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[54],
                Y[54 + 1],
                Y[22],
                Y[22 + 1],
                Y[54],
                Y[54 + 1],
                (double) -0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_yY (Y[24],               /* W12 */
                Y[24 + 1],
                Y[56],
                Y[56 + 1],
                Y[24],
                Y[24 + 1],
                Y[56],
                Y[56 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[58],
                Y[58 + 1],
                Y[26],
                Y[26 + 1],
                Y[58],
                Y[58 + 1],
                (double) -0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[60],
                Y[60 + 1],
                Y[28],
                Y[28 + 1],
                Y[60],
                Y[60 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[62],
                Y[62 + 1],
                Y[30],
                Y[30 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.980785280403230, (double) +0.195090322016129);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 32 F S S S X)
 **/
static void
gsl_power2_fft32analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 16 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[32], X[32 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[16], X[16 + 1],
                  X[48], X[48 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[8], X[8 + 1],
                  X[40], X[40 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[24], X[24 + 1],
                  X[56], X[56 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[4], X[4 + 1],
                  X[36], X[36 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[20], X[20 + 1],
                  X[52], X[52 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[12], X[12 + 1],
                  X[44], X[44 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[28], X[28 + 1],
                  X[60], X[60 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[2], X[2 + 1],
                  X[34], X[34 + 1],
                  Y[32], Y[32 + 1],
                  Y[34], Y[34 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[18], X[18 + 1],
                  X[50], X[50 + 1],
                  Y[36], Y[36 + 1],
                  Y[38], Y[38 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[10], X[10 + 1],
                  X[42], X[42 + 1],
                  Y[40], Y[40 + 1],
                  Y[42], Y[42 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[26], X[26 + 1],
                  X[58], X[58 + 1],
                  Y[44], Y[44 + 1],
                  Y[46], Y[46 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[6], X[6 + 1],
                  X[38], X[38 + 1],
                  Y[48], Y[48 + 1],
                  Y[50], Y[50 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[22], X[22 + 1],
                  X[54], X[54 + 1],
                  Y[52], Y[52 + 1],
                  Y[54], Y[54 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[14], X[14 + 1],
                  X[46], X[46 + 1],
                  Y[56], Y[56 + 1],
                  Y[58], Y[58 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[30], X[30 + 1],
                  X[62], X[62 + 1],
                  Y[60], Y[60 + 1],
                  Y[62], Y[62 + 1],
                  __1, __0);

  /* skipping 8 times fft4 */

  /* skipping 4 times fft8 */

  /* skipping 2 times fft16 */

  /* perform 1 times fft32 */
  gsl_power2_fft32analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 64 S R R R R F)
 **/
static void
gsl_power2_fft64analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 32 times fft2 */

  /* perform 16 times fft4 */
  for (block = 0; block < 128; block += 8) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_01 (Y[block + 2],               /* W16 */
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
  }

  /* perform 8 times fft8 */
  for (block = 0; block < 128; block += 16) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XX (Y[block + 2],               /* W8 */
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_01 (Y[block + 4],               /* W16 */
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_yY (Y[block + 6],               /* W24 */
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
  }

  /* perform 4 times fft16 */
  for (block = 0; block < 128; block += 32) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W4 */
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  (double) +0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XX (Y[block + 4],               /* W8 */
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_XY (Y[block + 6],               /* W12 */
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  (double) +0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_01 (Y[block + 8],               /* W16 */
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_XY (Y[block + 10],               /* W20 */
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  (double) -0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_yY (Y[block + 12],               /* W24 */
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
    BUTTERFLY_XY (Y[block + 14],               /* W28 */
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  (double) -0.923879532511287, (double) +0.382683432365090);
  }

  /* perform 2 times fft32 */
  for (block = 0; block < 128; block += 64) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W2 */
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  (double) +0.980785280403230, (double) +0.195090322016128);
    BUTTERFLY_XY (Y[block + 4],               /* W4 */
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  (double) +0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 6],               /* W6 */
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  (double) +0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XX (Y[block + 8],               /* W8 */
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_XY (Y[block + 10],               /* W10 */
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  (double) +0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_XY (Y[block + 12],               /* W12 */
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  (double) +0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 14],               /* W14 */
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  (double) +0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_01 (Y[block + 16],               /* W16 */
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_XY (Y[block + 18],               /* W18 */
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  (double) -0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_XY (Y[block + 20],               /* W20 */
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  (double) -0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 22],               /* W22 */
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  (double) -0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_yY (Y[block + 24],               /* W24 */
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
    BUTTERFLY_XY (Y[block + 26],               /* W26 */
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  (double) -0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XY (Y[block + 28],               /* W28 */
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  (double) -0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 30],               /* W30 */
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  (double) -0.980785280403230, (double) +0.195090322016129);
  }

  /* perform 1 times fft64 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[64],
                Y[64 + 1],
                Y[0],
                Y[0 + 1],
                Y[64],
                Y[64 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[66],
                Y[66 + 1],
                Y[2],
                Y[2 + 1],
                Y[66],
                Y[66 + 1],
                (double) +0.995184726672197, (double) +0.098017140329561);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[68],
                Y[68 + 1],
                Y[4],
                Y[4 + 1],
                Y[68],
                Y[68 + 1],
                (double) +0.980785280403230, (double) +0.195090322016128);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[70],
                Y[70 + 1],
                Y[6],
                Y[6 + 1],
                Y[70],
                Y[70 + 1],
                (double) +0.956940335732209, (double) +0.290284677254462);
  BUTTERFLY_XY (Y[8],               /* W4 */
                Y[8 + 1],
                Y[72],
                Y[72 + 1],
                Y[8],
                Y[8 + 1],
                Y[72],
                Y[72 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[74],
                Y[74 + 1],
                Y[10],
                Y[10 + 1],
                Y[74],
                Y[74 + 1],
                (double) +0.881921264348355, (double) +0.471396736825998);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[76],
                Y[76 + 1],
                Y[12],
                Y[12 + 1],
                Y[76],
                Y[76 + 1],
                (double) +0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[78],
                Y[78 + 1],
                Y[14],
                Y[14 + 1],
                Y[78],
                Y[78 + 1],
                (double) +0.773010453362737, (double) +0.634393284163645);
  BUTTERFLY_XX (Y[16],               /* W8 */
                Y[16 + 1],
                Y[80],
                Y[80 + 1],
                Y[16],
                Y[16 + 1],
                Y[80],
                Y[80 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[82],
                Y[82 + 1],
                Y[18],
                Y[18 + 1],
                Y[82],
                Y[82 + 1],
                (double) +0.634393284163645, (double) +0.773010453362737);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[84],
                Y[84 + 1],
                Y[20],
                Y[20 + 1],
                Y[84],
                Y[84 + 1],
                (double) +0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[86],
                Y[86 + 1],
                Y[22],
                Y[22 + 1],
                Y[86],
                Y[86 + 1],
                (double) +0.471396736825998, (double) +0.881921264348355);
  BUTTERFLY_XY (Y[24],               /* W12 */
                Y[24 + 1],
                Y[88],
                Y[88 + 1],
                Y[24],
                Y[24 + 1],
                Y[88],
                Y[88 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[90],
                Y[90 + 1],
                Y[26],
                Y[26 + 1],
                Y[90],
                Y[90 + 1],
                (double) +0.290284677254462, (double) +0.956940335732209);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[92],
                Y[92 + 1],
                Y[28],
                Y[28 + 1],
                Y[92],
                Y[92 + 1],
                (double) +0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[94],
                Y[94 + 1],
                Y[30],
                Y[30 + 1],
                Y[94],
                Y[94 + 1],
                (double) +0.098017140329561, (double) +0.995184726672197);
  BUTTERFLY_01 (Y[32],               /* W16 */
                Y[32 + 1],
                Y[96],
                Y[96 + 1],
                Y[32],
                Y[32 + 1],
                Y[96],
                Y[96 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_XY (Y[34],               /* W17 */
                Y[34 + 1],
                Y[98],
                Y[98 + 1],
                Y[34],
                Y[34 + 1],
                Y[98],
                Y[98 + 1],
                (double) -0.098017140329561, (double) +0.995184726672197);
  BUTTERFLY_XY (Y[36],               /* W18 */
                Y[36 + 1],
                Y[100],
                Y[100 + 1],
                Y[36],
                Y[36 + 1],
                Y[100],
                Y[100 + 1],
                (double) -0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_XY (Y[38],               /* W19 */
                Y[38 + 1],
                Y[102],
                Y[102 + 1],
                Y[38],
                Y[38 + 1],
                Y[102],
                Y[102 + 1],
                (double) -0.290284677254462, (double) +0.956940335732209);
  BUTTERFLY_XY (Y[40],               /* W20 */
                Y[40 + 1],
                Y[104],
                Y[104 + 1],
                Y[40],
                Y[40 + 1],
                Y[104],
                Y[104 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[42],               /* W21 */
                Y[42 + 1],
                Y[106],
                Y[106 + 1],
                Y[42],
                Y[42 + 1],
                Y[106],
                Y[106 + 1],
                (double) -0.471396736825998, (double) +0.881921264348355);
  BUTTERFLY_XY (Y[44],               /* W22 */
                Y[44 + 1],
                Y[108],
                Y[108 + 1],
                Y[44],
                Y[44 + 1],
                Y[108],
                Y[108 + 1],
                (double) -0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_XY (Y[46],               /* W23 */
                Y[46 + 1],
                Y[110],
                Y[110 + 1],
                Y[46],
                Y[46 + 1],
                Y[110],
                Y[110 + 1],
                (double) -0.634393284163645, (double) +0.773010453362737);
  BUTTERFLY_yY (Y[48],               /* W24 */
                Y[48 + 1],
                Y[112],
                Y[112 + 1],
                Y[48],
                Y[48 + 1],
                Y[112],
                Y[112 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_XY (Y[50],               /* W25 */
                Y[50 + 1],
                Y[114],
                Y[114 + 1],
                Y[50],
                Y[50 + 1],
                Y[114],
                Y[114 + 1],
                (double) -0.773010453362737, (double) +0.634393284163645);
  BUTTERFLY_XY (Y[52],               /* W26 */
                Y[52 + 1],
                Y[116],
                Y[116 + 1],
                Y[52],
                Y[52 + 1],
                Y[116],
                Y[116 + 1],
                (double) -0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XY (Y[54],               /* W27 */
                Y[54 + 1],
                Y[118],
                Y[118 + 1],
                Y[54],
                Y[54 + 1],
                Y[118],
                Y[118 + 1],
                (double) -0.881921264348355, (double) +0.471396736825998);
  BUTTERFLY_XY (Y[56],               /* W28 */
                Y[56 + 1],
                Y[120],
                Y[120 + 1],
                Y[56],
                Y[56 + 1],
                Y[120],
                Y[120 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[58],               /* W29 */
                Y[58 + 1],
                Y[122],
                Y[122 + 1],
                Y[58],
                Y[58 + 1],
                Y[122],
                Y[122 + 1],
                (double) -0.956940335732209, (double) +0.290284677254462);
  BUTTERFLY_XY (Y[60],               /* W30 */
                Y[60 + 1],
                Y[124],
                Y[124 + 1],
                Y[60],
                Y[60 + 1],
                Y[124],
                Y[124 + 1],
                (double) -0.980785280403230, (double) +0.195090322016129);
  BUTTERFLY_XY (Y[62],               /* W31 */
                Y[62 + 1],
                Y[126],
                Y[126 + 1],
                Y[62],
                Y[62 + 1],
                Y[126],
                Y[126 + 1],
                (double) -0.995184726672197, (double) +0.098017140329561);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 64 F S S S S X)
 **/
static void
gsl_power2_fft64analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 32 times fft2 */
  BUTTERFLY_10 (X[0], X[0 + 1],
                  X[64], X[64 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[32], X[32 + 1],
                  X[96], X[96 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[16], X[16 + 1],
                  X[80], X[80 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[48], X[48 + 1],
                  X[112], X[112 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[8], X[8 + 1],
                  X[72], X[72 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[40], X[40 + 1],
                  X[104], X[104 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[24], X[24 + 1],
                  X[88], X[88 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[56], X[56 + 1],
                  X[120], X[120 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[4], X[4 + 1],
                  X[68], X[68 + 1],
                  Y[32], Y[32 + 1],
                  Y[34], Y[34 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[36], X[36 + 1],
                  X[100], X[100 + 1],
                  Y[36], Y[36 + 1],
                  Y[38], Y[38 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[20], X[20 + 1],
                  X[84], X[84 + 1],
                  Y[40], Y[40 + 1],
                  Y[42], Y[42 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[52], X[52 + 1],
                  X[116], X[116 + 1],
                  Y[44], Y[44 + 1],
                  Y[46], Y[46 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[12], X[12 + 1],
                  X[76], X[76 + 1],
                  Y[48], Y[48 + 1],
                  Y[50], Y[50 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[44], X[44 + 1],
                  X[108], X[108 + 1],
                  Y[52], Y[52 + 1],
                  Y[54], Y[54 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[28], X[28 + 1],
                  X[92], X[92 + 1],
                  Y[56], Y[56 + 1],
                  Y[58], Y[58 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[60], X[60 + 1],
                  X[124], X[124 + 1],
                  Y[60], Y[60 + 1],
                  Y[62], Y[62 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[2], X[2 + 1],
                  X[66], X[66 + 1],
                  Y[64], Y[64 + 1],
                  Y[66], Y[66 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[34], X[34 + 1],
                  X[98], X[98 + 1],
                  Y[68], Y[68 + 1],
                  Y[70], Y[70 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[18], X[18 + 1],
                  X[82], X[82 + 1],
                  Y[72], Y[72 + 1],
                  Y[74], Y[74 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[50], X[50 + 1],
                  X[114], X[114 + 1],
                  Y[76], Y[76 + 1],
                  Y[78], Y[78 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[10], X[10 + 1],
                  X[74], X[74 + 1],
                  Y[80], Y[80 + 1],
                  Y[82], Y[82 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[42], X[42 + 1],
                  X[106], X[106 + 1],
                  Y[84], Y[84 + 1],
                  Y[86], Y[86 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[26], X[26 + 1],
                  X[90], X[90 + 1],
                  Y[88], Y[88 + 1],
                  Y[90], Y[90 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[58], X[58 + 1],
                  X[122], X[122 + 1],
                  Y[92], Y[92 + 1],
                  Y[94], Y[94 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[6], X[6 + 1],
                  X[70], X[70 + 1],
                  Y[96], Y[96 + 1],
                  Y[98], Y[98 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[38], X[38 + 1],
                  X[102], X[102 + 1],
                  Y[100], Y[100 + 1],
                  Y[102], Y[102 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[22], X[22 + 1],
                  X[86], X[86 + 1],
                  Y[104], Y[104 + 1],
                  Y[106], Y[106 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[54], X[54 + 1],
                  X[118], X[118 + 1],
                  Y[108], Y[108 + 1],
                  Y[110], Y[110 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[14], X[14 + 1],
                  X[78], X[78 + 1],
                  Y[112], Y[112 + 1],
                  Y[114], Y[114 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[46], X[46 + 1],
                  X[110], X[110 + 1],
                  Y[116], Y[116 + 1],
                  Y[118], Y[118 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[30], X[30 + 1],
                  X[94], X[94 + 1],
                  Y[120], Y[120 + 1],
                  Y[122], Y[122 + 1],
                  __1, __0);
  BUTTERFLY_10 (X[62], X[62 + 1],
                  X[126], X[126 + 1],
                  Y[124], Y[124 + 1],
                  Y[126], Y[126 + 1],
                  __1, __0);

  /* skipping 16 times fft4 */

  /* skipping 8 times fft8 */

  /* skipping 4 times fft16 */

  /* skipping 2 times fft32 */

  /* perform 1 times fft64 */
  gsl_power2_fft64analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 128 S R R R R R F)
 **/
static void
gsl_power2_fft128analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 64 times fft2 */

  /* perform 32 times fft4 */
  for (block = 0; block < 256; block += 8) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_01 (Y[block + 2],               /* W32 */
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
  }

  /* perform 16 times fft8 */
  for (block = 0; block < 256; block += 16) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XX (Y[block + 2],               /* W16 */
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_01 (Y[block + 4],               /* W32 */
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_yY (Y[block + 6],               /* W48 */
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
  }

  /* perform 8 times fft16 */
  for (block = 0; block < 256; block += 32) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W8 */
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  (double) +0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XX (Y[block + 4],               /* W16 */
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_XY (Y[block + 6],               /* W24 */
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  (double) +0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_01 (Y[block + 8],               /* W32 */
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_XY (Y[block + 10],               /* W40 */
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  (double) -0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_yY (Y[block + 12],               /* W48 */
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
    BUTTERFLY_XY (Y[block + 14],               /* W56 */
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  (double) -0.923879532511287, (double) +0.382683432365090);
  }

  /* perform 4 times fft32 */
  for (block = 0; block < 256; block += 64) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W4 */
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  (double) +0.980785280403230, (double) +0.195090322016128);
    BUTTERFLY_XY (Y[block + 4],               /* W8 */
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  (double) +0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 6],               /* W12 */
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  (double) +0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XX (Y[block + 8],               /* W16 */
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_XY (Y[block + 10],               /* W20 */
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  (double) +0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_XY (Y[block + 12],               /* W24 */
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  (double) +0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 14],               /* W28 */
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  (double) +0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_01 (Y[block + 16],               /* W32 */
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_XY (Y[block + 18],               /* W36 */
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  (double) -0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_XY (Y[block + 20],               /* W40 */
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  (double) -0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 22],               /* W44 */
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  (double) -0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_yY (Y[block + 24],               /* W48 */
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
    BUTTERFLY_XY (Y[block + 26],               /* W52 */
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  (double) -0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XY (Y[block + 28],               /* W56 */
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  (double) -0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 30],               /* W60 */
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  (double) -0.980785280403230, (double) +0.195090322016129);
  }

  /* perform 2 times fft64 */
  for (block = 0; block < 256; block += 128) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 64],
                  Y[block + 64 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 64],
                  Y[block + 64 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W2 */
                  Y[block + 2 + 1],
                  Y[block + 66],
                  Y[block + 66 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 66],
                  Y[block + 66 + 1],
                  (double) +0.995184726672197, (double) +0.098017140329561);
    BUTTERFLY_XY (Y[block + 4],               /* W4 */
                  Y[block + 4 + 1],
                  Y[block + 68],
                  Y[block + 68 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 68],
                  Y[block + 68 + 1],
                  (double) +0.980785280403230, (double) +0.195090322016128);
    BUTTERFLY_XY (Y[block + 6],               /* W6 */
                  Y[block + 6 + 1],
                  Y[block + 70],
                  Y[block + 70 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 70],
                  Y[block + 70 + 1],
                  (double) +0.956940335732209, (double) +0.290284677254462);
    BUTTERFLY_XY (Y[block + 8],               /* W8 */
                  Y[block + 8 + 1],
                  Y[block + 72],
                  Y[block + 72 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 72],
                  Y[block + 72 + 1],
                  (double) +0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 10],               /* W10 */
                  Y[block + 10 + 1],
                  Y[block + 74],
                  Y[block + 74 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 74],
                  Y[block + 74 + 1],
                  (double) +0.881921264348355, (double) +0.471396736825998);
    BUTTERFLY_XY (Y[block + 12],               /* W12 */
                  Y[block + 12 + 1],
                  Y[block + 76],
                  Y[block + 76 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 76],
                  Y[block + 76 + 1],
                  (double) +0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XY (Y[block + 14],               /* W14 */
                  Y[block + 14 + 1],
                  Y[block + 78],
                  Y[block + 78 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 78],
                  Y[block + 78 + 1],
                  (double) +0.773010453362737, (double) +0.634393284163645);
    BUTTERFLY_XX (Y[block + 16],               /* W16 */
                  Y[block + 16 + 1],
                  Y[block + 80],
                  Y[block + 80 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 80],
                  Y[block + 80 + 1],
                  (double) +0.707106781186548, (double) +0.707106781186547);
    BUTTERFLY_XY (Y[block + 18],               /* W18 */
                  Y[block + 18 + 1],
                  Y[block + 82],
                  Y[block + 82 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 82],
                  Y[block + 82 + 1],
                  (double) +0.634393284163645, (double) +0.773010453362737);
    BUTTERFLY_XY (Y[block + 20],               /* W20 */
                  Y[block + 20 + 1],
                  Y[block + 84],
                  Y[block + 84 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 84],
                  Y[block + 84 + 1],
                  (double) +0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_XY (Y[block + 22],               /* W22 */
                  Y[block + 22 + 1],
                  Y[block + 86],
                  Y[block + 86 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 86],
                  Y[block + 86 + 1],
                  (double) +0.471396736825998, (double) +0.881921264348355);
    BUTTERFLY_XY (Y[block + 24],               /* W24 */
                  Y[block + 24 + 1],
                  Y[block + 88],
                  Y[block + 88 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 88],
                  Y[block + 88 + 1],
                  (double) +0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 26],               /* W26 */
                  Y[block + 26 + 1],
                  Y[block + 90],
                  Y[block + 90 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 90],
                  Y[block + 90 + 1],
                  (double) +0.290284677254462, (double) +0.956940335732209);
    BUTTERFLY_XY (Y[block + 28],               /* W28 */
                  Y[block + 28 + 1],
                  Y[block + 92],
                  Y[block + 92 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 92],
                  Y[block + 92 + 1],
                  (double) +0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_XY (Y[block + 30],               /* W30 */
                  Y[block + 30 + 1],
                  Y[block + 94],
                  Y[block + 94 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 94],
                  Y[block + 94 + 1],
                  (double) +0.098017140329561, (double) +0.995184726672197);
    BUTTERFLY_01 (Y[block + 32],               /* W32 */
                  Y[block + 32 + 1],
                  Y[block + 96],
                  Y[block + 96 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 96],
                  Y[block + 96 + 1],
                  (double) +0.000000000000000, (double) +1.000000000000000);
    BUTTERFLY_XY (Y[block + 34],               /* W34 */
                  Y[block + 34 + 1],
                  Y[block + 98],
                  Y[block + 98 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 98],
                  Y[block + 98 + 1],
                  (double) -0.098017140329561, (double) +0.995184726672197);
    BUTTERFLY_XY (Y[block + 36],               /* W36 */
                  Y[block + 36 + 1],
                  Y[block + 100],
                  Y[block + 100 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 100],
                  Y[block + 100 + 1],
                  (double) -0.195090322016128, (double) +0.980785280403230);
    BUTTERFLY_XY (Y[block + 38],               /* W38 */
                  Y[block + 38 + 1],
                  Y[block + 102],
                  Y[block + 102 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 102],
                  Y[block + 102 + 1],
                  (double) -0.290284677254462, (double) +0.956940335732209);
    BUTTERFLY_XY (Y[block + 40],               /* W40 */
                  Y[block + 40 + 1],
                  Y[block + 104],
                  Y[block + 104 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 104],
                  Y[block + 104 + 1],
                  (double) -0.382683432365090, (double) +0.923879532511287);
    BUTTERFLY_XY (Y[block + 42],               /* W42 */
                  Y[block + 42 + 1],
                  Y[block + 106],
                  Y[block + 106 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 106],
                  Y[block + 106 + 1],
                  (double) -0.471396736825998, (double) +0.881921264348355);
    BUTTERFLY_XY (Y[block + 44],               /* W44 */
                  Y[block + 44 + 1],
                  Y[block + 108],
                  Y[block + 108 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 108],
                  Y[block + 108 + 1],
                  (double) -0.555570233019602, (double) +0.831469612302545);
    BUTTERFLY_XY (Y[block + 46],               /* W46 */
                  Y[block + 46 + 1],
                  Y[block + 110],
                  Y[block + 110 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 110],
                  Y[block + 110 + 1],
                  (double) -0.634393284163645, (double) +0.773010453362737);
    BUTTERFLY_yY (Y[block + 48],               /* W48 */
                  Y[block + 48 + 1],
                  Y[block + 112],
                  Y[block + 112 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 112],
                  Y[block + 112 + 1],
                  (double) -0.707106781186547, (double) +0.707106781186548);
    BUTTERFLY_XY (Y[block + 50],               /* W50 */
                  Y[block + 50 + 1],
                  Y[block + 114],
                  Y[block + 114 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 114],
                  Y[block + 114 + 1],
                  (double) -0.773010453362737, (double) +0.634393284163645);
    BUTTERFLY_XY (Y[block + 52],               /* W52 */
                  Y[block + 52 + 1],
                  Y[block + 116],
                  Y[block + 116 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 116],
                  Y[block + 116 + 1],
                  (double) -0.831469612302545, (double) +0.555570233019602);
    BUTTERFLY_XY (Y[block + 54],               /* W54 */
                  Y[block + 54 + 1],
                  Y[block + 118],
                  Y[block + 118 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 118],
                  Y[block + 118 + 1],
                  (double) -0.881921264348355, (double) +0.471396736825998);
    BUTTERFLY_XY (Y[block + 56],               /* W56 */
                  Y[block + 56 + 1],
                  Y[block + 120],
                  Y[block + 120 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 120],
                  Y[block + 120 + 1],
                  (double) -0.923879532511287, (double) +0.382683432365090);
    BUTTERFLY_XY (Y[block + 58],               /* W58 */
                  Y[block + 58 + 1],
                  Y[block + 122],
                  Y[block + 122 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 122],
                  Y[block + 122 + 1],
                  (double) -0.956940335732209, (double) +0.290284677254462);
    BUTTERFLY_XY (Y[block + 60],               /* W60 */
                  Y[block + 60 + 1],
                  Y[block + 124],
                  Y[block + 124 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 124],
                  Y[block + 124 + 1],
                  (double) -0.980785280403230, (double) +0.195090322016129);
    BUTTERFLY_XY (Y[block + 62],               /* W62 */
                  Y[block + 62 + 1],
                  Y[block + 126],
                  Y[block + 126 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 126],
                  Y[block + 126 + 1],
                  (double) -0.995184726672197, (double) +0.098017140329561);
  }

  /* perform 1 times fft128 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[128],
                Y[128 + 1],
                Y[0],
                Y[0 + 1],
                Y[128],
                Y[128 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[130],
                Y[130 + 1],
                Y[2],
                Y[2 + 1],
                Y[130],
                Y[130 + 1],
                (double) +0.998795456205172, (double) +0.049067674327418);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[132],
                Y[132 + 1],
                Y[4],
                Y[4 + 1],
                Y[132],
                Y[132 + 1],
                (double) +0.995184726672197, (double) +0.098017140329561);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[134],
                Y[134 + 1],
                Y[6],
                Y[6 + 1],
                Y[134],
                Y[134 + 1],
                (double) +0.989176509964781, (double) +0.146730474455362);
  BUTTERFLY_XY (Y[8],               /* W4 */
                Y[8 + 1],
                Y[136],
                Y[136 + 1],
                Y[8],
                Y[8 + 1],
                Y[136],
                Y[136 + 1],
                (double) +0.980785280403230, (double) +0.195090322016128);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[138],
                Y[138 + 1],
                Y[10],
                Y[10 + 1],
                Y[138],
                Y[138 + 1],
                (double) +0.970031253194544, (double) +0.242980179903264);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[140],
                Y[140 + 1],
                Y[12],
                Y[12 + 1],
                Y[140],
                Y[140 + 1],
                (double) +0.956940335732209, (double) +0.290284677254462);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[142],
                Y[142 + 1],
                Y[14],
                Y[14 + 1],
                Y[142],
                Y[142 + 1],
                (double) +0.941544065183021, (double) +0.336889853392220);
  BUTTERFLY_XY (Y[16],               /* W8 */
                Y[16 + 1],
                Y[144],
                Y[144 + 1],
                Y[16],
                Y[16 + 1],
                Y[144],
                Y[144 + 1],
                (double) +0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[146],
                Y[146 + 1],
                Y[18],
                Y[18 + 1],
                Y[146],
                Y[146 + 1],
                (double) +0.903989293123443, (double) +0.427555093430282);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[148],
                Y[148 + 1],
                Y[20],
                Y[20 + 1],
                Y[148],
                Y[148 + 1],
                (double) +0.881921264348355, (double) +0.471396736825998);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[150],
                Y[150 + 1],
                Y[22],
                Y[22 + 1],
                Y[150],
                Y[150 + 1],
                (double) +0.857728610000272, (double) +0.514102744193222);
  BUTTERFLY_XY (Y[24],               /* W12 */
                Y[24 + 1],
                Y[152],
                Y[152 + 1],
                Y[24],
                Y[24 + 1],
                Y[152],
                Y[152 + 1],
                (double) +0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[154],
                Y[154 + 1],
                Y[26],
                Y[26 + 1],
                Y[154],
                Y[154 + 1],
                (double) +0.803207531480645, (double) +0.595699304492433);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[156],
                Y[156 + 1],
                Y[28],
                Y[28 + 1],
                Y[156],
                Y[156 + 1],
                (double) +0.773010453362737, (double) +0.634393284163645);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[158],
                Y[158 + 1],
                Y[30],
                Y[30 + 1],
                Y[158],
                Y[158 + 1],
                (double) +0.740951125354959, (double) +0.671558954847018);
  BUTTERFLY_XX (Y[32],               /* W16 */
                Y[32 + 1],
                Y[160],
                Y[160 + 1],
                Y[32],
                Y[32 + 1],
                Y[160],
                Y[160 + 1],
                (double) +0.707106781186548, (double) +0.707106781186547);
  BUTTERFLY_XY (Y[34],               /* W17 */
                Y[34 + 1],
                Y[162],
                Y[162 + 1],
                Y[34],
                Y[34 + 1],
                Y[162],
                Y[162 + 1],
                (double) +0.671558954847018, (double) +0.740951125354959);
  BUTTERFLY_XY (Y[36],               /* W18 */
                Y[36 + 1],
                Y[164],
                Y[164 + 1],
                Y[36],
                Y[36 + 1],
                Y[164],
                Y[164 + 1],
                (double) +0.634393284163645, (double) +0.773010453362737);
  BUTTERFLY_XY (Y[38],               /* W19 */
                Y[38 + 1],
                Y[166],
                Y[166 + 1],
                Y[38],
                Y[38 + 1],
                Y[166],
                Y[166 + 1],
                (double) +0.595699304492433, (double) +0.803207531480645);
  BUTTERFLY_XY (Y[40],               /* W20 */
                Y[40 + 1],
                Y[168],
                Y[168 + 1],
                Y[40],
                Y[40 + 1],
                Y[168],
                Y[168 + 1],
                (double) +0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_XY (Y[42],               /* W21 */
                Y[42 + 1],
                Y[170],
                Y[170 + 1],
                Y[42],
                Y[42 + 1],
                Y[170],
                Y[170 + 1],
                (double) +0.514102744193222, (double) +0.857728610000272);
  BUTTERFLY_XY (Y[44],               /* W22 */
                Y[44 + 1],
                Y[172],
                Y[172 + 1],
                Y[44],
                Y[44 + 1],
                Y[172],
                Y[172 + 1],
                (double) +0.471396736825998, (double) +0.881921264348355);
  BUTTERFLY_XY (Y[46],               /* W23 */
                Y[46 + 1],
                Y[174],
                Y[174 + 1],
                Y[46],
                Y[46 + 1],
                Y[174],
                Y[174 + 1],
                (double) +0.427555093430282, (double) +0.903989293123443);
  BUTTERFLY_XY (Y[48],               /* W24 */
                Y[48 + 1],
                Y[176],
                Y[176 + 1],
                Y[48],
                Y[48 + 1],
                Y[176],
                Y[176 + 1],
                (double) +0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[50],               /* W25 */
                Y[50 + 1],
                Y[178],
                Y[178 + 1],
                Y[50],
                Y[50 + 1],
                Y[178],
                Y[178 + 1],
                (double) +0.336889853392220, (double) +0.941544065183021);
  BUTTERFLY_XY (Y[52],               /* W26 */
                Y[52 + 1],
                Y[180],
                Y[180 + 1],
                Y[52],
                Y[52 + 1],
                Y[180],
                Y[180 + 1],
                (double) +0.290284677254462, (double) +0.956940335732209);
  BUTTERFLY_XY (Y[54],               /* W27 */
                Y[54 + 1],
                Y[182],
                Y[182 + 1],
                Y[54],
                Y[54 + 1],
                Y[182],
                Y[182 + 1],
                (double) +0.242980179903264, (double) +0.970031253194544);
  BUTTERFLY_XY (Y[56],               /* W28 */
                Y[56 + 1],
                Y[184],
                Y[184 + 1],
                Y[56],
                Y[56 + 1],
                Y[184],
                Y[184 + 1],
                (double) +0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_XY (Y[58],               /* W29 */
                Y[58 + 1],
                Y[186],
                Y[186 + 1],
                Y[58],
                Y[58 + 1],
                Y[186],
                Y[186 + 1],
                (double) +0.146730474455362, (double) +0.989176509964781);
  BUTTERFLY_XY (Y[60],               /* W30 */
                Y[60 + 1],
                Y[188],
                Y[188 + 1],
                Y[60],
                Y[60 + 1],
                Y[188],
                Y[188 + 1],
                (double) +0.098017140329561, (double) +0.995184726672197);
  BUTTERFLY_XY (Y[62],               /* W31 */
                Y[62 + 1],
                Y[190],
                Y[190 + 1],
                Y[62],
                Y[62 + 1],
                Y[190],
                Y[190 + 1],
                (double) +0.049067674327418, (double) +0.998795456205172);
  BUTTERFLY_01 (Y[64],               /* W32 */
                Y[64 + 1],
                Y[192],
                Y[192 + 1],
                Y[64],
                Y[64 + 1],
                Y[192],
                Y[192 + 1],
                (double) +0.000000000000000, (double) +1.000000000000000);
  BUTTERFLY_XY (Y[66],               /* W33 */
                Y[66 + 1],
                Y[194],
                Y[194 + 1],
                Y[66],
                Y[66 + 1],
                Y[194],
                Y[194 + 1],
                (double) -0.049067674327418, (double) +0.998795456205172);
  BUTTERFLY_XY (Y[68],               /* W34 */
                Y[68 + 1],
                Y[196],
                Y[196 + 1],
                Y[68],
                Y[68 + 1],
                Y[196],
                Y[196 + 1],
                (double) -0.098017140329561, (double) +0.995184726672197);
  BUTTERFLY_XY (Y[70],               /* W35 */
                Y[70 + 1],
                Y[198],
                Y[198 + 1],
                Y[70],
                Y[70 + 1],
                Y[198],
                Y[198 + 1],
                (double) -0.146730474455362, (double) +0.989176509964781);
  BUTTERFLY_XY (Y[72],               /* W36 */
                Y[72 + 1],
                Y[200],
                Y[200 + 1],
                Y[72],
                Y[72 + 1],
                Y[200],
                Y[200 + 1],
                (double) -0.195090322016128, (double) +0.980785280403230);
  BUTTERFLY_XY (Y[74],               /* W37 */
                Y[74 + 1],
                Y[202],
                Y[202 + 1],
                Y[74],
                Y[74 + 1],
                Y[202],
                Y[202 + 1],
                (double) -0.242980179903264, (double) +0.970031253194544);
  BUTTERFLY_XY (Y[76],               /* W38 */
                Y[76 + 1],
                Y[204],
                Y[204 + 1],
                Y[76],
                Y[76 + 1],
                Y[204],
                Y[204 + 1],
                (double) -0.290284677254462, (double) +0.956940335732209);
  BUTTERFLY_XY (Y[78],               /* W39 */
                Y[78 + 1],
                Y[206],
                Y[206 + 1],
                Y[78],
                Y[78 + 1],
                Y[206],
                Y[206 + 1],
                (double) -0.336889853392220, (double) +0.941544065183021);
  BUTTERFLY_XY (Y[80],               /* W40 */
                Y[80 + 1],
                Y[208],
                Y[208 + 1],
                Y[80],
                Y[80 + 1],
                Y[208],
                Y[208 + 1],
                (double) -0.382683432365090, (double) +0.923879532511287);
  BUTTERFLY_XY (Y[82],               /* W41 */
                Y[82 + 1],
                Y[210],
                Y[210 + 1],
                Y[82],
                Y[82 + 1],
                Y[210],
                Y[210 + 1],
                (double) -0.427555093430282, (double) +0.903989293123443);
  BUTTERFLY_XY (Y[84],               /* W42 */
                Y[84 + 1],
                Y[212],
                Y[212 + 1],
                Y[84],
                Y[84 + 1],
                Y[212],
                Y[212 + 1],
                (double) -0.471396736825998, (double) +0.881921264348355);
  BUTTERFLY_XY (Y[86],               /* W43 */
                Y[86 + 1],
                Y[214],
                Y[214 + 1],
                Y[86],
                Y[86 + 1],
                Y[214],
                Y[214 + 1],
                (double) -0.514102744193222, (double) +0.857728610000272);
  BUTTERFLY_XY (Y[88],               /* W44 */
                Y[88 + 1],
                Y[216],
                Y[216 + 1],
                Y[88],
                Y[88 + 1],
                Y[216],
                Y[216 + 1],
                (double) -0.555570233019602, (double) +0.831469612302545);
  BUTTERFLY_XY (Y[90],               /* W45 */
                Y[90 + 1],
                Y[218],
                Y[218 + 1],
                Y[90],
                Y[90 + 1],
                Y[218],
                Y[218 + 1],
                (double) -0.595699304492433, (double) +0.803207531480645);
  BUTTERFLY_XY (Y[92],               /* W46 */
                Y[92 + 1],
                Y[220],
                Y[220 + 1],
                Y[92],
                Y[92 + 1],
                Y[220],
                Y[220 + 1],
                (double) -0.634393284163645, (double) +0.773010453362737);
  BUTTERFLY_XY (Y[94],               /* W47 */
                Y[94 + 1],
                Y[222],
                Y[222 + 1],
                Y[94],
                Y[94 + 1],
                Y[222],
                Y[222 + 1],
                (double) -0.671558954847018, (double) +0.740951125354959);
  BUTTERFLY_yY (Y[96],               /* W48 */
                Y[96 + 1],
                Y[224],
                Y[224 + 1],
                Y[96],
                Y[96 + 1],
                Y[224],
                Y[224 + 1],
                (double) -0.707106781186547, (double) +0.707106781186548);
  BUTTERFLY_XY (Y[98],               /* W49 */
                Y[98 + 1],
                Y[226],
                Y[226 + 1],
                Y[98],
                Y[98 + 1],
                Y[226],
                Y[226 + 1],
                (double) -0.740951125354959, (double) +0.671558954847019);
  BUTTERFLY_XY (Y[100],               /* W50 */
                Y[100 + 1],
                Y[228],
                Y[228 + 1],
                Y[100],
                Y[100 + 1],
                Y[228],
                Y[228 + 1],
                (double) -0.773010453362737, (double) +0.634393284163645);
  BUTTERFLY_XY (Y[102],               /* W51 */
                Y[102 + 1],
                Y[230],
                Y[230 + 1],
                Y[102],
                Y[102 + 1],
                Y[230],
                Y[230 + 1],
                (double) -0.803207531480645, (double) +0.595699304492433);
  BUTTERFLY_XY (Y[104],               /* W52 */
                Y[104 + 1],
                Y[232],
                Y[232 + 1],
                Y[104],
                Y[104 + 1],
                Y[232],
                Y[232 + 1],
                (double) -0.831469612302545, (double) +0.555570233019602);
  BUTTERFLY_XY (Y[106],               /* W53 */
                Y[106 + 1],
                Y[234],
                Y[234 + 1],
                Y[106],
                Y[106 + 1],
                Y[234],
                Y[234 + 1],
                (double) -0.857728610000272, (double) +0.514102744193222);
  BUTTERFLY_XY (Y[108],               /* W54 */
                Y[108 + 1],
                Y[236],
                Y[236 + 1],
                Y[108],
                Y[108 + 1],
                Y[236],
                Y[236 + 1],
                (double) -0.881921264348355, (double) +0.471396736825998);
  BUTTERFLY_XY (Y[110],               /* W55 */
                Y[110 + 1],
                Y[238],
                Y[238 + 1],
                Y[110],
                Y[110 + 1],
                Y[238],
                Y[238 + 1],
                (double) -0.903989293123443, (double) +0.427555093430282);
  BUTTERFLY_XY (Y[112],               /* W56 */
                Y[112 + 1],
                Y[240],
                Y[240 + 1],
                Y[112],
                Y[112 + 1],
                Y[240],
                Y[240 + 1],
                (double) -0.923879532511287, (double) +0.382683432365090);
  BUTTERFLY_XY (Y[114],               /* W57 */
                Y[114 + 1],
                Y[242],
                Y[242 + 1],
                Y[114],
                Y[114 + 1],
                Y[242],
                Y[242 + 1],
                (double) -0.941544065183021, (double) +0.336889853392220);
  BUTTERFLY_XY (Y[116],               /* W58 */
                Y[116 + 1],
                Y[244],
                Y[244 + 1],
                Y[116],
                Y[116 + 1],
                Y[244],
                Y[244 + 1],
                (double) -0.956940335732209, (double) +0.290284677254462);
  BUTTERFLY_XY (Y[118],               /* W59 */
                Y[118 + 1],
                Y[246],
                Y[246 + 1],
                Y[118],
                Y[118 + 1],
                Y[246],
                Y[246 + 1],
                (double) -0.970031253194544, (double) +0.242980179903264);
  BUTTERFLY_XY (Y[120],               /* W60 */
                Y[120 + 1],
                Y[248],
                Y[248 + 1],
                Y[120],
                Y[120 + 1],
                Y[248],
                Y[248 + 1],
                (double) -0.980785280403230, (double) +0.195090322016129);
  BUTTERFLY_XY (Y[122],               /* W61 */
                Y[122 + 1],
                Y[250],
                Y[250 + 1],
                Y[122],
                Y[122 + 1],
                Y[250],
                Y[250 + 1],
                (double) -0.989176509964781, (double) +0.146730474455362);
  BUTTERFLY_XY (Y[124],               /* W62 */
                Y[124 + 1],
                Y[252],
                Y[252 + 1],
                Y[124],
                Y[124 + 1],
                Y[252],
                Y[252 + 1],
                (double) -0.995184726672197, (double) +0.098017140329561);
  BUTTERFLY_XY (Y[126],               /* W63 */
                Y[126 + 1],
                Y[254],
                Y[254 + 1],
                Y[126],
                Y[126 + 1],
                Y[254],
                Y[254 + 1],
                (double) -0.998795456205172, (double) +0.049067674327418);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 128 L S S S S S X)
 **/
static void
gsl_power2_fft128analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (128, X, Y);

  /* skipping 32 times fft4 */

  /* skipping 16 times fft8 */

  /* skipping 8 times fft16 */

  /* skipping 4 times fft32 */

  /* skipping 2 times fft64 */

  /* perform 1 times fft128 */
  gsl_power2_fft128analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 256 S S S S S S X T)
 **/
static void
gsl_power2_fft256analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 128 times fft2 */

  /* skipping 64 times fft4 */

  /* skipping 32 times fft8 */

  /* skipping 16 times fft16 */

  /* skipping 8 times fft32 */

  /* skipping 4 times fft64 */

  /* perform 2 times fft128 */
  gsl_power2_fft128analysis_skip2 (X, Y);
  gsl_power2_fft128analysis_skip2 (X + 256, Y + 256);

  /* perform 1 times fft256 */
  {
    static const double Wconst256[] = {
      +0.999698818696204, +0.024541228522912,
      +0.998795456205172, +0.049067674327418,
      +0.997290456678690, +0.073564563599667,
      +0.995184726672197, +0.098017140329561,
      +0.992479534598710, +0.122410675199216,
      +0.989176509964781, +0.146730474455362,
      +0.985277642388941, +0.170961888760301,
      +0.980785280403230, +0.195090322016128,
      +0.975702130038529, +0.219101240156870,
      +0.970031253194544, +0.242980179903264,
      +0.963776065795440, +0.266712757474898,
      +0.956940335732209, +0.290284677254462,
      +0.949528180593037, +0.313681740398892,
      +0.941544065183021, +0.336889853392220,
      +0.932992798834739, +0.359895036534988,
      +0.923879532511287, +0.382683432365090,
      +0.914209755703531, +0.405241314004990,
      +0.903989293123443, +0.427555093430282,
      +0.893224301195515, +0.449611329654607,
      +0.881921264348355, +0.471396736825998,
      +0.870086991108711, +0.492898192229784,
      +0.857728610000272, +0.514102744193222,
      +0.844853565249707, +0.534997619887097,
      +0.831469612302545, +0.555570233019602,
      +0.817584813151584, +0.575808191417845,
      +0.803207531480645, +0.595699304492433,
      +0.788346427626606, +0.615231590580627,
      +0.773010453362737, +0.634393284163645,
      +0.757208846506485, +0.653172842953777,
      +0.740951125354959, +0.671558954847018,
      +0.724247082951467, +0.689540544737067,
      +0.707106781186548, +0.707106781186547,
      +0.689540544737067, +0.724247082951467,
      +0.671558954847018, +0.740951125354959,
      +0.653172842953777, +0.757208846506484,
      +0.634393284163645, +0.773010453362737,
      +0.615231590580627, +0.788346427626606,
      +0.595699304492433, +0.803207531480645,
      +0.575808191417845, +0.817584813151584,
      +0.555570233019602, +0.831469612302545,
      +0.534997619887097, +0.844853565249707,
      +0.514102744193222, +0.857728610000272,
      +0.492898192229784, +0.870086991108711,
      +0.471396736825998, +0.881921264348355,
      +0.449611329654607, +0.893224301195515,
      +0.427555093430282, +0.903989293123443,
      +0.405241314004990, +0.914209755703531,
      +0.382683432365090, +0.923879532511287,
      +0.359895036534988, +0.932992798834739,
      +0.336889853392220, +0.941544065183021,
      +0.313681740398892, +0.949528180593037,
      +0.290284677254462, +0.956940335732209,
      +0.266712757474898, +0.963776065795440,
      +0.242980179903264, +0.970031253194544,
      +0.219101240156870, +0.975702130038529,
      +0.195090322016128, +0.980785280403230,
      +0.170961888760301, +0.985277642388941,
      +0.146730474455362, +0.989176509964781,
      +0.122410675199216, +0.992479534598710,
      +0.098017140329561, +0.995184726672197,
      +0.073564563599667, +0.997290456678690,
      +0.049067674327418, +0.998795456205172,
      +0.024541228522912, +0.999698818696204,
    };
    const double *W = Wconst256 - 2;
    double *Z = Y + 128;
    for (offset = 0; offset < 512; offset += 512) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    __1, __0);
      BUTTERFLY_01 (Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 128; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 512; block += 512) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Wre, Wim);
        BUTTERFLY_Yx (Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Wre, Wim);
      }
    }
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 256 L S S S S S S X)
 **/
static void
gsl_power2_fft256analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (256, X, Y);

  /* skipping 64 times fft4 */

  /* skipping 32 times fft8 */

  /* skipping 16 times fft16 */

  /* skipping 8 times fft32 */

  /* skipping 4 times fft64 */

  /* skipping 2 times fft128 */

  /* perform 1 times fft256 */
  gsl_power2_fft256analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 512 S S S S S S X T T)
 **/
static void
gsl_power2_fft512analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 256 times fft2 */

  /* skipping 128 times fft4 */

  /* skipping 64 times fft8 */

  /* skipping 32 times fft16 */

  /* skipping 16 times fft32 */

  /* skipping 8 times fft64 */

  /* perform 4 times fft128 */
  gsl_power2_fft128analysis_skip2 (X, Y);
  gsl_power2_fft128analysis_skip2 (X + 256, Y + 256);
  gsl_power2_fft128analysis_skip2 (X + 512, Y + 512);
  gsl_power2_fft128analysis_skip2 (X + 768, Y + 768);

  /* perform 2 times fft256 */
  {
    static const double Wconst256[] = {
      +0.999698818696204, +0.024541228522912,
      +0.998795456205172, +0.049067674327418,
      +0.997290456678690, +0.073564563599667,
      +0.995184726672197, +0.098017140329561,
      +0.992479534598710, +0.122410675199216,
      +0.989176509964781, +0.146730474455362,
      +0.985277642388941, +0.170961888760301,
      +0.980785280403230, +0.195090322016128,
      +0.975702130038529, +0.219101240156870,
      +0.970031253194544, +0.242980179903264,
      +0.963776065795440, +0.266712757474898,
      +0.956940335732209, +0.290284677254462,
      +0.949528180593037, +0.313681740398892,
      +0.941544065183021, +0.336889853392220,
      +0.932992798834739, +0.359895036534988,
      +0.923879532511287, +0.382683432365090,
      +0.914209755703531, +0.405241314004990,
      +0.903989293123443, +0.427555093430282,
      +0.893224301195515, +0.449611329654607,
      +0.881921264348355, +0.471396736825998,
      +0.870086991108711, +0.492898192229784,
      +0.857728610000272, +0.514102744193222,
      +0.844853565249707, +0.534997619887097,
      +0.831469612302545, +0.555570233019602,
      +0.817584813151584, +0.575808191417845,
      +0.803207531480645, +0.595699304492433,
      +0.788346427626606, +0.615231590580627,
      +0.773010453362737, +0.634393284163645,
      +0.757208846506485, +0.653172842953777,
      +0.740951125354959, +0.671558954847018,
      +0.724247082951467, +0.689540544737067,
      +0.707106781186548, +0.707106781186547,
      +0.689540544737067, +0.724247082951467,
      +0.671558954847018, +0.740951125354959,
      +0.653172842953777, +0.757208846506484,
      +0.634393284163645, +0.773010453362737,
      +0.615231590580627, +0.788346427626606,
      +0.595699304492433, +0.803207531480645,
      +0.575808191417845, +0.817584813151584,
      +0.555570233019602, +0.831469612302545,
      +0.534997619887097, +0.844853565249707,
      +0.514102744193222, +0.857728610000272,
      +0.492898192229784, +0.870086991108711,
      +0.471396736825998, +0.881921264348355,
      +0.449611329654607, +0.893224301195515,
      +0.427555093430282, +0.903989293123443,
      +0.405241314004990, +0.914209755703531,
      +0.382683432365090, +0.923879532511287,
      +0.359895036534988, +0.932992798834739,
      +0.336889853392220, +0.941544065183021,
      +0.313681740398892, +0.949528180593037,
      +0.290284677254462, +0.956940335732209,
      +0.266712757474898, +0.963776065795440,
      +0.242980179903264, +0.970031253194544,
      +0.219101240156870, +0.975702130038529,
      +0.195090322016128, +0.980785280403230,
      +0.170961888760301, +0.985277642388941,
      +0.146730474455362, +0.989176509964781,
      +0.122410675199216, +0.992479534598710,
      +0.098017140329561, +0.995184726672197,
      +0.073564563599667, +0.997290456678690,
      +0.049067674327418, +0.998795456205172,
      +0.024541228522912, +0.999698818696204,
    };
    const double *W = Wconst256 - 2;
    double *Z = Y + 128;
    for (offset = 0; offset < 1024; offset += 512) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    __1, __0);
      BUTTERFLY_01 (Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 128; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 1024; block += 512) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Wre, Wim);
        BUTTERFLY_Yx (Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Wre, Wim);
      }
    }
  }

  /* perform 1 times fft512 */
  {
    static const double Wconst512[] = {
      +0.999924701839145, +0.012271538285720,
      +0.999698818696204, +0.024541228522912,
      +0.999322384588350, +0.036807222941359,
      +0.998795456205172, +0.049067674327418,
      +0.998118112900149, +0.061320736302209,
      +0.997290456678690, +0.073564563599667,
      +0.996312612182778, +0.085797312344440,
      +0.995184726672197, +0.098017140329561,
      +0.993906970002356, +0.110222207293883,
      +0.992479534598710, +0.122410675199216,
      +0.990902635427780, +0.134580708507126,
      +0.989176509964781, +0.146730474455362,
      +0.987301418157858, +0.158858143333861,
      +0.985277642388941, +0.170961888760301,
      +0.983105487431216, +0.183039887955141,
      +0.980785280403230, +0.195090322016128,
      +0.978317370719628, +0.207111376192219,
      +0.975702130038529, +0.219101240156870,
      +0.972939952205560, +0.231058108280671,
      +0.970031253194544, +0.242980179903264,
      +0.966976471044852, +0.254865659604515,
      +0.963776065795440, +0.266712757474898,
      +0.960430519415566, +0.278519689385053,
      +0.956940335732209, +0.290284677254462,
      +0.953306040354194, +0.302005949319228,
      +0.949528180593037, +0.313681740398892,
      +0.945607325380521, +0.325310292162263,
      +0.941544065183021, +0.336889853392220,
      +0.937339011912575, +0.348418680249435,
      +0.932992798834739, +0.359895036534988,
      +0.928506080473216, +0.371317193951838,
      +0.923879532511287, +0.382683432365090,
      +0.919113851690058, +0.393992040061048,
      +0.914209755703531, +0.405241314004990,
      +0.909167983090522, +0.416429560097637,
      +0.903989293123443, +0.427555093430282,
      +0.898674465693954, +0.438616238538528,
      +0.893224301195515, +0.449611329654607,
      +0.887639620402854, +0.460538710958240,
      +0.881921264348355, +0.471396736825998,
      +0.876070094195407, +0.482183772079123,
      +0.870086991108711, +0.492898192229784,
      +0.863972856121587, +0.503538383725718,
      +0.857728610000272, +0.514102744193222,
      +0.851355193105265, +0.524589682678469,
      +0.844853565249707, +0.534997619887097,
      +0.838224705554838, +0.545324988422046,
      +0.831469612302545, +0.555570233019602,
      +0.824589302785025, +0.565731810783613,
      +0.817584813151584, +0.575808191417845,
      +0.810457198252595, +0.585797857456439,
      +0.803207531480645, +0.595699304492433,
      +0.795836904608884, +0.605511041404326,
      +0.788346427626606, +0.615231590580627,
      +0.780737228572094, +0.624859488142386,
      +0.773010453362737, +0.634393284163645,
      +0.765167265622459, +0.643831542889791,
      +0.757208846506485, +0.653172842953777,
      +0.749136394523459, +0.662415777590172,
      +0.740951125354959, +0.671558954847018,
      +0.732654271672413, +0.680600997795453,
      +0.724247082951467, +0.689540544737067,
      +0.715730825283819, +0.698376249408973,
      +0.707106781186548, +0.707106781186547,
      +0.698376249408973, +0.715730825283819,
      +0.689540544737067, +0.724247082951467,
      +0.680600997795453, +0.732654271672413,
      +0.671558954847018, +0.740951125354959,
      +0.662415777590172, +0.749136394523459,
      +0.653172842953777, +0.757208846506484,
      +0.643831542889791, +0.765167265622459,
      +0.634393284163645, +0.773010453362737,
      +0.624859488142386, +0.780737228572094,
      +0.615231590580627, +0.788346427626606,
      +0.605511041404326, +0.795836904608883,
      +0.595699304492433, +0.803207531480645,
      +0.585797857456439, +0.810457198252595,
      +0.575808191417845, +0.817584813151584,
      +0.565731810783613, +0.824589302785025,
      +0.555570233019602, +0.831469612302545,
      +0.545324988422046, +0.838224705554838,
      +0.534997619887097, +0.844853565249707,
      +0.524589682678469, +0.851355193105265,
      +0.514102744193222, +0.857728610000272,
      +0.503538383725718, +0.863972856121587,
      +0.492898192229784, +0.870086991108711,
      +0.482183772079123, +0.876070094195407,
      +0.471396736825998, +0.881921264348355,
      +0.460538710958240, +0.887639620402854,
      +0.449611329654607, +0.893224301195515,
      +0.438616238538528, +0.898674465693954,
      +0.427555093430282, +0.903989293123443,
      +0.416429560097637, +0.909167983090522,
      +0.405241314004990, +0.914209755703531,
      +0.393992040061048, +0.919113851690058,
      +0.382683432365090, +0.923879532511287,
      +0.371317193951838, +0.928506080473215,
      +0.359895036534988, +0.932992798834739,
      +0.348418680249435, +0.937339011912575,
      +0.336889853392220, +0.941544065183021,
      +0.325310292162263, +0.945607325380521,
      +0.313681740398892, +0.949528180593037,
      +0.302005949319228, +0.953306040354194,
      +0.290284677254462, +0.956940335732209,
      +0.278519689385053, +0.960430519415566,
      +0.266712757474898, +0.963776065795440,
      +0.254865659604515, +0.966976471044852,
      +0.242980179903264, +0.970031253194544,
      +0.231058108280671, +0.972939952205560,
      +0.219101240156870, +0.975702130038529,
      +0.207111376192219, +0.978317370719628,
      +0.195090322016128, +0.980785280403230,
      +0.183039887955141, +0.983105487431216,
      +0.170961888760301, +0.985277642388941,
      +0.158858143333861, +0.987301418157858,
      +0.146730474455362, +0.989176509964781,
      +0.134580708507126, +0.990902635427780,
      +0.122410675199216, +0.992479534598710,
      +0.110222207293883, +0.993906970002356,
      +0.098017140329561, +0.995184726672197,
      +0.085797312344440, +0.996312612182778,
      +0.073564563599667, +0.997290456678690,
      +0.061320736302209, +0.998118112900149,
      +0.049067674327418, +0.998795456205172,
      +0.036807222941359, +0.999322384588350,
      +0.024541228522912, +0.999698818696204,
      +0.012271538285720, +0.999924701839145,
    };
    const double *W = Wconst512 - 2;
    double *Z = Y + 256;
    for (offset = 0; offset < 1024; offset += 1024) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 512], Y[offset + 512 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 512], Y[offset + 512 + 1],
                    __1, __0);
      BUTTERFLY_01 (Z[offset], Z[offset + 1],
                    Z[offset + 512], Z[offset + 512 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 512], Z[offset + 512 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 256; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 1024; block += 1024) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 512], Y[offset + 512 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 512], Y[offset + 512 + 1],
                      Wre, Wim);
        BUTTERFLY_Yx (Z[offset], Z[offset + 1],
                      Z[offset + 512], Z[offset + 512 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 512], Z[offset + 512 + 1],
                      Wre, Wim);
      }
    }
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 512 L S S S S S S S X)
 **/
static void
gsl_power2_fft512analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (512, X, Y);

  /* skipping 128 times fft4 */

  /* skipping 64 times fft8 */

  /* skipping 32 times fft16 */

  /* skipping 16 times fft32 */

  /* skipping 8 times fft64 */

  /* skipping 4 times fft128 */

  /* skipping 2 times fft256 */

  /* perform 1 times fft512 */
  gsl_power2_fft512analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 1024 S S S S S S S S X L)
 **/
static void
gsl_power2_fft1024analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 512 times fft2 */

  /* skipping 256 times fft4 */

  /* skipping 128 times fft8 */

  /* skipping 64 times fft16 */

  /* skipping 32 times fft32 */

  /* skipping 16 times fft64 */

  /* skipping 8 times fft128 */

  /* skipping 4 times fft256 */

  /* perform 2 times fft512 */
  gsl_power2_fft512analysis_skip2 (X, Y);
  gsl_power2_fft512analysis_skip2 (X + 1024, Y + 1024);

  /* perform 1 times fft1024 */
  for (offset = 0; offset < 2048; offset += 2048) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __1, __0);
  }
  Wre = +0.999981175282601; Wim = +0.006135884649154;
  for (butterfly = 2; butterfly < 512; butterfly += 2) {
    for (block = 0; block < 2048; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, +0.006135884649154);
  }
  for (offset = 512; offset < 2048; offset += 2048) {
    BUTTERFLY_01 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __0, __1);
  }
  Wre = -0.006135884649154; Wim = +0.999981175282601;
  for (butterfly = 514; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 2048; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, +0.006135884649154);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 1024 L S S S S S S S S X)
 **/
static void
gsl_power2_fft1024analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (1024, X, Y);

  /* skipping 256 times fft4 */

  /* skipping 128 times fft8 */

  /* skipping 64 times fft16 */

  /* skipping 32 times fft32 */

  /* skipping 16 times fft64 */

  /* skipping 8 times fft128 */

  /* skipping 4 times fft256 */

  /* skipping 2 times fft512 */

  /* perform 1 times fft1024 */
  gsl_power2_fft1024analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 2048 S S S S S S S S X L L)
 **/
static void
gsl_power2_fft2048analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 1024 times fft2 */

  /* skipping 512 times fft4 */

  /* skipping 256 times fft8 */

  /* skipping 128 times fft16 */

  /* skipping 64 times fft32 */

  /* skipping 32 times fft64 */

  /* skipping 16 times fft128 */

  /* skipping 8 times fft256 */

  /* perform 4 times fft512 */
  gsl_power2_fft512analysis_skip2 (X, Y);
  gsl_power2_fft512analysis_skip2 (X + 1024, Y + 1024);
  gsl_power2_fft512analysis_skip2 (X + 2048, Y + 2048);
  gsl_power2_fft512analysis_skip2 (X + 3072, Y + 3072);

  /* perform 2 times fft1024 */
  for (offset = 0; offset < 4096; offset += 2048) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __1, __0);
  }
  Wre = +0.999981175282601; Wim = +0.006135884649154;
  for (butterfly = 2; butterfly < 512; butterfly += 2) {
    for (block = 0; block < 4096; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, +0.006135884649154);
  }
  for (offset = 512; offset < 4096; offset += 2048) {
    BUTTERFLY_01 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __0, __1);
  }
  Wre = -0.006135884649154; Wim = +0.999981175282601;
  for (butterfly = 514; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 4096; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, +0.006135884649154);
  }

  /* perform 1 times fft2048 */
  for (offset = 0; offset < 4096; offset += 4096) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  __1, __0);
  }
  Wre = +0.999995293809576; Wim = +0.003067956762966;
  for (butterfly = 2; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 4096; block += 4096) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000004706190424, +0.003067956762966);
  }
  for (offset = 1024; offset < 4096; offset += 4096) {
    BUTTERFLY_01 (Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  __0, __1);
  }
  Wre = -0.003067956762966; Wim = +0.999995293809576;
  for (butterfly = 1026; butterfly < 2048; butterfly += 2) {
    for (block = 0; block < 4096; block += 4096) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000004706190424, +0.003067956762966);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 2048 L S S S S S S S S S X)
 **/
static void
gsl_power2_fft2048analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (2048, X, Y);

  /* skipping 512 times fft4 */

  /* skipping 256 times fft8 */

  /* skipping 128 times fft16 */

  /* skipping 64 times fft32 */

  /* skipping 32 times fft64 */

  /* skipping 16 times fft128 */

  /* skipping 8 times fft256 */

  /* skipping 4 times fft512 */

  /* skipping 2 times fft1024 */

  /* perform 1 times fft2048 */
  gsl_power2_fft2048analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4096 S S S S S S S S S S X L)
 **/
static void
gsl_power2_fft4096analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 2048 times fft2 */

  /* skipping 1024 times fft4 */

  /* skipping 512 times fft8 */

  /* skipping 256 times fft16 */

  /* skipping 128 times fft32 */

  /* skipping 64 times fft64 */

  /* skipping 32 times fft128 */

  /* skipping 16 times fft256 */

  /* skipping 8 times fft512 */

  /* skipping 4 times fft1024 */

  /* perform 2 times fft2048 */
  gsl_power2_fft2048analysis_skip2 (X, Y);
  gsl_power2_fft2048analysis_skip2 (X + 4096, Y + 4096);

  /* perform 1 times fft4096 */
  for (offset = 0; offset < 8192; offset += 8192) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  __1, __0);
  }
  Wre = +0.999998823451702; Wim = +0.001533980186285;
  for (butterfly = 2; butterfly < 2048; butterfly += 2) {
    for (block = 0; block < 8192; block += 8192) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000001176548298, +0.001533980186285);
  }
  for (offset = 2048; offset < 8192; offset += 8192) {
    BUTTERFLY_01 (Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  __0, __1);
  }
  Wre = -0.001533980186285; Wim = +0.999998823451702;
  for (butterfly = 2050; butterfly < 4096; butterfly += 2) {
    for (block = 0; block < 8192; block += 8192) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000001176548298, +0.001533980186285);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4096 L S S S S S S S S S S X)
 **/
static void
gsl_power2_fft4096analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (4096, X, Y);

  /* skipping 1024 times fft4 */

  /* skipping 512 times fft8 */

  /* skipping 256 times fft16 */

  /* skipping 128 times fft32 */

  /* skipping 64 times fft64 */

  /* skipping 32 times fft128 */

  /* skipping 16 times fft256 */

  /* skipping 8 times fft512 */

  /* skipping 4 times fft1024 */

  /* skipping 2 times fft2048 */

  /* perform 1 times fft4096 */
  gsl_power2_fft4096analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8192 S S S S S S S S S S S X L)
 **/
static void
gsl_power2_fft8192analysis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 4096 times fft2 */

  /* skipping 2048 times fft4 */

  /* skipping 1024 times fft8 */

  /* skipping 512 times fft16 */

  /* skipping 256 times fft32 */

  /* skipping 128 times fft64 */

  /* skipping 64 times fft128 */

  /* skipping 32 times fft256 */

  /* skipping 16 times fft512 */

  /* skipping 8 times fft1024 */

  /* skipping 4 times fft2048 */

  /* perform 2 times fft4096 */
  gsl_power2_fft4096analysis_skip2 (X, Y);
  gsl_power2_fft4096analysis_skip2 (X + 8192, Y + 8192);

  /* perform 1 times fft8192 */
  for (offset = 0; offset < 16384; offset += 16384) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  __1, __0);
  }
  Wre = +0.999999705862882; Wim = +0.000766990318743;
  for (butterfly = 2; butterfly < 4096; butterfly += 2) {
    for (block = 0; block < 16384; block += 16384) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000000294137118, +0.000766990318743);
  }
  for (offset = 4096; offset < 16384; offset += 16384) {
    BUTTERFLY_01 (Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  __0, __1);
  }
  Wre = -0.000766990318743; Wim = +0.999999705862882;
  for (butterfly = 4098; butterfly < 8192; butterfly += 2) {
    for (block = 0; block < 16384; block += 16384) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000000294137118, +0.000766990318743);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8192 L S S S S S S S S S S S X)
 **/
static void
gsl_power2_fft8192analysis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2analysis (8192, X, Y);

  /* skipping 2048 times fft4 */

  /* skipping 1024 times fft8 */

  /* skipping 512 times fft16 */

  /* skipping 256 times fft32 */

  /* skipping 128 times fft64 */

  /* skipping 64 times fft128 */

  /* skipping 32 times fft256 */

  /* skipping 16 times fft512 */

  /* skipping 8 times fft1024 */

  /* skipping 4 times fft2048 */

  /* skipping 2 times fft4096 */

  /* perform 1 times fft8192 */
  gsl_power2_fft8192analysis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 2 F)
 **/
static void
gsl_power2_fft2synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 1 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[2], X[2 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 2);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4 S F)
 **/
static void
gsl_power2_fft4synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 2 times fft2 */

  /* perform 1 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_0m (Y[2],               /* W1 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4 F X)
 **/
static void
gsl_power2_fft4synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 2 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[4], X[4 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 4);
  BUTTERFLY_10scale (X[2], X[2 + 1],
                  X[6], X[6 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  1.0 / (double) 4);

  /* perform 1 times fft4 */
  gsl_power2_fft4synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8 S F F)
 **/
static void
gsl_power2_fft8synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 4 times fft2 */

  /* perform 2 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_0m (Y[2],               /* W2 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[10],               /* W2 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);

  /* perform 1 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_yY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_0m (Y[4],               /* W2 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XX (Y[6],               /* W3 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8 F S X)
 **/
static void
gsl_power2_fft8synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 4 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[8], X[8 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 8);
  BUTTERFLY_10scale (X[4], X[4 + 1],
                  X[12], X[12 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  1.0 / (double) 8);
  BUTTERFLY_10scale (X[2], X[2 + 1],
                  X[10], X[10 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  1.0 / (double) 8);
  BUTTERFLY_10scale (X[6], X[6 + 1],
                  X[14], X[14 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  1.0 / (double) 8);

  /* skipping 2 times fft4 */

  /* perform 1 times fft8 */
  gsl_power2_fft8synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 16 S F F F)
 **/
static void
gsl_power2_fft16synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 8 times fft2 */

  /* perform 4 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                Y[16],
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[24],               /* W0 */
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                Y[24],
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_0m (Y[2],               /* W4 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[10],               /* W4 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[18],               /* W4 */
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                Y[18],
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[26],               /* W4 */
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                Y[26],
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);

  /* perform 2 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                Y[16],
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_yY (Y[2],               /* W2 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_yY (Y[18],               /* W2 */
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                Y[18],
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_0m (Y[4],               /* W4 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[20],               /* W4 */
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                Y[20],
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XX (Y[6],               /* W6 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XX (Y[22],               /* W6 */
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                Y[22],
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);

  /* perform 1 times fft16 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                Y[0],
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                Y[2],
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_yY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                Y[4],
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                Y[6],
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_0m (Y[8],               /* W4 */
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                Y[8],
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                Y[10],
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XX (Y[12],               /* W6 */
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                Y[12],
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                Y[14],
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 16 F S S X)
 **/
static void
gsl_power2_fft16synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 8 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[16], X[16 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[8], X[8 + 1],
                  X[24], X[24 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[4], X[4 + 1],
                  X[20], X[20 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[12], X[12 + 1],
                  X[28], X[28 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[2], X[2 + 1],
                  X[18], X[18 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[10], X[10 + 1],
                  X[26], X[26 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[6], X[6 + 1],
                  X[22], X[22 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  1.0 / (double) 16);
  BUTTERFLY_10scale (X[14], X[14 + 1],
                  X[30], X[30 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  1.0 / (double) 16);

  /* skipping 4 times fft4 */

  /* skipping 2 times fft8 */

  /* perform 1 times fft16 */
  gsl_power2_fft16synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 32 S F F F F)
 **/
static void
gsl_power2_fft32synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 16 times fft2 */

  /* perform 8 times fft4 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                Y[0],
                Y[0 + 1],
                Y[4],
                Y[4 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[8],               /* W0 */
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                Y[8],
                Y[8 + 1],
                Y[12],
                Y[12 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                Y[16],
                Y[16 + 1],
                Y[20],
                Y[20 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[24],               /* W0 */
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                Y[24],
                Y[24 + 1],
                Y[28],
                Y[28 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[36],
                Y[36 + 1],
                Y[32],
                Y[32 + 1],
                Y[36],
                Y[36 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[40],               /* W0 */
                Y[40 + 1],
                Y[44],
                Y[44 + 1],
                Y[40],
                Y[40 + 1],
                Y[44],
                Y[44 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[48],               /* W0 */
                Y[48 + 1],
                Y[52],
                Y[52 + 1],
                Y[48],
                Y[48 + 1],
                Y[52],
                Y[52 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[56],               /* W0 */
                Y[56 + 1],
                Y[60],
                Y[60 + 1],
                Y[56],
                Y[56 + 1],
                Y[60],
                Y[60 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_0m (Y[2],               /* W8 */
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                Y[2],
                Y[2 + 1],
                Y[6],
                Y[6 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[10],               /* W8 */
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                Y[10],
                Y[10 + 1],
                Y[14],
                Y[14 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[18],               /* W8 */
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                Y[18],
                Y[18 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[26],               /* W8 */
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                Y[26],
                Y[26 + 1],
                Y[30],
                Y[30 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[34],               /* W8 */
                Y[34 + 1],
                Y[38],
                Y[38 + 1],
                Y[34],
                Y[34 + 1],
                Y[38],
                Y[38 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[42],               /* W8 */
                Y[42 + 1],
                Y[46],
                Y[46 + 1],
                Y[42],
                Y[42 + 1],
                Y[46],
                Y[46 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[50],               /* W8 */
                Y[50 + 1],
                Y[54],
                Y[54 + 1],
                Y[50],
                Y[50 + 1],
                Y[54],
                Y[54 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[58],               /* W8 */
                Y[58 + 1],
                Y[62],
                Y[62 + 1],
                Y[58],
                Y[58 + 1],
                Y[62],
                Y[62 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);

  /* perform 4 times fft8 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                Y[0],
                Y[0 + 1],
                Y[8],
                Y[8 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[16],               /* W0 */
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                Y[16],
                Y[16 + 1],
                Y[24],
                Y[24 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[40],
                Y[40 + 1],
                Y[32],
                Y[32 + 1],
                Y[40],
                Y[40 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[48],               /* W0 */
                Y[48 + 1],
                Y[56],
                Y[56 + 1],
                Y[48],
                Y[48 + 1],
                Y[56],
                Y[56 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_yY (Y[2],               /* W4 */
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                Y[2],
                Y[2 + 1],
                Y[10],
                Y[10 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_yY (Y[18],               /* W4 */
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                Y[18],
                Y[18 + 1],
                Y[26],
                Y[26 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_yY (Y[34],               /* W4 */
                Y[34 + 1],
                Y[42],
                Y[42 + 1],
                Y[34],
                Y[34 + 1],
                Y[42],
                Y[42 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_yY (Y[50],               /* W4 */
                Y[50 + 1],
                Y[58],
                Y[58 + 1],
                Y[50],
                Y[50 + 1],
                Y[58],
                Y[58 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_0m (Y[4],               /* W8 */
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                Y[4],
                Y[4 + 1],
                Y[12],
                Y[12 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[20],               /* W8 */
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                Y[20],
                Y[20 + 1],
                Y[28],
                Y[28 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[36],               /* W8 */
                Y[36 + 1],
                Y[44],
                Y[44 + 1],
                Y[36],
                Y[36 + 1],
                Y[44],
                Y[44 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[52],               /* W8 */
                Y[52 + 1],
                Y[60],
                Y[60 + 1],
                Y[52],
                Y[52 + 1],
                Y[60],
                Y[60 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XX (Y[6],               /* W12 */
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                Y[6],
                Y[6 + 1],
                Y[14],
                Y[14 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XX (Y[22],               /* W12 */
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                Y[22],
                Y[22 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XX (Y[38],               /* W12 */
                Y[38 + 1],
                Y[46],
                Y[46 + 1],
                Y[38],
                Y[38 + 1],
                Y[46],
                Y[46 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XX (Y[54],               /* W12 */
                Y[54 + 1],
                Y[62],
                Y[62 + 1],
                Y[54],
                Y[54 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);

  /* perform 2 times fft16 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                Y[0],
                Y[0 + 1],
                Y[16],
                Y[16 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_10 (Y[32],               /* W0 */
                Y[32 + 1],
                Y[48],
                Y[48 + 1],
                Y[32],
                Y[32 + 1],
                Y[48],
                Y[48 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W2 */
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                Y[2],
                Y[2 + 1],
                Y[18],
                Y[18 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[34],               /* W2 */
                Y[34 + 1],
                Y[50],
                Y[50 + 1],
                Y[34],
                Y[34 + 1],
                Y[50],
                Y[50 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_yY (Y[4],               /* W4 */
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                Y[4],
                Y[4 + 1],
                Y[20],
                Y[20 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_yY (Y[36],               /* W4 */
                Y[36 + 1],
                Y[52],
                Y[52 + 1],
                Y[36],
                Y[36 + 1],
                Y[52],
                Y[52 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_XY (Y[6],               /* W6 */
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                Y[6],
                Y[6 + 1],
                Y[22],
                Y[22 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[38],               /* W6 */
                Y[38 + 1],
                Y[54],
                Y[54 + 1],
                Y[38],
                Y[38 + 1],
                Y[54],
                Y[54 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_0m (Y[8],               /* W8 */
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                Y[8],
                Y[8 + 1],
                Y[24],
                Y[24 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_0m (Y[40],               /* W8 */
                Y[40 + 1],
                Y[56],
                Y[56 + 1],
                Y[40],
                Y[40 + 1],
                Y[56],
                Y[56 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XY (Y[10],               /* W10 */
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                Y[10],
                Y[10 + 1],
                Y[26],
                Y[26 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[42],               /* W10 */
                Y[42 + 1],
                Y[58],
                Y[58 + 1],
                Y[42],
                Y[42 + 1],
                Y[58],
                Y[58 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XX (Y[12],               /* W12 */
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                Y[12],
                Y[12 + 1],
                Y[28],
                Y[28 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XX (Y[44],               /* W12 */
                Y[44 + 1],
                Y[60],
                Y[60 + 1],
                Y[44],
                Y[44 + 1],
                Y[60],
                Y[60 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XY (Y[14],               /* W14 */
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                Y[14],
                Y[14 + 1],
                Y[30],
                Y[30 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[46],               /* W14 */
                Y[46 + 1],
                Y[62],
                Y[62 + 1],
                Y[46],
                Y[46 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);

  /* perform 1 times fft32 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[32],
                Y[32 + 1],
                Y[0],
                Y[0 + 1],
                Y[32],
                Y[32 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[34],
                Y[34 + 1],
                Y[2],
                Y[2 + 1],
                Y[34],
                Y[34 + 1],
                (double) +0.980785280403230, (double) -0.195090322016128);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[36],
                Y[36 + 1],
                Y[4],
                Y[4 + 1],
                Y[36],
                Y[36 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[38],
                Y[38 + 1],
                Y[6],
                Y[6 + 1],
                Y[38],
                Y[38 + 1],
                (double) +0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_yY (Y[8],               /* W4 */
                Y[8 + 1],
                Y[40],
                Y[40 + 1],
                Y[8],
                Y[8 + 1],
                Y[40],
                Y[40 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[42],
                Y[42 + 1],
                Y[10],
                Y[10 + 1],
                Y[42],
                Y[42 + 1],
                (double) +0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[44],
                Y[44 + 1],
                Y[12],
                Y[12 + 1],
                Y[44],
                Y[44 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[46],
                Y[46 + 1],
                Y[14],
                Y[14 + 1],
                Y[46],
                Y[46 + 1],
                (double) +0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_0m (Y[16],               /* W8 */
                Y[16 + 1],
                Y[48],
                Y[48 + 1],
                Y[16],
                Y[16 + 1],
                Y[48],
                Y[48 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[50],
                Y[50 + 1],
                Y[18],
                Y[18 + 1],
                Y[50],
                Y[50 + 1],
                (double) -0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[52],
                Y[52 + 1],
                Y[20],
                Y[20 + 1],
                Y[52],
                Y[52 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[54],
                Y[54 + 1],
                Y[22],
                Y[22 + 1],
                Y[54],
                Y[54 + 1],
                (double) -0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XX (Y[24],               /* W12 */
                Y[24 + 1],
                Y[56],
                Y[56 + 1],
                Y[24],
                Y[24 + 1],
                Y[56],
                Y[56 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[58],
                Y[58 + 1],
                Y[26],
                Y[26 + 1],
                Y[58],
                Y[58 + 1],
                (double) -0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[60],
                Y[60 + 1],
                Y[28],
                Y[28 + 1],
                Y[60],
                Y[60 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[62],
                Y[62 + 1],
                Y[30],
                Y[30 + 1],
                Y[62],
                Y[62 + 1],
                (double) -0.980785280403230, (double) -0.195090322016129);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 32 F S S S X)
 **/
static void
gsl_power2_fft32synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 16 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[32], X[32 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[16], X[16 + 1],
                  X[48], X[48 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[8], X[8 + 1],
                  X[40], X[40 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[24], X[24 + 1],
                  X[56], X[56 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[4], X[4 + 1],
                  X[36], X[36 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[20], X[20 + 1],
                  X[52], X[52 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[12], X[12 + 1],
                  X[44], X[44 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[28], X[28 + 1],
                  X[60], X[60 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[2], X[2 + 1],
                  X[34], X[34 + 1],
                  Y[32], Y[32 + 1],
                  Y[34], Y[34 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[18], X[18 + 1],
                  X[50], X[50 + 1],
                  Y[36], Y[36 + 1],
                  Y[38], Y[38 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[10], X[10 + 1],
                  X[42], X[42 + 1],
                  Y[40], Y[40 + 1],
                  Y[42], Y[42 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[26], X[26 + 1],
                  X[58], X[58 + 1],
                  Y[44], Y[44 + 1],
                  Y[46], Y[46 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[6], X[6 + 1],
                  X[38], X[38 + 1],
                  Y[48], Y[48 + 1],
                  Y[50], Y[50 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[22], X[22 + 1],
                  X[54], X[54 + 1],
                  Y[52], Y[52 + 1],
                  Y[54], Y[54 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[14], X[14 + 1],
                  X[46], X[46 + 1],
                  Y[56], Y[56 + 1],
                  Y[58], Y[58 + 1],
                  1.0 / (double) 32);
  BUTTERFLY_10scale (X[30], X[30 + 1],
                  X[62], X[62 + 1],
                  Y[60], Y[60 + 1],
                  Y[62], Y[62 + 1],
                  1.0 / (double) 32);

  /* skipping 8 times fft4 */

  /* skipping 4 times fft8 */

  /* skipping 2 times fft16 */

  /* perform 1 times fft32 */
  gsl_power2_fft32synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 64 S R R R R F)
 **/
static void
gsl_power2_fft64synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 32 times fft2 */

  /* perform 16 times fft4 */
  for (block = 0; block < 128; block += 8) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_0m (Y[block + 2],               /* W16 */
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
  }

  /* perform 8 times fft8 */
  for (block = 0; block < 128; block += 16) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_yY (Y[block + 2],               /* W8 */
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_0m (Y[block + 4],               /* W16 */
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XX (Y[block + 6],               /* W24 */
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
  }

  /* perform 4 times fft16 */
  for (block = 0; block < 128; block += 32) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W4 */
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  (double) +0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_yY (Y[block + 4],               /* W8 */
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_XY (Y[block + 6],               /* W12 */
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  (double) +0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_0m (Y[block + 8],               /* W16 */
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XY (Y[block + 10],               /* W20 */
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  (double) -0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XX (Y[block + 12],               /* W24 */
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
    BUTTERFLY_XY (Y[block + 14],               /* W28 */
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  (double) -0.923879532511287, (double) -0.382683432365090);
  }

  /* perform 2 times fft32 */
  for (block = 0; block < 128; block += 64) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W2 */
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  (double) +0.980785280403230, (double) -0.195090322016128);
    BUTTERFLY_XY (Y[block + 4],               /* W4 */
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  (double) +0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 6],               /* W6 */
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  (double) +0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_yY (Y[block + 8],               /* W8 */
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_XY (Y[block + 10],               /* W10 */
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  (double) +0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XY (Y[block + 12],               /* W12 */
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  (double) +0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 14],               /* W14 */
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  (double) +0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_0m (Y[block + 16],               /* W16 */
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XY (Y[block + 18],               /* W18 */
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  (double) -0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_XY (Y[block + 20],               /* W20 */
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  (double) -0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 22],               /* W22 */
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  (double) -0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XX (Y[block + 24],               /* W24 */
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
    BUTTERFLY_XY (Y[block + 26],               /* W26 */
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  (double) -0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_XY (Y[block + 28],               /* W28 */
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  (double) -0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 30],               /* W30 */
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  (double) -0.980785280403230, (double) -0.195090322016129);
  }

  /* perform 1 times fft64 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[64],
                Y[64 + 1],
                Y[0],
                Y[0 + 1],
                Y[64],
                Y[64 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[66],
                Y[66 + 1],
                Y[2],
                Y[2 + 1],
                Y[66],
                Y[66 + 1],
                (double) +0.995184726672197, (double) -0.098017140329561);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[68],
                Y[68 + 1],
                Y[4],
                Y[4 + 1],
                Y[68],
                Y[68 + 1],
                (double) +0.980785280403230, (double) -0.195090322016128);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[70],
                Y[70 + 1],
                Y[6],
                Y[6 + 1],
                Y[70],
                Y[70 + 1],
                (double) +0.956940335732209, (double) -0.290284677254462);
  BUTTERFLY_XY (Y[8],               /* W4 */
                Y[8 + 1],
                Y[72],
                Y[72 + 1],
                Y[8],
                Y[8 + 1],
                Y[72],
                Y[72 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[74],
                Y[74 + 1],
                Y[10],
                Y[10 + 1],
                Y[74],
                Y[74 + 1],
                (double) +0.881921264348355, (double) -0.471396736825998);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[76],
                Y[76 + 1],
                Y[12],
                Y[12 + 1],
                Y[76],
                Y[76 + 1],
                (double) +0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[78],
                Y[78 + 1],
                Y[14],
                Y[14 + 1],
                Y[78],
                Y[78 + 1],
                (double) +0.773010453362737, (double) -0.634393284163645);
  BUTTERFLY_yY (Y[16],               /* W8 */
                Y[16 + 1],
                Y[80],
                Y[80 + 1],
                Y[16],
                Y[16 + 1],
                Y[80],
                Y[80 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[82],
                Y[82 + 1],
                Y[18],
                Y[18 + 1],
                Y[82],
                Y[82 + 1],
                (double) +0.634393284163645, (double) -0.773010453362737);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[84],
                Y[84 + 1],
                Y[20],
                Y[20 + 1],
                Y[84],
                Y[84 + 1],
                (double) +0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[86],
                Y[86 + 1],
                Y[22],
                Y[22 + 1],
                Y[86],
                Y[86 + 1],
                (double) +0.471396736825998, (double) -0.881921264348355);
  BUTTERFLY_XY (Y[24],               /* W12 */
                Y[24 + 1],
                Y[88],
                Y[88 + 1],
                Y[24],
                Y[24 + 1],
                Y[88],
                Y[88 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[90],
                Y[90 + 1],
                Y[26],
                Y[26 + 1],
                Y[90],
                Y[90 + 1],
                (double) +0.290284677254462, (double) -0.956940335732209);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[92],
                Y[92 + 1],
                Y[28],
                Y[28 + 1],
                Y[92],
                Y[92 + 1],
                (double) +0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[94],
                Y[94 + 1],
                Y[30],
                Y[30 + 1],
                Y[94],
                Y[94 + 1],
                (double) +0.098017140329561, (double) -0.995184726672197);
  BUTTERFLY_0m (Y[32],               /* W16 */
                Y[32 + 1],
                Y[96],
                Y[96 + 1],
                Y[32],
                Y[32 + 1],
                Y[96],
                Y[96 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XY (Y[34],               /* W17 */
                Y[34 + 1],
                Y[98],
                Y[98 + 1],
                Y[34],
                Y[34 + 1],
                Y[98],
                Y[98 + 1],
                (double) -0.098017140329561, (double) -0.995184726672197);
  BUTTERFLY_XY (Y[36],               /* W18 */
                Y[36 + 1],
                Y[100],
                Y[100 + 1],
                Y[36],
                Y[36 + 1],
                Y[100],
                Y[100 + 1],
                (double) -0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_XY (Y[38],               /* W19 */
                Y[38 + 1],
                Y[102],
                Y[102 + 1],
                Y[38],
                Y[38 + 1],
                Y[102],
                Y[102 + 1],
                (double) -0.290284677254462, (double) -0.956940335732209);
  BUTTERFLY_XY (Y[40],               /* W20 */
                Y[40 + 1],
                Y[104],
                Y[104 + 1],
                Y[40],
                Y[40 + 1],
                Y[104],
                Y[104 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[42],               /* W21 */
                Y[42 + 1],
                Y[106],
                Y[106 + 1],
                Y[42],
                Y[42 + 1],
                Y[106],
                Y[106 + 1],
                (double) -0.471396736825998, (double) -0.881921264348355);
  BUTTERFLY_XY (Y[44],               /* W22 */
                Y[44 + 1],
                Y[108],
                Y[108 + 1],
                Y[44],
                Y[44 + 1],
                Y[108],
                Y[108 + 1],
                (double) -0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XY (Y[46],               /* W23 */
                Y[46 + 1],
                Y[110],
                Y[110 + 1],
                Y[46],
                Y[46 + 1],
                Y[110],
                Y[110 + 1],
                (double) -0.634393284163645, (double) -0.773010453362737);
  BUTTERFLY_XX (Y[48],               /* W24 */
                Y[48 + 1],
                Y[112],
                Y[112 + 1],
                Y[48],
                Y[48 + 1],
                Y[112],
                Y[112 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XY (Y[50],               /* W25 */
                Y[50 + 1],
                Y[114],
                Y[114 + 1],
                Y[50],
                Y[50 + 1],
                Y[114],
                Y[114 + 1],
                (double) -0.773010453362737, (double) -0.634393284163645);
  BUTTERFLY_XY (Y[52],               /* W26 */
                Y[52 + 1],
                Y[116],
                Y[116 + 1],
                Y[52],
                Y[52 + 1],
                Y[116],
                Y[116 + 1],
                (double) -0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_XY (Y[54],               /* W27 */
                Y[54 + 1],
                Y[118],
                Y[118 + 1],
                Y[54],
                Y[54 + 1],
                Y[118],
                Y[118 + 1],
                (double) -0.881921264348355, (double) -0.471396736825998);
  BUTTERFLY_XY (Y[56],               /* W28 */
                Y[56 + 1],
                Y[120],
                Y[120 + 1],
                Y[56],
                Y[56 + 1],
                Y[120],
                Y[120 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[58],               /* W29 */
                Y[58 + 1],
                Y[122],
                Y[122 + 1],
                Y[58],
                Y[58 + 1],
                Y[122],
                Y[122 + 1],
                (double) -0.956940335732209, (double) -0.290284677254462);
  BUTTERFLY_XY (Y[60],               /* W30 */
                Y[60 + 1],
                Y[124],
                Y[124 + 1],
                Y[60],
                Y[60 + 1],
                Y[124],
                Y[124 + 1],
                (double) -0.980785280403230, (double) -0.195090322016129);
  BUTTERFLY_XY (Y[62],               /* W31 */
                Y[62 + 1],
                Y[126],
                Y[126 + 1],
                Y[62],
                Y[62 + 1],
                Y[126],
                Y[126 + 1],
                (double) -0.995184726672197, (double) -0.098017140329561);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 64 F S S S S X)
 **/
static void
gsl_power2_fft64synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform 32 times fft2 */
  BUTTERFLY_10scale (X[0], X[0 + 1],
                  X[64], X[64 + 1],
                  Y[0], Y[0 + 1],
                  Y[2], Y[2 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[32], X[32 + 1],
                  X[96], X[96 + 1],
                  Y[4], Y[4 + 1],
                  Y[6], Y[6 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[16], X[16 + 1],
                  X[80], X[80 + 1],
                  Y[8], Y[8 + 1],
                  Y[10], Y[10 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[48], X[48 + 1],
                  X[112], X[112 + 1],
                  Y[12], Y[12 + 1],
                  Y[14], Y[14 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[8], X[8 + 1],
                  X[72], X[72 + 1],
                  Y[16], Y[16 + 1],
                  Y[18], Y[18 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[40], X[40 + 1],
                  X[104], X[104 + 1],
                  Y[20], Y[20 + 1],
                  Y[22], Y[22 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[24], X[24 + 1],
                  X[88], X[88 + 1],
                  Y[24], Y[24 + 1],
                  Y[26], Y[26 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[56], X[56 + 1],
                  X[120], X[120 + 1],
                  Y[28], Y[28 + 1],
                  Y[30], Y[30 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[4], X[4 + 1],
                  X[68], X[68 + 1],
                  Y[32], Y[32 + 1],
                  Y[34], Y[34 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[36], X[36 + 1],
                  X[100], X[100 + 1],
                  Y[36], Y[36 + 1],
                  Y[38], Y[38 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[20], X[20 + 1],
                  X[84], X[84 + 1],
                  Y[40], Y[40 + 1],
                  Y[42], Y[42 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[52], X[52 + 1],
                  X[116], X[116 + 1],
                  Y[44], Y[44 + 1],
                  Y[46], Y[46 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[12], X[12 + 1],
                  X[76], X[76 + 1],
                  Y[48], Y[48 + 1],
                  Y[50], Y[50 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[44], X[44 + 1],
                  X[108], X[108 + 1],
                  Y[52], Y[52 + 1],
                  Y[54], Y[54 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[28], X[28 + 1],
                  X[92], X[92 + 1],
                  Y[56], Y[56 + 1],
                  Y[58], Y[58 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[60], X[60 + 1],
                  X[124], X[124 + 1],
                  Y[60], Y[60 + 1],
                  Y[62], Y[62 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[2], X[2 + 1],
                  X[66], X[66 + 1],
                  Y[64], Y[64 + 1],
                  Y[66], Y[66 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[34], X[34 + 1],
                  X[98], X[98 + 1],
                  Y[68], Y[68 + 1],
                  Y[70], Y[70 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[18], X[18 + 1],
                  X[82], X[82 + 1],
                  Y[72], Y[72 + 1],
                  Y[74], Y[74 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[50], X[50 + 1],
                  X[114], X[114 + 1],
                  Y[76], Y[76 + 1],
                  Y[78], Y[78 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[10], X[10 + 1],
                  X[74], X[74 + 1],
                  Y[80], Y[80 + 1],
                  Y[82], Y[82 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[42], X[42 + 1],
                  X[106], X[106 + 1],
                  Y[84], Y[84 + 1],
                  Y[86], Y[86 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[26], X[26 + 1],
                  X[90], X[90 + 1],
                  Y[88], Y[88 + 1],
                  Y[90], Y[90 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[58], X[58 + 1],
                  X[122], X[122 + 1],
                  Y[92], Y[92 + 1],
                  Y[94], Y[94 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[6], X[6 + 1],
                  X[70], X[70 + 1],
                  Y[96], Y[96 + 1],
                  Y[98], Y[98 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[38], X[38 + 1],
                  X[102], X[102 + 1],
                  Y[100], Y[100 + 1],
                  Y[102], Y[102 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[22], X[22 + 1],
                  X[86], X[86 + 1],
                  Y[104], Y[104 + 1],
                  Y[106], Y[106 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[54], X[54 + 1],
                  X[118], X[118 + 1],
                  Y[108], Y[108 + 1],
                  Y[110], Y[110 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[14], X[14 + 1],
                  X[78], X[78 + 1],
                  Y[112], Y[112 + 1],
                  Y[114], Y[114 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[46], X[46 + 1],
                  X[110], X[110 + 1],
                  Y[116], Y[116 + 1],
                  Y[118], Y[118 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[30], X[30 + 1],
                  X[94], X[94 + 1],
                  Y[120], Y[120 + 1],
                  Y[122], Y[122 + 1],
                  1.0 / (double) 64);
  BUTTERFLY_10scale (X[62], X[62 + 1],
                  X[126], X[126 + 1],
                  Y[124], Y[124 + 1],
                  Y[126], Y[126 + 1],
                  1.0 / (double) 64);

  /* skipping 16 times fft4 */

  /* skipping 8 times fft8 */

  /* skipping 4 times fft16 */

  /* skipping 2 times fft32 */

  /* perform 1 times fft64 */
  gsl_power2_fft64synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 128 S R R R R R F)
 **/
static void
gsl_power2_fft128synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 64 times fft2 */

  /* perform 32 times fft4 */
  for (block = 0; block < 256; block += 8) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_0m (Y[block + 2],               /* W32 */
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
  }

  /* perform 16 times fft8 */
  for (block = 0; block < 256; block += 16) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_yY (Y[block + 2],               /* W16 */
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_0m (Y[block + 4],               /* W32 */
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XX (Y[block + 6],               /* W48 */
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
  }

  /* perform 8 times fft16 */
  for (block = 0; block < 256; block += 32) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W8 */
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  (double) +0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_yY (Y[block + 4],               /* W16 */
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_XY (Y[block + 6],               /* W24 */
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  (double) +0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_0m (Y[block + 8],               /* W32 */
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XY (Y[block + 10],               /* W40 */
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  (double) -0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XX (Y[block + 12],               /* W48 */
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
    BUTTERFLY_XY (Y[block + 14],               /* W56 */
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  (double) -0.923879532511287, (double) -0.382683432365090);
  }

  /* perform 4 times fft32 */
  for (block = 0; block < 256; block += 64) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W4 */
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  (double) +0.980785280403230, (double) -0.195090322016128);
    BUTTERFLY_XY (Y[block + 4],               /* W8 */
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  (double) +0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 6],               /* W12 */
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  (double) +0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_yY (Y[block + 8],               /* W16 */
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_XY (Y[block + 10],               /* W20 */
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  (double) +0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XY (Y[block + 12],               /* W24 */
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  (double) +0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 14],               /* W28 */
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  (double) +0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_0m (Y[block + 16],               /* W32 */
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XY (Y[block + 18],               /* W36 */
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  (double) -0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_XY (Y[block + 20],               /* W40 */
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  (double) -0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 22],               /* W44 */
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  (double) -0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XX (Y[block + 24],               /* W48 */
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
    BUTTERFLY_XY (Y[block + 26],               /* W52 */
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  (double) -0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_XY (Y[block + 28],               /* W56 */
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  (double) -0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 30],               /* W60 */
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  (double) -0.980785280403230, (double) -0.195090322016129);
  }

  /* perform 2 times fft64 */
  for (block = 0; block < 256; block += 128) {
    BUTTERFLY_10 (Y[block + 0],               /* W0 */
                  Y[block + 0 + 1],
                  Y[block + 64],
                  Y[block + 64 + 1],
                  Y[block + 0],
                  Y[block + 0 + 1],
                  Y[block + 64],
                  Y[block + 64 + 1],
                  (double) +1.000000000000000, (double) +0.000000000000000);
    BUTTERFLY_XY (Y[block + 2],               /* W2 */
                  Y[block + 2 + 1],
                  Y[block + 66],
                  Y[block + 66 + 1],
                  Y[block + 2],
                  Y[block + 2 + 1],
                  Y[block + 66],
                  Y[block + 66 + 1],
                  (double) +0.995184726672197, (double) -0.098017140329561);
    BUTTERFLY_XY (Y[block + 4],               /* W4 */
                  Y[block + 4 + 1],
                  Y[block + 68],
                  Y[block + 68 + 1],
                  Y[block + 4],
                  Y[block + 4 + 1],
                  Y[block + 68],
                  Y[block + 68 + 1],
                  (double) +0.980785280403230, (double) -0.195090322016128);
    BUTTERFLY_XY (Y[block + 6],               /* W6 */
                  Y[block + 6 + 1],
                  Y[block + 70],
                  Y[block + 70 + 1],
                  Y[block + 6],
                  Y[block + 6 + 1],
                  Y[block + 70],
                  Y[block + 70 + 1],
                  (double) +0.956940335732209, (double) -0.290284677254462);
    BUTTERFLY_XY (Y[block + 8],               /* W8 */
                  Y[block + 8 + 1],
                  Y[block + 72],
                  Y[block + 72 + 1],
                  Y[block + 8],
                  Y[block + 8 + 1],
                  Y[block + 72],
                  Y[block + 72 + 1],
                  (double) +0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 10],               /* W10 */
                  Y[block + 10 + 1],
                  Y[block + 74],
                  Y[block + 74 + 1],
                  Y[block + 10],
                  Y[block + 10 + 1],
                  Y[block + 74],
                  Y[block + 74 + 1],
                  (double) +0.881921264348355, (double) -0.471396736825998);
    BUTTERFLY_XY (Y[block + 12],               /* W12 */
                  Y[block + 12 + 1],
                  Y[block + 76],
                  Y[block + 76 + 1],
                  Y[block + 12],
                  Y[block + 12 + 1],
                  Y[block + 76],
                  Y[block + 76 + 1],
                  (double) +0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_XY (Y[block + 14],               /* W14 */
                  Y[block + 14 + 1],
                  Y[block + 78],
                  Y[block + 78 + 1],
                  Y[block + 14],
                  Y[block + 14 + 1],
                  Y[block + 78],
                  Y[block + 78 + 1],
                  (double) +0.773010453362737, (double) -0.634393284163645);
    BUTTERFLY_yY (Y[block + 16],               /* W16 */
                  Y[block + 16 + 1],
                  Y[block + 80],
                  Y[block + 80 + 1],
                  Y[block + 16],
                  Y[block + 16 + 1],
                  Y[block + 80],
                  Y[block + 80 + 1],
                  (double) +0.707106781186548, (double) -0.707106781186547);
    BUTTERFLY_XY (Y[block + 18],               /* W18 */
                  Y[block + 18 + 1],
                  Y[block + 82],
                  Y[block + 82 + 1],
                  Y[block + 18],
                  Y[block + 18 + 1],
                  Y[block + 82],
                  Y[block + 82 + 1],
                  (double) +0.634393284163645, (double) -0.773010453362737);
    BUTTERFLY_XY (Y[block + 20],               /* W20 */
                  Y[block + 20 + 1],
                  Y[block + 84],
                  Y[block + 84 + 1],
                  Y[block + 20],
                  Y[block + 20 + 1],
                  Y[block + 84],
                  Y[block + 84 + 1],
                  (double) +0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XY (Y[block + 22],               /* W22 */
                  Y[block + 22 + 1],
                  Y[block + 86],
                  Y[block + 86 + 1],
                  Y[block + 22],
                  Y[block + 22 + 1],
                  Y[block + 86],
                  Y[block + 86 + 1],
                  (double) +0.471396736825998, (double) -0.881921264348355);
    BUTTERFLY_XY (Y[block + 24],               /* W24 */
                  Y[block + 24 + 1],
                  Y[block + 88],
                  Y[block + 88 + 1],
                  Y[block + 24],
                  Y[block + 24 + 1],
                  Y[block + 88],
                  Y[block + 88 + 1],
                  (double) +0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 26],               /* W26 */
                  Y[block + 26 + 1],
                  Y[block + 90],
                  Y[block + 90 + 1],
                  Y[block + 26],
                  Y[block + 26 + 1],
                  Y[block + 90],
                  Y[block + 90 + 1],
                  (double) +0.290284677254462, (double) -0.956940335732209);
    BUTTERFLY_XY (Y[block + 28],               /* W28 */
                  Y[block + 28 + 1],
                  Y[block + 92],
                  Y[block + 92 + 1],
                  Y[block + 28],
                  Y[block + 28 + 1],
                  Y[block + 92],
                  Y[block + 92 + 1],
                  (double) +0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_XY (Y[block + 30],               /* W30 */
                  Y[block + 30 + 1],
                  Y[block + 94],
                  Y[block + 94 + 1],
                  Y[block + 30],
                  Y[block + 30 + 1],
                  Y[block + 94],
                  Y[block + 94 + 1],
                  (double) +0.098017140329561, (double) -0.995184726672197);
    BUTTERFLY_0m (Y[block + 32],               /* W32 */
                  Y[block + 32 + 1],
                  Y[block + 96],
                  Y[block + 96 + 1],
                  Y[block + 32],
                  Y[block + 32 + 1],
                  Y[block + 96],
                  Y[block + 96 + 1],
                  (double) +0.000000000000000, (double) -1.000000000000000);
    BUTTERFLY_XY (Y[block + 34],               /* W34 */
                  Y[block + 34 + 1],
                  Y[block + 98],
                  Y[block + 98 + 1],
                  Y[block + 34],
                  Y[block + 34 + 1],
                  Y[block + 98],
                  Y[block + 98 + 1],
                  (double) -0.098017140329561, (double) -0.995184726672197);
    BUTTERFLY_XY (Y[block + 36],               /* W36 */
                  Y[block + 36 + 1],
                  Y[block + 100],
                  Y[block + 100 + 1],
                  Y[block + 36],
                  Y[block + 36 + 1],
                  Y[block + 100],
                  Y[block + 100 + 1],
                  (double) -0.195090322016128, (double) -0.980785280403230);
    BUTTERFLY_XY (Y[block + 38],               /* W38 */
                  Y[block + 38 + 1],
                  Y[block + 102],
                  Y[block + 102 + 1],
                  Y[block + 38],
                  Y[block + 38 + 1],
                  Y[block + 102],
                  Y[block + 102 + 1],
                  (double) -0.290284677254462, (double) -0.956940335732209);
    BUTTERFLY_XY (Y[block + 40],               /* W40 */
                  Y[block + 40 + 1],
                  Y[block + 104],
                  Y[block + 104 + 1],
                  Y[block + 40],
                  Y[block + 40 + 1],
                  Y[block + 104],
                  Y[block + 104 + 1],
                  (double) -0.382683432365090, (double) -0.923879532511287);
    BUTTERFLY_XY (Y[block + 42],               /* W42 */
                  Y[block + 42 + 1],
                  Y[block + 106],
                  Y[block + 106 + 1],
                  Y[block + 42],
                  Y[block + 42 + 1],
                  Y[block + 106],
                  Y[block + 106 + 1],
                  (double) -0.471396736825998, (double) -0.881921264348355);
    BUTTERFLY_XY (Y[block + 44],               /* W44 */
                  Y[block + 44 + 1],
                  Y[block + 108],
                  Y[block + 108 + 1],
                  Y[block + 44],
                  Y[block + 44 + 1],
                  Y[block + 108],
                  Y[block + 108 + 1],
                  (double) -0.555570233019602, (double) -0.831469612302545);
    BUTTERFLY_XY (Y[block + 46],               /* W46 */
                  Y[block + 46 + 1],
                  Y[block + 110],
                  Y[block + 110 + 1],
                  Y[block + 46],
                  Y[block + 46 + 1],
                  Y[block + 110],
                  Y[block + 110 + 1],
                  (double) -0.634393284163645, (double) -0.773010453362737);
    BUTTERFLY_XX (Y[block + 48],               /* W48 */
                  Y[block + 48 + 1],
                  Y[block + 112],
                  Y[block + 112 + 1],
                  Y[block + 48],
                  Y[block + 48 + 1],
                  Y[block + 112],
                  Y[block + 112 + 1],
                  (double) -0.707106781186547, (double) -0.707106781186548);
    BUTTERFLY_XY (Y[block + 50],               /* W50 */
                  Y[block + 50 + 1],
                  Y[block + 114],
                  Y[block + 114 + 1],
                  Y[block + 50],
                  Y[block + 50 + 1],
                  Y[block + 114],
                  Y[block + 114 + 1],
                  (double) -0.773010453362737, (double) -0.634393284163645);
    BUTTERFLY_XY (Y[block + 52],               /* W52 */
                  Y[block + 52 + 1],
                  Y[block + 116],
                  Y[block + 116 + 1],
                  Y[block + 52],
                  Y[block + 52 + 1],
                  Y[block + 116],
                  Y[block + 116 + 1],
                  (double) -0.831469612302545, (double) -0.555570233019602);
    BUTTERFLY_XY (Y[block + 54],               /* W54 */
                  Y[block + 54 + 1],
                  Y[block + 118],
                  Y[block + 118 + 1],
                  Y[block + 54],
                  Y[block + 54 + 1],
                  Y[block + 118],
                  Y[block + 118 + 1],
                  (double) -0.881921264348355, (double) -0.471396736825998);
    BUTTERFLY_XY (Y[block + 56],               /* W56 */
                  Y[block + 56 + 1],
                  Y[block + 120],
                  Y[block + 120 + 1],
                  Y[block + 56],
                  Y[block + 56 + 1],
                  Y[block + 120],
                  Y[block + 120 + 1],
                  (double) -0.923879532511287, (double) -0.382683432365090);
    BUTTERFLY_XY (Y[block + 58],               /* W58 */
                  Y[block + 58 + 1],
                  Y[block + 122],
                  Y[block + 122 + 1],
                  Y[block + 58],
                  Y[block + 58 + 1],
                  Y[block + 122],
                  Y[block + 122 + 1],
                  (double) -0.956940335732209, (double) -0.290284677254462);
    BUTTERFLY_XY (Y[block + 60],               /* W60 */
                  Y[block + 60 + 1],
                  Y[block + 124],
                  Y[block + 124 + 1],
                  Y[block + 60],
                  Y[block + 60 + 1],
                  Y[block + 124],
                  Y[block + 124 + 1],
                  (double) -0.980785280403230, (double) -0.195090322016129);
    BUTTERFLY_XY (Y[block + 62],               /* W62 */
                  Y[block + 62 + 1],
                  Y[block + 126],
                  Y[block + 126 + 1],
                  Y[block + 62],
                  Y[block + 62 + 1],
                  Y[block + 126],
                  Y[block + 126 + 1],
                  (double) -0.995184726672197, (double) -0.098017140329561);
  }

  /* perform 1 times fft128 */
  BUTTERFLY_10 (Y[0],               /* W0 */
                Y[0 + 1],
                Y[128],
                Y[128 + 1],
                Y[0],
                Y[0 + 1],
                Y[128],
                Y[128 + 1],
                (double) +1.000000000000000, (double) +0.000000000000000);
  BUTTERFLY_XY (Y[2],               /* W1 */
                Y[2 + 1],
                Y[130],
                Y[130 + 1],
                Y[2],
                Y[2 + 1],
                Y[130],
                Y[130 + 1],
                (double) +0.998795456205172, (double) -0.049067674327418);
  BUTTERFLY_XY (Y[4],               /* W2 */
                Y[4 + 1],
                Y[132],
                Y[132 + 1],
                Y[4],
                Y[4 + 1],
                Y[132],
                Y[132 + 1],
                (double) +0.995184726672197, (double) -0.098017140329561);
  BUTTERFLY_XY (Y[6],               /* W3 */
                Y[6 + 1],
                Y[134],
                Y[134 + 1],
                Y[6],
                Y[6 + 1],
                Y[134],
                Y[134 + 1],
                (double) +0.989176509964781, (double) -0.146730474455362);
  BUTTERFLY_XY (Y[8],               /* W4 */
                Y[8 + 1],
                Y[136],
                Y[136 + 1],
                Y[8],
                Y[8 + 1],
                Y[136],
                Y[136 + 1],
                (double) +0.980785280403230, (double) -0.195090322016128);
  BUTTERFLY_XY (Y[10],               /* W5 */
                Y[10 + 1],
                Y[138],
                Y[138 + 1],
                Y[10],
                Y[10 + 1],
                Y[138],
                Y[138 + 1],
                (double) +0.970031253194544, (double) -0.242980179903264);
  BUTTERFLY_XY (Y[12],               /* W6 */
                Y[12 + 1],
                Y[140],
                Y[140 + 1],
                Y[12],
                Y[12 + 1],
                Y[140],
                Y[140 + 1],
                (double) +0.956940335732209, (double) -0.290284677254462);
  BUTTERFLY_XY (Y[14],               /* W7 */
                Y[14 + 1],
                Y[142],
                Y[142 + 1],
                Y[14],
                Y[14 + 1],
                Y[142],
                Y[142 + 1],
                (double) +0.941544065183021, (double) -0.336889853392220);
  BUTTERFLY_XY (Y[16],               /* W8 */
                Y[16 + 1],
                Y[144],
                Y[144 + 1],
                Y[16],
                Y[16 + 1],
                Y[144],
                Y[144 + 1],
                (double) +0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[18],               /* W9 */
                Y[18 + 1],
                Y[146],
                Y[146 + 1],
                Y[18],
                Y[18 + 1],
                Y[146],
                Y[146 + 1],
                (double) +0.903989293123443, (double) -0.427555093430282);
  BUTTERFLY_XY (Y[20],               /* W10 */
                Y[20 + 1],
                Y[148],
                Y[148 + 1],
                Y[20],
                Y[20 + 1],
                Y[148],
                Y[148 + 1],
                (double) +0.881921264348355, (double) -0.471396736825998);
  BUTTERFLY_XY (Y[22],               /* W11 */
                Y[22 + 1],
                Y[150],
                Y[150 + 1],
                Y[22],
                Y[22 + 1],
                Y[150],
                Y[150 + 1],
                (double) +0.857728610000272, (double) -0.514102744193222);
  BUTTERFLY_XY (Y[24],               /* W12 */
                Y[24 + 1],
                Y[152],
                Y[152 + 1],
                Y[24],
                Y[24 + 1],
                Y[152],
                Y[152 + 1],
                (double) +0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_XY (Y[26],               /* W13 */
                Y[26 + 1],
                Y[154],
                Y[154 + 1],
                Y[26],
                Y[26 + 1],
                Y[154],
                Y[154 + 1],
                (double) +0.803207531480645, (double) -0.595699304492433);
  BUTTERFLY_XY (Y[28],               /* W14 */
                Y[28 + 1],
                Y[156],
                Y[156 + 1],
                Y[28],
                Y[28 + 1],
                Y[156],
                Y[156 + 1],
                (double) +0.773010453362737, (double) -0.634393284163645);
  BUTTERFLY_XY (Y[30],               /* W15 */
                Y[30 + 1],
                Y[158],
                Y[158 + 1],
                Y[30],
                Y[30 + 1],
                Y[158],
                Y[158 + 1],
                (double) +0.740951125354959, (double) -0.671558954847018);
  BUTTERFLY_yY (Y[32],               /* W16 */
                Y[32 + 1],
                Y[160],
                Y[160 + 1],
                Y[32],
                Y[32 + 1],
                Y[160],
                Y[160 + 1],
                (double) +0.707106781186548, (double) -0.707106781186547);
  BUTTERFLY_XY (Y[34],               /* W17 */
                Y[34 + 1],
                Y[162],
                Y[162 + 1],
                Y[34],
                Y[34 + 1],
                Y[162],
                Y[162 + 1],
                (double) +0.671558954847018, (double) -0.740951125354959);
  BUTTERFLY_XY (Y[36],               /* W18 */
                Y[36 + 1],
                Y[164],
                Y[164 + 1],
                Y[36],
                Y[36 + 1],
                Y[164],
                Y[164 + 1],
                (double) +0.634393284163645, (double) -0.773010453362737);
  BUTTERFLY_XY (Y[38],               /* W19 */
                Y[38 + 1],
                Y[166],
                Y[166 + 1],
                Y[38],
                Y[38 + 1],
                Y[166],
                Y[166 + 1],
                (double) +0.595699304492433, (double) -0.803207531480645);
  BUTTERFLY_XY (Y[40],               /* W20 */
                Y[40 + 1],
                Y[168],
                Y[168 + 1],
                Y[40],
                Y[40 + 1],
                Y[168],
                Y[168 + 1],
                (double) +0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XY (Y[42],               /* W21 */
                Y[42 + 1],
                Y[170],
                Y[170 + 1],
                Y[42],
                Y[42 + 1],
                Y[170],
                Y[170 + 1],
                (double) +0.514102744193222, (double) -0.857728610000272);
  BUTTERFLY_XY (Y[44],               /* W22 */
                Y[44 + 1],
                Y[172],
                Y[172 + 1],
                Y[44],
                Y[44 + 1],
                Y[172],
                Y[172 + 1],
                (double) +0.471396736825998, (double) -0.881921264348355);
  BUTTERFLY_XY (Y[46],               /* W23 */
                Y[46 + 1],
                Y[174],
                Y[174 + 1],
                Y[46],
                Y[46 + 1],
                Y[174],
                Y[174 + 1],
                (double) +0.427555093430282, (double) -0.903989293123443);
  BUTTERFLY_XY (Y[48],               /* W24 */
                Y[48 + 1],
                Y[176],
                Y[176 + 1],
                Y[48],
                Y[48 + 1],
                Y[176],
                Y[176 + 1],
                (double) +0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[50],               /* W25 */
                Y[50 + 1],
                Y[178],
                Y[178 + 1],
                Y[50],
                Y[50 + 1],
                Y[178],
                Y[178 + 1],
                (double) +0.336889853392220, (double) -0.941544065183021);
  BUTTERFLY_XY (Y[52],               /* W26 */
                Y[52 + 1],
                Y[180],
                Y[180 + 1],
                Y[52],
                Y[52 + 1],
                Y[180],
                Y[180 + 1],
                (double) +0.290284677254462, (double) -0.956940335732209);
  BUTTERFLY_XY (Y[54],               /* W27 */
                Y[54 + 1],
                Y[182],
                Y[182 + 1],
                Y[54],
                Y[54 + 1],
                Y[182],
                Y[182 + 1],
                (double) +0.242980179903264, (double) -0.970031253194544);
  BUTTERFLY_XY (Y[56],               /* W28 */
                Y[56 + 1],
                Y[184],
                Y[184 + 1],
                Y[56],
                Y[56 + 1],
                Y[184],
                Y[184 + 1],
                (double) +0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_XY (Y[58],               /* W29 */
                Y[58 + 1],
                Y[186],
                Y[186 + 1],
                Y[58],
                Y[58 + 1],
                Y[186],
                Y[186 + 1],
                (double) +0.146730474455362, (double) -0.989176509964781);
  BUTTERFLY_XY (Y[60],               /* W30 */
                Y[60 + 1],
                Y[188],
                Y[188 + 1],
                Y[60],
                Y[60 + 1],
                Y[188],
                Y[188 + 1],
                (double) +0.098017140329561, (double) -0.995184726672197);
  BUTTERFLY_XY (Y[62],               /* W31 */
                Y[62 + 1],
                Y[190],
                Y[190 + 1],
                Y[62],
                Y[62 + 1],
                Y[190],
                Y[190 + 1],
                (double) +0.049067674327418, (double) -0.998795456205172);
  BUTTERFLY_0m (Y[64],               /* W32 */
                Y[64 + 1],
                Y[192],
                Y[192 + 1],
                Y[64],
                Y[64 + 1],
                Y[192],
                Y[192 + 1],
                (double) +0.000000000000000, (double) -1.000000000000000);
  BUTTERFLY_XY (Y[66],               /* W33 */
                Y[66 + 1],
                Y[194],
                Y[194 + 1],
                Y[66],
                Y[66 + 1],
                Y[194],
                Y[194 + 1],
                (double) -0.049067674327418, (double) -0.998795456205172);
  BUTTERFLY_XY (Y[68],               /* W34 */
                Y[68 + 1],
                Y[196],
                Y[196 + 1],
                Y[68],
                Y[68 + 1],
                Y[196],
                Y[196 + 1],
                (double) -0.098017140329561, (double) -0.995184726672197);
  BUTTERFLY_XY (Y[70],               /* W35 */
                Y[70 + 1],
                Y[198],
                Y[198 + 1],
                Y[70],
                Y[70 + 1],
                Y[198],
                Y[198 + 1],
                (double) -0.146730474455362, (double) -0.989176509964781);
  BUTTERFLY_XY (Y[72],               /* W36 */
                Y[72 + 1],
                Y[200],
                Y[200 + 1],
                Y[72],
                Y[72 + 1],
                Y[200],
                Y[200 + 1],
                (double) -0.195090322016128, (double) -0.980785280403230);
  BUTTERFLY_XY (Y[74],               /* W37 */
                Y[74 + 1],
                Y[202],
                Y[202 + 1],
                Y[74],
                Y[74 + 1],
                Y[202],
                Y[202 + 1],
                (double) -0.242980179903264, (double) -0.970031253194544);
  BUTTERFLY_XY (Y[76],               /* W38 */
                Y[76 + 1],
                Y[204],
                Y[204 + 1],
                Y[76],
                Y[76 + 1],
                Y[204],
                Y[204 + 1],
                (double) -0.290284677254462, (double) -0.956940335732209);
  BUTTERFLY_XY (Y[78],               /* W39 */
                Y[78 + 1],
                Y[206],
                Y[206 + 1],
                Y[78],
                Y[78 + 1],
                Y[206],
                Y[206 + 1],
                (double) -0.336889853392220, (double) -0.941544065183021);
  BUTTERFLY_XY (Y[80],               /* W40 */
                Y[80 + 1],
                Y[208],
                Y[208 + 1],
                Y[80],
                Y[80 + 1],
                Y[208],
                Y[208 + 1],
                (double) -0.382683432365090, (double) -0.923879532511287);
  BUTTERFLY_XY (Y[82],               /* W41 */
                Y[82 + 1],
                Y[210],
                Y[210 + 1],
                Y[82],
                Y[82 + 1],
                Y[210],
                Y[210 + 1],
                (double) -0.427555093430282, (double) -0.903989293123443);
  BUTTERFLY_XY (Y[84],               /* W42 */
                Y[84 + 1],
                Y[212],
                Y[212 + 1],
                Y[84],
                Y[84 + 1],
                Y[212],
                Y[212 + 1],
                (double) -0.471396736825998, (double) -0.881921264348355);
  BUTTERFLY_XY (Y[86],               /* W43 */
                Y[86 + 1],
                Y[214],
                Y[214 + 1],
                Y[86],
                Y[86 + 1],
                Y[214],
                Y[214 + 1],
                (double) -0.514102744193222, (double) -0.857728610000272);
  BUTTERFLY_XY (Y[88],               /* W44 */
                Y[88 + 1],
                Y[216],
                Y[216 + 1],
                Y[88],
                Y[88 + 1],
                Y[216],
                Y[216 + 1],
                (double) -0.555570233019602, (double) -0.831469612302545);
  BUTTERFLY_XY (Y[90],               /* W45 */
                Y[90 + 1],
                Y[218],
                Y[218 + 1],
                Y[90],
                Y[90 + 1],
                Y[218],
                Y[218 + 1],
                (double) -0.595699304492433, (double) -0.803207531480645);
  BUTTERFLY_XY (Y[92],               /* W46 */
                Y[92 + 1],
                Y[220],
                Y[220 + 1],
                Y[92],
                Y[92 + 1],
                Y[220],
                Y[220 + 1],
                (double) -0.634393284163645, (double) -0.773010453362737);
  BUTTERFLY_XY (Y[94],               /* W47 */
                Y[94 + 1],
                Y[222],
                Y[222 + 1],
                Y[94],
                Y[94 + 1],
                Y[222],
                Y[222 + 1],
                (double) -0.671558954847018, (double) -0.740951125354959);
  BUTTERFLY_XX (Y[96],               /* W48 */
                Y[96 + 1],
                Y[224],
                Y[224 + 1],
                Y[96],
                Y[96 + 1],
                Y[224],
                Y[224 + 1],
                (double) -0.707106781186547, (double) -0.707106781186548);
  BUTTERFLY_XY (Y[98],               /* W49 */
                Y[98 + 1],
                Y[226],
                Y[226 + 1],
                Y[98],
                Y[98 + 1],
                Y[226],
                Y[226 + 1],
                (double) -0.740951125354959, (double) -0.671558954847019);
  BUTTERFLY_XY (Y[100],               /* W50 */
                Y[100 + 1],
                Y[228],
                Y[228 + 1],
                Y[100],
                Y[100 + 1],
                Y[228],
                Y[228 + 1],
                (double) -0.773010453362737, (double) -0.634393284163645);
  BUTTERFLY_XY (Y[102],               /* W51 */
                Y[102 + 1],
                Y[230],
                Y[230 + 1],
                Y[102],
                Y[102 + 1],
                Y[230],
                Y[230 + 1],
                (double) -0.803207531480645, (double) -0.595699304492433);
  BUTTERFLY_XY (Y[104],               /* W52 */
                Y[104 + 1],
                Y[232],
                Y[232 + 1],
                Y[104],
                Y[104 + 1],
                Y[232],
                Y[232 + 1],
                (double) -0.831469612302545, (double) -0.555570233019602);
  BUTTERFLY_XY (Y[106],               /* W53 */
                Y[106 + 1],
                Y[234],
                Y[234 + 1],
                Y[106],
                Y[106 + 1],
                Y[234],
                Y[234 + 1],
                (double) -0.857728610000272, (double) -0.514102744193222);
  BUTTERFLY_XY (Y[108],               /* W54 */
                Y[108 + 1],
                Y[236],
                Y[236 + 1],
                Y[108],
                Y[108 + 1],
                Y[236],
                Y[236 + 1],
                (double) -0.881921264348355, (double) -0.471396736825998);
  BUTTERFLY_XY (Y[110],               /* W55 */
                Y[110 + 1],
                Y[238],
                Y[238 + 1],
                Y[110],
                Y[110 + 1],
                Y[238],
                Y[238 + 1],
                (double) -0.903989293123443, (double) -0.427555093430282);
  BUTTERFLY_XY (Y[112],               /* W56 */
                Y[112 + 1],
                Y[240],
                Y[240 + 1],
                Y[112],
                Y[112 + 1],
                Y[240],
                Y[240 + 1],
                (double) -0.923879532511287, (double) -0.382683432365090);
  BUTTERFLY_XY (Y[114],               /* W57 */
                Y[114 + 1],
                Y[242],
                Y[242 + 1],
                Y[114],
                Y[114 + 1],
                Y[242],
                Y[242 + 1],
                (double) -0.941544065183021, (double) -0.336889853392220);
  BUTTERFLY_XY (Y[116],               /* W58 */
                Y[116 + 1],
                Y[244],
                Y[244 + 1],
                Y[116],
                Y[116 + 1],
                Y[244],
                Y[244 + 1],
                (double) -0.956940335732209, (double) -0.290284677254462);
  BUTTERFLY_XY (Y[118],               /* W59 */
                Y[118 + 1],
                Y[246],
                Y[246 + 1],
                Y[118],
                Y[118 + 1],
                Y[246],
                Y[246 + 1],
                (double) -0.970031253194544, (double) -0.242980179903264);
  BUTTERFLY_XY (Y[120],               /* W60 */
                Y[120 + 1],
                Y[248],
                Y[248 + 1],
                Y[120],
                Y[120 + 1],
                Y[248],
                Y[248 + 1],
                (double) -0.980785280403230, (double) -0.195090322016129);
  BUTTERFLY_XY (Y[122],               /* W61 */
                Y[122 + 1],
                Y[250],
                Y[250 + 1],
                Y[122],
                Y[122 + 1],
                Y[250],
                Y[250 + 1],
                (double) -0.989176509964781, (double) -0.146730474455362);
  BUTTERFLY_XY (Y[124],               /* W62 */
                Y[124 + 1],
                Y[252],
                Y[252 + 1],
                Y[124],
                Y[124 + 1],
                Y[252],
                Y[252 + 1],
                (double) -0.995184726672197, (double) -0.098017140329561);
  BUTTERFLY_XY (Y[126],               /* W63 */
                Y[126 + 1],
                Y[254],
                Y[254 + 1],
                Y[126],
                Y[126 + 1],
                Y[254],
                Y[254 + 1],
                (double) -0.998795456205172, (double) -0.049067674327418);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 128 L S S S S S X)
 **/
static void
gsl_power2_fft128synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (128, X, Y);

  /* skipping 32 times fft4 */

  /* skipping 16 times fft8 */

  /* skipping 8 times fft16 */

  /* skipping 4 times fft32 */

  /* skipping 2 times fft64 */

  /* perform 1 times fft128 */
  gsl_power2_fft128synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 256 S S S S S S X T)
 **/
static void
gsl_power2_fft256synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 128 times fft2 */

  /* skipping 64 times fft4 */

  /* skipping 32 times fft8 */

  /* skipping 16 times fft16 */

  /* skipping 8 times fft32 */

  /* skipping 4 times fft64 */

  /* perform 2 times fft128 */
  gsl_power2_fft128synthesis_skip2 (X, Y);
  gsl_power2_fft128synthesis_skip2 (X + 256, Y + 256);

  /* perform 1 times fft256 */
  {
    static const double Wconst256[] = {
      +0.999698818696204, -0.024541228522912,
      +0.998795456205172, -0.049067674327418,
      +0.997290456678690, -0.073564563599667,
      +0.995184726672197, -0.098017140329561,
      +0.992479534598710, -0.122410675199216,
      +0.989176509964781, -0.146730474455362,
      +0.985277642388941, -0.170961888760301,
      +0.980785280403230, -0.195090322016128,
      +0.975702130038529, -0.219101240156870,
      +0.970031253194544, -0.242980179903264,
      +0.963776065795440, -0.266712757474898,
      +0.956940335732209, -0.290284677254462,
      +0.949528180593037, -0.313681740398892,
      +0.941544065183021, -0.336889853392220,
      +0.932992798834739, -0.359895036534988,
      +0.923879532511287, -0.382683432365090,
      +0.914209755703531, -0.405241314004990,
      +0.903989293123443, -0.427555093430282,
      +0.893224301195515, -0.449611329654607,
      +0.881921264348355, -0.471396736825998,
      +0.870086991108711, -0.492898192229784,
      +0.857728610000272, -0.514102744193222,
      +0.844853565249707, -0.534997619887097,
      +0.831469612302545, -0.555570233019602,
      +0.817584813151584, -0.575808191417845,
      +0.803207531480645, -0.595699304492433,
      +0.788346427626606, -0.615231590580627,
      +0.773010453362737, -0.634393284163645,
      +0.757208846506485, -0.653172842953777,
      +0.740951125354959, -0.671558954847018,
      +0.724247082951467, -0.689540544737067,
      +0.707106781186548, -0.707106781186547,
      +0.689540544737067, -0.724247082951467,
      +0.671558954847018, -0.740951125354959,
      +0.653172842953777, -0.757208846506484,
      +0.634393284163645, -0.773010453362737,
      +0.615231590580627, -0.788346427626606,
      +0.595699304492433, -0.803207531480645,
      +0.575808191417845, -0.817584813151584,
      +0.555570233019602, -0.831469612302545,
      +0.534997619887097, -0.844853565249707,
      +0.514102744193222, -0.857728610000272,
      +0.492898192229784, -0.870086991108711,
      +0.471396736825998, -0.881921264348355,
      +0.449611329654607, -0.893224301195515,
      +0.427555093430282, -0.903989293123443,
      +0.405241314004990, -0.914209755703531,
      +0.382683432365090, -0.923879532511287,
      +0.359895036534988, -0.932992798834739,
      +0.336889853392220, -0.941544065183021,
      +0.313681740398892, -0.949528180593037,
      +0.290284677254462, -0.956940335732209,
      +0.266712757474898, -0.963776065795440,
      +0.242980179903264, -0.970031253194544,
      +0.219101240156870, -0.975702130038529,
      +0.195090322016128, -0.980785280403230,
      +0.170961888760301, -0.985277642388941,
      +0.146730474455362, -0.989176509964781,
      +0.122410675199216, -0.992479534598710,
      +0.098017140329561, -0.995184726672197,
      +0.073564563599667, -0.997290456678690,
      +0.049067674327418, -0.998795456205172,
      +0.024541228522912, -0.999698818696204,
    };
    const double *W = Wconst256 - 2;
    double *Z = Y + 128;
    for (offset = 0; offset < 512; offset += 512) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    __1, __0);
      BUTTERFLY_0m (Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 128; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 512; block += 512) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Wre, Wim);
        BUTTERFLY_yX (Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Wre, Wim);
      }
    }
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 256 L S S S S S S X)
 **/
static void
gsl_power2_fft256synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (256, X, Y);

  /* skipping 64 times fft4 */

  /* skipping 32 times fft8 */

  /* skipping 16 times fft16 */

  /* skipping 8 times fft32 */

  /* skipping 4 times fft64 */

  /* skipping 2 times fft128 */

  /* perform 1 times fft256 */
  gsl_power2_fft256synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 512 S S S S S S X T T)
 **/
static void
gsl_power2_fft512synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 256 times fft2 */

  /* skipping 128 times fft4 */

  /* skipping 64 times fft8 */

  /* skipping 32 times fft16 */

  /* skipping 16 times fft32 */

  /* skipping 8 times fft64 */

  /* perform 4 times fft128 */
  gsl_power2_fft128synthesis_skip2 (X, Y);
  gsl_power2_fft128synthesis_skip2 (X + 256, Y + 256);
  gsl_power2_fft128synthesis_skip2 (X + 512, Y + 512);
  gsl_power2_fft128synthesis_skip2 (X + 768, Y + 768);

  /* perform 2 times fft256 */
  {
    static const double Wconst256[] = {
      +0.999698818696204, -0.024541228522912,
      +0.998795456205172, -0.049067674327418,
      +0.997290456678690, -0.073564563599667,
      +0.995184726672197, -0.098017140329561,
      +0.992479534598710, -0.122410675199216,
      +0.989176509964781, -0.146730474455362,
      +0.985277642388941, -0.170961888760301,
      +0.980785280403230, -0.195090322016128,
      +0.975702130038529, -0.219101240156870,
      +0.970031253194544, -0.242980179903264,
      +0.963776065795440, -0.266712757474898,
      +0.956940335732209, -0.290284677254462,
      +0.949528180593037, -0.313681740398892,
      +0.941544065183021, -0.336889853392220,
      +0.932992798834739, -0.359895036534988,
      +0.923879532511287, -0.382683432365090,
      +0.914209755703531, -0.405241314004990,
      +0.903989293123443, -0.427555093430282,
      +0.893224301195515, -0.449611329654607,
      +0.881921264348355, -0.471396736825998,
      +0.870086991108711, -0.492898192229784,
      +0.857728610000272, -0.514102744193222,
      +0.844853565249707, -0.534997619887097,
      +0.831469612302545, -0.555570233019602,
      +0.817584813151584, -0.575808191417845,
      +0.803207531480645, -0.595699304492433,
      +0.788346427626606, -0.615231590580627,
      +0.773010453362737, -0.634393284163645,
      +0.757208846506485, -0.653172842953777,
      +0.740951125354959, -0.671558954847018,
      +0.724247082951467, -0.689540544737067,
      +0.707106781186548, -0.707106781186547,
      +0.689540544737067, -0.724247082951467,
      +0.671558954847018, -0.740951125354959,
      +0.653172842953777, -0.757208846506484,
      +0.634393284163645, -0.773010453362737,
      +0.615231590580627, -0.788346427626606,
      +0.595699304492433, -0.803207531480645,
      +0.575808191417845, -0.817584813151584,
      +0.555570233019602, -0.831469612302545,
      +0.534997619887097, -0.844853565249707,
      +0.514102744193222, -0.857728610000272,
      +0.492898192229784, -0.870086991108711,
      +0.471396736825998, -0.881921264348355,
      +0.449611329654607, -0.893224301195515,
      +0.427555093430282, -0.903989293123443,
      +0.405241314004990, -0.914209755703531,
      +0.382683432365090, -0.923879532511287,
      +0.359895036534988, -0.932992798834739,
      +0.336889853392220, -0.941544065183021,
      +0.313681740398892, -0.949528180593037,
      +0.290284677254462, -0.956940335732209,
      +0.266712757474898, -0.963776065795440,
      +0.242980179903264, -0.970031253194544,
      +0.219101240156870, -0.975702130038529,
      +0.195090322016128, -0.980785280403230,
      +0.170961888760301, -0.985277642388941,
      +0.146730474455362, -0.989176509964781,
      +0.122410675199216, -0.992479534598710,
      +0.098017140329561, -0.995184726672197,
      +0.073564563599667, -0.997290456678690,
      +0.049067674327418, -0.998795456205172,
      +0.024541228522912, -0.999698818696204,
    };
    const double *W = Wconst256 - 2;
    double *Z = Y + 128;
    for (offset = 0; offset < 1024; offset += 512) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 256], Y[offset + 256 + 1],
                    __1, __0);
      BUTTERFLY_0m (Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 256], Z[offset + 256 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 128; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 1024; block += 512) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 256], Y[offset + 256 + 1],
                      Wre, Wim);
        BUTTERFLY_yX (Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 256], Z[offset + 256 + 1],
                      Wre, Wim);
      }
    }
  }

  /* perform 1 times fft512 */
  {
    static const double Wconst512[] = {
      +0.999924701839145, -0.012271538285720,
      +0.999698818696204, -0.024541228522912,
      +0.999322384588350, -0.036807222941359,
      +0.998795456205172, -0.049067674327418,
      +0.998118112900149, -0.061320736302209,
      +0.997290456678690, -0.073564563599667,
      +0.996312612182778, -0.085797312344440,
      +0.995184726672197, -0.098017140329561,
      +0.993906970002356, -0.110222207293883,
      +0.992479534598710, -0.122410675199216,
      +0.990902635427780, -0.134580708507126,
      +0.989176509964781, -0.146730474455362,
      +0.987301418157858, -0.158858143333861,
      +0.985277642388941, -0.170961888760301,
      +0.983105487431216, -0.183039887955141,
      +0.980785280403230, -0.195090322016128,
      +0.978317370719628, -0.207111376192219,
      +0.975702130038529, -0.219101240156870,
      +0.972939952205560, -0.231058108280671,
      +0.970031253194544, -0.242980179903264,
      +0.966976471044852, -0.254865659604515,
      +0.963776065795440, -0.266712757474898,
      +0.960430519415566, -0.278519689385053,
      +0.956940335732209, -0.290284677254462,
      +0.953306040354194, -0.302005949319228,
      +0.949528180593037, -0.313681740398892,
      +0.945607325380521, -0.325310292162263,
      +0.941544065183021, -0.336889853392220,
      +0.937339011912575, -0.348418680249435,
      +0.932992798834739, -0.359895036534988,
      +0.928506080473216, -0.371317193951838,
      +0.923879532511287, -0.382683432365090,
      +0.919113851690058, -0.393992040061048,
      +0.914209755703531, -0.405241314004990,
      +0.909167983090522, -0.416429560097637,
      +0.903989293123443, -0.427555093430282,
      +0.898674465693954, -0.438616238538528,
      +0.893224301195515, -0.449611329654607,
      +0.887639620402854, -0.460538710958240,
      +0.881921264348355, -0.471396736825998,
      +0.876070094195407, -0.482183772079123,
      +0.870086991108711, -0.492898192229784,
      +0.863972856121587, -0.503538383725718,
      +0.857728610000272, -0.514102744193222,
      +0.851355193105265, -0.524589682678469,
      +0.844853565249707, -0.534997619887097,
      +0.838224705554838, -0.545324988422046,
      +0.831469612302545, -0.555570233019602,
      +0.824589302785025, -0.565731810783613,
      +0.817584813151584, -0.575808191417845,
      +0.810457198252595, -0.585797857456439,
      +0.803207531480645, -0.595699304492433,
      +0.795836904608884, -0.605511041404326,
      +0.788346427626606, -0.615231590580627,
      +0.780737228572094, -0.624859488142386,
      +0.773010453362737, -0.634393284163645,
      +0.765167265622459, -0.643831542889791,
      +0.757208846506485, -0.653172842953777,
      +0.749136394523459, -0.662415777590172,
      +0.740951125354959, -0.671558954847018,
      +0.732654271672413, -0.680600997795453,
      +0.724247082951467, -0.689540544737067,
      +0.715730825283819, -0.698376249408973,
      +0.707106781186548, -0.707106781186547,
      +0.698376249408973, -0.715730825283819,
      +0.689540544737067, -0.724247082951467,
      +0.680600997795453, -0.732654271672413,
      +0.671558954847018, -0.740951125354959,
      +0.662415777590172, -0.749136394523459,
      +0.653172842953777, -0.757208846506484,
      +0.643831542889791, -0.765167265622459,
      +0.634393284163645, -0.773010453362737,
      +0.624859488142386, -0.780737228572094,
      +0.615231590580627, -0.788346427626606,
      +0.605511041404326, -0.795836904608883,
      +0.595699304492433, -0.803207531480645,
      +0.585797857456439, -0.810457198252595,
      +0.575808191417845, -0.817584813151584,
      +0.565731810783613, -0.824589302785025,
      +0.555570233019602, -0.831469612302545,
      +0.545324988422046, -0.838224705554838,
      +0.534997619887097, -0.844853565249707,
      +0.524589682678469, -0.851355193105265,
      +0.514102744193222, -0.857728610000272,
      +0.503538383725718, -0.863972856121587,
      +0.492898192229784, -0.870086991108711,
      +0.482183772079123, -0.876070094195407,
      +0.471396736825998, -0.881921264348355,
      +0.460538710958240, -0.887639620402854,
      +0.449611329654607, -0.893224301195515,
      +0.438616238538528, -0.898674465693954,
      +0.427555093430282, -0.903989293123443,
      +0.416429560097637, -0.909167983090522,
      +0.405241314004990, -0.914209755703531,
      +0.393992040061048, -0.919113851690058,
      +0.382683432365090, -0.923879532511287,
      +0.371317193951838, -0.928506080473215,
      +0.359895036534988, -0.932992798834739,
      +0.348418680249435, -0.937339011912575,
      +0.336889853392220, -0.941544065183021,
      +0.325310292162263, -0.945607325380521,
      +0.313681740398892, -0.949528180593037,
      +0.302005949319228, -0.953306040354194,
      +0.290284677254462, -0.956940335732209,
      +0.278519689385053, -0.960430519415566,
      +0.266712757474898, -0.963776065795440,
      +0.254865659604515, -0.966976471044852,
      +0.242980179903264, -0.970031253194544,
      +0.231058108280671, -0.972939952205560,
      +0.219101240156870, -0.975702130038529,
      +0.207111376192219, -0.978317370719628,
      +0.195090322016128, -0.980785280403230,
      +0.183039887955141, -0.983105487431216,
      +0.170961888760301, -0.985277642388941,
      +0.158858143333861, -0.987301418157858,
      +0.146730474455362, -0.989176509964781,
      +0.134580708507126, -0.990902635427780,
      +0.122410675199216, -0.992479534598710,
      +0.110222207293883, -0.993906970002356,
      +0.098017140329561, -0.995184726672197,
      +0.085797312344440, -0.996312612182778,
      +0.073564563599667, -0.997290456678690,
      +0.061320736302209, -0.998118112900149,
      +0.049067674327418, -0.998795456205172,
      +0.036807222941359, -0.999322384588350,
      +0.024541228522912, -0.999698818696204,
      +0.012271538285720, -0.999924701839145,
    };
    const double *W = Wconst512 - 2;
    double *Z = Y + 256;
    for (offset = 0; offset < 1024; offset += 1024) {
      BUTTERFLY_10 (Y[offset], Y[offset + 1],
                    Y[offset + 512], Y[offset + 512 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 512], Y[offset + 512 + 1],
                    __1, __0);
      BUTTERFLY_0m (Z[offset], Z[offset + 1],
                    Z[offset + 512], Z[offset + 512 + 1],
                    Z[offset], Z[offset + 1],
                    Z[offset + 512], Z[offset + 512 + 1],
                    __0, __1);
    }
    for (butterfly = 2; butterfly < 256; butterfly += 2) {
      Wre = W[butterfly]; Wim = W[butterfly + 1];
      for (block = 0; block < 1024; block += 1024) {
        offset = butterfly + block;
        BUTTERFLY_XY (Y[offset], Y[offset + 1],
                      Y[offset + 512], Y[offset + 512 + 1],
                      Y[offset], Y[offset + 1],
                      Y[offset + 512], Y[offset + 512 + 1],
                      Wre, Wim);
        BUTTERFLY_yX (Z[offset], Z[offset + 1],
                      Z[offset + 512], Z[offset + 512 + 1],
                      Z[offset], Z[offset + 1],
                      Z[offset + 512], Z[offset + 512 + 1],
                      Wre, Wim);
      }
    }
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 512 L S S S S S S S X)
 **/
static void
gsl_power2_fft512synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (512, X, Y);

  /* skipping 128 times fft4 */

  /* skipping 64 times fft8 */

  /* skipping 32 times fft16 */

  /* skipping 16 times fft32 */

  /* skipping 8 times fft64 */

  /* skipping 4 times fft128 */

  /* skipping 2 times fft256 */

  /* perform 1 times fft512 */
  gsl_power2_fft512synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 1024 S S S S S S S S X L)
 **/
static void
gsl_power2_fft1024synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 512 times fft2 */

  /* skipping 256 times fft4 */

  /* skipping 128 times fft8 */

  /* skipping 64 times fft16 */

  /* skipping 32 times fft32 */

  /* skipping 16 times fft64 */

  /* skipping 8 times fft128 */

  /* skipping 4 times fft256 */

  /* perform 2 times fft512 */
  gsl_power2_fft512synthesis_skip2 (X, Y);
  gsl_power2_fft512synthesis_skip2 (X + 1024, Y + 1024);

  /* perform 1 times fft1024 */
  for (offset = 0; offset < 2048; offset += 2048) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __1, __0);
  }
  Wre = +0.999981175282601; Wim = -0.006135884649154;
  for (butterfly = 2; butterfly < 512; butterfly += 2) {
    for (block = 0; block < 2048; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, -0.006135884649154);
  }
  for (offset = 512; offset < 2048; offset += 2048) {
    BUTTERFLY_0m (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __0, __1);
  }
  Wre = -0.006135884649154; Wim = -0.999981175282601;
  for (butterfly = 514; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 2048; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, -0.006135884649154);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 1024 L S S S S S S S S X)
 **/
static void
gsl_power2_fft1024synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (1024, X, Y);

  /* skipping 256 times fft4 */

  /* skipping 128 times fft8 */

  /* skipping 64 times fft16 */

  /* skipping 32 times fft32 */

  /* skipping 16 times fft64 */

  /* skipping 8 times fft128 */

  /* skipping 4 times fft256 */

  /* skipping 2 times fft512 */

  /* perform 1 times fft1024 */
  gsl_power2_fft1024synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 2048 S S S S S S S S X L L)
 **/
static void
gsl_power2_fft2048synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 1024 times fft2 */

  /* skipping 512 times fft4 */

  /* skipping 256 times fft8 */

  /* skipping 128 times fft16 */

  /* skipping 64 times fft32 */

  /* skipping 32 times fft64 */

  /* skipping 16 times fft128 */

  /* skipping 8 times fft256 */

  /* perform 4 times fft512 */
  gsl_power2_fft512synthesis_skip2 (X, Y);
  gsl_power2_fft512synthesis_skip2 (X + 1024, Y + 1024);
  gsl_power2_fft512synthesis_skip2 (X + 2048, Y + 2048);
  gsl_power2_fft512synthesis_skip2 (X + 3072, Y + 3072);

  /* perform 2 times fft1024 */
  for (offset = 0; offset < 4096; offset += 2048) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __1, __0);
  }
  Wre = +0.999981175282601; Wim = -0.006135884649154;
  for (butterfly = 2; butterfly < 512; butterfly += 2) {
    for (block = 0; block < 4096; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, -0.006135884649154);
  }
  for (offset = 512; offset < 4096; offset += 2048) {
    BUTTERFLY_0m (Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 1024], Y[offset + 1024 + 1],
                  __0, __1);
  }
  Wre = -0.006135884649154; Wim = -0.999981175282601;
  for (butterfly = 514; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 4096; block += 2048) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 1024], Y[offset + 1024 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000018824717399, -0.006135884649154);
  }

  /* perform 1 times fft2048 */
  for (offset = 0; offset < 4096; offset += 4096) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  __1, __0);
  }
  Wre = +0.999995293809576; Wim = -0.003067956762966;
  for (butterfly = 2; butterfly < 1024; butterfly += 2) {
    for (block = 0; block < 4096; block += 4096) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000004706190424, -0.003067956762966);
  }
  for (offset = 1024; offset < 4096; offset += 4096) {
    BUTTERFLY_0m (Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 2048], Y[offset + 2048 + 1],
                  __0, __1);
  }
  Wre = -0.003067956762966; Wim = -0.999995293809576;
  for (butterfly = 1026; butterfly < 2048; butterfly += 2) {
    for (block = 0; block < 4096; block += 4096) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 2048], Y[offset + 2048 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000004706190424, -0.003067956762966);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 2048 L S S S S S S S S S X)
 **/
static void
gsl_power2_fft2048synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (2048, X, Y);

  /* skipping 512 times fft4 */

  /* skipping 256 times fft8 */

  /* skipping 128 times fft16 */

  /* skipping 64 times fft32 */

  /* skipping 32 times fft64 */

  /* skipping 16 times fft128 */

  /* skipping 8 times fft256 */

  /* skipping 4 times fft512 */

  /* skipping 2 times fft1024 */

  /* perform 1 times fft2048 */
  gsl_power2_fft2048synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4096 S S S S S S S S S S X L)
 **/
static void
gsl_power2_fft4096synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 2048 times fft2 */

  /* skipping 1024 times fft4 */

  /* skipping 512 times fft8 */

  /* skipping 256 times fft16 */

  /* skipping 128 times fft32 */

  /* skipping 64 times fft64 */

  /* skipping 32 times fft128 */

  /* skipping 16 times fft256 */

  /* skipping 8 times fft512 */

  /* skipping 4 times fft1024 */

  /* perform 2 times fft2048 */
  gsl_power2_fft2048synthesis_skip2 (X, Y);
  gsl_power2_fft2048synthesis_skip2 (X + 4096, Y + 4096);

  /* perform 1 times fft4096 */
  for (offset = 0; offset < 8192; offset += 8192) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  __1, __0);
  }
  Wre = +0.999998823451702; Wim = -0.001533980186285;
  for (butterfly = 2; butterfly < 2048; butterfly += 2) {
    for (block = 0; block < 8192; block += 8192) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000001176548298, -0.001533980186285);
  }
  for (offset = 2048; offset < 8192; offset += 8192) {
    BUTTERFLY_0m (Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 4096], Y[offset + 4096 + 1],
                  __0, __1);
  }
  Wre = -0.001533980186285; Wim = -0.999998823451702;
  for (butterfly = 2050; butterfly < 4096; butterfly += 2) {
    for (block = 0; block < 8192; block += 8192) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 4096], Y[offset + 4096 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000001176548298, -0.001533980186285);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 4096 L S S S S S S S S S S X)
 **/
static void
gsl_power2_fft4096synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (4096, X, Y);

  /* skipping 1024 times fft4 */

  /* skipping 512 times fft8 */

  /* skipping 256 times fft16 */

  /* skipping 128 times fft32 */

  /* skipping 64 times fft64 */

  /* skipping 32 times fft128 */

  /* skipping 16 times fft256 */

  /* skipping 8 times fft512 */

  /* skipping 4 times fft1024 */

  /* skipping 2 times fft2048 */

  /* perform 1 times fft4096 */
  gsl_power2_fft4096synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8192 S S S S S S S S S S S X L)
 **/
static void
gsl_power2_fft8192synthesis_skip2 (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* skipping 4096 times fft2 */

  /* skipping 2048 times fft4 */

  /* skipping 1024 times fft8 */

  /* skipping 512 times fft16 */

  /* skipping 256 times fft32 */

  /* skipping 128 times fft64 */

  /* skipping 64 times fft128 */

  /* skipping 32 times fft256 */

  /* skipping 16 times fft512 */

  /* skipping 8 times fft1024 */

  /* skipping 4 times fft2048 */

  /* perform 2 times fft4096 */
  gsl_power2_fft4096synthesis_skip2 (X, Y);
  gsl_power2_fft4096synthesis_skip2 (X + 8192, Y + 8192);

  /* perform 1 times fft8192 */
  for (offset = 0; offset < 16384; offset += 16384) {
    BUTTERFLY_10 (Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  __1, __0);
  }
  Wre = +0.999999705862882; Wim = -0.000766990318743;
  for (butterfly = 2; butterfly < 4096; butterfly += 2) {
    for (block = 0; block < 16384; block += 16384) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000000294137118, -0.000766990318743);
  }
  for (offset = 4096; offset < 16384; offset += 16384) {
    BUTTERFLY_0m (Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  Y[offset], Y[offset + 1],
                  Y[offset + 8192], Y[offset + 8192 + 1],
                  __0, __1);
  }
  Wre = -0.000766990318743; Wim = -0.999999705862882;
  for (butterfly = 4098; butterfly < 8192; butterfly += 2) {
    for (block = 0; block < 16384; block += 16384) {
      offset = butterfly + block;
      BUTTERFLY_XY (Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Y[offset], Y[offset + 1],
                    Y[offset + 8192], Y[offset + 8192 + 1],
                    Wre, Wim);
    }
    WMULTIPLY (Wre, Wim, -0.000000294137118, -0.000766990318743);
  }
}

/**
 ** Generated data ends here
 **/
/**
 ** Generated data (by gsl-genfft 8192 L S S S S S S S S S S S X)
 **/
static void
gsl_power2_fft8192synthesis (const double *X, double *Y)
{
  register unsigned int butterfly, block, offset;
  register double Wre, Wim;

  butterfly = block = offset = 0, Wre = Wim = 0.0; /* silence compiler */

  /* perform fft2 and bitreverse input */
  bitreverse_fft2synthesis (8192, X, Y);

  /* skipping 2048 times fft4 */

  /* skipping 1024 times fft8 */

  /* skipping 512 times fft16 */

  /* skipping 256 times fft32 */

  /* skipping 128 times fft64 */

  /* skipping 64 times fft128 */

  /* skipping 32 times fft256 */

  /* skipping 16 times fft512 */

  /* skipping 8 times fft1024 */

  /* skipping 4 times fft2048 */

  /* skipping 2 times fft4096 */

  /* perform 1 times fft8192 */
  gsl_power2_fft8192synthesis_skip2 (X, Y);
}

/**
 ** Generated data ends here
 **/
static void
gsl_power2_fftc_big (const unsigned int n_values,
		     const double  *rivalues_in,
		     double        *rivalues,
                     const int          esign)
{
  const unsigned int n_values2 = n_values << 1;
  double theta = esign < 0 ? -3.1415926535897932384626433832795029 : 3.1415926535897932384626433832795029;
  unsigned int i, block_size = 8192 << 1;
  double last_sin;

  if (esign > 0)
    {
      bitreverse_fft2analysis (n_values, rivalues_in, rivalues);
      for (i = 0; i < n_values; i += 8192)
        gsl_power2_fft8192analysis_skip2 (rivalues + (i << 1), rivalues + (i << 1));
    }
  else
    {
      bitreverse_fft2synthesis (n_values, rivalues_in, rivalues);
      for (i = 0; i < n_values; i += 8192)
        gsl_power2_fft8192synthesis_skip2 (rivalues + (i << 1), rivalues + (i << 1));
    }
  theta *= (double) 1.0 / 8192.;
  last_sin = sin (theta);
  
  /* we're not combining the first and second halves coefficients
   * in the below loop, since for fft sizes beyond 8192, it'd actually
   * harm performance due to paging
   */
  do
    {
      double Dre, Dim, Wre, Wim;
      unsigned int k, i, half_block = block_size >> 1;
      unsigned int block_size2 = block_size << 1;

      theta *= 0.5;
      Dim = last_sin;
      last_sin = sin (theta);
      Dre = last_sin * last_sin * -2.;
      
      /* loop over first coefficient in each block ==> w == {1,0} */
      for (i = 0; i < n_values2; i += block_size2)
	{
	  unsigned int v1 = i, v2 = i + block_size;

          BUTTERFLY_10 (rivalues[v1], rivalues[v1 + 1],
                        rivalues[v2], rivalues[v2 + 1],
                        rivalues[v1], rivalues[v1 + 1],
                        rivalues[v2], rivalues[v2 + 1],
                        __1, __0);
	}
      Wre = Dre + 1.0;	/* update Wk */
      Wim = Dim;	/* update Wk */
      /* loop for every Wk in the first half of each subblock */
      for (k = 2; k < half_block; k += 2)
	{
	  /* loop over kth coefficient in each block */
	  for (i = k; i < n_values2; i += block_size2)
	    {
	      unsigned int v1 = i, v2 = i + block_size;
	      
              BUTTERFLY_XY (rivalues[v1], rivalues[v1 + 1],
                            rivalues[v2], rivalues[v2 + 1],
                            rivalues[v1], rivalues[v1 + 1],
                            rivalues[v2], rivalues[v2 + 1],
                            Wre, Wim);
	    }
	  WMULTIPLY (Wre, Wim, Dre, Dim);	/* update Wk */
	}
      /* handle middle coefficient ==> w == {0,+-1} */
      if (k < block_size)
	{
	  /* loop over kth coefficient in each block */
	  if (esign > 0)
	    for (i = k; i < n_values2; i += block_size2)
	      {
	        unsigned int v1 = i, v2 = i + block_size;
	      
                BUTTERFLY_01 (rivalues[v1], rivalues[v1 + 1],
                              rivalues[v2], rivalues[v2 + 1],
                              rivalues[v1], rivalues[v1 + 1],
                              rivalues[v2], rivalues[v2 + 1],
                              __0, __1);
	      }
	  else
	    for (i = k; i < n_values2; i += block_size2)
	      {
	        unsigned int v1 = i, v2 = i + block_size;
	      
                BUTTERFLY_0m (rivalues[v1], rivalues[v1 + 1],
                              rivalues[v2], rivalues[v2 + 1],
                              rivalues[v1], rivalues[v1 + 1],
                              rivalues[v2], rivalues[v2 + 1],
                              __0, __1);
	      }
	  /* update Wk */
	  if (esign > 0)
	    {
	      Wre = -Dim;
	      Wim = Dre + 1.0;
	    }
	  else
	    {
	      Wre = Dim;
	      Wim = -Dre - 1.0;
	    }
	  k += 2;
	}
      /* loop for every Wk in the second half of each subblock */
      for (; k < block_size; k += 2)
	{
	  /* loop over kth coefficient in each block */
	  for (i = k; i < n_values2; i += block_size2)
	    {
	      unsigned int v1 = i, v2 = i + block_size;

              BUTTERFLY_XY (rivalues[v1], rivalues[v1 + 1],
                            rivalues[v2], rivalues[v2 + 1],
                            rivalues[v1], rivalues[v1 + 1],
                            rivalues[v2], rivalues[v2 + 1],
                            Wre, Wim);
	    }
	  WMULTIPLY (Wre, Wim, Dre, Dim);	/* update Wk */
	}
      block_size = block_size2;
    }
  while (block_size <= n_values);
}
void
gsl_power2_fftac (const unsigned int n_values,
                  const double  *rivalues_in,
                  double        *rivalues_out)
{
  g_return_if_fail ((n_values & (n_values - 1)) == 0 && n_values >= 1);
  
  switch (n_values)
    {
      case    1: rivalues_out[0] = rivalues_in[0], rivalues_out[1] = rivalues_in[1]; break;
      case    2: gsl_power2_fft2analysis (rivalues_in, rivalues_out);		break;
      case    4: gsl_power2_fft4analysis (rivalues_in, rivalues_out);		break;
      case    8: gsl_power2_fft8analysis (rivalues_in, rivalues_out);		break;
      case   16: gsl_power2_fft16analysis (rivalues_in, rivalues_out);		break;
      case   32: gsl_power2_fft32analysis (rivalues_in, rivalues_out);		break;
      case   64: gsl_power2_fft64analysis (rivalues_in, rivalues_out);		break;
      case  128: gsl_power2_fft128analysis (rivalues_in, rivalues_out);		break;
      case  256: gsl_power2_fft256analysis (rivalues_in, rivalues_out);		break;
      case  512: gsl_power2_fft512analysis (rivalues_in, rivalues_out);		break;
      case 1024: gsl_power2_fft1024analysis (rivalues_in, rivalues_out);	break;
      case 2048: gsl_power2_fft2048analysis (rivalues_in, rivalues_out);	break;
      case 4096: gsl_power2_fft4096analysis (rivalues_in, rivalues_out);	break;
      case 8192: gsl_power2_fft8192analysis (rivalues_in, rivalues_out);	break;
      default:	 gsl_power2_fftc_big (n_values, rivalues_in, rivalues_out, +1);
    }
}
void
gsl_power2_fftsc (const unsigned int n_values,
                  const double  *rivalues_in,
                  double        *rivalues_out)
{
  g_return_if_fail ((n_values & (n_values - 1)) == 0 && n_values >= 1);
  
  switch (n_values)
    {
      case    1: rivalues_out[0] = rivalues_in[0], rivalues_out[1] = rivalues_in[1]; break;
      case    2: gsl_power2_fft2synthesis (rivalues_in, rivalues_out);		break;
      case    4: gsl_power2_fft4synthesis (rivalues_in, rivalues_out);		break;
      case    8: gsl_power2_fft8synthesis (rivalues_in, rivalues_out);		break;
      case   16: gsl_power2_fft16synthesis (rivalues_in, rivalues_out);		break;
      case   32: gsl_power2_fft32synthesis (rivalues_in, rivalues_out);		break;
      case   64: gsl_power2_fft64synthesis (rivalues_in, rivalues_out);		break;
      case  128: gsl_power2_fft128synthesis (rivalues_in, rivalues_out);	break;
      case  256: gsl_power2_fft256synthesis (rivalues_in, rivalues_out);	break;
      case  512: gsl_power2_fft512synthesis (rivalues_in, rivalues_out);	break;
      case 1024: gsl_power2_fft1024synthesis (rivalues_in, rivalues_out);	break;
      case 2048: gsl_power2_fft2048synthesis (rivalues_in, rivalues_out);	break;
      case 4096: gsl_power2_fft4096synthesis (rivalues_in, rivalues_out);	break;
      case 8192: gsl_power2_fft8192synthesis (rivalues_in, rivalues_out);	break;
      default:	 gsl_power2_fftc_big (n_values, rivalues_in, rivalues_out, -1);
    }
  /* { unsigned int i; for (i = 0; i < n_values << 1; i++) rivalues_out[i] *= (double) n_values; } */
}
void
gsl_power2_fftar (const unsigned int n_values,
                  const double  *r_values_in,
                  double        *rivalues_out)
{
  unsigned int n_cvalues = n_values >> 1;
  double Dre, Dim, Wre, Wim, theta;
  unsigned int i;

  g_return_if_fail ((n_values & (n_values - 1)) == 0 && n_values >= 2);

  gsl_power2_fftac (n_cvalues, r_values_in, rivalues_out);
  theta = 3.1415926535897932384626433832795029;
  theta /= (double) n_cvalues;

  Dre = sin (0.5 * theta);
  Dim = sin (theta);
  Dre = Dre * Dre;
  Wre = 0.5 - Dre;
  Dre *= -2.;
  Wim = Dim * 0.5;
  for (i = 2; i < n_values >> 1; i += 2)
    {
      double F1re, F1im, F2re, F2im, H1re, H1im, H2re, H2im;
      unsigned int r = n_values - i;
      double FEre = rivalues_out[i] + rivalues_out[r];
      double FEim = rivalues_out[i + 1] - rivalues_out[r + 1];
      double FOre = rivalues_out[r] - rivalues_out[i];
      double FOim = rivalues_out[r + 1] + rivalues_out[i + 1];

      FEre *= 0.5;
      FEim *= 0.5;
      F2re = FOre * Wim;
      F2im = FOim * Wim;
      F1re = FOre * Wre;
      F1im = FOim * Wre;

      H1im = F2im + F1re;
      H1re = F1im - F2re;
      H2re = F2re - F1im;
      H2im = H1im - FEim;
      H1re += FEre;
      H2re += FEre;
      H1im += FEim;
      rivalues_out[i] = H1re;
      rivalues_out[i + 1] = H1im;
      rivalues_out[r] = H2re;
      rivalues_out[r + 1] = H2im;
      WMULTIPLY (Wre, Wim, Dre, Dim);
    }
  Dre = rivalues_out[0];
  rivalues_out[0] = Dre + rivalues_out[1];
  rivalues_out[1] = Dre - rivalues_out[1];
}
void
gsl_power2_fftsr (const unsigned int n_values,
                  const double      *rivalues_in,
                  double            *r_values_out)
{
  unsigned int n_cvalues = n_values >> 1;
  double Dre, Dim, Wre, Wim, theta, scale;
  unsigned int i, ri;

  g_return_if_fail ((n_values & (n_values - 1)) == 0 && n_values >= 2);

  theta = -3.1415926535897932384626433832795029;
  theta /= (double) n_cvalues;

  Dre = sin (0.5 * theta);
  Dim = sin (theta);
  Dre = Dre * Dre;
  Wre = 0.5 - Dre;
  Dre *= -2.;
  Wim = Dim * 0.5;
  for (i = 2, ri = 0; i < n_values >> 1; i += 2)
    {
      double F1re, F1im, F2re, F2im, H1re, H1im, H2re, H2im;
      unsigned int g = n_values - i, j = n_values >> 2, rg = n_values - (ri << 1) - 2;
      double FEre = rivalues_in[i] + rivalues_in[g];
      double FEim = rivalues_in[i + 1] - rivalues_in[g + 1];
      double FOre = rivalues_in[g] - rivalues_in[i];
      double FOim = rivalues_in[g + 1] + rivalues_in[i + 1];

      while (ri >= j)
        {
          ri -= j;
          j >>= 1;
        }
      ri |= j;

      FOre = -FOre;
      FOim = -FOim;
      FEre *= 0.5;
      FEim *= 0.5;
      F2re = FOre * Wim;
      F2im = FOim * Wim;
      F1re = FOre * Wre;
      F1im = FOim * Wre;

      H1im = F2im + F1re;
      H1re = F1im - F2re;
      H2re = F2re - F1im;
      H2im = H1im - FEim;
      H1re += FEre;
      H2re += FEre;
      H1im += FEim;

      j = ri << 1;
      r_values_out[j] = H1re;
      r_values_out[j + 1] = H1im;
      r_values_out[rg] = H2re;
      r_values_out[rg + 1] = H2im;
      WMULTIPLY (Wre, Wim, Dre, Dim);
    }
  Dre = rivalues_in[0];
  r_values_out[0] = Dre + rivalues_in[1];
  r_values_out[1] = Dre - rivalues_in[1];
  r_values_out[0] *= 0.5;
  r_values_out[1] *= 0.5;
  if (n_values < 4)
    return;
  r_values_out[2] = rivalues_in[i];
  r_values_out[2 + 1] = rivalues_in[i + 1];
  scale = n_cvalues;
  scale = 1.0 / scale;
  for (i = 0; i < n_values; i += 4)
    BUTTERFLY_10scale (r_values_out[i], r_values_out[i + 1],
                       r_values_out[i + 2], r_values_out[i + 3],
                       r_values_out[i], r_values_out[i + 1],
                       r_values_out[i + 2], r_values_out[i + 3],
                       scale);
  switch (n_cvalues)
    {
      case    2: break;
      case    4: gsl_power2_fft4synthesis_skip2 (NULL, r_values_out);	 break;
      case    8: gsl_power2_fft8synthesis_skip2 (NULL, r_values_out);	 break;
      case   16: gsl_power2_fft16synthesis_skip2 (NULL, r_values_out);	 break;
      case   32: gsl_power2_fft32synthesis_skip2 (NULL, r_values_out);	 break;
      case   64: gsl_power2_fft64synthesis_skip2 (NULL, r_values_out);	 break;
      case  128: gsl_power2_fft128synthesis_skip2 (NULL, r_values_out);	 break;
      case  256: gsl_power2_fft256synthesis_skip2 (NULL, r_values_out);	 break;
      case  512: gsl_power2_fft512synthesis_skip2 (NULL, r_values_out);	 break;
      case 1024: gsl_power2_fft1024synthesis_skip2 (NULL, r_values_out); break;
      case 2048: gsl_power2_fft2048synthesis_skip2 (NULL, r_values_out); break;
      case 4096: gsl_power2_fft4096synthesis_skip2 (NULL, r_values_out); break;
      case 8192: gsl_power2_fft8192synthesis_skip2 (NULL, r_values_out); break;
      default:	 gsl_power2_fftc_big (n_values, NULL, r_values_out, -1);
    }
}
