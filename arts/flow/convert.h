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

#ifndef _arts_convert_h
#define _arts_convert_h

/*
 * BC - Status (2000-09-30): conversion functions
 *
 * None of them will be removed or changed, so it is safe to use them in
 * your apps. It is *recommended* (though not necessary) to use the new
 * resampling code (Resampler) where possible.
 */

namespace Arts {

/*
 * Simple conversion routines. The function names are choosen like
 *
 *   convert_mono_FROM_TO for mono conversions
 *   convert_stereo_FROM_TO for stereo conversions
 *
 * while FROM/TO are
 *
 *      8 for  8bit signed data
 *   16le for 16bit signed little endian
 *  float for float data between -1 and 1
 *
 * and may be prefixed by 2 to indicate that stereo is done with two seperate
 * buffers or i to indicate interleaved stereo (one buffer which contains
 * one sample left, one sample right, one sample left etc.)
 *
 * The parameter speed (for interpolations) is *not* the samplingrate, but
 * a relative value. A speed of 2.0 means "play twice as fast".
 */

// upconversions from integer to float
void convert_mono_8_float(unsigned long samples,
			unsigned char *from, float *to);

void interpolate_mono_8_float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *to);

void convert_mono_16le_float(unsigned long samples,
			unsigned char *from, float *to);

void interpolate_mono_16le_float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *to);

void convert_mono_16be_float(unsigned long samples,
			unsigned char *from, float *to);

void interpolate_mono_16be_float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *to);

void convert_stereo_i8_2float(unsigned long samples,
			unsigned char *from, float *left, float *right);

void interpolate_stereo_i8_2float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *left, float *right);

void convert_stereo_i16le_2float(unsigned long samples,
			unsigned char *from, float *left, float *right);

void interpolate_stereo_i16le_2float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *left, float *right);

void convert_stereo_i16be_2float(unsigned long samples,
			unsigned char *from, float *left, float *right);

void interpolate_stereo_i16be_2float(unsigned long samples,
		    double startpos, double speed,
			unsigned char *from, float *left, float *right);

void convert_mono_float_float(unsigned long samples,
			float *from, float *to);

void interpolate_mono_float_float(unsigned long samples,
		    double startpos, double speed,
			float *from, float *to);

void convert_stereo_ifloat_2float(unsigned long samples,
			float *from, float *left, float *right);

void interpolate_stereo_ifloat_2float(unsigned long samples,
		    double startpos, double speed,
			float *from, float *left, float *right);

// downconversions from float to integer
void convert_mono_float_16le(unsigned long samples,
			float *from, unsigned char *to);

void convert_stereo_2float_i16le(unsigned long samples,
			float *left, float *right, unsigned char *to);

void convert_mono_float_16be(unsigned long samples,
			float *from, unsigned char *to);

void convert_stereo_2float_i16be(unsigned long samples,
			float *left, float *right, unsigned char *to);

void convert_mono_float_8(unsigned long samples,
			float *from, unsigned char *to);

void convert_stereo_2float_i8(unsigned long samples,
			float *left, float *right, unsigned char *to);


/*
 * This practical routine picks the right conversion routine for given
 * parameters and makes boundary checks
 */

// returnvalue: number of samples generated

/* formats for unified conversion */
enum {
  uni_convert_u8         = 8,		// unsigned 8 bit
  uni_convert_s16_le     = 16,		// signed 16 bit, little endian
  uni_convert_s16_be     = 17,		// signed 16 bit, big endian
  uni_convert_float_ne   = 0x100	// float, in native size/byteorder
};

unsigned long uni_convert_stereo_2float(
		unsigned long samples,		// number of required samples
		unsigned char *from,		// buffer containing the samples
		unsigned long fromLen,		// length of the buffer
	    unsigned int fromChannels,  // channels stored in the buffer
		unsigned int fromBits,		// number of bits per sample
									//  (also: uni_convert_ formats given above)
	    float *left, float *right,	// output buffers for left and right channel
		double speed,				// speed (2.0 means twice as fast)
		double startposition		// startposition
	);

};

#endif
