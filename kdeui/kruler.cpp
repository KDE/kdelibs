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
 * Revision 1.10  2000/01/17 19:07:59  bieker
 * Made it more QT_NO_CAST_ASCII and QT_NO_ASCII_CAST safe (this is not 100 %
 * yet).
 *
 * Revision 1.9  1999/05/10 12:59:04  kulow
 * applied 90% of Hans's patch and tried to fix other stuff. Some classes
 * (KAccel is a good example - libkab is even worse) mix string types so
 * dramacticly, that I bet 70% of the runtime is spent in string conversion ;(
 *
 * Revision 1.8  1999/03/01 23:35:08  kulow
 * CVS_SILENT ported to Qt 2.0
 *
 * Revision 1.7  1999/02/19 08:52:45  habenich
 * ID und LOG tags included
 *
 *
 *************************************************************************/

#include "kruler.h"

#include <qpainter.h>
#include <qfont.h>

#define INIT_VALUE 0
#define INIT_MIN_VALUE 0
#define INIT_MAX_VALUE 100
#define INIT_LITTLE_MARK_DISTANCE 5
#define INIT_MIDDLE_MARK_DISTANCE (INIT_LITTLE_MARK_DISTANCE * 2)
#define INIT_BIG_MARK_DISTANCE (INIT_LITTLE_MARK_DISTANCE * 10)
#define INIT_SHOW_LITTLE_MARK true
#define INIT_SHOW_MEDIUM_MARK true
#define INIT_SHOW_BIG_MARK true
#define INIT_SHOW_END_MARK true
#define INIT_SHOW_POINTER true
#define INIT_SHOW_END_LABEL true

#define INIT_PIXEL_PER_MARK (double)10.0 /* distance between 2 base marks in pixel */
#define INIT_OFFSET (-20)
#define INIT_LENGTH_FIX true
#define INIT_END_OFFSET 0

#define FIX_WIDTH 20 /* widget width in pixel */
#define LINE_END (FIX_WIDTH - 3)
#define END_MARK_LENGTH (FIX_WIDTH - 6)
#define END_MARK_X2 LINE_END
#define END_MARK_X1 (END_MARK_X2 - END_MARK_LENGTH)
#define BIG_MARK_LENGTH (END_MARK_LENGTH*3/4)
#define BIG_MARK_X2 LINE_END
#define BIG_MARK_X1 (BIG_MARK_X2 - BIG_MARK_LENGTH)
#define MIDDLE_MARK_LENGTH (END_MARK_LENGTH/2)
#define MIDDLE_MARK_X2 LINE_END
#define MIDDLE_MARK_X1 (MIDDLE_MARK_X2 - MIDDLE_MARK_LENGTH)
#define LITTLE_MARK_LENGTH (MIDDLE_MARK_LENGTH/2)
#define LITTLE_MARK_X2 LINE_END
#define LITTLE_MARK_X1 (LITTLE_MARK_X2 - LITTLE_MARK_LENGTH)
#define BASE_MARK_LENGTH (LITTLE_MARK_LENGTH/2)
#define BASE_MARK_X2 LINE_END
#define BASE_MARK_X1 (BASE_MARK_X2 - BASE_MARK_LENGTH)

#define LABEL_SIZE 8
#define END_LABEL_X 4
#define END_LABEL_Y (END_LABEL_X + LABEL_SIZE - 2)

#undef PROFILING

#ifdef PROFILING
# include <qdatetime.h>
#endif

class KRuler::KRulerPrivate
{
public:
  bool showpointer;
  bool showEndL;
  bool lengthFix;
  int  endOffset_length;  /* marks the offset at the end of the ruler
			   * i.e. right side at horizontal and down side
			   * at vertical rulers.
			   * the ruler end mark is moved endOffset_length
			   * ticks away from the widget end.
			   * positive offset moves end mark inside the ruler.
			   * if lengthFix is true, endOffset_length holds the 
			   * length of the ruler.
			   */
  int fontWidth; // ONLY valid for vertical rulers
};



KRuler::KRuler(KRuler::direction dir, QWidget *parent, const char *name,
	       WFlags f, bool allowLines)
  : QFrame(parent,name,f,allowLines),
    dir(dir),
    lmDist(INIT_LITTLE_MARK_DISTANCE),
    mmDist(INIT_MIDDLE_MARK_DISTANCE),
    bmDist(INIT_BIG_MARK_DISTANCE),
    offset(INIT_OFFSET),
    showlm(INIT_SHOW_LITTLE_MARK),
    showmm(INIT_SHOW_MEDIUM_MARK),
    showbm(INIT_SHOW_BIG_MARK),
    showem(INIT_SHOW_END_MARK),
    ppm(INIT_PIXEL_PER_MARK),
    range(INIT_MIN_VALUE, INIT_MAX_VALUE, 1, 10, INIT_VALUE)
{
  d = new KRuler::KRulerPrivate;

  setFrameStyle(WinPanel | Raised);

  if (horizontal == dir) {
    setFixedHeight(FIX_WIDTH);
  }
  else {
    setFixedWidth(FIX_WIDTH);
  }

  d->showpointer = INIT_SHOW_POINTER;
  d->showEndL = INIT_SHOW_END_LABEL;
  d->lengthFix = INIT_LENGTH_FIX;
  d->endOffset_length = INIT_END_OFFSET;

}

KRuler::KRuler(KRuler::direction dir, int widgetWidth, QWidget *parent, const char *name, WFlags f, bool allowLines)
  : QFrame(parent,name,f,allowLines),
    dir(dir),
    lmDist(INIT_LITTLE_MARK_DISTANCE),
    mmDist(INIT_MIDDLE_MARK_DISTANCE),
    bmDist(INIT_BIG_MARK_DISTANCE),
    offset(INIT_OFFSET),
    showlm(INIT_SHOW_LITTLE_MARK),
    showmm(INIT_SHOW_MEDIUM_MARK),
    showbm(INIT_SHOW_BIG_MARK),
    showem(INIT_SHOW_END_MARK),
    ppm(INIT_PIXEL_PER_MARK),
    range(INIT_MIN_VALUE, INIT_MAX_VALUE, 1, 10, INIT_VALUE)
{
  d = new KRulerPrivate;

  setFrameStyle(WinPanel | Raised);

  if (horizontal == dir) {
    setFixedHeight(widgetWidth);
  }
  else {
    setFixedWidth(widgetWidth);
  }

  d->showpointer = INIT_SHOW_POINTER;
  d->showEndL = INIT_SHOW_END_LABEL;
  d->lengthFix = INIT_LENGTH_FIX;
  d->endOffset_length = INIT_END_OFFSET;

}

KRuler::~KRuler()
{
  delete d;
}

void 
KRuler::setMinValue(int value)
{
  if (range.minValue() != value) {
    range.setRange( value, range.maxValue() );
    update(contentsRect());
  }
}

void 
KRuler::setMaxValue(int value)
{
  if (range.maxValue() != value) {
    range.setRange( range.minValue(), value );
    update(contentsRect());
  }
}

void 
KRuler::setRange(int min, int max)
{// debug("set range from %i to %i", min, max);
  if ((range.minValue() != min) || (range.maxValue() != max)) {
    range.setRange( min, max );
    update(contentsRect());
  }
}

void 
KRuler::setValue(int value)
{
  range.setValue(value);
  update(contentsRect());
}

void 
KRuler::setTinyMarkDistance(int dist)
{
  if (dist != tmDist) {
    tmDist = dist;
    update(contentsRect());
  }
}

void 
KRuler::setLittleMarkDistance(int dist)
{
  if (dist != lmDist) {
    lmDist = dist;
    update(contentsRect());
  }
}

void 
KRuler::setMediumMarkDistance(int dist)
{
  if (dist != mmDist) {
    mmDist = dist;
    valuemm = valuelm * mmDist;
    update(contentsRect());
  }
}

void 
KRuler::setBigMarkDistance(int dist)
{
  if (dist != bmDist) {
    bmDist = dist;
    valuebm = valuelm * bmDist;
    update(contentsRect());
  }
}

void 
KRuler::showTinyMarks(bool show)
{
  if (show != showtm) {
    showtm = show;
    update(contentsRect());
  }
}

bool 
KRuler::getShowTinyMarks() const
{
  return showtm;
}

void 
KRuler::showLittleMarks(bool show)
{
  if (show != showlm) {
    showlm = show;
    update(contentsRect());
  }
}

bool 
KRuler::getShowLittleMarks() const
{
  return showlm;
}

void 
KRuler::showMediumMarks(bool show)
{
  if (show != showmm) {
    showmm = show;
    update(contentsRect());
  }
}

bool 
KRuler::getShowMediumMarks() const
{
  return showmm;
}

void 
KRuler::showBigMarks(bool show)
{
  if (show != showbm) {
    showbm = show;
    update(contentsRect());
  }
}


bool 
KRuler::getShowBigMarks() const
{
  return showbm;
}

void 
KRuler::showEndMarks(bool show)
{
  if (show != showem) {
    showem = show;
    update(contentsRect());
  }
}

bool 
KRuler::getShowEndMarks() const
{
  return showem;
}

void 
KRuler::showPointer(bool show)
{
  if (show != d->showpointer) {
    d->showpointer = show;
    update(contentsRect());
  }
}


bool 
KRuler::getShowPointer() const
{
  return d->showpointer;
}

void 
KRuler::setValuePerLittleMark(int value)
{
  if ((value != valuelm) && (!showValuelm)) {
    showValuelm = TRUE;
    valuelm = value;
    showValuemm = TRUE;
    valuemm = value * mmDist;
    showValuebm = TRUE;
    valuebm = value * bmDist;
    update(contentsRect());
  }
}

void 
KRuler::setValuePerMediumMark(int value)
{
  if ((value != valuemm) && (!showValuemm)) {
    showValuelm = FALSE;
    showValuemm = TRUE;
    valuemm = value;
    showValuebm = TRUE;
    valuebm = value * (bmDist/mmDist);
    update(contentsRect());
  }
}

void 
KRuler::setValuePerBigMark(int value)
{
  if ((value != valuebm) && (!showValuebm)) {
    showValuelm = FALSE;
    showValuemm = FALSE;
    showValuebm = TRUE;
    valuebm = value;
    update(contentsRect());
  }
}

void 
KRuler::showEndLabel(bool show)
{
  if (d->showEndL != show) {
    d->showEndL = show;
    update(contentsRect());
  }
}


void 
KRuler::setEndLabel(const QString& label)
{
  endlabel = label;

  // premeasure the fontwidth and save it
  if (vertical == dir) {
    QFont font = this->font();
    font.setPointSize(LABEL_SIZE);
    QFontMetrics fm(font);
    d->fontWidth = fm.width(endlabel);
  }
  update(contentsRect());
}

void 
KRuler::setRulerStyle(KRuler::metric_style style)
{
  switch (style) {
  default: /* fall through */
  case custom:
    return;
  case pixel:
    setLittleMarkDistance(1);
    setMediumMarkDistance(5);
    setBigMarkDistance(10);

    showTinyMarks(false);
    showLittleMarks(true);
    showMediumMarks(true);
    showBigMarks(true);
    showEndMarks(true);

    setValuePerMediumMark(50);
    setPixelPerMark(10.0);

    break;
  case inch:
    setTinyMarkDistance(1);
    setLittleMarkDistance(2);
    setMediumMarkDistance(4);
    setBigMarkDistance(8);

    showTinyMarks(true);
    showLittleMarks(true);
    showMediumMarks(true);
    showBigMarks(true);
    showEndMarks(true);

    setValuePerBigMark(1);
    setPixelPerMark(9.0);

    break;
  case millimetres: /* fall through */
  case centimetres: /* fall through */
  case metres:
    setLittleMarkDistance(1);
    setMediumMarkDistance(5);
    setBigMarkDistance(10);

    showTinyMarks(false);
    showLittleMarks(true);
    showMediumMarks(true);
    showBigMarks(true);
    showEndMarks(true);

    setValuePerBigMark(10);
    setPixelPerMark(3.0);
  }
  switch (style) {
  case pixel:
    setEndLabel(QString::fromLatin1("pixel"));
    break;
  case inch:
    setEndLabel(QString::fromLatin1("inch"));
    break;
  case millimetres:
    setEndLabel(QString::fromLatin1("mm"));
    break;
  case centimetres:
    setEndLabel(QString::fromLatin1("cm"));
    break;
  case metres:
    setEndLabel(QString::fromLatin1("m"));
  default: /* never reached, see above switch */
    /* empty command */;
  }
  // if the style changes one of the values,
  // update would have been called inside the methods
  // -> no update() call needed here !
}

#if implemented
void 
KRuler::setRulerStyle(KRuler::paint_style);
void 
KRuler::setTickStyle(KRuler::paint_style);
#endif

/*
void 
KRuler::setPixelPerMark(int rate)
{
  ppm = (double)rate;
  update(contentsRect());
}
*/

void 
KRuler::setPixelPerMark(double rate)
{ // never compare floats against each other :)
  ppm = rate;
  update(contentsRect());
}


void 
KRuler::setLength(int length)
{
  int tmp;
  if (d->lengthFix) {
    tmp = length;
  }
  else {
    tmp = this->width() - length;
  }
  if (tmp != d->endOffset_length) {
    d->endOffset_length = tmp;
    update(contentsRect());
  }
}

int 
KRuler::getLength() const
{
  if (d->lengthFix) {  
    return d->endOffset_length;
  }
  return (this->width() - d->endOffset_length);
}


void 
KRuler::setLengthFix(bool fix)
{
  d->lengthFix = fix;
}

bool 
KRuler::getLengthFix() const
{
  return d->lengthFix;
}

void
KRuler::setOffset(int _offset)
{// debug("set offset %i", _offset);
  if (offset != _offset) {
    offset = _offset;
    update(contentsRect());
  }
}

int 
KRuler::getEndOffset() const
{
  if (d->lengthFix) {
    return (this->width() - d->endOffset_length);
  }
  return d->endOffset_length;
}


void 
KRuler::slideup(int count)
{
  if (count) {
    offset += count;
    update(contentsRect());
  }
}

void 
KRuler::slidedown(int count)
{
  if (count) {
    offset -= count;
    update(contentsRect());
  }
}


  /* the ruler may slide up and down "count" times*/
/*
void 
KRuler::slideup(int count = 1);

void 
KRuler::slidedown(int count = 1);
*/
/* set ruler slide to "offset" from beginning */
/*
  void 
KRuler::setOffset(int);
*/

void  
KRuler::slotNewValue(int _value)
{
  int oldvalue = range.value();
  if (oldvalue == _value) {
    return;
  }
  //    setValue(_value);
  range.setValue(_value);
  if (range.value() == oldvalue) {
    return;
  }
  // get the rectangular of the old and the new ruler pointer
  // and repaint only him
  if (horizontal == dir) {
    QRect oldrec(-5+oldvalue,10, 11,6);
    QRect newrec(-5+_value,10, 11,6);
    repaint( oldrec.unite(newrec) );
  }
  else {
    QRect oldrec(10,-5+oldvalue, 6,11);
    QRect newrec(10,-5+_value, 6,11);
    repaint( oldrec.unite(newrec) );
  }
}

void  
KRuler::slotNewOffset(int _offset)
{
  if (offset != _offset) {
    //setOffset(_offset);
    offset = _offset;
    repaint(contentsRect());
  }
}


void 
KRuler::slotEndOffset(int offset)
{
  int tmp;
  if (d->lengthFix) {
    tmp = this->width() - offset;
  }
  else {
    tmp = offset;
  }
  if (d->endOffset_length != tmp) {
    d->endOffset_length = tmp;
    repaint(contentsRect());
  }
}

void  
KRuler::drawContents(QPainter *p)
{
  //  debug ("KRuler::drawContents, %s",(horizontal==dir)?"horizontal":"vertical");

#ifdef PROFILING
  QTime time;
  time.start();
  for (int profile=0; profile<10; profile++) {
#endif

  int value  = range.value(),
    minval = range.minValue(),
    maxval = range.maxValue() 
    + offset 
    - (d->lengthFix?(width()-d->endOffset_length):d->endOffset_length);
    //ioffsetval = value-offset;
    //    pixelpm = (int)ppm;
  //    left  = clip.left(),
  //    right = clip.right();
  double f, fend,
    offsetmin=(double)(minval-offset),
    offsetmax=(double)(maxval-offset),
    fontOffset = (((double)minval)>offsetmin)?(double)minval:offsetmin;

  // draw labels
  QFont font = p->font();
  font.setPointSize(LABEL_SIZE);
  p->setFont( font );
  // draw littlemarklabel
  
  // draw mediummarklabel

  // draw bigmarklabel

  // draw endlabel
  if (d->showEndL) {
    if (horizontal==dir) {
      p->translate( fontOffset, 0 );
      p->drawText( END_LABEL_X, END_LABEL_Y, endlabel );
    }
    else { // rotate text +pi/2 and move down a bit
      //QFontMetrics fm(font);
#ifdef KRULER_ROTATE_TEST
      p->rotate( -90.0 + rotate );
      p->translate( -8.0 - fontOffset - d->fontWidth + xtrans, 
		    ytrans );
#else
      p->rotate( -90.0 );
      p->translate( -8.0 - fontOffset - d->fontWidth, 0.0 );
#endif
      p->drawText( END_LABEL_X, END_LABEL_Y, endlabel );
    }
    p->resetXForm();
  }

  // draw the tiny marks
  if (showtm) {
    for ( f=offsetmin; f<offsetmax; f+=ppm ) {
      if (horizontal==dir) {
	p->drawLine((int)f, BASE_MARK_X1, (int)f, BASE_MARK_X2);
      }
      else {
	p->drawLine(BASE_MARK_X1, (int)f, BASE_MARK_X2, (int)f);
      }
    }
  }
  if (showlm) {
    // draw the little marks
    fend = ppm*lmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (horizontal==dir) {
	p->drawLine((int)f, LITTLE_MARK_X1, (int)f, LITTLE_MARK_X2);
      }
      else {
	p->drawLine(LITTLE_MARK_X1, (int)f, LITTLE_MARK_X2, (int)f);
      }
    }
  }
  if (showmm) {
    // draw medium marks
    fend = ppm*mmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (horizontal==dir) {
	p->drawLine((int)f, MIDDLE_MARK_X1, (int)f, MIDDLE_MARK_X2);
      }
      else {
	p->drawLine(MIDDLE_MARK_X1, (int)f, MIDDLE_MARK_X2, (int)f);
      }
    }
  }
  if (showbm) {
    // draw big marks
    fend = ppm*bmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (horizontal==dir) {
	p->drawLine((int)f, BIG_MARK_X1, (int)f, BIG_MARK_X2);
      }
      else {
	p->drawLine(BIG_MARK_X1, (int)f, BIG_MARK_X2, (int)f);
      }
    }
  }
  if (showem) {
    // draw end marks
    if (horizontal==dir) {
      p->drawLine(minval-offset, END_MARK_X1, minval-offset, END_MARK_X2);
      p->drawLine(maxval-offset, END_MARK_X1, maxval-offset, END_MARK_X2);
    }
    else {
      p->drawLine(END_MARK_X1, minval-offset, END_MARK_X2, minval-offset);
      p->drawLine(END_MARK_X1, maxval-offset, END_MARK_X2, maxval-offset);    
    }
  }

  // draw pointer
  if (d->showpointer) {
    QPointArray pa(4);
    if (horizontal==dir) {
      pa.setPoints(3, value-5, 10, value+5, 10, value/*+0*/,15);
    }
    else {
      pa.setPoints(3, 10, value-5, 10, value+5, 15, value/*+0*/);
    }
    p->setBrush( p->backgroundColor() );
    p->drawPolygon( pa );
  }

#ifdef PROFILING
  }
  int elapsed = time.elapsed();
  debug("paint time %i",elapsed);
#endif
  
}
#include "kruler.moc"
