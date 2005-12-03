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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kruler.h"

#include <qpainter.h>
#include <qfont.h>
#include <QPolygon>

#define INIT_VALUE 0
#define INIT_MIN_VALUE 0
#define INIT_MAX_VALUE 100
#define INIT_TINY_MARK_DISTANCE 1
#define INIT_LITTLE_MARK_DISTANCE 5
#define INIT_MIDDLE_MARK_DISTANCE (INIT_LITTLE_MARK_DISTANCE * 2)
#define INIT_BIG_MARK_DISTANCE (INIT_LITTLE_MARK_DISTANCE * 10)
#define INIT_SHOW_TINY_MARK false
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



KRuler::KRuler(QWidget *parent, const char *name)
  : QAbstractSlider(parent),
    range(INIT_MIN_VALUE, INIT_MAX_VALUE, 1, 10, INIT_VALUE),
    dir(Qt::Horizontal)
{
  init(name);
  setFixedHeight(FIX_WIDTH);
}


KRuler::KRuler(Qt::Orientation orient,
               QWidget *parent, const char *name, Qt::WFlags f)
  : QAbstractSlider(parent),
    range(INIT_MIN_VALUE, INIT_MAX_VALUE, 1, 10, INIT_VALUE),
    dir(orient)
{
  setWindowFlags(f);
  init(name);
  if (orient == Qt::Horizontal)
    setFixedHeight(FIX_WIDTH);
  else
    setFixedWidth(FIX_WIDTH);
}


KRuler::KRuler(Qt::Orientation orient, int widgetWidth,
               QWidget *parent, const char *name, Qt::WFlags f)
  : QAbstractSlider(parent),
    range(INIT_MIN_VALUE, INIT_MAX_VALUE, 1, 10, INIT_VALUE),
    dir(orient)
{
  setWindowFlags(f);
  init(name);
  if (orient == Qt::Horizontal)
    setFixedHeight(widgetWidth);
  else
    setFixedWidth(widgetWidth);
}


void KRuler::init(const char* name)
{
  if (name) setObjectName(name);
#ifdef __GNUC__
  #warning FIXME setFrameStyle(WinPanel | Raised);
#endif

  tmDist = INIT_TINY_MARK_DISTANCE;
  lmDist = INIT_LITTLE_MARK_DISTANCE;
  mmDist = INIT_MIDDLE_MARK_DISTANCE;
  bmDist = INIT_BIG_MARK_DISTANCE;
  offset_= INIT_OFFSET;
  showtm = INIT_SHOW_TINY_MARK;
  showlm = INIT_SHOW_LITTLE_MARK;
  showmm = INIT_SHOW_MEDIUM_MARK;
  showbm = INIT_SHOW_BIG_MARK;
  showem = INIT_SHOW_END_MARK;
  ppm = INIT_PIXEL_PER_MARK;

  d = new KRuler::KRulerPrivate;
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
  setMinimum(value);
}

void
KRuler::setMaxValue(int value)
{
  setMaximum(value);
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
    update(contentsRect());
  }
}

void
KRuler::setBigMarkDistance(int dist)
{
  if (dist != bmDist) {
    bmDist = dist;
    update(contentsRect());
  }
}

void
KRuler::setShowTinyMarks(bool show)
{
  if (show != showtm) {
    showtm = show;
    update(contentsRect());
  }
}

bool
KRuler::showTinyMarks() const
{
  return showtm;
}

void
KRuler::setShowLittleMarks(bool show)
{
  if (show != showlm) {
    showlm = show;
    update(contentsRect());
  }
}

bool
KRuler::showLittleMarks() const
{
  return showlm;
}

void
KRuler::setShowMediumMarks(bool show)
{
  if (show != showmm) {
    showmm = show;
    update(contentsRect());
  }
}

bool
KRuler::showMediumMarks() const
{
  return showmm;
}

void
KRuler::setShowBigMarks(bool show)
{
  if (show != showbm) {
    showbm = show;
    update(contentsRect());
  }
}


bool
KRuler::showBigMarks() const
{
  return showbm;
}

void
KRuler::setShowEndMarks(bool show)
{
  if (show != showem) {
    showem = show;
    update(contentsRect());
  }
}

bool
KRuler::showEndMarks() const
{
  return showem;
}

void
KRuler::setShowPointer(bool show)
{
  if (show != d->showpointer) {
    d->showpointer = show;
    update(contentsRect());
  }
}


bool
KRuler::showPointer() const
{
  return d->showpointer;
}


void
KRuler::setShowEndLabel(bool show)
{
  if (d->showEndL != show) {
    d->showEndL = show;
    update(contentsRect());
  }
}


bool
KRuler::showEndLabel() const
{
    return d->showEndL;
}


void
KRuler::setEndLabel(const QString& label)
{
  endlabel = label;

  // premeasure the fontwidth and save it
  if (dir == Qt::Vertical) {
    QFont font = this->font();
    font.setPointSize(LABEL_SIZE);
    QFontMetrics fm(font);
    d->fontWidth = fm.width(endlabel);
  }
  update(contentsRect());
}

QString KRuler::endLabel() const
{
  return endlabel;
}

void
KRuler::setRulerMetricStyle(KRuler::MetricStyle style)
{
  switch (style) {
  default: /* fall through */
  case Custom:
    return;
  case Pixel:
    setLittleMarkDistance(1);
    setMediumMarkDistance(5);
    setBigMarkDistance(10);

    setShowTinyMarks(false);
    setShowLittleMarks(true);
    setShowMediumMarks(true);
    setShowBigMarks(true);
    setShowEndMarks(true);

    update(contentsRect());
    setPixelPerMark(10.0);

    break;
  case Inch:
    setTinyMarkDistance(1);
    setLittleMarkDistance(2);
    setMediumMarkDistance(4);
    setBigMarkDistance(8);

    setShowTinyMarks(true);
    setShowLittleMarks(true);
    setShowMediumMarks(true);
    setShowBigMarks(true);
    setShowEndMarks(true);

    update(contentsRect());
    setPixelPerMark(9.0);

    break;
  case Millimetres: /* fall through */
  case Centimetres: /* fall through */
  case Metres:
    setLittleMarkDistance(1);
    setMediumMarkDistance(5);
    setBigMarkDistance(10);

    setShowTinyMarks(false);
    setShowLittleMarks(true);
    setShowMediumMarks(true);
    setShowBigMarks(true);
    setShowEndMarks(true);

    update(contentsRect());
    setPixelPerMark(3.0);
  }
  switch (style) {
  case Pixel:
    setEndLabel(QLatin1String("pixel"));
    break;
  case Inch:
    setEndLabel(QLatin1String("inch"));
    break;
  case Millimetres:
    setEndLabel(QLatin1String("mm"));
    break;
  case Centimetres:
    setEndLabel(QLatin1String("cm"));
    break;
  case Metres:
    setEndLabel(QLatin1String("m"));
  default: /* never reached, see above switch */
    /* empty command */;
  }
  // if the style changes one of the values,
  // update would have been called inside the methods
  // -> no update() call needed here !
}

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
    tmp = width() - length;
  }
  if (tmp != d->endOffset_length) {
    d->endOffset_length = tmp;
    update(contentsRect());
  }
}

int
KRuler::length() const
{
  if (d->lengthFix) {
    return d->endOffset_length;
  }
  return (width() - d->endOffset_length);
}


void
KRuler::setLengthFixed(bool fix)
{
  d->lengthFix = fix;
}

bool
KRuler::lengthFixed() const
{
  return d->lengthFix;
}

void
KRuler::setOffset(int _offset)
{// debug("set offset %i", _offset);
  if (offset_ != _offset) {
    offset_ = _offset;
    update(contentsRect());
  }
}

int
KRuler::endOffset() const
{
  if (d->lengthFix) {
    return (width() - d->endOffset_length);
  }
  return d->endOffset_length;
}


void
KRuler::slideUp(int count)
{
  if (count) {
    offset_ += count;
    update(contentsRect());
  }
}

void
KRuler::slideDown(int count)
{
  if (count) {
    offset_ -= count;
    update(contentsRect());
  }
}


void
KRuler::slotNewValue(int _value)
{
  int oldvalue = value();
  if (oldvalue == _value) {
    return;
  }
  //    setValue(_value);
  setValue(_value);
  if (value() == oldvalue) {
    return;
  }
  // get the rectangular of the old and the new ruler pointer
  // and repaint only him
  if (dir == Qt::Horizontal) {
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
  if (offset_ != _offset) {
    //setOffset(_offset);
    offset_ = _offset;
    repaint(contentsRect());
  }
}


void
KRuler::slotEndOffset(int offset)
{
  int tmp;
  if (d->lengthFix) {
    tmp = width() - offset;
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
KRuler::paintEvent(QPaintEvent *e)
{
  //  debug ("KRuler::drawContents, %s",(horizontal==dir)?"horizontal":"vertical");

  QStylePainter p(this);
#ifdef PROFILING
  QTime time;
  time.start();
  for (int profile=0; profile<10; profile++) {
#endif

  int value  = this->value(),
    minval = minimum(),
    maxval;
    if (dir == Qt::Horizontal) {
    maxval = maximum()
    + offset_
    - (d->lengthFix?(height()-d->endOffset_length):d->endOffset_length);
    }
    else
    {
    maxval = maximum()
    + offset_
    - (d->lengthFix?(width()-d->endOffset_length):d->endOffset_length);
    }
    //ioffsetval = value-offset;
    //    pixelpm = (int)ppm;
  //    left  = clip.left(),
  //    right = clip.right();
  double f, fend,
    offsetmin=(double)(minval-offset_),
    offsetmax=(double)(maxval-offset_),
    fontOffset = (((double)minval)>offsetmin)?(double)minval:offsetmin;

  // draw labels
  QFont font = p.font();
  font.setPointSize(LABEL_SIZE);
  p.setFont( font );
  // draw littlemarklabel

  // draw mediummarklabel

  // draw bigmarklabel

  // draw endlabel
  if (d->showEndL) {
    if (dir == Qt::Horizontal) {
      p.translate( fontOffset, 0 );
      p.drawText( END_LABEL_X, END_LABEL_Y, endlabel );
    }
    else { // rotate text +pi/2 and move down a bit
      //QFontMetrics fm(font);
#ifdef KRULER_ROTATE_TEST
      p.rotate( -90.0 + rotate );
      p.translate( -8.0 - fontOffset - d->fontWidth + xtrans,
                    ytrans );
#else
      p.rotate( -90.0 );
      p.translate( -8.0 - fontOffset - d->fontWidth, 0.0 );
#endif
      p.drawText( END_LABEL_X, END_LABEL_Y, endlabel );
    }
    p.resetXForm();
  }

  // draw the tiny marks
  if (showtm) {
    fend = ppm*tmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (dir == Qt::Horizontal) {
        p.drawLine((int)f, BASE_MARK_X1, (int)f, BASE_MARK_X2);
      }
      else {
        p.drawLine(BASE_MARK_X1, (int)f, BASE_MARK_X2, (int)f);
      }
    }
  }
  if (showlm) {
    // draw the little marks
    fend = ppm*lmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (dir == Qt::Horizontal) {
        p.drawLine((int)f, LITTLE_MARK_X1, (int)f, LITTLE_MARK_X2);
      }
      else {
        p.drawLine(LITTLE_MARK_X1, (int)f, LITTLE_MARK_X2, (int)f);
      }
    }
  }
  if (showmm) {
    // draw medium marks
    fend = ppm*mmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (dir == Qt::Horizontal) {
        p.drawLine((int)f, MIDDLE_MARK_X1, (int)f, MIDDLE_MARK_X2);
      }
      else {
        p.drawLine(MIDDLE_MARK_X1, (int)f, MIDDLE_MARK_X2, (int)f);
      }
    }
  }
  if (showbm) {
    // draw big marks
    fend = ppm*bmDist;
    for ( f=offsetmin; f<offsetmax; f+=fend ) {
      if (dir == Qt::Horizontal) {
        p.drawLine((int)f, BIG_MARK_X1, (int)f, BIG_MARK_X2);
      }
      else {
        p.drawLine(BIG_MARK_X1, (int)f, BIG_MARK_X2, (int)f);
      }
    }
  }
  if (showem) {
    // draw end marks
    if (dir == Qt::Horizontal) {
      p.drawLine(minval-offset_, END_MARK_X1, minval-offset_, END_MARK_X2);
      p.drawLine(maxval-offset_, END_MARK_X1, maxval-offset_, END_MARK_X2);
    }
    else {
      p.drawLine(END_MARK_X1, minval-offset_, END_MARK_X2, minval-offset_);
      p.drawLine(END_MARK_X1, maxval-offset_, END_MARK_X2, maxval-offset_);
    }
  }

  // draw pointer
  if (d->showpointer) {
    QPolygon pa(4);
    if (dir == Qt::Horizontal) {
      pa.setPoints(3, value-5, 10, value+5, 10, value/*+0*/,15);
    }
    else {
      pa.setPoints(3, 10, value-5, 10, value+5, 15, value/*+0*/);
    }
    p.setBrush( p.backgroundColor() );
    p.drawPolygon( pa );
  }

#ifdef PROFILING
  }
  int elapsed = time.elapsed();
  debug("paint time %i",elapsed);
#endif

}

void KRuler::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kruler.moc"
