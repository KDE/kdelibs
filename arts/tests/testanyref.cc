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

#include "anyref.h"
#include "core.h"
#include "test.h"

using namespace Arts;
using namespace std;

struct TestAnyRef : public TestCase
{
	TESTCASE(TestAnyRef);

	Dispatcher dispatcher;
	Buffer *buffer, *buffer2;
	Any any;

	void setUp() {
		buffer = new Buffer();
		any.type = "";
		any.value.clear();
	}
	void tearDown() {
		delete buffer;		buffer = 0;
	}
	void copy(AnyRef& src, AnyRef& dest)
	{
		Buffer tmp;
		src.write(&tmp);
		tmp.rewind();
		dest.read(&tmp);
	}
	TEST(ioAnyFloat) {
		// create a buffer with a float and a end-of-buffer delimiter
		float f = 1.1;
		buffer->writeFloat(f);
		buffer->writeLong(12345678);
		buffer->rewind();

		// see if reading it as float reads the *only* the float
		any.type = "float";

		AnyRef ref(any);
		ref.read(buffer);
		testEquals(12345678,buffer->readLong());
		testEquals(4, any.value.size());

		// check if the value is all right by copying it to a "real" float
		float f2;
		AnyRef ref2(f2);
		copy(ref,ref2);
		testEquals(f,f2);
	}
	TEST(ioAnyType) {
		// do the same again with a complex structured type
		Arts::InterfaceDef objInterface =
			Dispatcher::the()->interfaceRepo().queryInterface("Arts::Object");
		testEquals("Arts::Object", objInterface.name);

		objInterface.writeType(*buffer);
		buffer->writeLong(12345678);
		buffer->rewind();

		vector<mcopbyte> objInterfaceRaw;
		buffer->read(objInterfaceRaw, buffer->remaining() - 4);
		buffer->rewind();
		testAssert(objInterfaceRaw.size() > 50);

		any.type = "Arts::InterfaceDef";
		AnyRef ref(any);
		ref.read(buffer);
		testEquals(12345678,buffer->readLong());
		testAssert(objInterfaceRaw == any.value);
	}

};

TESTMAIN(TestAnyRef);
