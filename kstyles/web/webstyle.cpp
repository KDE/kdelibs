/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <q3pointarray.h>
#include <qscrollbar.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qdrawutil.h>
#include <qpainter.h>

#include <kapplication.h>
#include <kdrawutil.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "webstyle.h"

static const int  _indicatorSize = 13;
static Q3Button *  _highlightedButton = 0;
static const int  _scrollBarExtent = 14;

static QFrame *   _currentFrame = 0;
static int        _savedFrameLineWidth;
static int        _savedFrameMidLineWidth;
static ulong      _savedFrameStyle;

static QColor contrastingForeground(const QColor & fg, const QColor & bg)
{
  int h, s, vbg, vfg;

  bg.hsv(&h, &s, &vbg);
  fg.hsv(&h, &s, &vfg);

  int diff(vbg - vfg);

  if ((diff > -72) && (diff < 72))
  {
    return (vbg < 128) ? Qt::white : Qt::black;
  }
  else
  {
    return fg;
  }
}

// Gotta keep it separated.

  static void
scrollBarControlsMetrics
(
 const QScrollBar * sb,
 int sliderStart,
 int /* sliderMin */,
 int sliderMax,
 int sliderLength,
 int buttonDim,
 QRect & rSub,
 QRect & rAdd,
 QRect & rSubPage,
 QRect & rAddPage,
 QRect & rSlider
 )
{
  bool horizontal = sb->orientation() == Qt::Horizontal;

  int len     = horizontal ? sb->width()  : sb->height();

  int extent  = horizontal ? sb->height() : sb->width();

  QColorGroup g = sb->colorGroup();

  if (sliderStart > sliderMax)
    sliderStart = sliderMax;

  int sliderEnd = sliderStart + sliderLength;

  int addX, addY;
  int subX, subY;
  int subPageX, subPageY, subPageW, subPageH;
  int addPageX, addPageY, addPageW, addPageH;
  int sliderX, sliderY, sliderW, sliderH;

  if (horizontal)
  {
    subY      = 0;
    addY      = 0;
    subX      = 0;
    addX      = buttonDim;

    subPageX  = buttonDim * 2;
    subPageY  = 0;
    subPageW  = sliderStart - 1;
    subPageH  = extent;

    addPageX  = sliderEnd;
    addPageY  = 0;
    addPageW  = len - sliderEnd;
    addPageH  = extent;

    sliderX   = sliderStart;
    sliderY   = 0;
    sliderW   = sliderLength;
    sliderH   = extent;
  }
  else
  {
    subX    = 0;
    addX    = 0;
    subY    = len - buttonDim * 2;
    addY    = len - buttonDim;

    subPageX = 0;
    subPageY = 0;
    subPageW = extent;
    subPageH = sliderStart;

    addPageX  = 0;
    addPageY  = sliderEnd;
    addPageW  = extent;
    addPageH  = subY - sliderEnd;

    sliderX   = 0;
    sliderY   = sliderStart;
    sliderW   = extent;
    sliderH   = sliderLength;
  }

  rSub      .setRect(    subX,      subY, buttonDim, buttonDim);
  rAdd      .setRect(    addX,      addY, buttonDim, buttonDim);
  rSubPage  .setRect(subPageX,  subPageY,  subPageW,  subPageH);
  rAddPage  .setRect(addPageX,  addPageY,  addPageW,  addPageH);
  rSlider   .setRect( sliderX,   sliderY,   sliderW,   sliderH);
}

// Rounded rects my way.

  static void
drawFunkyRect
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 bool small
)
{
  p->translate(x, y);

  if (small)
  {
    p->drawLine(      2,      0,  w - 3,      0  );
    p->drawLine(  w - 1,      2,  w - 1,  h - 3  );
    p->drawLine(  w - 3,  h - 1,      2,  h - 1  );
    p->drawLine(      0,  h - 3,      0,      2  );

    // Use an array of points so that there's only one round-trip with the
    // X server.

    QCOORD pointList[] =
    {
          1,      1,
      w - 2,      1,
      w - 2,  h - 2,
          1,  h - 2
    };

    p->drawPoints(Q3PointArray(4, pointList));
  }
  else
  {
    p->drawLine(      3,      0,  w - 4,      0  );
    p->drawLine(  w - 1,      3,  w - 1,  h - 4  );
    p->drawLine(  w - 4,  h - 1,      3,  h - 1  );
    p->drawLine(      0,  h - 4,      0,      3  );

    QCOORD pointList[] =
    {
          1,      2,
          2,      1,
      w - 3,      1,
      w - 2,      2,
      w - 2,  h - 3,
      w - 3,  h - 2,
          2,  h - 2,
          1,  h - 3
    };

    p->drawPoints(Q3PointArray(8, pointList));
  }

  p->translate(-x, -y);
}

WebStyle::WebStyle()
  : KStyle()
{
  setButtonDefaultIndicatorWidth(6);
  setScrollBarExtent(_scrollBarExtent, _scrollBarExtent);
}

WebStyle::~WebStyle()
{
  // Empty.
}

  void
WebStyle::polish(QApplication *)
{
  // Empty.
}

  void
WebStyle::polish(QPalette &)
{
  // Empty.
}

  void
WebStyle::unPolish(QApplication *)
{
  // Empty.
}

  void
WebStyle::polish(QWidget * w)
{
  if (w->inherits("QPushButton"))
    w->installEventFilter(this);

  else if (w->inherits("QGroupBox") || w->inherits("QFrame"))
  {
    QFrame * f(static_cast<QFrame *>(w));

    if (f->frameStyle() != QFrame::NoFrame)
    {
      _currentFrame = f;

      _savedFrameLineWidth = f->lineWidth();
      _savedFrameMidLineWidth = f->midLineWidth();
      _savedFrameStyle = f->frameStyle();

      if (f->frameShape() == QFrame::HLine || f->frameShape() == QFrame::VLine)
      {
        f->setMidLineWidth(1);
        f->setFrameStyle(f->frameShape() | QFrame::Plain);
      }
      else
      {
        f->setLineWidth(1);
        f->setFrameStyle(QFrame::Box | QFrame::Plain);
      }
    }
  }
}

  void
WebStyle::unPolish(QWidget * w)
{
  if (w->inherits("QPushButton"))
    w->removeEventFilter(this);

  else if (w == _currentFrame)
  {
    QFrame * f(static_cast<QFrame *>(w));

    f->setLineWidth(_savedFrameLineWidth);
    f->setMidLineWidth(_savedFrameMidLineWidth);
    f->setFrameStyle(_savedFrameStyle);
  }
}

  bool
WebStyle::eventFilter(QObject * o, QEvent * e)
{
  QPushButton * pb(static_cast<QPushButton *>(o));

  if (e->type() == QEvent::Enter)
  {
    _highlightedButton = pb;
    pb->repaint(false);
  }
  else if (e->type() == QEvent::Leave)
  {
    _highlightedButton = 0;
    pb->repaint(false);
  }

  return false;
}

  void
WebStyle::drawButton
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool sunken,
 const QBrush * fill
)
{
  p->save();

  if (sunken)
    p->setPen(contrastingForeground(g.light(), g.button()));
  else
    p->setPen(contrastingForeground(g.mid(), g.button()));

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  drawFunkyRect(p, x, y, w, h, true);

  p->restore();
}

  QRect
WebStyle::buttonRect(int x, int y, int w, int h)
{
  return QRect(x + 2, y + 2, w - 4, h - 4);
}

  void
WebStyle::drawBevelButton
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool sunken,
 const QBrush * fill
)
{
  drawButton(p, x, y, w, h, g, sunken, fill);
}

  void
WebStyle::drawPushButton(QPushButton * b, QPainter * p)
{
  // Note: painter is already translated for us.

  bool sunken(b->isDown() || b->isOn());
  bool hl(_highlightedButton == b);

  QColor bg(b->colorGroup().button());

  p->save();
  p->fillRect(b->rect(), b->colorGroup().brush(QColorGroup::Background));

  if (b->isDefault())
  {
    QColor c(hl ? b->colorGroup().highlight() : b->colorGroup().mid());

    p->setPen(contrastingForeground(c, bg));

    drawFunkyRect(p, 0, 0, b->width(), b->height(), false);
  }

  p->fillRect
    (
     4,
     4,
     b->width() - 8,
     b->height() - 8,
     b->colorGroup().brush(QColorGroup::Button)
    );

  if (b->isEnabled())
  {
    if (sunken)
    {
      p->setPen(contrastingForeground(b->colorGroup().light(), bg));
    }
    else
    {
      if (hl)
        p->setPen(contrastingForeground(b->colorGroup().highlight(), bg));
      else
        p->setPen(contrastingForeground(b->colorGroup().mid(), bg));
    }
  }
  else
  {
    p->setPen(b->colorGroup().button());
  }

  drawFunkyRect(p, 3, 3, b->width() - 6, b->height() - 6, true);

  p->restore();
}

  void
WebStyle::drawPushButtonLabel(QPushButton * b, QPainter * p)
{
  // This is complicated stuff and we don't really want to mess with it.

  KStyle::drawPushButtonLabel(b, p);
}

  void
WebStyle::drawScrollBarControls
(
 QPainter * p,
 const QScrollBar * sb,
 int sliderStart,
 uint controls,
 uint activeControl
)
{
  p->save();

  int sliderMin, sliderMax, sliderLength, buttonDim;

  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

  QRect rSub, rAdd, rSubPage, rAddPage, rSlider;

  scrollBarControlsMetrics
    (
     sb,
     sliderStart,
     sliderMin,
     sliderMax,
     sliderLength,
     buttonDim,
     rSub,
     rAdd,
     rSubPage,
     rAddPage,
     rSlider
    );

  QColorGroup g(sb->colorGroup());

  if (controls & AddLine && rAdd.isValid())
  {
    bool active(activeControl & AddLine);

    QColor c(active ? g.highlight() : g.dark());

    p->setPen(c);
    p->setBrush(g.button());
    p->drawRect(rAdd);

    Qt::ArrowType t =
      sb->orientation() == Qt::Horizontal ? Qt::RightArrow : Qt::DownArrow;

    // Is it me or is KStyle::drawArrow broken ?

    drawArrow
      (
       p,
       t,
       true, // FIXME - down ?
       rAdd.x(),
       rAdd.y(),
       rAdd.width(),
       rAdd.height(),
       g,
       true // FIXME - enabled ?
      );
  }

  if (controls & SubLine && rSub.isValid())
  {
    bool active(activeControl & SubLine);

    QColor c(active ? g.highlight() : g.dark());

    p->setPen(c);
    p->setBrush(g.button());
    p->drawRect(rSub);

    Qt::ArrowType t =
      sb->orientation() == Qt::Horizontal ? Qt::LeftArrow : Qt::UpArrow;

    drawArrow
      (
       p,
       t,
       true, // FIXME - down ?
       rSub.x(),
       rSub.y(),
       rSub.width(),
       rSub.height(),
       g,
       true // FIXME - enabled ?
      );
  }

  if (controls & SubPage && rSubPage.isValid())
  {
    p->setPen(g.mid());
    p->setBrush(g.base());
    p->drawRect(rSubPage);
  }

  if (controls & AddPage && rAddPage.isValid())
  {
    p->setPen(g.mid());
    p->setBrush(g.base());
    p->drawRect(rAddPage);
  }

  if (controls & Slider && rSlider.isValid())
  {
    p->setPen(activeControl & Slider ? g.highlight() : g.dark());

    p->setBrush(g.button());
    p->drawRect(rSlider);

    p->setBrush(g.light());
    p->setPen(g.dark());

    if (sliderLength > _scrollBarExtent * 2)
    {
      int ellipseSize = 
        Qt::Horizontal == sb->orientation()
        ?
        rSlider.height() - 4
        :
        rSlider.width()  - 4
        ;

      QPoint center(rSlider.center());

      if (Qt::Horizontal == sb->orientation())
      {
        p->drawEllipse
          (
           center.x() - ellipseSize / 2, rSlider.y() + 2,
           ellipseSize, ellipseSize
          );
      }
      else
      { 
        p->drawEllipse
          (
           rSlider.x() + 2, center.y() - ellipseSize / 2,
           ellipseSize, ellipseSize
          );
      }
    }
  }

  p->restore();
}

  QStyle::ScrollControl
WebStyle::scrollBarPointOver
(
 const QScrollBar * sb,
 int sliderStart,
 const QPoint & point
)
{
  if (!sb->rect().contains(point))
    return NoScroll;

  int sliderMin, sliderMax, sliderLength, buttonDim;

  scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

  if (sb->orientation() == Qt::Horizontal)
  {
    int x = point.x();

    if (x <= buttonDim)
      return SubLine;

    else if (x <= buttonDim * 2)
      return AddLine;

    else if (x < sliderStart)
      return SubPage;

    else if (x < sliderStart+sliderLength)
      return Slider;

    return AddPage;
  }
  else
  {
    int y = point.y();

    if (y < sliderStart)
      return SubPage;

    else if (y < sliderStart + sliderLength)
      return Slider;

    else if (y < sliderMax + sliderLength)
      return AddPage;

    else if (y < sliderMax + sliderLength + buttonDim)
      return SubLine;

    return AddLine;
  }
}

  void
WebStyle::scrollBarMetrics
(
 const QScrollBar * sb,
 int & sliderMin,
 int & sliderMax,
 int & sliderLength,
 int & buttonDim
)
{
  int maxlen;

  bool horizontal = sb->orientation() == Qt::Horizontal;

  int len = (horizontal) ? sb->width() : sb->height();

  int extent = (horizontal) ? sb->height() : sb->width();

  if (len > (extent - 1) * 2)
    buttonDim = extent;
  else
    buttonDim = len / 2 - 1;

  if (horizontal)
    sliderMin = buttonDim * 2;
  else
    sliderMin = 1;

  maxlen = len - buttonDim * 2 - 1;

  sliderLength =
    (sb->pageStep() * maxlen) /
    (sb->maxValue() - sb->minValue() + sb->pageStep());

  if (sliderLength < _scrollBarExtent)
    sliderLength = _scrollBarExtent;

  if (sliderLength > maxlen)
    sliderLength = maxlen;

  sliderMax = sliderMin + maxlen - sliderLength;
}

  QSize
WebStyle::indicatorSize() const
{
  return QSize(_indicatorSize, _indicatorSize);
}

  void
WebStyle::drawIndicator
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 int state,
 bool down,
 bool enabled
)
{
  p->save();

  p->fillRect(x, y, w, h, g.background());

  if (enabled)
  {
    p->setPen(down ? g.highlight() : contrastingForeground(g.dark(), g.background()));
  }
  else
  {
    g.mid();
  }

  p->drawRect(x, y, w, h);

  if (state != QCheckBox::Off)
  {
    p->fillRect(x + 2, y + 2, w - 4, h - 4, enabled ? g.highlight() : g.mid());

    if (state == QCheckBox::NoChange)
    {
      p->fillRect(x + 4, y + 4, w - 8, h - 8, g.background());
    }
  }

  p->restore();
}

  QSize
WebStyle::exclusiveIndicatorSize() const
{
  return QSize(_indicatorSize, _indicatorSize);
}

  void
WebStyle::drawExclusiveIndicator
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool on,
 bool down,
 bool enabled
)
{
  p->save();

  p->fillRect(x, y, w, h, g.background());

  if (enabled)
  {
    p->setPen(down ? g.highlight() : contrastingForeground(g.dark(), g.background()));
  }
  else
  {
    p->setPen(g.mid());
  }

  p->setBrush(g.brush(QColorGroup::Background));

  // Avoid misshapen ellipses. Qt or X bug ? Who knows...

  if (0 == w % 2)
    --w;

  if (0 == h % 2)
    --h;

  p->drawEllipse(x, y, w, h);

  if (on)
  {
    p->setPen(enabled ? g.highlight() : g.mid());
    p->setBrush(enabled ? g.highlight() : g.mid());
    p->drawEllipse(x + 3, y + 3, w - 6, h - 6);
  }

  p->restore();
}

  void
WebStyle::drawIndicatorMask
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 int /* state */
)
{
  p->fillRect(x, y, w, h, Qt::color1);
}

  void
WebStyle::drawExclusiveIndicatorMask
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 bool /* on */
)
{
  if (0 == w % 2)
    --w;

  if (0 == h % 2)
    --h;

  p->setPen(Qt::color1);
  p->setBrush(Qt::color1);
  p->drawEllipse(x, y, w, h);
}

  void
WebStyle::drawComboButton
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool sunken,
 bool editable,
 bool enabled,
 const QBrush * fill
)
{
  p->save();

  p->setPen(Qt::NoPen);
  p->setBrush(0 == fill ? g.brush(QColorGroup::Background) : *fill);
  p->drawRect(x, y, w, h);

  if (enabled)
  {
    if (sunken)
      p->setPen(contrastingForeground(g.highlight(), g.background()));
    else
      p->setPen(contrastingForeground(g.mid(), g.background()));
  }
  else
  {
    p->setPen(contrastingForeground(g.mid(), g.background()));
  }

  drawFunkyRect(p, x, y, w, h, true);

  p->drawPoint(w - 10, h - 6);
  p->drawPoint(w - 9, h - 6);
  p->drawPoint(w - 8, h - 6);
  p->drawPoint(w - 7, h - 6);
  p->drawPoint(w - 6, h - 6);

  p->drawPoint(w - 9, h - 7);
  p->drawPoint(w - 8, h - 7);
  p->drawPoint(w - 7, h - 7);
  p->drawPoint(w - 6, h - 7);

  p->drawPoint(w - 8, h - 8);
  p->drawPoint(w - 7, h - 8);
  p->drawPoint(w - 6, h - 8);

  p->drawPoint(w - 7, h - 9);
  p->drawPoint(w - 6, h - 9);

  p->drawPoint(w - 6, h - 10);

  if (editable)
    p->fillRect(comboButtonFocusRect(x, y, w, h), Qt::red);

  p->restore();
}

  QRect
WebStyle::comboButtonRect(int x, int y, int w, int h)
{
  return QRect(x + 2, y + 2, w - 20, h - 4);
}

  QRect
WebStyle::comboButtonFocusRect(int x, int y, int w, int h)
{
  return QRect(x + 2, y + 2, w - 20, h - 4);
}

  int
WebStyle::sliderLength() const
{
  return 13;
}

  void
WebStyle::drawSliderGroove
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 QCOORD /* c */,
 Qt::Orientation o
)
{
  p->save();

  p->setPen(QPen(g.dark(), 0, Qt::DotLine));

  if( o == Qt::Horizontal )
    p->drawLine(x, y + h / 2, w, y + h / 2);
  else
  if( o == Qt::Vertical )
    p->drawLine(x + w / 2, y, x + w / 2, h);

  p->restore();
}

  void
WebStyle::drawArrow
(
 QPainter * p,
 Qt::ArrowType type,
 bool down,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool enabled,
 const QBrush * fill
)
{
  KStyle::drawArrow(p, type, down, x, y, w, h, g, enabled, fill);
}

  void
WebStyle::drawSlider
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 Qt::Orientation o,
 bool /* tickAbove */,
 bool /* tickBelow */
)
{
  p->save();

  p->fillRect(x + 1, y + 1, w - 2, h - 2, g.background());
  p->setPen(g.dark());
  p->setBrush(g.light());

  int sl = sliderLength();

  if( o == Qt::Horizontal )
    p->drawEllipse(x, y + h / 2 - sl / 2, sl, sl);
  else
  if( o == Qt::Vertical )
    p->drawEllipse(x + w / 2 - sl / 2, y, sl, sl);

  p->restore();
}

  void
WebStyle::drawKToolBar
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 KToolBarPos /* pos */,
 QBrush * /* fill */
)
{
  p->save();
  p->setPen(g.background());
  p->setBrush(g.background());
  p->drawRect(x, y, w, h);
  p->restore();
}

  void
WebStyle::drawKBarHandle
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 KToolBarPos /* pos */,
 QBrush * /* fill */
)
{
  p->save();
  p->setPen(g.mid());
  p->setBrush(g.background());
  p->drawRect(x + 1, y + 1, w - 2, h - 2);
  p->restore();
}

  void
WebStyle::drawKMenuBar
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool /* macMode */,
 QBrush * /* fill */
)
{
  p->save();
  p->setPen(g.mid());
  p->setBrush(g.background());
  p->drawRect(x, y, w, h);
  p->restore();
}

  void
WebStyle::drawKToolBarButton
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool sunken,
 bool raised,
 bool enabled,
 bool popup,
 KToolButtonType type,
 const QString & btext,
 const QPixmap * pixmap,
 QFont * font,
 QWidget * button
)
{
  bool toggleAndOn = false;

  if (button->inherits("QButton"))
  {
    Q3Button * b = static_cast<Q3Button *>(button);
    toggleAndOn = b->isToggleButton() && b->isOn();
  }

  p->save();

  QColor borderColour;
  QColor textColour;
  QColor fillColour;

  if (!enabled)
  {
    borderColour  = g.background();
    fillColour    = g.background();
    textColour    = g.text();
  }
  else
  {
    if (toggleAndOn)
    {
      borderColour  = g.dark();
      fillColour    = g.button();
      textColour    = g.buttonText();
    }
    else if (sunken)
    {
      borderColour  = g.light();
      fillColour    = g.button();
      textColour    = g.buttonText();
    }
    else if (raised)
    {
      borderColour  = g.highlight();
      fillColour    = g.background();
      textColour    = g.text();
    }
    else
    {
      borderColour  = g.background();
      fillColour    = g.background();
      textColour    = g.text();
    }
  }

  p->setPen(borderColour);
  p->setBrush(fillColour);

  p->drawRect(x, y, w, h);

  p->setPen(g.background());

  p->drawPoint(x, y);
  p->drawPoint(x + w, y);
  p->drawPoint(x, y + h);
  p->drawPoint(x + w, y + h);

  switch (type)
  {
    case Icon:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + (w - pixmap->width()) / 2,
           y + (h - pixmap->height()) / 2,
           *pixmap
          );
      break;

    case Text:

    if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        p->drawText
          (
           x,
           y,
           w,
           h,
           Qt::AlignCenter,
           btext
          );
      }

      break;

    case IconTextRight:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + 2,
           y + (h - pixmap->height()) / 2,
           *pixmap
          );

      if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        if (0 != pixmap)
        {
          int textLeft = pixmap->width() + 4;

          p->drawText
            (
             x + textLeft,
             y,
             w - textLeft,
             h,
             Qt::AlignVCenter | Qt::AlignLeft,
             btext
            );
        }
        else
        {
          p->drawText
            (
             x,
             y,
             w,
             h,
             Qt::AlignVCenter | Qt::AlignLeft,
             btext
            );
        }
      }
      break;

    case IconTextBottom:

      if (0 != pixmap)
        p->drawPixmap
          (
           x + (w - pixmap->width()) / 2,
           y + 2,
           *pixmap
          );

      if (!btext.isNull())
      {
        if (0 != font)
          p->setFont(*font);

        p->setPen(textColour);

        if (0 != pixmap)
        {
          int textTop = y + pixmap->height() + 4;

          p->drawText
            (
             x + 2,
             textTop,
             w - 4,
             h - x - textTop,
             Qt::AlignCenter,
             btext
            );
        }
        else
        {
          p->drawText
            (
             x,
             y,
             w,
             h,
             Qt::AlignCenter,
             btext
            );
        }
      }
      break;

    default:
      break;
  }

  if (popup)
  {
    p->setPen(g.dark());
    for (int px = 0; px < 5; ++px)
      for (int py = 0; py < 5 - px; ++py)
        p->drawPoint(w - 6 - px, h - 6 - py);
  }
      
  p->restore();
}

  void
WebStyle::drawKMenuItem
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool active,
 QMenuItem * mi,
 QBrush * /* fill */
)
{
  p->save();

  QColor bg(active ? g.highlight() : g.background());

  p->fillRect(x, y, w, h, bg);

  QColor textColour =
    active ?
    contrastingForeground(g.highlightedText(), bg) :
    contrastingForeground(g.text(), bg);

  QApplication::style().drawItem
    (
     p,
     x,
     y,
     w,
     h,
     Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine,
     g,
     mi->isEnabled(),
     mi->pixmap(),
     mi->text(),
     -1,
     &textColour
    );

  p->restore();
}

  void
WebStyle::drawPopupMenuItem
(
 QPainter * p,
 bool checkable,
 int maxpmw,
 int tab,
 QMenuItem * mi,
 const QPalette & pal,
 bool act,
 bool enabled,
 int x,
 int y,
 int w,
 int h
)
{
  // TODO
  KStyle::drawPopupMenuItem(p, checkable, maxpmw, tab, mi, pal, act, enabled, x, y, w, h);
}

  void
WebStyle::drawKProgressBlock
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 QBrush * fill
)
{
  p->save();

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  p->fillRect(x, y, w, h, g.highlight());

  p->restore();
}

  void
WebStyle::drawFocusRect
(
 QPainter * p,
 const QRect & r,
 const QColorGroup & g,
 const QColor * pen,
 bool atBorder
)
{
  p->save();

  if (0 != pen)
  p->setPen(0 == pen ? g.foreground() : *pen);
  p->setBrush(Qt::NoBrush);

  if (atBorder)
  {
    p->drawRect(QRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2));
  }
  else
  {
    p->drawRect(r);
  }

  p->restore();
}

  void
WebStyle::drawPanel
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool /* sunken */,
 int /* lineWidth */,
 const QBrush * fill
)
{
  p->save();

  p->setPen(g.dark());

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  p->drawRect(x, y, w, h);

  p->restore();
}

  void
WebStyle::drawPopupPanel
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 int /* lineWidth */,
 const QBrush * fill
)
{
  p->save();

  p->setPen(g.dark());

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  p->drawRect(x, y, w, h);

  p->restore();
}

  void
WebStyle::drawSeparator
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 bool /* sunken */,
 int /* lineWidth */,
 int /* midLineWidth */
)
{
  p->save();

  p->setPen(g.dark());

  if (w > h)
  {
    p->drawLine(x, y + h / 2, x + w, y + h / 2);
  }
  else
  {
    p->drawLine(x + w / 2, y, x + w / 2, y + h);
  }

  p->restore();
}

  void
WebStyle::drawTab
(
 QPainter * p,
 const QTabBar * tabBar,
 QTab * tab,
 bool selected
)
{
  QRect r(tab->rect());

  QColorGroup g(tabBar->colorGroup());

  p->save();

  p->setPen(selected ? g.dark() : g.mid());
  p->fillRect(r, g.brush(QColorGroup::Background));

  switch (tabBar->shape())
  {
    case QTabBar::RoundedNorth:
    case QTabBar:: TriangularNorth:
      p->drawLine(r.left(), r.top(), r.left(), r.bottom());
      p->drawLine(r.left(), r.top(), r.right(), r.top());
      p->drawLine(r.right(), r.top(), r.right(), r.bottom());
      if (!selected)
      {
        p->setPen(g.dark());
        p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
      }
      break;
    case QTabBar:: RoundedSouth:
    case QTabBar:: TriangularSouth:
      if (!selected)
      {
        p->setPen(g.dark());
        p->drawLine(r.left(), r.top(), r.right(), r.top());
      }
      p->drawLine(r.left(), r.top(), r.left(), r.bottom());
      p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
      p->drawLine(r.right(), r.top(), r.right(), r.bottom());
      break;
  }

  p->restore();
}

  void
WebStyle::drawTabMask
(
 QPainter * p,
 const QTabBar *,
 QTab * tab,
 bool
)
{
  p->fillRect(tab->rect(), Qt::color1);
}

  void
WebStyle::drawKickerHandle
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 QBrush * fill
)
{
  p->save();

  p->setPen(g.mid());

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  p->drawRect(x, y, w, h);
  
  p->restore();
}

  void
WebStyle::drawKickerAppletHandle
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 QBrush * fill
)
{
  p->save();

  p->setPen(g.mid());

  p->setBrush(0 == fill ? Qt::NoBrush : *fill);

  p->drawRect(x, y, w, h);
  
  p->restore();
}

  void
WebStyle::drawKickerTaskButton
(
 QPainter * p,
 int x,
 int y,
 int w,
 int h,
 const QColorGroup & g,
 const QString & text,
 bool active,
 QPixmap * icon,
 QBrush * /* fill */
)
{
  p->save();

  QColor bg;

  if (active)
  {
    p->setPen(g.light());
    bg = g.highlight();
  }
  else
  {
    p->setPen(g.mid());
    bg = g.button();
  }

  p->setBrush(bg);

  p->drawRect(x, y, w, h);

  if (text.isEmpty() && 0 == icon)
  {
    p->restore();
    return;
  }

  const int pxWidth = 20;

  int textPos = pxWidth;

  QRect br(buttonRect(x, y, w, h));

  if ((0 != icon) && !icon->isNull())
  {
    int dx = (pxWidth - icon->width())  / 2;
    int dy = (h - icon->height())       / 2;

    p->drawPixmap(br.x() + dx, dy, *icon);
  }

  QString s(text);

  static QString modStr =
    QString::fromUtf8("[") + i18n("modified") + QString::fromUtf8("]");

  int modStrPos = s.find(modStr);

  if (-1 != modStrPos)
  {
    // +1 because we include a space after the closing brace.
    s.remove(modStrPos, modStr.length() + 1);

    QPixmap modPixmap = SmallIcon("modified");

    int dx = (pxWidth - modPixmap.width())  / 2;
    int dy = (h       - modPixmap.height()) / 2;

    p->drawPixmap(br.x() + textPos + dx, dy, modPixmap);

    textPos += pxWidth;
  }

  if (!s.isEmpty())
  {
    if (p->fontMetrics().width(s) > br.width() - textPos)
    {
      int maxLen = br.width() - textPos - p->fontMetrics().width("...");

      while ((!s.isEmpty()) && (p->fontMetrics().width(s) > maxLen))
        s.truncate(s.length() - 1);

      s.append("...");
    }

    if (active)
    {
      p->setPen(contrastingForeground(g.buttonText(), bg));
    }
    else
    {
      p->setPen(contrastingForeground(g.text(), bg));
    }

    p->setPen(Qt::white);

    p->drawText
      (
       br.x() + textPos,
       -1,
       w - textPos,
       h,
       Qt::AlignLeft | Qt::AlignVCenter,
       s
      );
  }

  p->restore();
  p->setPen(Qt::white);
}

  int
WebStyle::popupMenuItemHeight(bool, QMenuItem * i, const QFontMetrics & fm)
{
  if (i->isSeparator())
    return 1;

  int h = 0;

  if (0 != i->pixmap())
  {
    h = i->pixmap()->height();
  }

  if (0 != i->iconSet())
  {
    h = QMAX
      (
       i->iconSet()->pixmap(QIcon::Small, QIcon::Normal).height(),
       h
      );
  }

  h = QMAX(fm.height() + 4, h);

  h = QMAX(18, h);

  return h;

}

