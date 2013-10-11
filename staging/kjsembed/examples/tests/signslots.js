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


var slider1 = new QWidget("QSlider", this);
var slider2 = new QWidget("QSlider", this);

slider1.show();
slider2.show();

connect( slider1, "valueChanged(int)", slider2, "setValue(int)");
connect( slider2, "valueChanged(int)", slider1, "setValue(int)");
if( slider1.isWidgetType() )
{
	print("This is a widget");
}

exec();


