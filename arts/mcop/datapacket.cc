#include "datapacket.h"
#include "buffer.h"

/**** DataPacket<T> ****/

template<class T>
DataPacket<T>::~DataPacket()
{
	delete contents;
}

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

	// we know that bytes are marshalled as bytes,
	// so we can read them as block
	unsigned char *buffer = (unsigned char *)stream.read(size);
	if(buffer)
		memcpy(contents,buffer,size);
}

void ByteDataPacket::write(Buffer& stream)
{
	// we know that bytes are marshalled as bytes,
	// so we can write them as block

	stream.writeLong(size);
	stream.write(contents,size);
}
