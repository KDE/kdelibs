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
#include <stdio.h>
#include <list>
#include "test.h"

using namespace Arts;

struct TestBuffer : public TestCase
{
	TESTCASE(TestBuffer);

	Arts::Buffer *buffer, *buffer1234;

	void setUp() {
		buffer = new Buffer();
		buffer1234 = new Buffer();
		buffer1234->writeByte(1);
		buffer1234->writeByte(2);
		buffer1234->writeByte(3);
		buffer1234->writeByte(4);
	}
	void tearDown() {
		delete buffer;		buffer = 0;
		delete buffer1234;	buffer1234 = 0;
	}
	TEST(writeByte) {
		buffer->writeByte(0x42);

		testEquals("42",buffer->toString(""))
	}
	TEST(writeLong) {
		buffer->writeLong(10001025);

		testEquals("00989a81",buffer->toString(""));
	}
	TEST(write) {
		vector<mcopbyte> b;
		b.push_back(1);
		b.push_back(2);

		buffer->write(b);

		testEquals("0102",buffer->toString(""));

		char c[2] = { 3,4 };
		buffer->write(c,2);

		testEquals("01020304",buffer->toString(""));
	}
	TEST(read) {
		vector<mcopbyte> bytes;
		buffer1234->read(bytes,4);

		testEquals(4, bytes.size());
		testEquals(1, bytes[0]);
		testEquals(2, bytes[1]);
		testEquals(3, bytes[2]);
		testEquals(4, bytes[3]);

		// read shall overwrite the vector it gets as parameter
		buffer1234->rewind();
		buffer1234->read(bytes,3);
		testEquals(3, bytes.size());
	}
};

TESTMAIN(TestBuffer);
