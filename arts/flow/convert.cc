    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "convert.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include <config.h>

#ifdef HAVE_X86_FLOAT_INT
static inline long QRound (float inval)
{
#if (__GNUC__ == 3 && __GNUC_MINOR__ == 0)
  volatile
#warning "workaround for gcc-3.0 bug c++/2733 enabled"
#endif
  long ret;
  asm ("fistpl %0" : "=m" (ret) : "t" (inval) : "st");
  return ret;
}
#else
static inline long QRound (float inval)
{
  return (long)inval;
}
#endif

/*---------------------------- new code begin -------------------------- */

#define compose_16le(ptr) \
	((((*((ptr)+1)+128)&0xff) << 8)+*(ptr))

#define compose_16be(ptr) \
	((((*(ptr)+128)&0xff) << 8)+*((ptr)+1))

#define conv_16_float(x) \
	((float)((x)-32768)/32768.0)

#define convert_16le_float(x) conv_16_float(compose_16le(&(x)))
#define convert_16be_float(x) conv_16_float(compose_16be(&(x)))

#define conv_8_float(x) \
	((float)((x)-128)/128.0)

#define convert_8_float(x) \
	((float)((x)-128)/128.0)

#define convert_float_float(x) x

/*
 * 16le, 16be, 8, float
 */

#define datatype_16le unsigned char
#define datasize_16le 2

#define datatype_16be unsigned char
#define datasize_16be 2

#define datatype_8 unsigned char
#define datasize_8 1

#define datatype_float float
#define datasize_float 1

#define mk_converter(from_format,to_format) \
void Arts::convert_mono_ ## from_format ## _ ## to_format (unsigned long samples, \
									datatype_ ## from_format *from, \
                                    datatype_ ## to_format *to) \
{ \
	datatype_ ## to_format *end = to+samples * (datasize_ ## to_format); \
	while(to<end) { \
		*to = convert_ ## from_format ## _ ## to_format(*from); \
		from += datasize_ ## from_format; \
		to += datasize_ ## to_format; \
	} \
} \
void Arts::interpolate_mono_ ## from_format ## _ ## to_format (unsigned long samples,\
									double startpos, double speed, \
									datatype_ ## from_format *from, \
                                    datatype_ ## to_format *to) \
{ \
	double flpos = startpos; \
	while(samples) { \
		long position = ((long)(flpos)) * (datasize_ ## from_format); \
		double error = flpos - floor(flpos); \
		*to =	(convert_ ## from_format ## _ ## to_format(from[position])) * \
				(1.0-error) + (convert_ ## from_format ## _ ## \
				to_format(from[position + datasize_ ## from_format])) * error; \
		to += datasize_ ## to_format; \
		flpos += speed; \
		samples--; \
	} \
} \
void Arts::convert_stereo_i ## from_format ## _2 ## to_format (unsigned long samples,\
									datatype_ ## from_format *from, \
                                    datatype_ ## to_format *left, \
									datatype_ ## to_format *right) \
{ \
	datatype_ ## to_format *end = left+samples * (datasize_ ## to_format); \
	while(left<end) { \
		*left = convert_ ## from_format ## _ ## to_format(*from); \
		from += datasize_ ## from_format; \
		left += datasize_ ## to_format; \
		*right = convert_ ## from_format ## _ ## to_format(*from); \
		from += datasize_ ## from_format; \
		right += datasize_ ## to_format; \
	} \
} \
void Arts::interpolate_stereo_i ## from_format ## _2 ## to_format (unsigned long samples,\
									double startpos, double speed, \
									datatype_ ## from_format *from, \
                                    datatype_ ## to_format *left, \
                                    datatype_ ## to_format *right) \
{ \
	double flpos = startpos; \
	while(samples) { \
		long position = ((long)(flpos)) * (datasize_ ## from_format) * 2; \
		double error = flpos - floor(flpos); \
		*left =	(convert_ ## from_format ## _ ## to_format(from[position])) * \
				(1.0-error) + (convert_ ## from_format ## _ ## \
				to_format(from[position + 2*datasize_ ## from_format]))*error; \
		left += datasize_ ## to_format; \
		position += datasize_ ## from_format; \
		*right =(convert_ ## from_format ## _ ## to_format(from[position])) * \
				(1.0-error) + (convert_ ## from_format ## _ ## \
				to_format(from[position + 2*datasize_ ## from_format]))*error; \
		right += datasize_ ## to_format; \
		flpos += speed; \
		samples--; \
	} \
}

mk_converter(8,float)
mk_converter(16le,float)
mk_converter(16be,float)
mk_converter(float,float)

/*----------------------------- new code end --------------------------- */

/*
void old_convert_mono_8_float(unsigned long samples, unsigned char *from, float *to)
{
	float *end = to+samples;

	while(to<end) *to++ = conv_8_float(*from++);
}

void old_convert_mono_16le_float(unsigned long samples, unsigned char *from, float *to)
{
	float *end = to+samples;

	while(to<end)
	{
		*to++ = conv_16le_float(compose_16le(from));
		from += 2;
	}
}


void old_convert_stereo_i8_2float(unsigned long samples, unsigned char *from, float *left, float *right)
{
	float *end = left+samples;
	while(left < end)
	{
		*left++ = conv_8_float(*from++);
		*right++ = conv_8_float(*from++);
	}
}

void old_convert_stereo_i16le_2float(unsigned long samples, unsigned char *from, float *left, float *right)
{
	float *end = left+samples;
	while(left < end)
	{
		*left++ = conv_16le_float(compose_16le(from));
		*right++ = conv_16le_float(compose_16le(from+2));
		from += 4;
	}
}
*/

void Arts::convert_stereo_2float_i16le(unsigned long samples, float *left, float *right, unsigned char *to)
{
	float *end = left+samples;
	long syn;

	while(left < end)
	{
		syn = QRound((*left++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = syn & 0xff;
		*to++ = (syn >> 8) & 0xff;

		syn = QRound((*right++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = syn & 0xff;
		*to++ = (syn >> 8) & 0xff;
	}	
}

void Arts::convert_mono_float_16le(unsigned long samples, float *from, unsigned char *to)
{
	float *end = from+samples;

	while(from < end)
	{
		long syn = QRound((*from++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = syn & 0xff;
		*to++ = (syn >> 8) & 0xff;
	}	
}

void Arts::convert_stereo_2float_i16be(unsigned long samples, float *left, float *right, unsigned char *to)
{
	float *end = left+samples;
	long syn;

	while(left < end)
	{
		syn = QRound((*left++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = (syn >> 8) & 0xff;
		*to++ = syn & 0xff;

		syn = QRound((*right++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = (syn >> 8) & 0xff;
		*to++ = syn & 0xff;
	}	
}

void Arts::convert_mono_float_16be(unsigned long samples, float *from, unsigned char *to)
{
	float *end = from+samples;

	while(from < end)
	{
		long syn = QRound((*from++)*32767);

		if(syn < -32768) syn = -32768;				/* clipping */
		if(syn > 32767) syn = 32767;

		*to++ = (syn >> 8) & 0xff;
		*to++ = syn & 0xff;
	}	
}

void Arts::convert_stereo_2float_i8(unsigned long samples, float *left, float *right, unsigned char *to)
{
	float *end = left+samples;
	long syn;

	while(left < end)
	{
		syn = (int)((*left++)*127+128);

		if(syn < 0) syn = 0;					/* clipping */
		if(syn > 255) syn = 255;

		*to++ = syn;

		syn = (int)((*right++)*127+128);

		if(syn < 0) syn = 0;					/* clipping */
		if(syn > 255) syn = 255;

		*to++ = syn;
	}	
}

void Arts::convert_mono_float_8(unsigned long samples, float *from, unsigned char *to)
{
	float *end = from+samples;

	while(from < end)
	{
		long syn = (int)((*from++)*127+128);

		if(syn < 0) syn = 0;					/* clipping */
		if(syn > 255) syn = 255;

		*to++ = syn;
	}	
}

unsigned long Arts::uni_convert_stereo_2float(
		unsigned long samples,		// number of required samples
		unsigned char *from,		// buffer containing the samples
		unsigned long fromLen,		// length of the buffer
	    unsigned int fromChannels,  // channels stored in the buffer
		unsigned int fromBits,		// number of bits per sample
	    float *left, float *right,	// output buffers for left and right channel
		double speed,				// speed (2.0 means twice as fast)
		double startposition		// startposition
	)
{
	unsigned long doSamples = 0, sampleSize = fromBits/8;

	if(fromBits == uni_convert_float_ne)
		sampleSize = sizeof(float);

	// how many samples does the from-buffer contain?
	double allSamples = fromLen / (fromChannels * sampleSize);

	// how many samples are remaining?
	//    subtract one due to interpolation and another against rounding errors
	double fHaveSamples = allSamples - startposition - 2.0;
	fHaveSamples /= speed;

	// convert do "how many samples to do"?
	if(fHaveSamples > 0)
	{
		doSamples = (int)fHaveSamples;
		if(doSamples > samples) doSamples = samples;
	}

	// do conversion
	if(doSamples)
	{
		if(fromChannels == 1)
		{
			if(fromBits == uni_convert_float_ne) {
				interpolate_mono_float_float(doSamples,
							startposition,speed,(float *)from,left);
			}
			else if(fromBits == uni_convert_s16_be) {
				interpolate_mono_16be_float(doSamples,
							startposition,speed,from,left);
			}
			else if(fromBits == uni_convert_s16_le) {
				interpolate_mono_16le_float(doSamples,
							startposition,speed,from,left);
			}
			else {
				interpolate_mono_8_float(doSamples,
							startposition,speed,from,left);
			}
			memcpy(right,left,sizeof(float)*doSamples);
		}
		else if(fromChannels == 2)
		{
			if(fromBits == uni_convert_float_ne) {
				interpolate_stereo_ifloat_2float(doSamples,
							startposition,speed,(float *)from,left,right);
			}
			else if(fromBits == uni_convert_s16_be) {
				interpolate_stereo_i16be_2float(doSamples,
							startposition,speed,from,left,right);
			}
			else if(fromBits == uni_convert_s16_le) {
				interpolate_stereo_i16le_2float(doSamples,
							startposition,speed,from,left,right);
			}
			else {
				interpolate_stereo_i8_2float(doSamples,
							startposition,speed,from,left,right);
			}
		} else {
			assert(false);
		}
	}
	return doSamples;
}

// undefine all that stuff (due to --enable-final)
#undef compose_16le
#undef compose_16be
#undef conv_16_float
#undef convert_16le_float
#undef convert_16be_float
#undef conv_8_float
#undef convert_8_float
#undef convert_float_float
#undef datatype_16le
#undef datasize_16le
#undef datatype_16be
#undef datasize_16be
#undef datatype_8
#undef datasize_8
#undef datatype_float
#undef datasize_float
#undef mk_converter
