    /*

    Copyright (C) 2000 Stefan Westerfeld stefan@space.twc.de

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

#include "value.h"
#include "connect.h"
#include "test.h"
#include <iostream.h>
#include <stdio.h>

using namespace std;
using namespace Arts;

struct TestChangeNotify : public TestCase
{
	TESTCASE(TestChangeNotify);

	Arts::Dispatcher dispatcher;
	float zero, three, four;

	void setUp()
	{
		zero = 0.0;
		three = 3.0;
		four = 4.0;
	}

	void process()
	{
		dispatcher.ioManager()->processOneEvent(false);
	}

	TEST(floatValue)
	{
		FloatValue f;
	
		testEquals(zero, f.value());

		f.value(three);
		testEquals(three, f.value());
	}
	TEST(stringValue)
	{
		StringValue s;
		
		testEquals("",s.value());

		s.value("hello");
		testEquals("hello", s.value());
	}
	TEST(floatNotify)
	{
		FloatValue f1, f2;

		connect(f1,"value_changed",f2,"value");
		f1.value(three);

		process();
		testEquals(three, f2.value());
	}
	TEST(floatMultiNotify)
	{
		FloatValue f1, f2, f3;

		connect(f1,"value_changed",f2,"value");
		connect(f1,"value_changed",f3,"value");
		f1.value(three);

		process();
		testEquals(three, f2.value());
		testEquals(three, f3.value());
	}
	TEST(floatCrossNotify)
	{
		FloatValue f1, f2;

		connect(f1,"value_changed",f2,"value");
		connect(f2,"value_changed",f1,"value");
		f1.value(three);

		process();
		testEquals(three, f1.value());
		testEquals(three, f2.value());

		f2.value(four);

		process();
		testEquals(four, f1.value());
		testEquals(four, f2.value());
	}
	TEST(stringNotify)
	{
		StringValue s1, s2;

		connect(s1,"value_changed",s2,"value");
		s1.value("hello");

		process();
		testEquals("hello", s2.value());
	}
	TEST(stringMultiNotify)
	{
		StringValue s1, s2, s3;

		connect(s1,"value_changed",s2,"value");
		connect(s1,"value_changed",s3,"value");
		s1.value("hello");

		process();
		testEquals("hello", s2.value());
		testEquals("hello", s3.value());
	}
	TEST(stringCrossNotify)
	{
		StringValue s1, s2;

		connect(s1,"value_changed",s2,"value");
		connect(s2,"value_changed",s1,"value");
		s1.value("world");

		process();
		testEquals("world", s1.value());
		testEquals("world", s2.value());

		s2.value("test");

		process();
		testEquals("test", s1.value());
		testEquals("test", s2.value());
	}
	TEST(floatSenderToValue1)
	{
		FloatSender sender;
		FloatValue  f;

		connect(sender,"outstream",f,"value");
		vector<float> data;
		data.push_back(three);
		sender.send(data);

		process();
		testEquals(three,f.value());
	}
	TEST(floatSenderToValue2)
	{
		FloatSender sender;
		FloatValue  f;

		connect(sender,"outstream",f,"value");
		vector<float> data;
		data.push_back(three);
		data.push_back(four);
		sender.send(data);

		process();
		testEquals(four,f.value());
	}
	TEST(myEnumNotify)
	{
		MyEnumValue e1, e2;

		connect(e1,"value_changed",e2,"value");

		testEquals(meIdle, e1.value());
		testEquals(meIdle, e2.value());

		e1.value(meHelloWorld);
		process();

		testEquals(meHelloWorld, e1.value());
		testEquals(meHelloWorld, e2.value());
	}
};

TESTMAIN(TestChangeNotify);
