// CDE style LED lamp widget for Qt
// Richard J. Moore 23.11.96
// Email: moorer@cs.man.ac.uk

#include <stdio.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qcolor.h>
#include "kledlamp.h"
#include "kledlamp.moc"

KLedLamp::KLedLamp(QWidget *parent) : QFrame(parent)
{
  // Make sure we're in a sane state
  s= Off;

  // Set the frame style
  setFrameStyle(Sunken | Box);
  setGeometry(0,0,width,height);
}

void KLedLamp::drawContents(QPainter *painter)
{
  QBrush lightBrush(yellow);
  QBrush darkBrush(QColor(60,60,0));
  QPen pen(QColor(40,40,0));
  switch(s) {
  case On:
    painter->setBrush(lightBrush);
	painter->drawRect(1,1,width-2, height-2);
    break;
  case Off:
    painter->setBrush(darkBrush);
	painter->drawRect(1,1,width-2, height-2);
    painter->setPen(pen);
	painter->drawLine(2,2,width-2, 2);
	painter->drawLine(2,height-2,width-2,height-2);
	// Draw verticals
    int i;
	for (i= 2; i < width-1; i+= dx)
	   painter->drawLine(i,2,i,height-2);
    break;
  default:
    fprintf(stderr, "KLedLamp: INVALID State (%d)\n", s);
  }
}





