#include "artsflow.h"
#include "stdsynthmodule.h"

using namespace std;

#define compose_16le(first,second) \
	(((((second)+128)&0xff) << 8)+(first))

#define conv_16le_float(x) \
	((float)((x)-32768)/32768.0)

class ByteStreamToAudio_impl : public ByteStreamToAudio_skel,
                               public StdSynthModule
{
	int haveBytes, pos;
	queue< DataPacket<mcopbyte>* > inqueue;
	long _samplingRate, _channels, _bits;
public:
	ByteStreamToAudio_impl() :haveBytes(0), pos(0),
			_samplingRate(44100), _channels(2), _bits(16)
	{
		//
	}

	long samplingRate() { return _samplingRate; }
	void samplingRate(long newRate) { _samplingRate = newRate; }

	long channels() { return _channels; }
	void channels(long newChannels) { _channels = newChannels; }

	long bits() { return _bits; }
	void bits(long newBits) { _bits = newBits; }

	void process_indata(DataPacket<mcopbyte> *packet)
	{
		haveBytes += packet->size;
		inqueue.push(packet);
	}

	inline mcopbyte getByte()
	{
		mcopbyte result;

		assert(haveBytes);
		DataPacket<mcopbyte> *packet = inqueue.front();
		result = packet->contents[pos++];

		if(pos == packet->size) {
			packet->processed();
			pos = 0;
			inqueue.pop();
		}

		haveBytes--;
		return result;
	}

	inline float mkSample()
	{
		mcopbyte first = getByte();
		mcopbyte second = getByte();
		return conv_16le_float(compose_16le(first,second));
	}
	void calculateBlock(unsigned long samples)
	{
		for(unsigned long i=0;i<samples;i++)
		{
			if(haveBytes >= 4)
			{
				left[i] = mkSample();
				right[i] = mkSample();
			}
			else
			{
				left[i] = right[i] = 0.0;
			}
		}
	}
};

REGISTER_IMPLEMENTATION(ByteStreamToAudio_impl);
