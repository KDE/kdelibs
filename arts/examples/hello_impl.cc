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

#include "hello_impl.h"
#include <stdio.h>

void Hello_impl::hello(const string& s)
{
	printf("Hello '%s'!\n",s.c_str());
}

string Hello_impl::concat(const string& s1, const string& s2)
{
	return s1+s2;
}

long Hello_impl::sum2(long a, long b)
{
	return a+b;
}

long Hello_impl::sum3(long a, long b, long c)
{
	return a+b+c;
}
