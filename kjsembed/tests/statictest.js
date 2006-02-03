#!/bin/env kjscmd

print("TestPointer length: " + TestPointer.length);

for( x in TestPointer )
	print( x );

TestPointer.print("Static print" + Application.args[0]);

var testPointer = new TestPointer("Test");
testPointer.print( Application.args );
testPointer.print( Application.args[0] );
