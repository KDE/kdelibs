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

var arr = new Array();
var frame = new QWidget("QFrame", this );
for( var idx = 0; idx < 500; ++idx )
{
	arr[idx] = new QWidget("QLabel", 0);
	arr[idx].text = "foo " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	//println( arr[idx].text );
	arr[idx] = new QWidget("QLabel", frame);
	arr[idx].text = "bar " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	//println( arr[idx].text );
	arr[idx] = new QWidget("QLabel", frame);
	arr[idx].text = "baz " + idx;
}

for( var idx = 0; idx < 500; ++idx )
{
	println( arr[idx].text );
}

