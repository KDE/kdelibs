#!/bin/env kjscmd
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


s = sum(10, sum(20, 30));
debug("s = " + s);
debug("i = " + i);

var a = new Array(11, 22, 33, 44);
a.length = 2;
a[4] = 'apple';

for(var i = 0; i != a.length; i++)
	debug("a[" + i + "] = " + a[i]);

var b = new Boolean(1==1);
b.toString=Object.prototype.toString;
debug("b = " + b.toString());

// regular expression
rx = /b*c/;
debug(rx.exec("abbbcd"));


function sum(a, b)
{
	print("inside sum()");
	i = i + 1;
	debug(a);
	debug(b);
	return a + b;
}

Test.print("Test Static");

var testObj = new TestObject(2);
testObj.print( Application.args );
testObj.print( Application.args[0] );

var testPointer = new TestPointer("Test");
testPointer.print( Application.args );
testPointer.print( Application.args[0] );

Application.pointer.print("I made it");

debug( "Object type: " + Application.type() );
debug( "Appname: " + Application.applicationName );
Application.applicationName = "Test Application";
debug( "New Appname: " + Application.applicationName );
debug( "Appname objectName: " + Application.objectName );
debug( "Application quitOnLastWindowClosed: " +  Application.quitOnLastWindowClosed );

debug( "Call slot" );
val = Application.aboutQt();
debug( "Called slot" );

for( y in Application )
	print("Application method: " + y)

for( y in this)
	print("Global: " + y );

for( y in testPointer )
	print("TestPointer: " + y);
