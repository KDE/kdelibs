#ifndef ASYNCSTREAM_H
#define ASYNCSTREAM_H

#include "buffer.h"
#include "datapacket.h"

class GenericAsyncStream {
public:
	/**
	 * interface to create packets and to get rid of them
	 */
	virtual GenericDataPacket *createPacket(int capacity) = 0;
	virtual void freePacket(GenericDataPacket *packet) = 0;

	virtual GenericAsyncStream *createNewStream() = 0;

	GenericDataChannel *channel;
	int _notifyID;

	inline int notifyID() { return _notifyID; }
};

template<class T>
class AsyncStream : public GenericAsyncStream {
protected:
	GenericDataPacket *createPacket(int capacity)
	{
		return allocPacket(capacity);
	}
	void freePacket(GenericDataPacket *packet)
	{
		delete packet;
	}
public:
	// for outgoing streams
	virtual DataPacket<T> *allocPacket(int capacity) = 0;

	inline void setPull(int packets, int capacity)
	{
		channel->setPull(packets,capacity);
	}
	inline void endPull()
	{
		channel->endPull();
	}
};

class FloatAsyncStream : public AsyncStream<float>
{
public:
	DataPacket<float> *allocPacket(int capacity);
	GenericAsyncStream *createNewStream();
};

class ByteAsyncStream : public AsyncStream<mcopbyte>
{
public:
	DataPacket<mcopbyte> *allocPacket(int capacity);
	GenericAsyncStream *createNewStream();
};

#endif /* ASYNCSTREAM_H */
