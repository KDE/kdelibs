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
#include "kled.h"
#include "kled.h" // whats this use for ?? (jha)

const QRgb
  KLed::lightcolor[]={ 0x00FFFF, 0x00A5FF, 0x0000FF, 0x00FF00, 0xFF0000 },
  KLed::darkcolor[] ={ 0x00B0B0, 0x0064A2, 0x000080, 0x008000, 0x800000 };

KLed::KLed(Color ledcolor, QWidget *parent, const char *name)
  : QWidget( parent, name),
    led_state( On ),
    led_color( ledcolor ),
    led_look( round )
{
  current_color=(led_state ? lightcolor[led_color] : darkcolor[led_color]);
}

KLed::KLed(KLed::Color ledcolor, KLed::State state, KLed::Look look, QWidget *parent, const char *name )
  : QWidget( parent, name),
    led_state( state ),
    led_color( ledcolor ),
    led_look( look )
{
  current_color=(led_state ? lightcolor[led_color] : darkcolor[led_color]);
}

void
KLed::paintEvent(QPaintEvent *)
{
#ifdef PAINT_BENCH
  const int rounds = 1000;
  QTime t;
  t.start();
  for (int i=0; i<rounds; i++) 
#endif
  switch (led_look) {
  case flat  : paintflat(); break;
  case round : paintround(); break;
  case sunken: paintsunken(); break;
  }
#ifdef PAINT_BENCH
  int ready = t.elapsed();
  debug("elapsed: %d msec. for %d rounds", ready, rounds);
#endif
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

  //  QRect ring1(x,y,w,h);
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
  c.setRgb(current_color);
  p.setPen(c);
  p.setBrush(c);
  p.drawPie(x,y,w,h,0,5760);
}

void
KLed::paintround()
{
  QPainter p(this);
  int x=this->x(), y=this->y(), w=width(), h=height();

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
  c.setRgb(current_color);
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
  c.setRgb(current_color);
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
#include "kled.moc"

