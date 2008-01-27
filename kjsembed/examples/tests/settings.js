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

var settings = new QSettings("test.ini", QSettings.IniFormat, this);

settings.setValue("Main/value1", 10);
settings.setValue("Main/value2", new QColor("blue") );
settings.setValue("Main/value3", "Text");
settings.setValue("Main/value4", new QPoint(10,10) );
settings.sync();
var keys = settings.allKeys();
for( x in keys )
	println( keys[x] + ":\t" + settings.value(keys[x]));