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

#include "remotetest.h"
#include "value.h"
#include "test.h"
#include "connect.h"
#include <iostream.h>
#include <stdio.h>

using namespace std;
using namespace Arts;

class RemoteTest_impl: virtual public RemoteTest_skel {
private:
	bool alive;

public:
	RemoteTest_impl()
	{
		alive = true;
		_copy();
	}
	~RemoteTest_impl()
	{
		Dispatcher::the()->terminate();
	}

	Object createObject(const string& name)
	{
		return SubClass(name);
	}
	long fortyTwo()
	{
		return 42;
	}
	void die()
	{
		if(alive)
		{
			_release();
			alive = false;
		}
	}
};

REGISTER_IMPLEMENTATION(RemoteTest_impl);

/* name of the remote object - should be sufficiently unique */
char objectName[60];

struct TestRemote : public TestCase
{
	TESTCASE(TestRemote);

	Arts::Dispatcher dispatcher;
	RemoteTest remoteTest;

	void setUp() {
		string globalRef = "global:";
		globalRef += objectName;

		remoteTest = Arts::Reference(globalRef);
	}
	void tearDown() {
		remoteTest = RemoteTest::null();
	}

	void process()
	{
		dispatcher.ioManager()->processOneEvent(false);
	}


	/* test whether the server is running, and we could connect ok */
	TEST(connected) {
		testEquals(false, remoteTest.isNull());
	}

	/* test method call */
	TEST(fortyTwo) {
		testEquals(42, remoteTest.fortyTwo());
	}

	/* test remote change notifications */
	TEST(remoteChangeNotify) {
		StringValue local;
		local.value("local");
		testEquals("local", local.value());

		Object o = remoteTest.createObject("StringValue");
		testAssert(!o.isNull());

		StringValue remote = Arts::DynamicCast(o);
		testAssert(!remote.isNull());
		remote.value("remote");
		testEquals("remote", remote.value());

		connect(local,"value_changed",remote,"value");
		local.value("transferme");

		process();
		sleep(1);	/* change notifications are asynchronous */

		testEquals("transferme", local.value());
		testEquals("transferme", remote.value());

		disconnect(local,"value_changed",remote,"value");

		local.value("notransfer");

		testEquals("notransfer", local.value());
		testEquals("transferme", remote.value());
	}

	/* terminate server */
	TEST(die) {
		remoteTest.die();
		remoteTest = RemoteTest::null();
		sleep(1);
	}

	/* check if server is dead now */
	TEST(dead) {
		testEquals(true, remoteTest.isNull());
	}
};

TESTMAINFUNC(TestRemote,performTests);

/*
 * this test case is a bit tricky because we need a client and a server
 *
 * we create the server in a fork()ed process, and then evaluate the test
 * inside the client
 */
int main()
{
	/* create unique object name for test server/test client */
	sprintf(objectName, "RemoteTest_%d_%ld", getpid(), time(0));

	int pid = fork();
	if(pid == 0)	/* child process - we play the server here */
	{
		Dispatcher dispatcher(0, Dispatcher::startUnixServer);
		ObjectManager::the()->addGlobalReference(RemoteTest(), objectName);
		dispatcher.run();
		return 0;
	}
	else if(pid >= 0)	/* parent process - do the test here */
	{
		sleep(1);
		return performTests();
	}
}
