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

class KFileItem;
class QWidget;
class QKeyEvent;

#include <klistview.h>
#include <kmimetyperesolver.h>

#include "kfileview.h"

/**
 * An item for the listiew, that has a reference to its corresponding
 * @ref KFileItem.
 */
class KFileListViewItem : public KListViewItem
{
public:
    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, KFileItem *fi )
	: KListViewItem( parent, text ), inf( fi ) {
	    setPixmap( 0, icon );
        setText( 0, text );
    }

    KFileListViewItem( QListView *parent, KFileItem *fi )
        : KListViewItem( parent ), inf( fi ) {
        init();
    }

    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, KFileItem *fi,
		       QListViewItem *after)
	: KListViewItem( parent, after ), inf( fi ) {
	    setPixmap( 0, icon );
	    setText( 0, text );
    }
    ~KFileListViewItem() {
        inf->removeExtraData( listView() );
    }

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

    virtual QString key( int /*column*/, bool /*ascending*/ ) const {
        return m_key;
    }

    void setKey( const QString& key ) { m_key = key; }

    QRect rect() const
    {
        QRect r = listView()->itemRect(this);
        return QRect( listView()->viewportToContents( r.topLeft() ),
                      QSize( r.width(), r.height() ) );
    }

    void init();

private:
    KFileItem *inf;
    QString m_key;

private:
    class KFileListViewItemPrivate;
    KFileListViewItemPrivate *d;

};

/**
 * A list-view capable of showing @ref KFileItem'. Used in the filedialog
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
    virtual void updateView(const KFileItem*);
    virtual void removeItem( const KFileItem *);
    virtual void listingCompleted();

    virtual void setSelected(const KFileItem *, bool);
    virtual bool isSelected(const KFileItem *i) const;
    virtual void clearSelection();
    virtual void selectAll();
    virtual void invertSelection();

    virtual void setCurrentItem( const KFileItem * );
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem( const KFileItem * ) const;
    virtual KFileItem * prevItem( const KFileItem * ) const;

    virtual void insertItem( KFileItem *i );

    // implemented to get noticed about sorting changes (for sortingIndicator)
    virtual void setSorting( QDir::SortSpec );

    void ensureItemVisible( const KFileItem * );

    // for KMimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( KFileListViewItem *item );
    QScrollView *scrollWidget() const { return (QScrollView*) this; }


protected:
    virtual void keyPressEvent( QKeyEvent * );

    int m_sortingCol;

protected slots:
    void slotSelectionChanged();

private slots:
    void slotSortingChanged( int );
    void selected( QListViewItem *item );
    void slotActivate( QListViewItem *item );
    void highlighted( QListViewItem *item );
    void slotActivateMenu ( QListViewItem *item, const QPoint& pos );

private:
    virtual void insertItem(QListViewItem *i) { KListView::insertItem(i); }
    virtual void setSorting(int i, bool b) { KListView::setSorting(i, b); }
    virtual void setSelected(QListViewItem *i, bool b) { KListView::setSelected(i, b); }

    inline KFileListViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (KFileListViewItem *) item->extraData( this );
        return 0L;
    }

    void setSortingKey( KFileListViewItem *item, const KFileItem *i );


    bool m_blockSortingSignal;
    KMimeTypeResolver<KFileListViewItem,KFileDetailView> *m_resolver;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileDetailViewPrivate;
    KFileDetailViewPrivate *d;
};

#endif // KFILEDETAILVIEW_H
