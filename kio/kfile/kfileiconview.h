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

#ifndef KFILEICONVIEW_H
#define KFILEICONVIEW_H

class KFileItem;
class QWidget;
class QLabel;

#include <kiconview.h>
#include <kiconloader.h>
#include <kfileview.h>
#include <kmimetyperesolver.h>

#include <kfile.h>

/**
 * An item for the iconview, that has a reference to its corresponding
 * @ref KFileItem.
 */
class KFileIconViewItem : public KIconViewItem
{
public:
    KFileIconViewItem( QIconView *parent, const QString &text,
		       const QPixmap &pixmap,
		       KFileItem *fi )
	: KIconViewItem( parent, text, pixmap ), inf( fi ) {}
    KFileIconViewItem( QIconView *parent, KFileItem *fi )
	: KIconViewItem( parent ), inf( fi ) {}

    virtual ~KFileIconViewItem();

    /**
     * @returns the corresponding KFileItem
     */
    KFileItem *fileInfo() const {
	return inf;
    }

private:
    KFileItem *inf;

private:
    class KFileIconViewItemPrivate;
    KFileIconViewItemPrivate *d;

};

namespace KIO {
    class Job;
}

/**
 * An icon-view capable of showing @ref KFileItem's. Used in the filedialog
 * for example. Most of the documentation is in @ref KFileView class.
 *
 * @see KDirOperator
 * @see KCombiView
 * @see KFileDetailView
 */
class KFileIconView : public KIconView, public KFileView
{
    Q_OBJECT

public:
    KFileIconView(QWidget *parent, const char *name);
    virtual ~KFileIconView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();
    virtual void setAutoUpdate( bool ) {}

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem(const KFileItem*);

    virtual void listingCompleted();

    virtual void insertItem( KFileItem *i );
    virtual void setSelectionMode( KFile::SelectionMode sm );

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

    /**
     * Sets the size of the icons to show. Defaults to @ref KIcon::SizeSmall.
     */
    void setIconSize( int size );

    /**
     * Sets the size of the previews. Defaults to @ref KIcon::SizeLarge.
     */
    void setPreviewSize( int size );

    /**
     * @returns the current size used for icons.
     */
    int iconSize() const { return myIconSize; }

    void ensureItemVisible( const KFileItem * );

    virtual void setSorting(QDir::SortSpec sort);

    virtual void readConfig( KConfig *, const QString& group = QString::null );
    virtual void writeConfig( KConfig *, const QString& group = QString::null);

    // for KMimeTypeResolver
    void mimeTypeDeterminationFinished();
    void determineIcon( KFileIconViewItem *item );
    QScrollView *scrollWidget() const { return (QScrollView*) this; }

public slots:
    /**
     * Starts loading previews for all files shown and shows them. Switches
     * into 'large rows' mode, if that isn't the current mode yet.
     */
    void showPreviews();

protected:
    /**
     * Reimplemented to not let QIconView eat return-key events
     */
    virtual void keyPressEvent( QKeyEvent * );

    /**
     * Reimplemented to remove an eventual tooltip
     */
    virtual void hideEvent( QHideEvent * );

    // ### workaround for Qt3 bug (see #35080)
    virtual void showEvent( QShowEvent * );

    virtual bool eventFilter( QObject *o, QEvent *e );
    
private slots:
    void selected( QIconViewItem *item );
    void slotActivate( QIconViewItem * );
    void highlighted( QIconViewItem *item );
    void showToolTip( QIconViewItem *item );
    void removeToolTip();
    void slotActivateMenu( QIconViewItem *, const QPoint& );
    void slotSelectionChanged();

    void slotSmallColumns();
    void slotLargeRows();
    void slotPreviewsToggled( bool );

    void slotPreviewResult( KIO::Job * );
    void gotPreview( const KFileItem *item, const QPixmap& pix );

private:
    KMimeTypeResolver<KFileIconViewItem,KFileIconView> *m_resolver;

    QLabel *toolTip;
    int th;
    int myIconSize;

    virtual void insertItem(QIconViewItem *a, QIconViewItem *b) { KIconView::insertItem(a, b); }
    virtual void setSelectionMode(QIconView::SelectionMode m) { KIconView::setSelectionMode(m); }
    virtual void setSelected(QIconViewItem *i, bool a, bool b) { KIconView::setSelected(i, a, b); }

    bool canPreview( const KFileItem * ) const;
    void stopPreview();

    void updateIcons();

    inline KFileIconViewItem * viewItem( const KFileItem *item ) const {
        if ( item )
            return (KFileIconViewItem *) item->extraData( this );
        return 0L;
    }

    void initItem(KFileIconViewItem *item, const KFileItem *i );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileIconViewPrivate;
    KFileIconViewPrivate *d;
};

#endif // KFILESIMPLEVIEW_H
