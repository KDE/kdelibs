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

#include "buffer.h"
#include "string.h"
#include <assert.h>

using namespace std;
using namespace Arts;

namespace Arts {
	class BufferPrivate {
	public:
		long rpos;
		bool _readError;
		std::vector<unsigned char> contents;

		BufferPrivate() : rpos(0), _readError(false)
		{
			contents.reserve(128);
		}
	};
};

Buffer::Buffer() : d(new BufferPrivate())
{
}

Buffer::~Buffer()
{
	delete d;
}

long Buffer::size()
{
	return d->contents.size();
}

long Buffer::remaining()
{
	return size()-d->rpos;
}

bool Buffer::readError() {
	return d->_readError;
}

void Buffer::writeBool(bool b) {
	d->contents.push_back(b?1:0);
}

void Buffer::writeByte(mcopbyte b) {
	d->contents.push_back(b);
}

void Buffer::writeLong(long l) {
	d->contents.push_back((l >> 24) & 0xff);
	d->contents.push_back((l >> 16) & 0xff);
	d->contents.push_back((l >> 8) & 0xff);
	d->contents.push_back(l & 0xff);
}

void Buffer::writeByteSeq(const vector<mcopbyte>& seq) {
	writeLong(seq.size());

	vector<mcopbyte>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeByte(*i);
}

void Buffer::writeLongSeq(const vector<long>& seq) {
	writeLong(seq.size());

	vector<long>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeLong(*i);
}

void Buffer::writeFloat(float f) {
	// FIXME: on some machines this may fail badly (there is explicit
	// float marshalling and demarshalling code in mico/orb/util.cc)

	long *f_as_long = (long *)&f;
	writeLong(*f_as_long);
}

void Buffer::writeFloatSeq(const std::vector<float>& seq) {
	writeLong(seq.size());

	vector<float>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeFloat(*i);
}

void Buffer::writeString(const string& s) {
	long len = s.size()+1;

	writeLong(len);
	d->contents.insert(d->contents.end(),reinterpret_cast<const unsigned char*>(s.c_str()),
		        reinterpret_cast<const unsigned char*>(s.c_str()+len));
}

void Buffer::writeStringSeq(const vector<string>& seq) {
	writeLong(seq.size());

	vector<string>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeString(*i);
}

void Buffer::write(void *data, long len) {
	unsigned char *c = (unsigned char *)data;

	d->contents.insert(d->contents.end(),c,c+len);
}


void *Buffer::read(long l) {
	void *result = 0;

	if(remaining() >= l) {
		result = &d->contents[d->rpos];
		d->rpos += l;
	} else {
		d->_readError = true;
	}
	return result;
}

void *Buffer::peek(long l) {
	assert(remaining() >= l);
	return &d->contents[d->rpos];
}

void Buffer::skip(long l) {
	if(remaining() >= l) {
		d->rpos += l;
	} else {
		d->_readError = true;
	}
}

void Buffer::rewind() {
	d->rpos = 0;
}

bool Buffer::readBool()
{
	long result = false;
	if(remaining() >= 1) {
		if(d->contents[d->rpos] == 1)
			result = true;
		else
		{
			assert(d->contents[d->rpos] == 0);
		}
		d->rpos += 1;
	} else {
		d->_readError = true;
	}
	return result;
}

mcopbyte Buffer::readByte()
{
	if(remaining() >= 0)
	{
		return d->contents[d->rpos++];
	}
	else
	{
		d->_readError = true;
		return 0;
	}
}

void Buffer::readByteSeq(vector<mcopbyte>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	for(i=0;i<seqlen;i++) result.push_back(readByte());
}

long Buffer::readLong()
{
	long result = 0;
	if(remaining() >= 4) {
		result = (d->contents[d->rpos]   << 24)
			   + (d->contents[d->rpos+1] << 16)
			   + (d->contents[d->rpos+2] << 8)
	 		   +  d->contents[d->rpos+3];
		d->rpos += 4;
	} else {
		d->_readError = true;
	}
	return result;
}

void Buffer::readLongSeq(vector<long>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	for(i=0;i<seqlen;i++) result.push_back(readLong());
}

float Buffer::readFloat()
{
	// FIXME: see writeFloat()
	long f_as_long = readLong();

	if(!d->_readError) return *(float *)&f_as_long;
	return 0.0;
}

void Buffer::readFloatSeq(vector<float>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	for(i=0;i<seqlen;i++) result.push_back(readFloat());
}

void Buffer::readString(string& result)
{
	long len = readLong();
	char *data = (char *)read(len);

	if(data && len)
		result.assign(data,len-1);
	else
		result = "";
}

void Buffer::readStringSeq(vector<string>& result)
{
	// might be optimizable a bit

	long i,seqlen = readLong();

	result.clear();
	//result.reserve(seqlen);

	for(i=0;i<seqlen;i++) {
		string s;

		readString(s);
		result.push_back(s);
	}
}


void Buffer::patchLength()
{
	long len = size();
	assert(len >= 8);

	d->contents[4] = (len >> 24) & 0xff;
	d->contents[5] = (len >> 16) & 0xff;
	d->contents[6] = (len >> 8) & 0xff;
	d->contents[7] = len & 0xff;
}

void Buffer::patchLong(long position, long value)
{
	long len = size();
	assert(len >= position+4);

	d->contents[position]   = (value >> 24) & 0xff;
	d->contents[position+1] = (value >> 16) & 0xff;
	d->contents[position+2] = (value >> 8) & 0xff;
	d->contents[position+3] = value & 0xff;
}

string Buffer::toString(const string& name)
{
	string result;
	char hex[17] = "0123456789abcdef";

	vector<unsigned char>::iterator ci;
	for(ci = d->contents.begin(); ci != d->contents.end(); ci++)
	{
		result += hex[(*ci >> 4) & 0xf];
		result += hex[*ci & 0xf];
	}

	if(name == "") return result;
	return name + ":" + result;
}

unsigned char Buffer::fromHexNibble(char c)
{
	int uc = (unsigned char)c;

	if(uc >= '0' && uc <= '9') return uc - (unsigned char)'0';
	if(uc >= 'a' && uc <= 'f') return uc + 10 - (unsigned char)'a';
	if(uc >= 'A' && uc <= 'F') return uc + 10 - (unsigned char)'A';

	return 16;	// error
}

static int stringncmp(const string& s1, const string& s2, size_t n)
{
	// I don't know a way to implement that compliant to all STL string
	// implementations (compare seems to work differently sometimes)
	return strncmp(s1.c_str(),s2.c_str(),n);
}

bool Buffer::fromString(const string& data, const string& name)
{
	string start = name+":";
	if(name == "") start = "";

	if(stringncmp(data,start,start.size()) != 0) return false;
	d->contents.clear();

	string::const_iterator di = data.begin() + start.size();
	
	while(di != data.end())
	{
		unsigned char h = fromHexNibble(*di++);	// high nibble
		if(di == data.end()) return false;

		unsigned char l = fromHexNibble(*di++);	// low nibble

		if(h >= 16 || l >= 16) return false;	// no proper hex digit
		d->contents.push_back((h << 4) + l);
	}
	return true;
}
