/*  -*- c++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>

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

#ifndef _KCOMBIVIEW_H
#define _KCOMBIVIEW_H

#include <qsplitter.h>
#include <klocale.h>

#include <kfile.h>
#include <kfileview.h>

class KFileIconView;
class QIconViewItem;

/**
 * This view is designed to combine two KFileViews into one widget, to show
 * directories on the left side and files on the right side.
 *
 * Methods like selectedItems() to query status _only_ work on the right side,
 * i.e. on the files.
 *
 * After creating the KCombiView, you need to supply the view shown in the
 * right, (see @ref setRight()). Available KFileView implementations are
 * @ref KFileIconView and @ref KFileDetailView.
 *
 * Most of the below methods are just implementations of the baseclass
 * @ref KFileView, so look there for documentation.
 *
 * @see KFileView
 * @see KFileIconView
 * @see KFileDetailView
 * @see KDirOperator
 */
class KCombiView : public QSplitter,
		   public KFileView
{
    Q_OBJECT

public:
    KCombiView( QWidget *parent, const char *name);
    virtual ~KCombiView();

    virtual QWidget *widget() { return this; }
    virtual void clearView();

    virtual void updateView( bool );
    virtual void updateView(const KFileItem*);
    virtual void removeItem( const KFileItem * );
    virtual void listingCompleted();

    /**
     * Sets the view to be shown in the right. You need to call this before
     * doing anything else with this widget.
     */
    void setRight(KFileView *view);

    virtual void setSelectionMode( KFile::SelectionMode sm );

    virtual void setSelected(const KFileItem *, bool);
    virtual bool isSelected( const KFileItem * ) const;
    virtual void clearSelection();
    virtual void selectAll();
    virtual void invertSelection();

    virtual void setCurrentItem( const KFileItem * );
    virtual KFileItem * currentFileItem() const;
    virtual KFileItem * firstFileItem() const;
    virtual KFileItem * nextItem( const KFileItem * ) const;
    virtual KFileItem * prevItem( const KFileItem * ) const;

    virtual void insertItem( KFileItem *i );
    virtual void clear();

    virtual void setSorting( QDir::SortSpec sort );

    void ensureItemVisible( const KFileItem * );

protected:
    KFileIconView *left;
    KFileView *right;

protected slots:
    void slotSortingChanged( QDir::SortSpec );

private:
    KFileView *focusView( KFileView *preferred ) const;
    
    // in nextItem() and prevItem(), we have to switch views, when the first
    // view returns 0L. So we need to remember which view was used in the
    // previous call to next/prevItem(). Yes, it's a hack, but it works for
    // some cases at least.
    mutable KFileView *m_lastViewForNextItem;
    mutable KFileView *m_lastViewForPrevItem;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KCombiViewPrivate;
    KCombiViewPrivate *d;

};

#endif
