// Execute static method of an object type.
Line.print("This is from a static testPointer.");

// Create a Pointer based object.
var testPointer = new Line("Test");
println( "Test Pointer name '" + testPointer.name() +"'" );
testPointer.print( "This is from a testPointer instance" );
testPointer.print( "This is again from a testPointer instance" );

// Create a Variant based object
var testObject = new TestObject(Line.END);
testObject.print("Test object line 10");
testObject.print("Test object line 11");
testObject.print("Test object line 12");

// Access a  custom C++ object
Test.print("C++ object");