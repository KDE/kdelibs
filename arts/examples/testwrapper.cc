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
#include <iostream.h>
#include <stdio.h>

using namespace std;
using namespace Arts;

int x = 0;
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
	void dummy() { x++; }

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

#define CALLS 50000000

void check(const char *name,bool passed)
{
	printf("%30s ... %s\n",name,passed?"PASSED":"FAIL");
}

/*
generic inheritance test, tests that using a D object, you can call the a,b,c,d
methods without problems
*/
void test0()
{
	D d;
	assert(active_d_objects == 0);
	C c = d;
	assert(active_d_objects == 0);
	string abcd = A(d).a()+d.b()+c.c()+d.d();
	check("generic inheritance test",abcd == "abcd");

	/*
	   should be exactly one here, not more than one through the various
	   assignments
	 */
	assert(active_d_objects == 1);
	
	Object o = d;
	c = DynamicCast(o);      // operator
	C cc = DynamicCast(o);   // constructor
	bool dyna_ok = !cc.isNull() && cc.c() == "c" && !c.isNull() && c.c() == "c";
	A a = Reference(d.toString());
	cc = DynamicCast(a);
	dyna_ok &= !cc.isNull() && cc.c() == "c";
	check("Dynamic Casting", dyna_ok);
	
	// Test isNull() and error()
	bool nullOK = !d.isNull();
	nullOK &= !d.error();
	d = D::null();
	nullOK &= d.isNull();
	nullOK &= !c.isNull();
	nullOK &= !d.error();
	c = C::null();
	o = Object::null();
	cc = C::null();
	a = A::null();
	assert(active_d_objects == 0);
	B b;
	B b2=b;
	b = B::null();
	nullOK &= b.isNull();
	nullOK &= !b2.isNull();
	B b3;
	B b4=b3;
	b3.value(3);
	nullOK &= (b4.value()==3);
	check("nullity and error conditions",nullOK);
	
	
}

D afunc(D arg)
{
	arg.value(42);
	return arg;
}

void test1()
{
	// test cache invalidation
	B b;
	b.value(3); // stores 3 using parent method
	assert(active_d_objects == 0);
	D d;
	d.value(6);
	assert(active_d_objects == 1);
	b = d; // uses operator, not constructor. should invalidate A::cacheOK
	check("proper cache handling",b.value()==6);
	assert(active_d_objects == 1);
	
	A a = afunc(d);
	assert(active_d_objects == 1);
	check("SmartWrapper as argument",a.value()==42 && b.value()==42 && d.value()==42);
}

int main()
{
	Dispatcher dispatcher;

	assert(active_d_objects == 0);
	test0();
	assert(active_d_objects == 0);
	test1();
	assert(active_d_objects == 0);

	return 0;
}
