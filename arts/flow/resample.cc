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

#include "resample.h"
#include "debug.h"
#include <iostream.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

#define compose_16le(first,second) \
    (((((second)+128)&0xff) << 8)+(first))
 
#define compose_16be(first,second) \
    (((((first)+128)&0xff) << 8)+(second))

#define conv_16_float(x) \
    ((float)((x)-32768)/32768.0)                                                

#define conv_8_float(x) \
	((float)((x)-128)/128.0)

using namespace Arts;

class Arts::ResamplerPrivate {
public:
	bool underrun;	
	Resampler::Endianness endianness;
};

const unsigned int Resampler::bufferSize;
const unsigned int Resampler::bufferWrap;

Resampler::Resampler(Refiller *refiller) :
	dropBytes(0), refiller(refiller), pos(0.0), step(1.0), channels(2),
	bits(16),
	block(0), haveBlock(-1)
{
	d = new ResamplerPrivate();
	d->underrun = false;
	d->endianness = littleEndian;
	updateSampleSize();
}

Resampler::~Resampler()
{
	delete d;
}

void Resampler::updateSampleSize()
{
	sampleSize = channels * bits / 8;
	bufferSamples = bufferSize / sampleSize;
}

void Resampler::setStep(double newStep)
{
	arts_return_if_fail(newStep > 0);

	step = newStep;
}

void Resampler::setChannels(int newChannels)
{
	arts_return_if_fail(newChannels == 1 || newChannels == 2);

	channels = newChannels;
	updateSampleSize();
}

void Resampler::setBits(int newBits)
{
	arts_return_if_fail(newBits == 8 || newBits == 16);

	bits = newBits;
	updateSampleSize();
}

void Resampler::setEndianness(Endianness newEndianness)
{
	arts_return_if_fail(newEndianness == bigEndian || newEndianness == littleEndian);

	d->endianness = newEndianness;
}

bool Resampler::underrun()
{
	return d->underrun;
}

void Resampler::ensureRefill()
{
	if(haveBlock == block) return;

	unsigned long missing;
	if(block == 0)
	{
		missing = bufferSize+sampleSize
				- refiller->read(buffer,bufferSize+sampleSize);

		d->underrun = (missing == bufferSize+sampleSize);
	}
	else
	{
		/*
		 * try to drop away "half-sample" reads from the last refill
		 */
		if(dropBytes > 0)
			dropBytes -= refiller->read(buffer,dropBytes);

		/*
		 * only if this worked there is hope that we can read sane data
		 */
		if(dropBytes == 0)
		{
			missing = bufferSize
					- refiller->read(&buffer[sampleSize], bufferSize);

			d->underrun = (missing == bufferSize);
		}
		else
		{
			missing = bufferSize;
			d->underrun = true;
		}
	}
	haveBlock++;
	assert(haveBlock == block);

	/*
	 * If we don't have enough input to fill the block fully, it might be
	 * that the input stall occurred in the middle of a sample. For instance,
	 * if samples are 4 bytes long, it might be that we would have needed
	 * 13 more bytes to do a full refill.
	 *
	 * In this situation, there are four samples and one byte missing to
	 * refill the buffer - the one byte is what we need to care about here:
	 * on the next read, we'll have one byte too much (if we simply ignore
	 * the fact, we end up with misaligned reading, causing noise, or
	 * swapped stereo channels or similar).
	 *
	 * So we set dropBytes here, which is a variable which indicates how
	 * many bytes to drop away upon next refill.
	 */
	if(missing & (sampleSize - 1))
		dropBytes = missing & (sampleSize - 1);

	unsigned int i = 0, wrap = (block == 0)?0:sampleSize;
	if(bits == 16)
	{
		// wrap the last part of the buffer back to the beginning (history)
		while(i<wrap)
		{
			fbuffer[i/2] = fbuffer[(bufferSize+i)/2];
			i += 2;
		}

		// convert data from incoming
		if(d->endianness == littleEndian)
		{
			while(i<bufferSize+sampleSize-missing)
			{
				fbuffer[i/2] = conv_16_float(compose_16le(buffer[i],buffer[i+1]));
				i += 2;
			}
		}
		else
		{
			while(i<bufferSize+sampleSize-missing)
			{
				fbuffer[i/2] = conv_16_float(compose_16be(buffer[i],buffer[i+1]));
				i += 2;
			}
		}

		// fill up missing bytes with zero samples
		while(i<bufferSize+sampleSize)
		{
			fbuffer[i/2] = 0.0;
			i += 2;
		}
	}
	else if(bits == 8)
	{
		// wrap the last part of the buffer back to the beginning (history)
		while(i<wrap)
		{
			fbuffer[i] = fbuffer[bufferSize+i];
			i++;
		}

		// convert data from incoming
		while(i<bufferSize+sampleSize-missing)
		{
			fbuffer[i] = conv_8_float(buffer[i]);
			i++;
		}

		// fill up missing bytes with zero samples
		while(i<bufferSize+sampleSize)
		{
			fbuffer[i++] = 0.0;
		}
	}
	else
	{
		assert(false);
	}
}

#define RESAMPLER_STEP()							\
	pos += step;									\
	i++;											\
	while(pos >= bufferSamples)						\
	{												\
		pos -= bufferSamples;						\
		block++;									\
		ensureRefill();								\
	}

void Resampler::run(float *left, float *right, unsigned long samples)
{
	ensureRefill();
	unsigned long i = 0;
	double delta = step - floor(step);
	bool interpolate = fabs(delta) > 0.001;

	if(channels == 2 && interpolate)
	{
		while(i < samples)
		{
			double error = pos - floor(pos);
			unsigned long offset = 2*(unsigned long)pos;

			left[i]  = fbuffer[offset+0]*(1.0-error)+fbuffer[offset+2]*error;
			right[i] = fbuffer[offset+1]*(1.0-error)+fbuffer[offset+3]*error;
			RESAMPLER_STEP();
		}
	}
	else if(channels == 1 && interpolate)
	{
		while(i < samples)
		{
			double error = pos - floor(pos);
			unsigned long offset = (unsigned long)pos;

			left[i] = right[i] = fbuffer[offset]*(1.0-error)
							   + fbuffer[offset+1]*error;
			RESAMPLER_STEP();
		}
	}
	else if(channels == 2)
	{
		while(i < samples)
		{
			unsigned long offset = 2*(unsigned long)pos;

			left[i]  = fbuffer[offset+0];
			right[i] = fbuffer[offset+1];
			RESAMPLER_STEP();
		}
	}
	else if(channels == 1)
	{
		while(i < samples)
		{
			unsigned long offset = (unsigned long)pos;

			left[i] = right[i] = fbuffer[offset];
			RESAMPLER_STEP();
		}
	}
	else
	{
		assert(false);
	}
}

Refiller::~Refiller()
{
}

#undef RESAMPLER_STEP
#undef compose_16le
#undef compose_16be
#undef conv_16_float
#undef conv_8_float
