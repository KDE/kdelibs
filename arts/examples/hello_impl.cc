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

#include "hello_impl.h"
#include <stdio.h>

using namespace std;

void Hello_impl::hellobase(const string& s)
{
	printf("Hellobase '%s'!\n",s.c_str());
}

void Hello_impl::hello(const string& s)
{
	printf("Hello '%s'!\n",s.c_str());
}

void Hello_impl::constructor(long i)
{
	myvalue=i;
}

long Hello_impl::myValue()
{
	return myvalue;
}

void Hello_impl::myValue(long newValue)
{
	myvalue=newValue;
}

void Hello_impl::add(long a)
{
	myvalue += a;
}

void Hello_impl::printTotal()
{
	printf("Total=%ld\n",myvalue);
}

long Hello_impl::sum(long a, long b)
{
	return a+b;
}

string Hello_impl::concat(const string& s1, const string& s2)
{
	return s1+s2;
}

REGISTER_IMPLEMENTATION(Hello_impl);
