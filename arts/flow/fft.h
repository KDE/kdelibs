#ifndef FFT_H
#define FFT_H

/*
 * BC - Status (2000-09-30): fft_float
 *
 * Do not rely on this - it is part of StereoFFTScope implementation, and
 * subject to move with it.
 */


/*
 * I put all that fft stuff together in fft.c and fft.h. It only exports
 * the fft_float function now, all others are static.
 */

/* this is from ddcmath.h */

#define  DDC_PI  (3.14159265358979323846)

/*============================================================================

       fourier.h  -  Don Cross <dcross@intersrv.com>

       http://www.intersrv.com/~dcross/fft.html

       Contains definitions for doing Fourier transforms
       and inverse Fourier transforms.

============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*
**   fft() computes the Fourier transform or inverse transform
**   of the complex inputs to produce the complex outputs.
**   The number of samples must be a power of two to do the
**   recursive decomposition of the FFT algorithm.
**   See Chapter 12 of "Numerical Recipes in FORTRAN" by
**   Press, Teukolsky, Vetterling, and Flannery,
**   Cambridge University Press.
**
**   Notes:  If you pass ImaginaryIn = NULL, this function will "pretend"
**           that it is an array of all zeroes.  This is convenient for
**           transforming digital samples of real number data without
**           wasting memory.
*/

void fft_float (
    unsigned  NumSamples,          /* must be a power of 2 */
    int       InverseTransform,    /* 0=forward FFT, 1=inverse FFT */
    float    *RealIn,              /* array of input's real samples */
    float    *ImaginaryIn,         /* array of input's imag samples */
    float    *RealOut,             /* array of output's reals */
    float    *ImaginaryOut );      /* array of output's imaginaries */


/*
int IsPowerOfTwo ( unsigned x );
unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo );
unsigned ReverseBits ( unsigned index, unsigned NumBits );
*/

/*
**   The following function returns an "abstract frequency" of a
**   given index into a buffer with a given number of frequency samples.
**   Multiply return value by sampling rate to get frequency expressed in Hz.
*/
/*
double Index_to_frequency ( unsigned NumSamples, unsigned Index );
*/

#ifdef __cplusplus
}
#endif
#endif /* FFT_H */
