    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de
    Modified by Nicolas Brodu, nicolas.brodu@free.fr

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

#ifndef HELLO_IMPL_H
#define HELLO_IMPL_H

#include "hello.h"

using namespace std;

class Hello_impl : virtual public Hello_skel {
private:
	long myvalue;
public:
	long myValue();
	void myValue(long newValue);
	void constructor(long i);
	void add(long increment);
	void printTotal();

	void hello(const string& s);
	long sum(long a, long b);
	string concat(const string& s1, const string& s2);

	void hellobase(const string& s);
};

#endif /* HELLO_IMPL_H */
