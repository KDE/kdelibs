/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/  


#include "kdbtn.moc"

KDirectionButton::KDirectionButton(QWidget *parent, const char *name)
 : QButton(parent, name)
{
  initMetaObject();
  direct = RightArrow;
}

KDirectionButton::KDirectionButton(ArrowType d, QWidget *parent, const char *name)
 : QButton(parent, name)
{
  initMetaObject();
  direct = d;
}

KDirectionButton::~KDirectionButton()
{
  //debug("KTabButton - destructor");

  //debug("KTabBar - destructor done");
}

void KDirectionButton::drawButton( QPainter *p)
{
  // This is only tested in Motif style. May give poor results in Win9x style.

  int style = kapp->applicationStyle;
  int h = height(), w = width(), x, y, cx, cy;

  switch (direct)
  {
    case LeftArrow:
      x = 2;
      y = 2;
      cx = w-4;
      cy = h-4;
      break;

    case RightArrow:
      x = 2;
      y = 3;
      cx = w-4;
      cy = h-4;
      break;

    case UpArrow:
      if(style == MotifStyle)
      {
        x = 4;
        y = 3;
        cx = w-4;
        cy = h-4;
      }
      else
      {
        x = 2;
        y = 1;
        cx = w-4;
        cy = h-4;
      }
      break;

    case DownArrow:
      if(style == MotifStyle)
      {
        x = 0;
        y = 3;
        cx = w-4;
        cy = h-3;
      }
      else
      {
        x = 2;
        y = 1;
        cx = w-4;
        cy = h-3;
      }
      break;

    default:
      x = 3;
      y = 3;
      cx = w-4;
      cy = h-4;
      break;
  }

  QColorGroup g = QWidget::colorGroup();

  qDrawArrow( p, direct, (GUIStyle)style, isDown(), x, y, cx, cy, g, true );
}

KTabButton::KTabButton(QWidget *parent, const char *name)
 : KDirectionButton(parent, name)
{
  initMetaObject();
}

KTabButton::KTabButton(ArrowType d, QWidget *parent, const char *name)
 : KDirectionButton(d, parent, name)
{
  initMetaObject();
}

void KTabButton::drawButton( QPainter *p)
{
  int h = height(), w = width();

  KDirectionButton::drawButton(p);

  QPen pen( white, 1 );
  p->setPen( pen );

  // left outer
  p->drawLine( 0, h, 0, 1);

  // top outer
  p->drawLine( 1, 0, w-1, 0);
  
  pen.setColor( black );
  p->setPen( pen );

  // right outer
  p->drawLine( w-1 , 1, w-1, h);
}


