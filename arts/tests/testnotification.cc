    /*

    Copyright (C) 2001 Stefan Westerfeld
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

#include "notification.h"
#include "dispatcher.h"
#include "test.h"

using namespace Arts;
using namespace std;

static string messagesReceived;
static long messagesAlive;

static void destroyString(const Notification& n)
{
	messagesAlive--;
	delete (string *)n.data;
}

class Receiver : public NotificationClient {
private:
	long alive;

public:
	Receiver() { alive = 42; }
	virtual ~Receiver() {
		NotificationManager::the()->removeClient(this);
		alive = 0;
	}

	void notify(const Notification& n) {
		if(alive != 42) messagesReceived += "<dead>";

		string *data = (string *)n.data;
		messagesReceived += *data;
		destroyString(n);

		if(alive != 42) messagesReceived += "</dead>";
	}
};

struct TestNotification : public TestCase
{
	TESTCASE(TestNotification);

	Dispatcher dispatcher;

	void setUp() {
		messagesReceived = "";
		messagesAlive = 0;
	}
	void tearDown() {
	}
	void sendNotification(Receiver& r, string text)
	{
		messagesAlive++;
		Notification n;
		n.receiver = &r;
		n.data = new string(text);
		n.internal = 0;
		n.setDestroy(destroyString);
		NotificationManager::the()->send(n);
	}

	/* sending a notification (should be asynchronous) */
	TEST(send) {
		Receiver r1;
		sendNotification(r1,"r1");

		testEquals(1,messagesAlive);
		testEquals("",messagesReceived);
		testEquals(true, NotificationManager::the()->pending());

		NotificationManager::the()->run();

		testEquals("r1",messagesReceived);
		testEquals(false, NotificationManager::the()->pending());
		testEquals(0,messagesAlive);
	}

	/* order of sending notifications (first-in-first-out) */
	TEST(sendOrder) {
		Receiver r1,r2;

		sendNotification(r1,"r1");
		sendNotification(r2,"r2");

		testEquals(2,messagesAlive);
		testEquals("",messagesReceived);
		testEquals(true, NotificationManager::the()->pending());

		NotificationManager::the()->run();

		testEquals(false, NotificationManager::the()->pending());
		testEquals(0,messagesAlive);
		testEquals("r1r2",messagesReceived);
	}

	/* deletion of unprocessed notifications once a client dies */
	TEST(clientDeletion) {
		Receiver *r1 = new Receiver,
		         *r2 = new Receiver,
		         *r3 = new Receiver,
				 *r4 = new Receiver;

		sendNotification(*r1,"r1");
		sendNotification(*r2,"r2");
		sendNotification(*r3,"r3");
		sendNotification(*r4,"r4");

		testEquals(4,messagesAlive);
		testEquals("",messagesReceived);

		delete r2;
		delete r4;

		testEquals(2,messagesAlive);
		testEquals("",messagesReceived);

		NotificationManager::the()->run();

		testEquals("r1r3",messagesReceived);
		testEquals(0,messagesAlive);
	}
};

TESTMAIN(TestNotification);
