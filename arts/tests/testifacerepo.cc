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

#include <algorithm>
#include "core.h"
#include "test.h"

using namespace Arts;
using namespace std;

struct TestInterfaceRepo : public TestCase
{
	TESTCASE(TestInterfaceRepo);

	Arts::Dispatcher dispatcher;
	Arts::InterfaceRepo interfaceRepo;

	void setUp() {
		interfaceRepo = Arts::Dispatcher::the()->interfaceRepo();
	}
	TEST(queryInterface) {
		InterfaceDef def = interfaceRepo.queryInterface("Arts::InterfaceRepo");

		testEquals("Arts::InterfaceRepo",def.name);
	}
	TEST(queryType) {
		TypeDef def = interfaceRepo.queryType("Arts::MethodDef");

		testEquals("Arts::MethodDef",def.name);
	}
	TEST(queryEnum) {
		EnumDef def = interfaceRepo.queryEnum("Arts::AttributeType");

		testEquals("Arts::AttributeType",def.name);
	}
	bool contains(vector<string>& sequence, const string& element)
	{
		vector<string>::iterator it;
		it = find(sequence.begin(),sequence.end(), element);
		return (it != sequence.end());
	}
	TEST(queryInterfaces) {
		vector<string> *interfaces = interfaceRepo.queryInterfaces();

		testAssert(contains(*interfaces,"Arts::InterfaceRepo"));
		testAssert(!contains(*interfaces,"Arts::MethodDef"));
		delete interfaces;
	}
	TEST(queryTypes) {
		vector<string> *types = interfaceRepo.queryTypes();

		testAssert(contains(*types,"Arts::MethodDef"));
		testAssert(!contains(*types,"Arts::AttributeType"));
		delete types;
	}
	TEST(queryEnums) {
		vector<string> *enums = interfaceRepo.queryEnums();

		testAssert(contains(*enums,"Arts::AttributeType"));
		testAssert(!contains(*enums,"Arts::InterfaceRepo"));
		delete enums;
	}
};

TESTMAIN(TestInterfaceRepo);
