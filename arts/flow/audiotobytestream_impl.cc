    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de
				  2001 Matthias Kretz
				       kretz@kde.org

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

#include "artsflow.h"
#include "stdsynthmodule.h"
#include "debug.h"

#include <vector>
#include <iostream>
#include <math.h>

using namespace std;
using namespace Arts;

namespace Arts {

class AudioToByteStream_impl : public AudioToByteStream_skel,
                               public StdSynthModule
{
	long _samplingRate, _channels, _bits;
	long sampleSize;
	double step;
	bool interpolate;
	vector<float> leftbuffer;
	vector<float> rightbuffer;
	int range;
	double _pos;
protected:
	void updateSampleSize()
	{
		sampleSize = _channels * _bits / 8;
	}

public:
	AudioToByteStream_impl() :
		_pos(0)
	{
		samplingRate(44100);
		channels(2);
		bits(16);
	}

	long samplingRate() { return _samplingRate; }
	void samplingRate(long newRate) {
		double newStep = samplingRateFloat / (float)newRate;
		arts_return_if_fail(newStep > 0);
		_samplingRate = newRate;
		step = newStep;

		double delta = step - floor(step);
		interpolate = fabs(delta) > 0.001;
	}

	long channels() { return _channels; }
	void channels(long newChannels) {
		arts_return_if_fail(newChannels == 1 || newChannels == 2);
		_channels = newChannels;
		updateSampleSize();
	}

	long bits() { return _bits; }
	void bits(long newBits) {
		arts_return_if_fail(newBits == 8 || newBits == 16);
		_bits = newBits;
		range = (newBits == 8 ) ? 128 : 32768;
		updateSampleSize();
	}

	void calculateBlock(unsigned long samples)
	{
		leftbuffer.resize( 1 + samples );
		rightbuffer.resize( 1 + samples );
		for( unsigned long i = 0; i < samples; ++i ) {
			leftbuffer[1 + i] = (left[i] > 1) ? 1 : (left[i] < -1) ? -1 : left[i];
			rightbuffer[1 + i] = (right[i] > 1) ? 1 : (right[i] < -1) ? -1 : right[i];
		}

		int packetstosend = (int)ceil(leftbuffer.size() / step);
		DataPacket<mcopbyte> *packet = outdata.allocPacket( packetstosend * sampleSize );

		int processed = 0;
		if( interpolate ) {
			double pos = 0;
			if( _channels == 2 ) {
				if( _bits == 16 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						double error = modf(_pos, &pos);
						int intpos = (int)pos;
						long leftchannel = long((leftbuffer[intpos] * error + leftbuffer[intpos + 1] * (1 - error)) * 32768) + 32768;
						long rightchannel = long((rightbuffer[intpos] * error + rightbuffer[intpos + 1] * (1 - error)) * 32768) + 32768;
						packet->contents[processed * 4    ] = leftchannel;
						packet->contents[processed * 4 + 1] = (leftchannel >> 8) - 128;
						packet->contents[processed * 4 + 2] = rightchannel;
						packet->contents[processed * 4 + 3] = (rightchannel >> 8) - 128;
						_pos += step;
						++processed;
					}
				} else if( _bits == 8 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						double error = modf(_pos, &pos);
						int intpos = (int)pos;
						long leftchannel = long((leftbuffer[intpos] * error + leftbuffer[intpos + 1] * (1 - error)) * 128) + 128;
						long rightchannel = long((rightbuffer[intpos] * error + rightbuffer[intpos + 1] * (1 - error)) * 128) + 128;
						packet->contents[processed * 2    ] = leftchannel;
						packet->contents[processed * 2 + 1] = rightchannel;
						_pos += step;
						++processed;
					}
				}
			} else {
				if( _bits == 16 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						double error = modf(_pos, &pos);
						int intpos = (int)pos;
						long leftchannel = long(((leftbuffer[intpos] + rightbuffer[intpos]) * error + (leftbuffer[intpos + 1] + rightbuffer[intpos + 1]) * (1 - error)) * 16384) + 32768;
						packet->contents[processed * 2    ] = leftchannel;
						packet->contents[processed * 2 + 1] = (leftchannel >> 8) - 128;
						_pos += step;
						++processed;
					}
				} else if( _bits == 8 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						double error = modf(_pos, &pos);
						int intpos = (int)pos;
						long leftchannel = long(((leftbuffer[intpos] + rightbuffer[intpos]) * error + (leftbuffer[intpos + 1] + rightbuffer[intpos + 1]) * (1 - error)) * 64) + 128;
						packet->contents[processed] = leftchannel;
						_pos += step;
						++processed;
					}
				}
			}
		} else {
			if( _channels == 2 ) {
				if( _bits == 16 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						int intpos = (int)_pos;
						long leftchannel = long(leftbuffer[intpos] * 32768) + 32768;
						long rightchannel = long(rightbuffer[intpos] * 32768) + 32768;
						packet->contents[processed * 4    ] = leftchannel;
						packet->contents[processed * 4 + 1] = (leftchannel >> 8) - 128;
						packet->contents[processed * 4 + 2] = rightchannel;
						packet->contents[processed * 4 + 3] = (rightchannel >> 8) - 128;
						_pos += step;
						++processed;
					}
				} else if( _bits == 8 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						int intpos = (int)_pos;
						long leftchannel = long(leftbuffer[intpos] * 128) + 128;
						long rightchannel = long(rightbuffer[intpos] * 128) + 128;
						packet->contents[processed * 2    ] = leftchannel;
						packet->contents[processed * 2 + 1] = rightchannel;
						_pos += step;
						++processed;
					}
				}
			} else {
				if( _bits == 16 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						int intpos = (int)_pos;
						long leftchannel = long((leftbuffer[intpos] + rightbuffer[intpos]) * 16384) + 32768;
						packet->contents[processed * 2    ] = leftchannel;
						packet->contents[processed * 2 + 1] = (leftchannel >> 8) - 128;
						_pos += step;
						++processed;
					}
				} else if( _bits == 8 ) {
					while( _pos < (double)leftbuffer.size() - 1 ) {
						int intpos = (int)_pos;
						long leftchannel = long((leftbuffer[intpos] + rightbuffer[intpos]) * 64) + 128;
						packet->contents[processed] = leftchannel;
						_pos += step;
						++processed;
					}
				}
			}
		}
		leftbuffer[0] = leftbuffer.back();
		rightbuffer[0] = rightbuffer.back();
		_pos = _pos - floor(_pos);
		packet->size = processed * sampleSize;
		packet->send();
	}
};

REGISTER_IMPLEMENTATION(AudioToByteStream_impl);

};
