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

/**
 * An item for the listiew, that has a reference to its corresponding
 * @ref KFileViewItem.
 */
class KFileListViewItem : public KListViewItem
{
public:
    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, KFileViewItem *fi )
	: KListViewItem( parent, text ), inf( fi ) {
	    setPixmap( 0, icon );
    }
    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, KFileViewItem *fi,
		       QListViewItem *after)
	: KListViewItem( parent, after ), inf( fi ) {
	    setPixmap( 0, icon );
	    setText( 0, text );
    }

    /**
     * @returns the corresponding KFileViewItem
     */
    KFileViewItem *fileInfo() const {
	return inf;
    }

private:
    KFileViewItem *inf;

private:
    class KFileListViewItemPrivate;
    KFileListViewItemPrivate *d;

};

/**
 * A list-view capable of showing @ref KFileViewItem'. Used in the filedialog
 * for example. Most of the documentation is in @ref KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileIconView
 */
class KFileDetailView : public KListView, public KFileView
{
    Q_OBJECT

public:
    KFileDetailView(QWidget *parent, const char *name);
    virtual ~KFileDetailView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {}

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void updateView( bool );
    virtual void updateView(const KFileViewItem*);
    virtual void removeItem( const KFileViewItem *);

    virtual void setSelected(const KFileViewItem *, bool);
    virtual bool isSelected(const KFileViewItem *i) const;
    virtual void clearSelection();

    virtual void setCurrentItem( const KFileViewItem * );
    virtual KFileViewItem * currentFileItem() const;

    virtual void insertItem( KFileViewItem *i );

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void setSorting( QDir::SortSpec );
    virtual void sortReversed();

    void ensureItemVisible( const KFileViewItem * );

protected:
    void setSortIndicator();

    QListViewItem *myLastItem;

protected slots:
    void slotSelectionChanged();

private slots:
    void slotSortingChanged( int );
    void selected( QListViewItem *item );
    void slotDoubleClicked( QListViewItem *item );
    void highlighted( QListViewItem *item );
    void rightButtonPressed ( QListViewItem *item );

private:
    virtual void insertItem(QListViewItem *i) { KListView::insertItem(i); }
    virtual void setSorting(int i, bool b) { KListView::setSorting(i, b); }
    virtual void setSelected(QListViewItem *i, bool b) { KListView::setSelected(i, b); }
    class KFileDetailViewPrivate;
    KFileDetailViewPrivate *d;
};

#endif // KFILEDETAILVIEW_H
