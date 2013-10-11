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

// create the loader
var loader = new QUiLoader();

// display loader info
println( "loader value: " + loader );
for( y in loader ) println("loader property: " + y);

// get and display the UILoader's pluginPaths
paths = loader.pluginPaths();
println( "pluginPath value:" + paths );
println( "pluginPath len:" + paths.length );
for ( var x = 0; x < paths.length; x++ ) 
	println( "path " + x + ": " + paths[x] );

// load a UI from test.ui
var widget = loader.load("test.ui");
println( "widget value: " + widget );

// iterate through the base widgets properties
for( y in widget ) println("widget property: " + y);

// show it
widget.show();

// give the user time to see it
exec();
