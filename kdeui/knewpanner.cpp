// $Id$

/* This file is part of the KDE libraries
    Copyright (C) 1997 Richard Moore (moorer@cs.man.ac.uk)

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


#include "knewpanner.moc"
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KNewPanner::KNewPanner(QWidget *parent, const char *name,
			   Orientation orient, Units units, int pos)
    : QWidget(parent, name)
{
    orientation= orient;
    currentunits= units;
    position= pos;
    initialised= false;
    showlabels= false;
    startHeight= 0;

    label0= 0;
    label1= 0;
}

KNewPanner::~KNewPanner()
{
}

void KNewPanner::activate(QWidget *c0, QWidget *c1)
{
    int minx, maxx, miny, maxy, pos;

    child0= c0;
    child1= c1;

    // Set the minimum and maximum sizes
    if (orientation == Horizontal) {
	miny= c0->minimumSize().height() + c1->minimumSize().height()+4;
	maxy= c0->maximumSize().height() + c1->maximumSize().height()+4;
	minx= (c0->minimumSize().width() > c1->minimumSize().width())
	    ? c0->minimumSize().width() : c1->minimumSize().width();
	maxx= (c0->maximumSize().width() > c1->maximumSize().width())
	    ? c0->maximumSize().width() : c1->maximumSize().width();

	miny= (miny > 4) ? miny : 4;
	maxy= (maxy < 2000) ? maxy : 2000;
	minx= (minx > 2) ? minx : 2;
	maxx= (maxx < 2000) ? maxx : 2000;

	setMinimumSize(minx, miny);
	setMaximumSize(maxx, maxy);
    }
    else {
	minx= c0->minimumSize().width() + c1->minimumSize().width()+4;
	maxx= c0->maximumSize().width() + c1->maximumSize().width()+4;
	miny= (c0->minimumSize().height() > c1->minimumSize().height())
	    ? c0->minimumSize().height() : c1->minimumSize().height();
	maxy= (c0->maximumSize().height() > c1->maximumSize().height())
	    ? c0->maximumSize().height() : c1->maximumSize().height();

	minx= (minx > 4) ? minx : 4;
	maxx= (maxx < 2000) ? maxx : 2000;
	miny= (miny > 2) ? miny : 2;
	maxy= (maxy < 2000) ? maxy : 2000;

	setMinimumSize(minx, miny);
	setMaximumSize(maxx, maxy);
    }

    divider= new QFrame(this, "pannerdivider");
    divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
    divider->setLineWidth(1);
    divider->raise();

    if (orientation == Horizontal)
        divider->setCursor(QCursor(sizeVerCursor));
    else
        divider->setCursor(QCursor(sizeHorCursor));

    divider->installEventFilter(this);

    initialised= true;

    pos= position;
    position=0;

    setSeparatorPos(pos);
}

void KNewPanner::deactivate()
{
  delete divider;
  initialised= false;
}

void KNewPanner::setLabels(const char *text0, const char *text1)
{
   if (label0 == 0) {
      label0= new QLabel(this, "label0");
      label0->setFrameStyle(QFrame::Sunken | QFrame::Panel);
   }
   if (label1 == 0) {
      label1= new QLabel(this, "label1");
      label1->setFrameStyle(QFrame::Sunken | QFrame::Panel);
   }

   label0->setText(text0);
   label1->setText(text1);
   label0->adjustSize();
   label1->adjustSize();

   if (!showlabels) {
      label0->hide();
      label1->hide();
   }
      
   if (showlabels && initialised) {
      resizeEvent(0);
   }
}

void KNewPanner::showLabels(bool newval)
{
  if (orientation == Vertical)
    showlabels= newval;

   if ((label0 == 0) || (label1 == 0))
     setLabels("", "");

   if (showlabels) {
      label0->show();
      label1->show();
   }
   else {
      label0->hide();
      label1->hide();
   }

  if (initialised && showlabels) {
     label0->adjustSize();
     label1->adjustSize();
  }
  if (initialised)
     resizeEvent(0);
}

int KNewPanner::seperatorPos()
{
  fprintf(stderr,
	  "seperatorPos() is deprecated, use separatorPos() insead.");
  return separatorPos();
}

void KNewPanner::setSeperatorPos(int pos)
{
  fprintf(stderr,
	  "setSeparatorPos() is deprecated, use setSeparatorPos() insead.");
  setSeparatorPos(pos);
}

int KNewPanner::absSeperatorPos()
{
  fprintf(stderr,
	  "absSeparatorPos() is deprecated, use absSeparatorPos() insead.");
  return absSeparatorPos();
}

void KNewPanner::setAbsSeperatorPos(int pos, bool do_resize)
{
  fprintf(stderr,
	  "setAbsSeparatorPos() is deprecated, use setAbsSeparatorPos() insead.");
  setAbsSeparatorPos(pos, do_resize);
}

int KNewPanner::separatorPos()
{
  return position;
}

void KNewPanner::setSeparatorPos(int pos)
{
  position = pos;
}

void KNewPanner::setAbsSeparatorPos(int pos, bool do_resize)
{
    pos= checkValue(pos);

    if (pos != absSeparatorPos()) {
      if (currentunits == Percent)
	position= pos * 100 / (orientation == Vertical?width():height());
      else
	position= pos;
      if (do_resize)
	resizeEvent(0);
    }
}

int KNewPanner::absSeparatorPos()
{
    int value;

    if (currentunits == Percent)
	value= (orientation == Vertical?width():height())*position/100;
    else
	value= position;
	    
    return value;
}

KNewPanner::Units KNewPanner::units()
{
    return currentunits;
}

void KNewPanner::setUnits(Units u)
{
    currentunits= u;
}

void KNewPanner::resizeEvent(QResizeEvent*)
{
  if (initialised) {
     if (orientation == Horizontal) {
      child0->setGeometry(0, 0, width(), absSeparatorPos());
      child1->setGeometry(0, absSeparatorPos()+4, width(), 
			  height()-absSeparatorPos()-4);
      divider->setGeometry(0, absSeparatorPos(), width(), 4);
    }
    else {
      if (showlabels) {
	label0->move(0,0);
	label0->resize(absSeparatorPos(), label0->height());
	label1->move(absSeparatorPos()+4, 0);
	label1->resize(width()-absSeparatorPos()-4, label1->height());
	startHeight= label0->height();
      }
      else {
	 startHeight= 0;
      }
      child0->setGeometry(0, startHeight, absSepqratorPos(), 
			  (height())-startHeight);
      child1->setGeometry(absSeparatorPos()+4, startHeight,
			  (width())-(absSeparatorPos()+4), 
			  (height())-startHeight);
      divider->setGeometry(absSeparatorPos(), startHeight, 4, 
			   (height())-startHeight);
    }
  }
}

int KNewPanner::checkValue(int pos)
{
   if (initialised) {
    if (orientation == Vertical) {
	if (pos < (child0->minimumSize().width()))
	    pos= child0->minimumSize().width();
	if ((width()-4-pos) < (child1->minimumSize().width()))
	    pos= width() - (child1->minimumSize().width()) -4;
    }
    else {
	if (pos < (child0->minimumSize().height()))
	    pos= (child0->minimumSize().height());
	if ((height()-4-pos) < (child1->minimumSize().height()))
	    pos= height() - (child1->minimumSize().height()) -4;
    }
   }
   
    if (pos < 0) pos= 0;

   if ((orientation == Vertical) && (pos > width()))
     pos= width();
   if ((orientation == Horizontal) && (pos > height()))
     pos= height();

    return pos;
}

bool KNewPanner::eventFilter(QObject *, QEvent *e)
{
    QMouseEvent *mev;
    bool handled= false;

    switch (e->type()) {
    case Event_MouseMove:
	mev= (QMouseEvent *)e;
	child0->setUpdatesEnabled(false);
	child1->setUpdatesEnabled(false);
	if (orientation == Horizontal) {
	    setAbsSeparatorPos(divider->mapToParent(mev->pos()).y(), false);
	    divider->setGeometry(0, absSeparatorPos(), width(), 4);
	    divider->repaint(0);
	}
	else {
	    setAbsSeparatorPos(divider->mapToParent(mev->pos()).x(), false);
	    divider->setGeometry(absSeparatorPos(), 0, 4, height());
	    divider->repaint(0);
	}
	handled= true;
	break;
    case Event_MouseButtonRelease:
	mev= (QMouseEvent *)e;

	child0->setUpdatesEnabled(true);
	child1->setUpdatesEnabled(true);

	if (orientation == Horizontal) {
	    resizeEvent(0);
// 	    child0->repaint(true);
// 	    child1->repaint(true);
	    divider->repaint(true);
	}
	else {
	    resizeEvent(0);
// 	    child0->repaint(true);
// 	    child1->repaint(true);
	    divider->repaint(true);
	}
	handled= true;
	break;
    }

    return handled;
}



