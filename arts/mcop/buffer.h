    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

#ifndef MCOPBYTE_DEFINED
#define MCOPBYTE_DEFINED
typedef unsigned char mcopbyte;
#endif

class Buffer {
	std::vector<unsigned char> contents;
	long rpos;
	bool _readError;
	unsigned char fromHexNibble(char c);

public:
	Buffer();
	bool readError();
	void writeBool(bool b);
	void writeByte(mcopbyte b);
	void writeByteSeq(const vector<mcopbyte>& seq);
	void writeLong(long l);
	void writeLongSeq(const std::vector<long>& seq);
	void writeFloat(float f);
	void writeString(const std::string& s);
	void writeStringSeq(const std::vector<std::string>& seq);
	inline long size() {
		return contents.size();
	}

	inline long remaining() {
		return size()-rpos;
	}

	void *read(long l);
	void *peek(long l);
	void skip(long l);

	void write(void *data, long l);

	bool readBool();
	mcopbyte readByte();
	void readByteSeq(vector<mcopbyte>& result);
	long readLong();
	void readLongSeq(std::vector<long>& result);
	float readFloat();
	void readString(std::string& result);
	void readStringSeq(std::vector<std::string>& result);

	void patchLength();

	std::string toString(const std::string& name);
	bool fromString(const std::string& data, const std::string& name);
};

#endif
