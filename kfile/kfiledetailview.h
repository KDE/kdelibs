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

#ifndef KFILEDETAILVIEW_H
#define KFILEDETAILVIEW_H

class KFileViewItem;
class QWidget;

#include <klistview.h>
#include "kfileview.h"

class KFileListViewItem : public QListViewItem
{
public:
    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, const KFileViewItem *fi )
	: QListViewItem( parent, text ), inf( fi ) {
	    setPixmap( 0, icon );
    }

    const KFileViewItem *fileInfo() const {
	return inf;
    }

private:
    const KFileViewItem *inf;

};

class KFileDetailView : public KListView, public KFileView
{
    friend class KSimpleCombiView;
    friend class KDetailCombiView;

    Q_OBJECT

public:
    KFileDetailView(QWidget *parent, const char *name);
    virtual ~KFileDetailView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {}

    virtual void setSelectMode( KFileView::SelectionMode sm );

    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);
    virtual void clearSelection();

protected:
    virtual void highlightItem(const KFileViewItem *);
    virtual void insertItem( KFileViewItem *i );

private slots:
    void selected( QListViewItem *item );
    void highlighted( QListViewItem *item );
    void rightButtonPressed ( QListViewItem *item );
};

#endif // KFILEDETAILVIEW_H
