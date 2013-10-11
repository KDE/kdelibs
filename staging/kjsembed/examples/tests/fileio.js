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

var file = new File("test.txt");
if( file.open( File.WriteOnly ) )
{
	for( var x = 0; x < 100; ++x)
	{
		var line = "";
		for( var y = 0; y < 80; ++y)
			line += "#";
		file.writeln(line);
	}
	file.close();
}

if( file.open( File.ReadOnly ) )
{
	var lines = 0;
	while( !file.atEnd() )
	{
		var line = file.readln();
		++lines;
	}
	println( lines + " lines found");
}
