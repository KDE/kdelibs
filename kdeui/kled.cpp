/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)

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

/*************************************************************************
 * $Id$
 *
 * $Log$
 * Revision 1.7  1999/10/10 13:34:14  mirko
 * First merge with KLedLamp that shows a rectangular LED.
 * It does not yet work reliably.
 *
 * Revision 1.6  1999/03/01 23:34:49  kulow
 * CVS_SILENT ported to Qt 2.0
 *
 * Revision 1.5  1999/02/19 08:52:42  habenich
 * ID und LOG tags included
 *
 *
 *************************************************************************/

#define PAINT_BENCH
#undef PAINT_BENCH

#ifdef PAINT_BENCH
#include <qdatetime.h>
#include <stdio.h>
#endif

#include <qpainter.h>
#include <qcolor.h>
#include <kpixmapeffect.h>
#include "kled.h"

KLed::KLed(const QColor& col, QWidget *parent, const char *name)
  : QWidget( parent, name),
    led_state(On),
    led_look(round)
{
  setColor(col);
  setShape(Circular);
}

KLed::KLed(const QColor& col, KLed::State state, 
	   KLed::Look look, KLed::Shape shape, QWidget *parent, const char *name )
  : QWidget(parent, name),
    led_state(state),
    led_look(look)
{
  setShape(shape);
  setColor(col);
}

void
KLed::paintEvent(QPaintEvent *)
{
  switch(led_shape)
  {
  case Rectangular:
    paintrect();
    break;
  case Circular:
    switch (led_look) 
    {
    case flat  : 
      paintflat(); 
      break;
    case round : 
      paintround(); 
      break;
    case sunken: 
      paintsunken(); 
      break;
    }
  case NoShape:
    break;
  }
}

void
KLed::paintflat()
{
  QPainter p(this);
  int x=this->x(), y=this->y(), w=width(), h=height();

  // round the ellipse 
  if (w > h)
    w = h;
  else if (h > w)
    h = w;

  QColor c;
  
  // draw light grey upper left circle
  c.setRgb(0xCFCFCF);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 45*16, 180*16);
  p.drawArc(x,y,w,h, 45*16, -180*16);

  // draw white upper left circle
  c.setRgb(0xFFFFFF);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 90*16, 90*16);
  p.drawArc(x,y,w,h, 0, -90*16);

  // draw dark grey lower right circle
  c.setRgb(0xA0A0A0);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 45*16, -180*16);
  p.drawArc(x,y,w,h, 45*16, 180*16);

  // draw black lower right circle
  c.setRgb(0x000000);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 0, -90*16);
  p.drawArc(x,y,w,h, 90*16, 90*16);

  // make led smaller for shading
  w-=2; h-=2;
  x++; y++;
  // draw the flat led grounding
  c=current_color;
  p.setPen(c);
  p.setBrush(c);
  p.drawPie(x,y,w,h,0,5760);
}

void
KLed::paintround()
{
  QPainter p(this);
  KPixmap pix;
  QColor c;
  // -----
  c.setRgb(qRed(led_color.rgb())/2, qGreen(led_color.rgb())/2, qBlue(led_color.rgb())/2);
  if(led_state==On)
    {
      KPixmapEffect::gradient(pix, led_color, c, KPixmapEffect::EllipticGradient, 2^24);
    } else {
      KPixmapEffect::gradient(pix, c, led_color, KPixmapEffect::EllipticGradient, 2^24);
    }
}


void
KLed::paintsunken()
{
  QPainter p(this);
  int x=this->x(), y=this->y(), w=width(), h=height();

  // round the ellipse 
  if (w > h)
    w = h;
  else if (h > w)
    h = w;

  // 3 rings of shade
  QRect ring1(x,y,w,h);
  x++,y++;
  w-=2;h-=2;
  QRect ring2(x,y,w,h);
  x++,y++;
  w-=2;h-=2;
  QRect ring3(x,y,w,h);
  x++,y++;
  w-=2;h-=2;

  QColor c;

  // look from upper left
#define ARC_BLACK_RING1 15
#define ARC_BLACK_RING2 25
#define ARC_BLACK_RING3 45
#define ARC_DGRAY_RING1 55
#define ARC_DGRAY_RING2 80
#define ARC_DGRAY_RING3 90
  // look from lower right
#define ARC_LGRAY_RING1 (180-ARC_DGRAY_RING1)
#define ARC_LGRAY_RING2 (180-ARC_DGRAY_RING2)
#define ARC_LGRAY_RING3 (180-ARC_DGRAY_RING3)
#define ARC_WHITE_RING1 80
#define ARC_WHITE_RING2 55
#define ARC_WHITE_RING3 40

  // draw upper left darkgray arc
  c.setRgb(0xA0A0A0);
  p.setPen(c);
  p.drawArc(ring1, (135-ARC_DGRAY_RING1)*16, 2*ARC_DGRAY_RING1*16);
  p.drawArc(ring2, (135-ARC_DGRAY_RING2)*16, 2*ARC_DGRAY_RING2*16);
  p.drawArc(ring3, (135-ARC_DGRAY_RING3)*16, 2*ARC_DGRAY_RING3*16);

  // draw upper left black arc above the darkgrey arc
  c.setRgb(0x000000);
  p.setPen(c);
  p.drawArc(ring1, (135-ARC_BLACK_RING1)*16, 2*ARC_BLACK_RING1*16);
  p.drawArc(ring2, (135-ARC_BLACK_RING2)*16, 2*ARC_BLACK_RING2*16);
  p.drawArc(ring3, (135-ARC_BLACK_RING3)*16, 2*ARC_BLACK_RING3*16);

  // draw lower right light gray arc
  c.setRgb(0xCFCFCF);
  p.setPen(c);
  p.drawArc(ring1, (-45+ARC_LGRAY_RING1)*16, -2*ARC_LGRAY_RING1*16);
  p.drawArc(ring2, (-45+ARC_LGRAY_RING2)*16, -2*ARC_LGRAY_RING2*16);
  p.drawArc(ring3, (-45+ARC_LGRAY_RING3)*16, -2*ARC_LGRAY_RING3*16);

  // draw lower right white arc
  c.setRgb(0xFFFFFF);
  p.setPen(c);
  p.drawArc(ring1, (-45+ARC_WHITE_RING1)*16, -2*ARC_WHITE_RING1*16);
  p.drawArc(ring2, (-45+ARC_WHITE_RING2)*16, -2*ARC_WHITE_RING2*16);
  p.drawArc(ring3, (-45+ARC_WHITE_RING3)*16, -2*ARC_WHITE_RING3*16);

  // draw the flat led grounding
  c=current_color;
  p.setPen(c);
  p.setBrush(c);
  p.drawPie(x,y,w,h,0,5760);

  // shrink the light on 2/3 the size
  // x := x+ ( w - w*2/3)/2
  x+=w/6;
  y+=w/6;
  // w := w*2/3
  w*=2;
  w/=3;
  h=w;

  // now draw the bright spot on the led  
  int light_quote = (100*3/w)+100;
  while (w) {
    c = c.light(light_quote);
    p.setPen( c );
    p.drawEllipse(x,y,w,h);
    w--; h--;
    if (!w)
      break;
    p.drawEllipse(x,y,w,h);
    w--; h--;
    if (!w)
      break;
    p.drawEllipse(x,y,w,h);
    x++; y++; w--;h--;
  }
}

void 
KLed::paintrect()
{
  QPainter painter(this);
  QBrush lightBrush(led_color);
  QBrush darkBrush(led_color.dark());
  QPen pen(led_color.dark());
  int w=width();
  int h=height();
  // -----
  switch(led_shape) 
  {
  case On:
    painter.setBrush(lightBrush);
    painter.drawRect(0, 0, w, h);
    break;
  case Off:
    painter.setBrush(darkBrush);
    painter.drawRect(0, 0, w, h);
    painter.setPen(pen);
    painter.drawLine(0, 0, w, 0);
    painter.drawLine(0, h-1, w, h-1);
    // Draw verticals
    int i;
    for(i=0; i < w; i+= 4 /* dx */)
      painter.drawLine(i, 1, i, h-1);
    break;
  default: break;
  } 
}

KLed::State
KLed::state() const
{ 
  return led_state; 
}

const QColor
KLed::color() const
{ 
  return led_color; 
}

KLed::Look
KLed::look() const
{ 
  return led_look; 
}

void
KLed::setState( State state )
{
  if (led_state != state) 
    {
      led_state = state;
      setColor(led_color);
    }
}

void
KLed::toggleState()
{
  led_state = (State)!led_state;
  // setColor(led_color);
  setColor(led_state==On ? Qt::green : Qt::darkGreen);
}

void
KLed::setShape(KLed::Shape s)
{
  if(led_shape!=s)
    {
      led_shape=s;
      repaint(false);
    }
}

void
KLed::setColor(const QColor& col)
{ 
  if(led_color!=col) 
    {
      led_color=col;
      current_color=led_state 
	? led_color.light() 
	: led_color.dark();
      update(); 
    }
}

void
KLed::setLook( Look look )
{
  if(led_look!=look) 
    {
      led_look = look;
      update();
    }
}

void
KLed::toggle()
{ 
  toggleState(); 
}

void
KLed::on()
{ 
  setState(On); 
}

void
KLed::off()
{ 
  setState(Off); 
}

#include "kled.moc"

