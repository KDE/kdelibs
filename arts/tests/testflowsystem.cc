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

#include "flowsystem.h"
#include "connect.h"
#include "value.h"
#include "test.h"

using namespace Arts;
using namespace std;

struct TestFlowSystem : public TestCase
{
	TESTCASE(TestFlowSystem);

	Dispatcher dispatcher;
	StringValue v1, v2;

	void process() {
		dispatcher.ioManager()->processOneEvent(false);
	}
	void setUp() {
		/* initialize v1, v2 with values */
		v1 = StringValue();	v1.value("v1");
		v2 = StringValue(); v2.value("v2");
	}
	void tearDown() {
		v1 = StringValue::null();
		v2 = StringValue::null();
	}
	TEST(connectObject) {
		dispatcher.flowSystem()->connectObject(v1,"value_changed",v2,"value");

		testEquals("v1",v1.value());
		testEquals("v2",v2.value());

		v1.value("test");
		process();

		testEquals("test",v1.value());
		testEquals("test",v2.value());
	}
	// at the time of writing, this fails: disconnectObject is not implemented
	TEST(disconnectObject) {
		connect(v1,"value_changed",v2,"value");

		/* here we have a connection, so this should be sent */
		v1.value("sendThis");
		process();
		testEquals("sendThis", v2.value());

		dispatcher.flowSystem()->disconnectObject(v1,"value_changed",
															v2,"value");

		/* here we don't have a connection, so this should not be sent */
		v1.value("v1only");
		process();

		testEquals("v1only", v1.value());
		testEquals("sendThis", v2.value());
	}
};

TESTMAIN(TestFlowSystem);
