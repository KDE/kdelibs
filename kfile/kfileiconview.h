// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#ifndef KFILESIMPLEVIEW_H
#define KFILESIMPLEVIEW_H

class KFileViewItem;
class QWidget;
class QLabel;

#include <kiconview.h>
#include "kfileview.h"

class KFileIconViewItem : public QIconViewItem
{
    Q_OBJECT

public:
    KFileIconViewItem( QIconView *parent, const QString &text,
		       const QPixmap &pixmap,
		       const KFileViewItem *fi )
	: QIconViewItem( parent, text, pixmap ), inf( fi ) {}

    virtual ~KFileIconViewItem();

    const KFileViewItem *fileInfo() const {
	return inf;
    }

private:
    const KFileViewItem *inf;

};

class KFileIconView : public KIconView, public KFileView
{
    Q_OBJECT

public:
    KFileIconView(QWidget *parent, const char *name);
    virtual ~KFileIconView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {}

    virtual void setSelectMode( KFileView::SelectionMode sm );
    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);

    virtual void insertItem( KFileViewItem *i );
    virtual void clearSelection();

protected:
    virtual void highlightItem(const KFileViewItem *);

private slots:
    void selected( QIconViewItem *item );
    void highlighted( QIconViewItem *item );
    void showToolTip( QIconViewItem *item );
    void removeToolTip();
    void slotRightButtonPressed( QIconViewItem * );

private:
    QLabel *toolTip;
    int th;

};

#endif // KFILESIMPLEVIEW_H
