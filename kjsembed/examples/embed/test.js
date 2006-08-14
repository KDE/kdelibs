/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Execute static method of an object type.
Line.print("This is from a static testPointer.");

// Create a Pointer based object.
var testPointer = new Line("Test");
println( "Test Pointer name '" + testPointer.name() +"'" );
testPointer.print( "This is from a testPointer instance" );
testPointer.print( "This is again from a testPointer instance" );

// Print a static member of line
println("Line.END " + Line.END );

// Create a Variant based object
var testObject = new TestObject();
testObject.print("Test object line 10");
testObject.print("Test object line 11");
testObject.print("Test object line 12");

// Access a static QObject
println("Application name " + Application.objectName );

// Access a custom static C++ object
Test.print("C++ object Line 1");
Test.print("C++ object Line 2");