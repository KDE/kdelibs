    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

/*
 * Conversion routines. The function names are choosen like
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
 */

// upconversions from integer to float
void convert_mono_8_float(unsigned long samples,
			unsigned char *from, float *to);

void interpolate_mono_8_float(unsigned long samples,
		    float startpos, float speed,
			unsigned char *from, float *to);

void convert_mono_16le_float(unsigned long samples,
			unsigned char *from, float *to);

void interpolate_mono_16le_float(unsigned long samples,
		    float startpos, float speed,
			unsigned char *from, float *to);

void convert_stereo_i8_2float(unsigned long samples,
			unsigned char *from, float *left, float *right);

void interpolate_stereo_i8_2float(unsigned long samples,
		    float startpos, float speed,
			unsigned char *from, float *left, float *right);

void convert_stereo_i16le_2float(unsigned long samples,
			unsigned char *from, float *left, float *right);

void interpolate_stereo_i16le_2float(unsigned long samples,
		    float startpos, float speed,
			unsigned char *from, float *left, float *right);

// downconversions from float to integer
void convert_mono_float_16le(unsigned long samples,
			float *from, unsigned char *to);

void convert_stereo_2float_i16le(unsigned long samples,
			float *left, float *right, unsigned char *to);
