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

#include "hello.h"
#include "dynamicskeleton.h"
#include "debug.h"
#include "stdio.h"

using namespace Arts;
using namespace std;

namespace Arts { typedef DynamicSkeleton<Arts::Object_skel> Object_dskel; }

class HelloDynamic : public Arts::Object_dskel {
protected:
	long myValue;

public:
	HelloDynamic() : Arts::Object_dskel("Hello") {
	}
	void process(long methodID, Arts::Buffer *request, Arts::Buffer *result)
	{
		const Arts::MethodDef& methodDef = getMethodDef(methodID);

		if(methodDef.name == "hello")	// void hello(string);
		{
			string s; request->readString(s);
			printf("Hello '%s'!\n",s.c_str());
		}
		else if(methodDef.name == "_set_myValue")	// attribute long myValue;
		{
			myValue = request->readLong();
		}
		else if(methodDef.name == "_get_myValue")
		{
			result->writeLong(myValue);
		}
		else if(methodDef.name == "concat")
		{
			string s1; request->readString(s1);
			string s2; request->readString(s2);
			result->writeString(s1+s2);
		}
		else if(methodDef.name == "sum")
		{
			long a = request->readLong();
			long b = request->readLong();
			result->writeLong(a+b);
		}
		else
		{
			arts_fatal("method %s unimplemented", methodDef.name.c_str());
		}
	}
};

/*
 * this program illustrates that you /can/ implement an interface without
 * using the IDL compiler skeleton do to so - this is useful if you want
 * to create a language or object system binding, where you don't know
 * all interfaces that are present at compile time
 *
 * however, it's definitely advanced stuff, and not for daily use ;)
 */
int main()
{
	Dispatcher dispatcher;

	Object obj = Object::_from_base(new HelloDynamic());
	if(obj.isNull())
		arts_fatal("dimpl is no object?");

	HelloBase b = DynamicCast(obj);
	if(b.isNull())
		arts_fatal("can't cast the object to HelloBase");


	Hello h = DynamicCast(obj);
	/*arts_info("%d",obj.isCompatibleWith("Hello"));*/

	if(h.isNull())
		arts_fatal("can't destringify to the object");

	const char *who = getenv("LOGNAME");
	if(!who) who = "stefan";

	arts_info("calling h.hello(\"%s\")", who);
	h.hello(who);

	h.myValue(6);
	arts_info("h.myValue(6), h.myValue()  is %ld",
		h.myValue());
	
	arts_info("h.concat(\"MCOP \",\"is great!\") is \"%s\"",
		h.concat("MCOP ","is great!").c_str());

	/*
	int i,j = 0;
	for(i=0;i<100000;i++)
		j += h.sum(2,4);
	printf("%d\n",j);
	*/

	return 0;
}
