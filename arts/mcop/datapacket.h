#ifndef DATAPACKET_H
#define DATAPACKET_H

#include "buffer.h"

/*
 * The GenericDataChannel interface is to be implemented by the flowsystem
 */
class GenericDataChannel {
protected:
	friend class GenericDataPacket;

	/*
	 * this is used internally by DataPacket
	 */
	virtual void processedPacket(GenericDataPacket *packet) = 0;

	/*
	 * used internally by DataPacket
	 */
	virtual void sendPacket(GenericDataPacket *packet) = 0;
};

/*
 * DataPackets are the heard of asynchronous streaming (MCOP has synchronous
 * and asynchronous streams). They are used
 *
 *  - in the interface async streams expose to C++ implementations of MCOP
 *    interfaces (they directly deal with datapackets)
 *
 *  - from the FlowSystem implemenentations
 */

/**
 * The GenericDataPacket class provides the interface the flow system can
 * use to deal with data packets.
 */
class GenericDataPacket {
protected:
	/**
	 * the channel this datapacket belongs to
	 */
	GenericDataChannel *channel;

	/**
	 * ensureCapactity ensures that there is room for at least capacity
	 * Elements in the packet. This is implemented destructive - that
	 * means: you may not find your old contents in the packet after
	 * calling ensureCapacity
	 */
	virtual void ensureCapacity(int capacity) = 0;

	/**
	 * read/write write the contents of the packet. Read will also
	 * automatically ensure that capacity is adapted before reading.
	 */
	virtual void read(Buffer& stream) = 0;
	virtual void write(Buffer& stream) = 0;

	GenericDataPacket(GenericDataChannel *channel) : channel(channel)
	{
	}

public:
	/** 
	 * having size here (and not in the derived concrete DataPackets) is so
	 * that we can see whether the sender can't supply more data (and starts
	 * sending zero size packets
	 */
	int size;

	/**
	 * useCount is to be set from sendPacket
	 */
	int useCount;

	inline void send()
	{
		channel->sendPacket(this);
	}
	inline void processed()
	{
		useCount--;
		if(useCount == 0) channel->processedPacket(this);
	}
};

/**
 * The DataPacket<T> interface is what C++ implementations of MCOP interfaces
 * will need to use.
 */
template<class T>
class DataPacket : public GenericDataPacket {
public:
	T *contents;

protected:
	int capacity;
	void ensureCapacity(int capacity);

	DataPacket<T>(int capacity, GenericDataChannel *channel)
		:GenericDataPacket(channel), capacity(capacity)
	{
		size = capacity;
		contents = new T[capacity];
	}
};

/**
 * FloatDataPacket finally is one concrete DataPacket (which contains the
 * information how to marshal a datapacket of type float)
 */
class FloatDataPacket : public DataPacket<float> {
public:
	FloatDataPacket(int capacity, GenericDataChannel *channel)
			: DataPacket<float>(capacity, channel)
	{
		//
	}
	void read(Buffer& stream);
	void write(Buffer& stream);
};

class ByteDataPacket : public DataPacket<mcopbyte> {
public:
	ByteDataPacket(int capacity, GenericDataChannel *channel)
			: DataPacket<mcopbyte>(capacity, channel)
	{
		//
	}
	void read(Buffer& stream);
	void write(Buffer& stream);
};


#endif
