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
#include <cstring>
#include <assert.h>

using namespace std;
using namespace Arts;

Buffer::Buffer() : rpos(0), _readError(false),d(0)
{
	contents.reserve(128);
}

Buffer::~Buffer()
{
}

long Buffer::size()
{
	return contents.size();
}

long Buffer::remaining()
{
	return size()-rpos;
}

bool Buffer::readError() {
	return _readError;
}

void Buffer::writeBool(bool b) {
	contents.push_back(b?1:0);
}

void Buffer::writeByte(mcopbyte b) {
	contents.push_back(b);
}

void Buffer::writeLong(long l) {
	contents.push_back((l >> 24) & 0xff);
	contents.push_back((l >> 16) & 0xff);
	contents.push_back((l >> 8) & 0xff);
	contents.push_back(l & 0xff);
}

void Buffer::writeBoolSeq(const vector<bool>& seq) {
	writeLong(seq.size());

	vector<bool>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeBool(*i);
}

void Buffer::writeByteSeq(const vector<mcopbyte>& seq) {
	writeLong(seq.size());	 // bytes are sent raw, so we can call read here
	write(seq);
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
	contents.insert(contents.end(),reinterpret_cast<const unsigned char*>(s.c_str()),
		        reinterpret_cast<const unsigned char*>(s.c_str()+len));
}

void Buffer::writeStringSeq(const vector<string>& seq) {
	writeLong(seq.size());

	vector<string>::const_iterator i;
	for(i = seq.begin(); i != seq.end(); i++) writeString(*i);
}

void Buffer::write(void *data, long len) {
	unsigned char *c = (unsigned char *)data;

	contents.insert(contents.end(),c,c+len);
}

void Buffer::write(const vector<mcopbyte>& raw)
{
	contents.insert(contents.end(), raw.begin(), raw.end());
}

void Buffer::read(vector<mcopbyte>& raw, long l)
{
	if(l >= 0 && remaining() >= l) {
		raw.clear();
		raw.insert(raw.end(), contents.begin()+rpos, contents.begin()+rpos+l);
		rpos += l;
	} else {
		_readError = true;
	}
}

void *Buffer::read(long l) {
	void *result = 0;

	if(l >= 0 && remaining() >= l) {
		result = &contents[rpos];
		rpos += l;
	} else {
		_readError = true;
	}
	return result;
}

void *Buffer::peek(long l) {
	assert(l >= 0 && remaining() >= l);
	return &contents[rpos];
}

void Buffer::skip(long l) {
	if(l >= 0 && remaining() >= l) {
		rpos += l;
	} else {
		_readError = true;
	}
}

void Buffer::rewind() {
	rpos = 0;
}

bool Buffer::readBool()
{
	long result = false;
	if(remaining() >= 1) {
		if(contents[rpos] == 1)
			result = true;
		else
		{
			assert(contents[rpos] == 0);
		}
		rpos += 1;
	} else {
		_readError = true;
	}
	return result;
}

void Buffer::readBoolSeq(vector<bool>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	if(seqlen >= 0 && remaining() >= seqlen)
	{
		for(i=0;i<seqlen;i++) result.push_back(readBool());
	}
	else
	{
		_readError = true;
	}
}


mcopbyte Buffer::readByte()
{
	if(remaining() >= 1)
	{
		return contents[rpos++];
	}
	else
	{
		_readError = true;
		return 0;
	}
}

void Buffer::readByteSeq(vector<mcopbyte>& result)
{
	long seqlen = readLong(); // bytes are sent raw, so we can call read here
	read(result, seqlen);
}

long Buffer::readLong()
{
	long result = 0;
	if(remaining() >= 4) {
		result = (contents[rpos]   << 24)
			   + (contents[rpos+1] << 16)
			   + (contents[rpos+2] << 8)
	 		   +  contents[rpos+3];
		rpos += 4;
	} else {
		_readError = true;
	}
	return result;
}

void Buffer::readLongSeq(vector<long>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	if(seqlen * 4 >= 0 && remaining() >= seqlen * 4)
	{
		for(i=0;i<seqlen;i++) result.push_back(readLong());
	}
	else
	{
		_readError = true;
	}
}

float Buffer::readFloat()
{
	// FIXME: see writeFloat()
	long f_as_long = readLong();

	if(!_readError) return *(float *)&f_as_long;
	return 0.0;
}

void Buffer::readFloatSeq(vector<float>& result)
{
	// might be optimizable a bit
	long i,seqlen = readLong();

	result.clear();
	if(seqlen * 4 >= 0 && remaining() >= seqlen * 4)
	{
		for(i=0;i<seqlen;i++) result.push_back(readFloat());
	}
	else
	{
		_readError = true;
	}
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
		if(_readError) return;

		result.push_back(s);
	}
}


void Buffer::patchLength()
{
	long len = size();
	assert(len >= 8);

	contents[4] = (len >> 24) & 0xff;
	contents[5] = (len >> 16) & 0xff;
	contents[6] = (len >> 8) & 0xff;
	contents[7] = len & 0xff;
}

void Buffer::patchLong(long position, long value)
{
	long len = size();
	assert(len >= position+4);

	contents[position]   = (value >> 24) & 0xff;
	contents[position+1] = (value >> 16) & 0xff;
	contents[position+2] = (value >> 8) & 0xff;
	contents[position+3] = value & 0xff;
}

string Buffer::toString(const string& name)
{
	string result;
	char hex[17] = "0123456789abcdef";

	vector<unsigned char>::iterator ci;
	for(ci = contents.begin(); ci != contents.end(); ci++)
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
	contents.clear();

	string::const_iterator di = data.begin() + start.size();
	
	while(di != data.end())
	{
		unsigned char h = fromHexNibble(*di++);	// high nibble
		if(di == data.end()) return false;

		unsigned char l = fromHexNibble(*di++);	// low nibble

		if(h >= 16 || l >= 16) return false;	// no proper hex digit
		contents.push_back((h << 4) + l);
	}
	return true;
}

