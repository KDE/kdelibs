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

#include <string>
#include <list>
#include <iostream>
#include <stdio.h>

const char *noTest = "noTest";
const char *noClass = "noClass";
const char *currentClass = noClass;
const char *currentTest = noTest;

#define testCaption()													\
	 	fprintf (stderr,"\n  %s::%s (%s:%d)\n",					 		\
				currentClass, currentTest, __FILE__, __LINE__);

#define testAssert(expr) 											  \
     if (!(expr)) { 												  \
	 	testCaption();												  \
	 	fprintf (stderr,"   => test assertion failed: (%s)\n", #expr);  \
		exit(1); \
	 }

template<typename T1, typename T2>
inline void testEqualsCheck(const T1& expected, const T2& got,
		 const char *file, int line, const char *expr, const char *expr2)
{
	if(! (static_cast<T2>(expected) == got) ) {
 		cerr << endl

			 << "  " << currentClass << "::" << currentTest
			 << " (" << file << ":" << line << ")" << endl

 			 << "   => test assertion failed:"
			 << " (" << expr << " == " << expr2 << ")" << endl

 			 << "   => expected '" << expected << "'" << " got "
		 	 << "'" << got << "'." << endl;
		exit(1);
	}
}

#define testEquals(expr, expr2) 										\
	testEqualsCheck(expr, expr2, __FILE__, __LINE__, #expr, #expr2)
	
#define TEST(func)													\
	struct t ## func : TestClass 									\
	{																\
		t ## func() {												\
			if(!tests) tests = new list<TestClass *>; 				\
			tests->push_back(this);									\
		}															\
		void invoke() {												\
			currentTest = #func;									\
			instance->func();										\
			currentTest = noTest;									\
		}															\
	}	i ## func;													\
	void func()

struct TestClass {
	virtual void invoke() = 0;
};

struct TestCase {
	virtual void setUp() { };
	virtual void tearDown() { };
	virtual ~TestCase() { };
};

#define TESTCASE(name)												\
	static list<TestClass *> *tests;								\
	static name *instance;											\
	name () {														\
		instance = this;											\
	}																\
	void testAll() {												\
		currentClass = #name;										\
		list<TestClass *>::iterator i;								\
		for(i = tests->begin(); i != tests->end(); i++)				\
		{															\
			setUp();												\
			(*i)->invoke();											\
			tearDown();												\
		}															\
		currentClass = noClass;										\
	}																\
	int count() {													\
		return tests->size();										\
	}

#define TESTMAINFUNC(name,func)										\
	name *name::instance = 0;										\
	list<TestClass *> *name::tests = 0;								\
	int func()														\
	{																\
		name tb;													\
		fprintf(stderr,"%-20s: %5d test methods - ",				\
										#name,tb.count());			\
		tb.testAll();												\
		return 0;													\
	}

#define TESTMAIN(name) TESTMAINFUNC(name,main)
