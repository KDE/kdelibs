#include "asyncstream.h"
#include "datapacket.h"

DataPacket<float> *FloatAsyncStream::allocPacket(int capacity)
{
	return new FloatDataPacket(capacity,channel);
}

DataPacket<mcopbyte> *ByteAsyncStream::allocPacket(int capacity)
{
	return new ByteDataPacket(capacity,channel);
}
