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


var label = new QWidget("QLabel", this );
label.width = 0;
label.height = 0;
label.onResizeEvent = function ( ev )
{
  println("Size: " + ev.size.width() + "x" + ev.size.height() );
  this.width = ev.size.width();
  this.height = ev.size.height();
}

label.onPaintEvent = function ( ev )
{
  try {
    var painter = new QPainter(this);
    if (painter.begin( this ) )
    {
      painter.drawLine( 0,0,this.width,this.height );
      painter.drawLine( this.width,0, 0, this.height );
      painter.end();
    }
    else
      println("Failed to paint " + this );

  } catch ( error ) {
    println( "Error painting: " + error );
  }
}

label.size = new QSize(100,100);
label.show();
exec();
