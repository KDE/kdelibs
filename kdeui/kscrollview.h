/* This file is part of the KDE libraries
   Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KSCROLLVIEW_H
#define KSCROLLVIEW_H

#include <QScrollArea>

#include <kdelibs_export.h>

/**
 * @short A variant of QScrollArea that supports smooth scrolling.
 **/
class KDEUI_EXPORT KScrollView : public QScrollArea
{
  Q_OBJECT

  public:
    KScrollView( QWidget *parent = 0 );

    ~KScrollView();

  protected:
    virtual void scrollContentsBy(int dx, int dy);
    virtual void wheelEvent( QWheelEvent *e );

    void startScrolling();
    void stopScrolling();

  protected Q_SLOTS:
    void scrollTick();

  private:
    // All scrolls must be completed within 240ms of last keypress
    static const int SCROLL_TIME = 240;
    // Each step is 20 ms == 50 frames/second
    static const int SCROLL_TICK = 20;

    struct KScrollViewPrivate;
    KScrollViewPrivate *const d;
};

#endif
