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

#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

/*
 * BC - Status (2000-09-30): Buffer.
 *
 * Has to be kept binary compatible. As buffer is speed relevant, currently
 * there are no private d ptrs, and the idea is to keep this as possible.
 *
 * If not, but additional stuff in the d ptr, but keep as much data as
 * possible in the main items.
 */

namespace Arts {

#ifndef MCOPBYTE_DEFINED
#define MCOPBYTE_DEFINED
typedef unsigned char mcopbyte;
#endif

class BufferPrivate;

class Buffer {
private:
	long rpos;
	bool _readError;
	std::vector<unsigned char> contents;

	BufferPrivate *d;	// unused
	unsigned char fromHexNibble(char c);

public:
	Buffer();
	~Buffer();

	bool readError();
	void writeBool(bool b);
	void writeBoolSeq(const std::vector<bool>& seq);
	void writeByte(mcopbyte b);
	void writeByteSeq(const std::vector<mcopbyte>& seq);
	void writeLong(long l);
	void writeLongSeq(const std::vector<long>& seq);
	void writeFloat(float f);
	void writeFloatSeq(const std::vector<float>& seq);
	void writeString(const std::string& s);
	void writeStringSeq(const std::vector<std::string>& seq);

	long size();
	long remaining();
	void *read(long l);
	void read(std::vector<mcopbyte>& raw, long l);
	void *peek(long l);
	void skip(long l);
	void rewind();

	void write(void *data, long l);
	void write(const std::vector<mcopbyte>& raw);

	bool readBool();
	void readBoolSeq(std::vector<bool>& result);
	mcopbyte readByte();
	void readByteSeq(std::vector<mcopbyte>& result);
	long readLong();
	void readLongSeq(std::vector<long>& result);
	float readFloat();
	void readFloatSeq(std::vector<float>& result);
	void readString(std::string& result);
	void readStringSeq(std::vector<std::string>& result);

	void patchLength();
	void patchLong(long position, long value);

	std::string toString(const std::string& name);
	bool fromString(const std::string& data, const std::string& name);
};

};

#endif
