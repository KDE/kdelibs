#include "asyncstream.h"
#include "datapacket.h"

DataPacket<float> *FloatAsyncStream::allocPacket(int capacity)
{
	return new FloatDataPacket(capacity,channel);
}

GenericAsyncStream *FloatAsyncStream::createNewStream()
{
	return new FloatAsyncStream();
}

DataPacket<mcopbyte> *ByteAsyncStream::allocPacket(int capacity)
{
	return new ByteDataPacket(capacity,channel);
}

GenericAsyncStream *ByteAsyncStream::createNewStream()
{
	return new ByteAsyncStream();
}
