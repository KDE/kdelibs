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


#ifndef ARTS_FLOW_RESAMPLE_H
#define ARTS_FLOW_RESAMPLE_H

/*
 * BC - Status (2000-09-30): Resampler / Refiller.
 *
 * These classes will be kept binary compatible. Resampler has a private
 * data pointer for this purpose.
 */

namespace Arts {

class Refiller {
public:
	virtual unsigned long read(unsigned char *buffer, unsigned long len) = 0;
	virtual ~Refiller();
};

class ResamplerPrivate;

class Resampler {
protected:
	static const unsigned int bufferSize = 256;		//  64 samples in buffer
	static const unsigned int bufferWrap = 4;		// + 1 sample "wrap around"

	int bufferSamples;
	int sampleSize;
	int dropBytes;

	Refiller *refiller;
	double pos, step;
	int channels,bits;

	unsigned char buffer[bufferSize+bufferWrap];
	float fbuffer[bufferSize+bufferWrap];
	long block, haveBlock;

	class ResamplerPrivate *d;

	void updateSampleSize();
	void ensureRefill();
public:
	Resampler(Refiller *refiller);
	~Resampler();

	void setStep(double step);
	void setChannels(int channels);
	void setBits(int bits);

	enum Endianness { bigEndian, littleEndian };
	void setEndianness(Endianness endianness);

	void run(float *left, float *right, unsigned long samples);

	bool underrun();
};

};

#endif /* ARTS_FLOW_REFILL_H */
