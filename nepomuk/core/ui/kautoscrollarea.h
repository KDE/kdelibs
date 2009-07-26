/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KAUTOSCROLL_AREA_H_
#define _KAUTOSCROLL_AREA_H_

#include <QtGui/QWidget>

class QEvent;

/**
 * KAutoScrollArea provides a scroll area for an arbitrary widget.
 * However, instead of scroll bars the widget is scrolled based on
 * the mouse pointer position.
 */
class KAutoScrollArea : public QWidget
{
    Q_OBJECT

 public:
    KAutoScrollArea( QWidget* parent = 0 );
    ~KAutoScrollArea();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /**
     * Set w to be the child widget which will be auto scrolled.
     * KAutoScrollArea will take ownership of the widget.
     */
    void setWidget( QWidget* w );

    /**
     * Auto scrolling can take place in two directions but only
     * one at a time. The other direction will not be scrollable
     * at all and KAutoScrollArea will adjust the child's size
     * to its own.
     */
    void setOrientation( Qt::Orientation );

 public Q_SLOTS:
    /**
     * Set the fade width to be used when the child widget is scrolled.
     * It can also be set to 0 to disable fading alltogether.
     * The default value is 20.
     */
    void setFadeWidth( int w );

 protected:
    bool eventFilter( QObject* obj, QEvent* event );
    void resizeEvent( QResizeEvent* e );

 private Q_SLOTS:
    void slotScroll();

 private:
    class Private;
    Private* d;
};

#endif
