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

// Create the loader
var loader = new QUiLoader();
println( "loader value: " + loader );
for( y in loader ) println("loader property: " + y);

// create the containing frame and it's layout
var box = loader.createWidget("QFrame", 0 );
var layout = loader.createLayout("QVBoxLayout", box);

// create and add the child labels
for ( var x = 0; x < 10; ++x)
{
	var lab = loader.createWidget("QLabel", box);
	lab.text = "This is a test of box " + x;
	layout.addWidget(lab);
}

// adjust the box size and show it
box.adjustSize();

// display layout info
println( "layout value: " + layout );
for( l in layout ) println("layout property: " + l);

// display box info
println( "box value: " + box );
for( y in box ) println("box property: " + y);

// show the base box
box.show();

// give the user time to see it
exec()
