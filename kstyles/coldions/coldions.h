/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Matthias Elter <elter@kde.org>
  Copyright (c) 1999 Mosfet <mosfet@kde.org>
 
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

#ifndef __coldionsstyle_h__
#define __coldionsstyle_h__

#include <limits.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qpushbutton.h>
#include <qscrollbar.h>

#include <kstyle.h>

class ColdIonsStyle : public KStyle
{
public:
    ColdIonsStyle();
    ~ColdIonsStyle();
    void polish(QWidget *w);
    void unPolish(QWidget *w);
    void drawPushButton(QPushButton *btn, QPainter *p);
    void drawPushButtonLabel(QPushButton *btn, QPainter *p);
    virtual void drawButtonMask(QPainter *p, int x, int y, int w, int h);
    void drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &cg, bool sunken,
                    const QBrush *fill=NULL);
    QRect buttonRect(int x, int y, int w, int h);
    void drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &cg, bool sunken, bool, bool,
                         const QBrush *fill);
    void drawComboButtonMask(QPainter *p, int x, int y, int w, int h);
    QRect comboButtonRect(int x, int y, int w, int h);
    void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken,
                         const QBrush *fill=NULL);
    void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool);
    void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, QBrush *fill=0);
    void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g, bool horizontal=false,
                        QBrush *fill=NULL);
    void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, bool active,
                       QMenuItem *item, QBrush *fill=NULL);
    void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            bool raised, bool enabled, bool popup,
                            KToolButtonType icontext,
                            const QString& btext, const QPixmap *pixmap,
                            QFont *font);
    void drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                               int sliderStart, unsigned int controls,
                               unsigned int activeControl);
    QSize indicatorSize() const;
    void drawIndicator(QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, int state, bool down, bool);
    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                int h, const QColorGroup &g, bool on,
                                bool down, bool);
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, QCOORD, Orientation);
    int sliderLength() const;
    void drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orient, bool, bool);
    void drawArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                   int y, int w, int h, const QColorGroup &g,
                   bool enabled=true, const QBrush *fill=NULL);
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                            int tab, QMenuItem* mi, const QPalette &pal,
                            bool act, bool enabled, int x, int y, int w,
                            int h);
    void drawLightShadeRect(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, const QBrush *fill=NULL);
protected:
    QBitmap *checkOutline, *checkFill;
};

#endif
