#ifndef ASYNCSTREAM_H
#define ASYNCSTREAM_H

#include "buffer.h"
#include "datapacket.h"

class GenericAsyncStream {
protected:
	/**
	 * interface to create packets and to get rid of them
	 */
	virtual GenericDataPacket *createPacket(int capacity) = 0;
	virtual void freePacket(GenericDataPacket *packet) = 0;

public:
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
	//virtual void setPull(int packets, int capacity) = 0;
};

class FloatAsyncStream : public AsyncStream<float>
{
public:
	DataPacket<float> *allocPacket(int capacity);
};

class ByteAsyncStream : public AsyncStream<mcopbyte>
{
public:
	DataPacket<mcopbyte> *allocPacket(int capacity);
};

#endif /* ASYNCSTREAM_H */
