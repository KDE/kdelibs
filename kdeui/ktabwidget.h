/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#ifndef KTABWIDGET_H
#define KTABWIDGET_H

#include <qtabwidget.h>
#include "ktabbar.h"

class KTabBar;
class KTabWidgetPrivate;

class KTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    KTabWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    void setTabColor( QWidget *, const QColor& color );
    QColor tabColor( QWidget * ) const;

/*!
    If \a enable is TRUE, tab reordering with middle button will be enabled.

    Note that once enabled you shouldn't rely on previously queried
    currentPageIndex() or indexOf( QWidget * ) values anymore.

    You can connect to signal movedTab(int, int) which will notify
    you from which index to which index a tab has been moved.
*/
    void setTabReorderingEnabled( bool enable );
    bool isTabReorderingEnabled() const;

protected slots:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dropEvent( QDropEvent * );
    virtual void moveTab( int, int );

signals:
    void tabbarContextMenu( const QPoint & );
    void receivedDropEvent( QDropEvent * );
    void receivedDropEvent( QWidget *, QDropEvent * );
    void dragInitiated( QWidget * );
    void movedTab( int, int );

    void contextMenu( QWidget *, const QPoint & );
    void mouseDoubleClick( QWidget * );
    void mouseMiddleClick( QWidget * );

private:
    bool isEmptyTabbarSpace( const QPoint & );

    bool mTabReordering;
    KTabBar *m_pTabBar;

    KTabWidgetPrivate *d;
};

#endif
