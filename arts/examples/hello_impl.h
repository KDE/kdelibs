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

#ifndef HELLO_IMPL_H
#define HELLO_IMPL_H

#include "hello.h"

class Hello_impl : virtual public Hello_skel {
public:
	void hello(const string& s);
	string concat(const string& s1, const string& s2);
	long sum2(long a, long b);
	long sum3(long a, long b, long c);
};

#endif /* HELLO_IMPL_H */
