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

println('Start brush test...');

var pixmap = new QPixmap(120, 120);
var color = new QColor("blue");

// Test 1 arg constructor
var brush1 = new QBrush(pixmap);

if(brush1.texture().width() == 120 && brush1.texture().height() == 120)
    println("Test1: OK");
else
    println("Wrong size: "+ brush1.texture().width()+"x"+brush1.texture().height());

// QBrush ( const QColor & color, const QPixmap & pixmap )
var brush2 = new QBrush(color, pixmap);
if(brush2.texture().height() == 120 && brush2.texture().width() == 120)
    println("Test2: OK");
else
    println("Wrong size: "+ brush2.texture().width()+"x"+brush2.texture().height());

if(brush2.color().blue() == 255)
    println("Test3: OK");
else
    println("Wrong color");

// QBrush ( const QColor & color, Qt::BrushStyle style = Qt::SolidPattern )
var brush3 = new QBrush(color, 11);

