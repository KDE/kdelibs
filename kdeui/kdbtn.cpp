/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@kde.org)

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

#include <kapp.h>
#include <qbutton.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qdrawutil.h>


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
  // I abandoned qDrawArrow(...) 'cause the result was totally unpredictable :-(

  int style = kapp->applicationStyle;
  int wm = width()/2, hm = height()/2;
  QColorGroup g = QWidget::colorGroup();
  QPen lightPen, darkPen;
  QColor lightColor, darkColor;

  if(isEnabled())
  {
    lightColor = g.midlight();
    darkColor = g.dark();
  }
  else
  {
    lightColor = g.midlight();
    darkColor = g.mid();
  }

  lightPen.setColor(lightColor);
  darkPen.setColor(darkColor);

  switch (direct)
  {
    case LeftArrow:
      if(style == MotifStyle)
      {
        if(isDown())
        {
          p->setPen(darkPen);
          p->drawLine(2, hm, (2*wm)-4, 2);
          p->setPen(lightPen);
          p->drawLine((2*wm)-4, 2, (2*wm)-4, (2*hm)-4);
          p->drawLine((2*wm)-4, (2*hm)-4, 2, hm);
        }
        else
        {
          p->setPen(lightPen);
          p->drawLine(2, hm, (2*wm)-4, 2);
          p->setPen(darkPen);
          p->drawLine((2*wm)-4, 2, (2*wm)-4, (2*hm)-4);
          p->drawLine((2*wm)-4, (2*hm)-4, 2, hm);
        }
      }
      else
      {
        p->setBrush(darkColor);
        p->setPen(darkPen);
        QPointArray a(3);
        if(isDown())
        {
          a.setPoint(0, (2*wm)-2, 2);
          a.setPoint(1, (2*wm)-2, (2*hm)-4);
          a.setPoint(2, 4, hm);
          p->drawPolygon(a);
        }
        else
        {
          a.setPoint(0, (2*wm)-4, 2);
          a.setPoint(1, (2*wm)-4, (2*hm)-4);
          a.setPoint(2, 2, hm);
          p->drawPolygon(a);
        }
      }
      break;

    case RightArrow:
      if(style == MotifStyle)
      {
        if(isDown())
        {
          p->setPen(darkPen);
          p->drawLine(2, (2*hm)-4, 2, 2);
          p->drawLine(2, 2, (2*wm)-4, hm);
          p->setPen(lightPen);
          p->drawLine((2*wm)-4, hm, 2, (2*hm)-4);
        }
        else
        {
          p->setPen(lightPen);
          p->drawLine(2, (2*hm)-4, 2, 2);
          p->drawLine(2, 2, (2*wm)-4, hm);
          p->setPen(darkPen);
          p->drawLine((2*wm)-4, hm, 2, (2*hm)-4);
        }
      }
      else
      {
        p->setBrush(darkColor);
        p->setPen(darkPen);
        QPointArray a(3);
        if(isDown())
        {
          a.setPoint(0, 0, 2);
          a.setPoint(1, 0, (2*hm)-4);
          a.setPoint(2, (2*wm)-6, hm);
          p->drawPolygon(a);
        }
        else
        {
          a.setPoint(0, 2, 2);
          a.setPoint(1, 2, (2*hm)-4);
          a.setPoint(2, (2*wm)-4, hm);
          p->drawPolygon(a);
        }
      }
      break;

    case UpArrow:
      if(style == MotifStyle)
      {
        if(isDown())
        {
          p->setPen(lightPen);
          p->drawLine(2, (2*hm)-4, (2*wm)-4, (2*hm)-4);
          p->drawLine((2*wm)-4, (2*hm)-4, wm, 2);
          p->setPen(darkPen);
          p->drawLine(wm, 2, 2, (2*hm)-4);
        }
        else
        {
          p->setPen(darkPen);
          p->drawLine(2, (2*hm)-4, (2*wm)-4, (2*hm)-4);
          p->drawLine((2*wm)-4, (2*hm)-4, wm, 2);
          p->setPen(lightPen);
          p->drawLine(wm, 2, 2, (2*hm)-4);
        }
      }
      else
      {
        p->setBrush(darkColor);
        p->setPen(darkPen);
        QPointArray a(3);
        if(isDown())
        {
          a.setPoint(0, 2, (2*hm)-2);
          a.setPoint(1, (2*wm)-4, (2*hm)-2);
          a.setPoint(2, wm, 4);
          p->drawPolygon(a);
        }
        else
        {
          a.setPoint(0, 2, (2*hm)-4);
          a.setPoint(1, (2*wm)-4, (2*hm)-4);
          a.setPoint(2, wm, 2);
          p->drawPolygon(a);
        }
      }
      break;

    case DownArrow:
      if(style == MotifStyle)
      {
        if(isDown())
        {
          p->setPen(darkPen);
          p->drawLine(wm, (2*hm)-4, 2, 2);
          p->drawLine(2, 2, (2*wm)-4, 2);
          p->setPen(lightPen);
          p->drawLine((2*wm)-4, 2, wm, (2*hm)-4);
        }
        else
        {
          p->setPen(lightPen);
          p->drawLine(wm, (2*hm)-4, 2, 2);
          p->drawLine(2, 2, (2*wm)-4, 2);
          p->setPen(darkPen);
          p->drawLine((2*wm)-4, 2, wm, (2*hm)-4);
        }
      }
      else
      {
        p->setBrush(darkColor);
        p->setPen(darkPen);
        QPointArray a(3);
        if(isDown())
        {
          a.setPoint(0, 2, 0);
          a.setPoint(1, (2*wm)-4, 0);
          a.setPoint(2, wm, (2*hm)-6);
          p->drawPolygon(a);
        }
        else
        {
          a.setPoint(0, 2, 2);
          a.setPoint(1, (2*wm)-4, 2);
          a.setPoint(2, wm, (2*hm)-4);
          p->drawPolygon(a);
        }
      }
      break;

    default:
#if defined(CHECK_RANGE)
      warning( "KDirectionButton: Requested Arrow style not supported" );
#endif
      break;
  }
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


