#include "datapacket.h"
#include "buffer.h"

/**** DataPacket<T> ****/

template<class T>
void DataPacket<T>::ensureCapacity(int newCapacity)
{
	if(newCapacity > capacity)
	{
		delete contents;
		capacity = newCapacity;
		contents = new T[capacity];
	}
}

/**** specific DataPacket types ****/

void FloatDataPacket::read(Buffer& stream)
{
	size = stream.readLong();
	ensureCapacity(size);
	for(int i=0;i<size;i++) contents[i] = stream.readFloat();
}

void FloatDataPacket::write(Buffer& stream)
{
	stream.writeLong(size);
	for(int i=0;i<size;i++) stream.writeFloat(contents[i]);
}

void ByteDataPacket::read(Buffer& stream)
{
	size = stream.readLong();
	ensureCapacity(size);
	for(int i=0;i<size;i++) contents[i] = stream.readByte();
}

void ByteDataPacket::write(Buffer& stream)
{
	stream.writeLong(size);
	for(int i=0;i<size;i++) stream.writeByte(contents[i]);
}
