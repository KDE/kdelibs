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
 * Revision 1.13  2000/05/08 19:38:49  sschiman
 * Calling setColor before setting up the private data is a bad idea ;-)
 *
 * Revision 1.12  2000/05/07 09:49:57  habenich
 * provided method to set the factor to dark the LED
 * precalculated the dark color, is just retrieved at paint events
 *
 * Revision 1.11  2000/04/09 16:08:33  habenich
 * fixed nasty bug #70 disappearing led
 * reenabled flat and raised led painting
 *
 * Revision 1.10  1999/12/25 17:12:18  mirko
 * Modified Look "round" to "raised", as the others are flat and
 * sunken. All enums start with uppercase letters now to fit the overall
 * KDE style.
 * Implemented raised rectangluar look.
 * --Mirko.
 *
 * Revision 1.9  1999/11/12 21:17:09  antlarr
 * Fixed some bugs.
 * Added the possibility to draw a sunk rectangle as the "old" KLedLamp did.
 *
 * Revision 1.9  1999/11/11 16:08:15  antlarr
 * Fixed some bugs.
 * Added the possibility to draw a sunk rectangle as the "old" KLedLamp did. 
 *
 * Revision 1.8  1999/11/01 22:03:15  dmuell
 * fixing all kinds of compile warnings
 * (unused var, unused argument etc)
 *
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
#include <kapp.h>
#include <kpixmapeffect.h>
#include "kled.h"



class KLed::KLedPrivate 
{
  friend KLed; 

  int dark_factor;
  QColor offcolor;
};



KLed::KLed(QWidget *parent, const char *name)
  : QWidget( parent, name),
    led_state(On),
    led_look(Raised),
    led_shape(Circular)
{
  QColor col(green);
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);

  setColor(col);
}


KLed::KLed(const QColor& col, QWidget *parent, const char *name)
  : QWidget( parent, name),
    led_state(On),
    led_look(Raised),
    led_shape(Circular)
{  
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);

  setColor(col);
  //setShape(Circular);
}

KLed::KLed(const QColor& col, KLed::State state, 
	   KLed::Look look, KLed::Shape shape, QWidget *parent, const char *name )
  : QWidget(parent, name),
    led_state(state),
    led_look(look),
    led_shape(shape)
{  
  d = new KLed::KLedPrivate;
  d->dark_factor = 300;
  d->offcolor = col.dark(300);

  //setShape(shape);
  setColor(col);
}


KLed::~KLed()
{
  delete d;
}

void
KLed::paintEvent(QPaintEvent *)
{
  switch(led_shape)
    {
    case Rectangular:
      switch (led_look) 
	{
	case Sunken :
	  paintRectFrame(false); 
	  break;
	case Raised :
	  paintRectFrame(true);
	  break;
	case Flat   :
	  paintRect();
	  break;
	default  :
	  qWarning("%s: in class KLed: no KLed::Look set",kapp->argv()[0]);
	}
      break;
    case Circular:
      switch (led_look) 
	{
	case Flat   :
	  paintFlat();
	  break;
	case Raised :
	  paintRound();
	  break;
	case Sunken :
	  paintSunken();
	  break;
	default: 
	  qWarning("%s: in class KLed: no KLed::Look set",kapp->argv()[0]);
	}
      break;
    default:
      qWarning("%s: in class KLed: no KLed::Shape set",kapp->argv()[0]);
      break;
    }
}

void
KLed::paintFlat() // paint a ROUND FLAT led lamp
{
  QPainter p(this);
  int w=width(), h=height();

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
  p.drawArc(0,0,w,h, 45*16, -180*16);

  // draw white upper left circle
  c.setRgb(0xFFFFFF);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 90*16, 90*16);
  p.drawArc(0,0,w,h, 0, -90*16);

  // draw dark grey lower right circle
  c.setRgb(0xA0A0A0);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 45*16, -180*16);
  p.drawArc(0,0,w,h, 45*16, 180*16);

  // draw black lower right circle
  c.setRgb(0x000000);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 0, -90*16);
  p.drawArc(0,0,w,h, 90*16, 90*16);

  // make led smaller for shading
  w-=2; h-=2;

  // draw the flat led grounding
  c = (led_state==On) ? led_color : d->offcolor;
  p.setPen(c);
  p.setBrush(c);
  p.drawPie(1,1,w,h,0,5760);
}

void
KLed::paintRound() // paint a ROUND RAISED led lamp
{
  QPainter p(this);
  int x, y, w=width(), h=height();

  // round the ellipse 
  if (w > h)
    w = h;
  else if (h > w)
    h = w;

  //  QRect ring1(x,y,w,h);
  QColor c;

  // draw light grey upper left circle
  c.setRgb(0xCFCFCF);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 45*16, 180*16);
  p.drawArc(0,0,w,h, 45*16, -180*16);

  // draw white upper left circle
  c.setRgb(0xFFFFFF);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 90*16, 90*16);
  p.drawArc(0,0,w,h, 0, -90*16);

  // draw dark grey lower right circle
  c.setRgb(0xA0A0A0);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 45*16, -180*16);
  p.drawArc(0,0,w,h, 45*16, 180*16);

  // draw black lower right circle
  c.setRgb(0x000000);
  p.setPen(c);
  //  p.drawArc(x,y,w,h, 0, -90*16);
  p.drawArc(0,0,w,h, 90*16, 90*16);

  // make led smaller for shading
  w-=2; h-=2;
  //x++; y++;
  // draw the flat led grounding
  c = ( led_state ) ? led_color : d->offcolor;

  p.setPen(c);
  p.setBrush(c);
  p.drawPie(1,1,w,h,0,5760);

  // shrink the light on 2/3 the size
  // x := x+ ( w - w*2/3)/2
  x=w/6 +1;
  y=w/6 +1;
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

  /* 
   * ?? what's this use for ??
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
  */
}


void
KLed::paintSunken() // paint a ROUND SUNKEN led lamp
{
  QPainter p(this);
  //int x=this->x(), y=this->y(), w=width(), h=height();
  int x=0, y=0, w=width(), h=height();

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
  c= (led_state==On) ? led_color : d->offcolor;
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
KLed::paintRect()
{
  QPainter painter(this);
  QBrush lightBrush(led_color);
  QBrush darkBrush(d->offcolor);
  QPen pen(led_color.dark(300));
  int w=width();
  int h=height();
  // -----
  switch(led_state) 
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

void 
KLed::paintRectFrame(bool raised)
{
  QPainter painter(this);
  QBrush lightBrush(led_color);
  QBrush darkBrush(d->offcolor);
  int w=width();
  int h=height();
  QColor black=Qt::black;
  QColor white=Qt::white;
  // -----
  if(raised)
    {
      painter.setPen(white);
      painter.drawLine(0, 0, 0, h-1);
      painter.drawLine(1, 0, w-1, 0);
      painter.setPen(black);
      painter.drawLine(1, h-1, w-1, h-1);
      painter.drawLine(w-1, 1, w-1, h-1);
      painter.fillRect(1, 1, w-2, h-2,
       		       (led_state==On)? lightBrush : darkBrush);
    } else { 
      painter.setPen(black);
      painter.drawRect(0,0,w,h);
      painter.drawRect(0,0,w-1,h-1);
      painter.setPen(white);
      painter.drawRect(1,1,w-1,h-1);
      painter.fillRect(2, 2, w-4, h-4,
		       (led_state==On)? lightBrush : darkBrush);
    }
}

KLed::State
KLed::state() const
{ 
  return led_state; 
}

KLed::Shape
KLed::shape() const
{
  return led_shape;
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
      update();
    }
}

void
KLed::toggleState()
{
  led_state = (State)!led_state;
  // setColor(led_color);
  update();
}

void
KLed::setShape(KLed::Shape s)
{
  if(led_shape!=s)
    {
      led_shape = s;
      update();
    }
}

void
KLed::setColor(const QColor& col)
{ 
  if(led_color!=col) {
    led_color = col;
    d->offcolor = col.dark(d->dark_factor);
    update(); 
  }
}

void
KLed::setDarkFactor(int darkfactor)
{
  if (d->dark_factor != darkfactor) {
    d->dark_factor = darkfactor;
    d->offcolor = led_color.dark(darkfactor);
    update();
  }
}

int 
KLed::darkFactor() const
{
  return d->dark_factor;
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
