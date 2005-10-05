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

#ifndef WEB_STYLE_H
#define WEB_STYLE_H

#include <kstyle.h>
#include <qpalette.h>

class QPainter;
class QScrollBar;
class QPushButton;
class QWidget;

class WebStyle : public KStyle
{
  public:

    WebStyle();

    ~WebStyle();

    void polish(QApplication *);

    void unPolish(QWidget *);

    void polish(QWidget *);

    void polish(QPalette &);

    void unPolish(QApplication *);

    void drawButton
      (
       QPainter * p,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup & g,
       bool sunken = false,
       const QBrush * fill = 0
      );

    QRect buttonRect(int x, int y, int w, int h);

    void drawBevelButton
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       bool sunken = false,
       const QBrush * fill = 0
      );

    void drawPushButton(QPushButton *, QPainter *);

    virtual void drawPushButtonLabel(QPushButton *, QPainter *);

    void drawScrollBarControls
      (
       QPainter *,
       const QScrollBar *,
       int sliderStart,
       uint controls,
       uint activeControl
      );

    QStyle::ScrollControl scrollBarPointOver
      (
       const QScrollBar *,
       int sliderStart,
       const QPoint &
      );

    void scrollBarMetrics
      (
       const QScrollBar *,
       int & sliderMin,
       int & sliderMax,
       int & sliderLength,
       int & buttonDim
      );

    QSize indicatorSize() const;

    void drawIndicator
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       int state,
       bool down = false,
       bool enabled = true
      );

    QSize exclusiveIndicatorSize() const;

    void drawExclusiveIndicator
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       bool on,
       bool down = false,
       bool enabled = true
      );

    void drawIndicatorMask
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       int state
      );

    void drawExclusiveIndicatorMask
      (
       QPainter *,
       int x, 
       int y, 
       int w,
       int h, 
       bool on
      );

    void drawComboButton
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       bool sunken = false,
       bool editable = false, 
       bool enabled = true,
       const QBrush * fill = 0
      );

    QRect comboButtonRect(int x, int y, int w, int h);

    QRect comboButtonFocusRect(int x, int y, int w, int h);

    int sliderLength() const;

    void drawSliderGroove
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &,
       QCOORD, 
       Qt::Orientation
      );

    void drawArrow
      (
       QPainter *,
       Qt::ArrowType, 
       bool down,
       int x, 
       int y, 
       int w, 
       int h, 
       const QColorGroup &,
       bool enabled = true, 
       const QBrush * fill = 0
      );

    void drawSlider
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       Qt::Orientation,
       bool tickAbove, 
       bool tickBelow
      );

    void drawKToolBar
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       KToolBarPos,
       QBrush * fill = 0
      );

    void drawKBarHandle
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &,
       KToolBarPos, 
       QBrush * fill = 0
      );

    void drawKMenuBar
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       bool macMode,
       QBrush * fill = 0
      );

    void drawKToolBarButton
      (
       QPainter * p, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup & g, 
       bool sunken = false,
       bool raised = true, 
       bool enabled = true,
       bool popup = false,
       KToolButtonType = Icon,
       const QString & btext = QString::null,
       const QPixmap * = 0,
       QFont * = 0,
       QWidget * button = 0
      );

    void drawKMenuItem
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       bool active,
       QMenuItem *, 
       QBrush * fill = 0
      );

    void drawPopupMenuItem
      (
       QPainter *, 
       bool checkable, 
       int maxpmw,
       int tab,
       QMenuItem *, 
       const QPalette &,
       bool act, 
       bool enabled, 
       int x, 
       int y, 
       int w,
       int h
      );

    void drawKProgressBlock
      (
       QPainter *, 
       int x, 
       int y, 
       int w, 
       int h,
       const QColorGroup &, 
       QBrush * fill
      );

    void drawFocusRect
      (
       QPainter *, 
       const QRect &, 
       const QColorGroup &,
       const QColor * pen, 
       bool atBorder
      );

    void drawPanel
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       bool sunken,
       int lineWidth = 1,
       const QBrush * = 0
      );

    void drawPopupPanel
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       int lineWidth = 2,
       const QBrush * = 0
      );

    void drawSeparator
      (
       QPainter *,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup &,
       bool sunken = true,
       int lineWidth = 1,
       int midLineWidth = 0
      );

    void drawTab
      (
       QPainter * p,
       const QTabBar * tabBar,
       QTab * tab,
       bool selected
      );

    void drawTabMask
      (
       QPainter * p,
       const QTabBar *,
       QTab * tab,
       bool
      );

    void drawKickerHandle
      (
       QPainter * p,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup & g,
       QBrush *
      );

    void drawKickerAppletHandle
      (
       QPainter * p,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup & g,
       QBrush *
      );

    void drawKickerTaskButton
      (
       QPainter * p,
       int x,
       int y,
       int w,
       int h,
       const QColorGroup & g,
       const QString & title,
       bool active,
       QPixmap * icon,
       QBrush *
      );

    int popupMenuItemHeight(bool, QMenuItem *, const QFontMetrics &);

    Qt::GUIStyle guiStyle() const { return Qt::MotifStyle; }

    bool eventFilter(QObject *, QEvent *);
};

#endif
