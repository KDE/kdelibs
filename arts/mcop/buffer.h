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

    */

#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

class Buffer {
	vector<unsigned char> contents;
	long rpos;
	bool _readError;
	unsigned char fromHexNibble(char c);

public:
	Buffer();
	bool readError();
	void writeBool(bool b);
	void writeLong(long l);
	void writeLongSeq(const vector<long>& seq);
	void writeFloat(float f);
	void writeString(const string& s);
	void writeStringSeq(const vector<string>& seq);
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
	long readLong();
	void readLongSeq(vector<long>& result);
	float readFloat();
	void readString(string& result);
	void readStringSeq(vector<string>& result);

	void patchLength();

	string toString(const string& name);
	bool fromString(const string& data, const string& name);
};

#endif
