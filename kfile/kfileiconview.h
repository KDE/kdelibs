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

class KFileViewItem;
class QWidget;
class QLabel;

#include <kiconview.h>
#include <kiconloader.h>
#include <kfileview.h>

#include <kfile.h>

/**
 * An item for the iconview, that has a reference to its corresponding
 * @ref KFileViewItem.
 */
class KFileIconViewItem : public QIconViewItem
{
public:
    KFileIconViewItem( QIconView *parent, const QString &text,
		       const QPixmap &pixmap,
		       const KFileViewItem *fi )
	: QIconViewItem( parent, text, pixmap ), inf( fi ) {}

    virtual ~KFileIconViewItem();

    /**
     * @returns the corresponding KFileViewItem
     */
    const KFileViewItem *fileInfo() const {
	return inf;
    }

private:
    const KFileViewItem *inf;

private:
    class KFileIconViewItemPrivate;
    KFileIconViewItemPrivate *d;

};

/**
 * An icon-view capable of showing @ref KFileViewItems. Used in the filedialog
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
    virtual void updateView(const KFileViewItem*);
    virtual void removeItem(const KFileViewItem*);

    virtual void insertItem( KFileViewItem *i );
    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void setSelected(const KFileViewItem *, bool);
    virtual bool isSelected(const KFileViewItem *i) const;
    virtual void clearSelection();

    /**
     * Sets the size of the icons to show. Defaults to @ref KIcon::SizeSmall.
     */
    void setIconSize( int size );

    /**
     * @returns the current size used for icons.
     */
    int iconSize() const { return myIconSize; }

    void ensureItemVisible( const KFileViewItem * );

protected:
    /**
     * Reimplemented to not let QIconView eat return-key events
     */
    virtual void keyPressEvent( QKeyEvent * );

    /**
     * Reimplemented to remove an eventual tooltip
     */
    virtual void hideEvent( QHideEvent * );

private slots:
    void selected( QIconViewItem *item );
    void highlighted( QIconViewItem *item );
    void showToolTip( QIconViewItem *item );
    void removeToolTip();
    void slotRightButtonPressed( QIconViewItem * );
    void slotDoubleClicked( QIconViewItem * );
    void slotSelectionChanged();

    void slotSmallColumns();
    void slotLargeRows();

private:
    QLabel *toolTip;
    int th;
    int myIconSize;
    virtual void insertItem(QIconViewItem *a, QIconViewItem *b) { KIconView::insertItem(a, b); }
    virtual void setSelectionMode(QIconView::SelectionMode m) { KIconView::setSelectionMode(m); }
    virtual void setSelected(QIconViewItem *i, bool a, bool b) { KIconView::setSelected(i, a, b); }

    void readConfig();
    void writeConfig();

private:
    class KFileIconViewPrivate;
    KFileIconViewPrivate *d;
};

#endif // KFILESIMPLEVIEW_H
