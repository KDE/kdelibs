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

#include "aggregation.h"
#include <iostream>

using namespace std;
using namespace Arts;

class String_impl : virtual public String_skel {
	string _value;
public:
	void constructor(const std::string& value) { _value = value; }
	string value() { return _value; }
	void value(const std::string& newValue) { _value = newValue; }
};

REGISTER_IMPLEMENTATION(String_impl);

void printall(Arts::Object o, std::string pre = "")
{
	String s;
	s = DynamicCast(o);
	if(!s.isNull())
		cout << pre << " - " << s.value() << endl;

	vector<string> *children = s._queryChildren();
	vector<string>::iterator i;
	for(i = children->begin(); i != children->end(); i++)
		printall(s._getChild(*i), pre+"  ");
}

/*
 * Small test case for aggregation
 *
 * The idea behind all this is that you can add children to objects, building
 * a tree (or if you like graph) of objects without the objects knowing anything
 * about this. Aggregation has strong references, that means, the objects you
 * add together in a tree will stay this way, as long as you keep a reference
 * to the root.
 */
int main()
{
	Dispatcher d;

	String s("spreadsheet");
	s._addChild(String("chart"),"child1");
	s._addChild(String("image"),"child2");
	printall(s);

	cout << "deleting \"child1\"" << endl;
	if(!s._removeChild("child1")) cout << "FAIL" << endl;
	printall(s);
}
