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

#include "wrapper.h"
#include "test.h"
#include <iostream.h>
#include <stdio.h>

using namespace std;
using namespace Arts;

int active_d_objects = 0;

class D_impl: virtual public D_skel {
private:
	long _value;
public:
	void value(long newVal) { _value = newVal; }
	long value() { return _value; }

	D_impl() :_value(0) { active_d_objects++; }
	~D_impl() { active_d_objects--; }
	string a() { return "a"; }
	string b() { return "b"; }
	string c() { return "c"; }
	string d() { return "d"; }
	void dummy() {  }

	long sum(A v1, A v2) { return v1.value() + v2.value(); }
	D self() { return D::_from_base(_copy()); }
};

REGISTER_IMPLEMENTATION(D_impl);

class B_impl: virtual public B_skel {
private:
	long _value;
public:
	void value(long newVal) { _value = newVal; }
	long value() { return _value; }
	B_impl() :_value(0) {}
	string a() { return "a"; }
	string b() { return "b"; }
};
REGISTER_IMPLEMENTATION(B_impl);


struct TestWrapper : public TestCase
{
	TESTCASE(TestWrapper);

	Arts::Dispatcher dispatcher;

	/*
	  generic inheritance test, tests that using a D object, you
	  can call the a,b,c,d methods without problems
	*/
	TEST(genericInheritance)
	{
		D d;

		/* implications of lazy creation */
		testEquals(0, active_d_objects);
		
		C c = d;
		testEquals(0, active_d_objects);

		string abcd = A(d).a()+d.b()+c.c()+d.d();
		testEquals("abcd",abcd);

		/*
			should be exactly one here, not more than one through the various
			assignments
		 */
		testEquals(1, active_d_objects);
	
		Object o = d;

		/* dynamic casting tests */
		c = DynamicCast(o);      // operator
		testEquals(false, c.isNull());
		testEquals("c", c.c());

		C cc = DynamicCast(o);   // constructor
		testEquals(false, cc.isNull());
		testEquals("c", cc.c());

		A a = Reference(d.toString());
		cc = DynamicCast(a);
		testEquals(false, cc.isNull());
		testEquals("c", cc.c());
	
		/* isNull() and error() */
		testEquals(false, d.isNull());
		testEquals(false, d.error());

		d = D::null();
		testEquals(true, d.isNull());
		testEquals(false, c.isNull());
		testEquals(false, d.error());

		/* reference counting */
		c = C::null();
		o = Object::null();
		cc = C::null();
		a = A::null();
		testEquals(0, active_d_objects);
	}
	TEST(nullInAssign) {
		B b;
		B b2 = b;
		b = B::null();
		testEquals(true, b.isNull());
		testEquals(false, b2.isNull());
	}
	TEST(coreference) {
		B b3;
		B b4=b3;
		b3.value(3);
		testEquals(3, b4.value());
	}

	D afunc(D arg)
	{
		arg.value(42);
		return arg;
	}

	TEST(cacheAndArgs) {
		/* test cache invalidation */
		B b;
		b.value(3); // stores 3 using parent method
		testEquals(0, active_d_objects);

		D d;
		d.value(6);
		testEquals(1, active_d_objects);

		b = d; // uses operator, not constructor. should invalidate A::cacheOK
		testEquals(6, b.value());
		testEquals(1, active_d_objects);
		
		A a = afunc(d);
		testEquals(1, active_d_objects);
		testEquals(42, a.value());
		testEquals(42, b.value());
		testEquals(42, d.value());
	}

	TEST(dynamicCastNull) {
		D normalD;
		D nullD = D::null();
		A aNormalD = normalD;
		A aNullD = nullD;
		D d;

		d = DynamicCast(aNormalD);
		testEquals(false, d.isNull());

		d = DynamicCast(aNullD);
		testEquals(true, d.isNull());
	}

	TEST(stringification) {
		D d;
		d.value(5);

		string s = d.toString();

		D anotherD = Arts::Reference(s);
		testEquals(false, anotherD.isNull());
		testEquals(5, anotherD.value());
	}
	/*
	unfortunately, this currently fails, because resolving a reference to a
	non-existing object is not handled properly by the dispatcher - I am not
	sure whether it is worth fixing it (which might be complicated)

	TEST(stringificationFailures) {
		D d;
		d.value(5);
		string s = d.toString();
		d = D::null();

		D anotherD = Arts::Reference(s);
		testEquals(true, anotherD.isNull());
	}*/
};

TESTMAIN(TestWrapper);
