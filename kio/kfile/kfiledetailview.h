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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
 * KFileItem.
 */
class KIO_EXPORT KFileListViewItem : public KListViewItem
{
public:
    KFileListViewItem( QListView *parent, const QString &text,
		       const QPixmap &icon, KFileItem *fi )
	: KListViewItem( parent, text ), inf( fi ) {
        setPixmap( 0, icon );
        setText( 0, text );
    }

    /**
     * @since 3.1
     */
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

    /**
     * @since 3.1
     */
    void init();

private:
    KFileItem *inf;
    QString m_key;

private:
    class KFileListViewItemPrivate;
    KFileListViewItemPrivate *d;

};

/**
 * A list-view capable of showing KFileItem'. Used in the filedialog
 * for example. Most of the documentation is in KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileIconView
 */
class KIO_EXPORT KFileDetailView : public KListView, public KFileView
{
    Q_OBJECT

public:
    KFileDetailView(QWidget *parent, const char *name);
    virtual ~KFileDetailView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {} // ### unused. remove in KDE4

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

    virtual void readConfig( KConfig *, const QString& group = QString::null );
    virtual void writeConfig( KConfig *, const QString& group = QString::null);

signals:
    /**
     * The user dropped something.
     * @p fileItem points to the item dropped on or can be 0 if the
     * user dropped on empty space.
     * @since 3.2
     */
    void dropped(QDropEvent *event, KFileItem *fileItem);
    /**
     * The user dropped the URLs @p urls.
     * @p url points to the item dropped on or can be empty if the
     * user dropped on empty space.
     * @since 3.2
     */
    void dropped(QDropEvent *event, const KURL::List &urls, const KURL &url);

protected:
    virtual void keyPressEvent( QKeyEvent * );

    // DND support
    virtual QDragObject *dragObject();
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *ev );
    virtual bool acceptDrag(QDropEvent* e ) const;

    int m_sortingCol;

protected slots:
    void slotSelectionChanged();

private slots:
    void slotSortingChanged( int );
    void selected( QListViewItem *item );
    void slotActivate( QListViewItem *item );
    void highlighted( QListViewItem *item );
    void slotActivateMenu ( QListViewItem *item, const QPoint& pos );
    void slotAutoOpen();

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
