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

#ifndef DATAPACKET_H
#define DATAPACKET_H

#include "buffer.h"

/*
 * BC - Status (2000-09-30): GenericDataChannel, DataPacket types
 *
 * These classes must be kept binary compatible, as the do interact with
 * generated code. So you MUST KNOW WHAT YOU ARE DOING, once you start
 * using the provided d pointers for extensions.
 */

namespace Arts {
class GenericDataChannelPrivate;
class GenericDataPacket;
/*
 * The GenericDataChannel interface is to be implemented by the flowsystem
 */
class GenericDataChannel {
private:
	GenericDataChannelPrivate *d;	// unused

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

public:
	/*
	 * used to set pull delivery mode
	 */
	virtual void setPull(int packets, int capacity) = 0;
	virtual void endPull() = 0;

	GenericDataChannel() : d(0)
	{
	}
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
class GenericDataPacketPrivate;

class GenericDataPacket {
private:
	GenericDataPacketPrivate *d;
	static long _staticDataPacketCount;

public:
	/**
	 * the amount of active data packets (memory leak debugging only)
	 */
	static long _dataPacketCount() { return _staticDataPacketCount; }

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
		if(useCount == 0)
		{
			if(channel)
				channel->processedPacket(this);
			else
				delete this;
		}
	}

	virtual ~GenericDataPacket()
	{
		_staticDataPacketCount--;
	}

protected:
	GenericDataPacket(GenericDataChannel *channel)
		:d(0),channel(channel),useCount(0)
	{
		_staticDataPacketCount++;
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
	DataPacket(GenericDataChannel *channel)
	    : GenericDataPacket(channel) {}
	~DataPacket() {}
};

/**
 * The RawDataPacket<T> interface handles raw class T arrays of data
 */
template<class T>
class RawDataPacket : public DataPacket<T> {
protected:
	int capacity;
	void ensureCapacity(int newCapacity)
	{
		if(newCapacity > capacity)
		{
			delete[] contents;
			capacity = newCapacity;
			contents = new T[capacity];
		}
	}
	RawDataPacket(int capacity, GenericDataChannel *channel)
		:DataPacket<T>(channel), capacity(capacity)
	{
		size = capacity;
		contents = new T[capacity];
	}
	~RawDataPacket()
	{
		delete[] contents;
	}
};

/**
 * FloatDataPacket finally is one concrete DataPacket (which contains the
 * information how to marshal a datapacket of type float)
 */
class FloatDataPacket : public RawDataPacket<float> {
public:
	FloatDataPacket(int capacity, GenericDataChannel *channel)
			: RawDataPacket<float>(capacity, channel)
	{
		//
	}
	void read(Buffer& stream);
	void write(Buffer& stream);
};

class ByteDataPacket : public RawDataPacket<mcopbyte> {
public:
	ByteDataPacket(int capacity, GenericDataChannel *channel)
			: RawDataPacket<mcopbyte>(capacity, channel)
	{
		//
	}
	void read(Buffer& stream);
	void write(Buffer& stream);
};

};
#endif
