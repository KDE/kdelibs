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
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kfilereader.h"
#include <qheader.h>
#include <qpixmap.h>
#include "kfiledetailview.h"
#include "qkeycode.h"
#include <qpainter.h>
#include <kapp.h>
#include "config-kfile.h"
#include <kfileviewitem.h>
#include <klocale.h>

KFileDetailView::KFileDetailView(QWidget *parent, const char *name)
    : KListView(parent, name), KFileView()
{
    KListView::setSorting( -1 );
    setViewName( i18n("Detailed View") );
    myLastItem = 0L;

    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Size" ) );
    addColumn( i18n( "Permissions" ) );
    addColumn( i18n( "Date" ) );
    addColumn( i18n( "Owner" ) );
    addColumn( i18n( "Group" ) );
    setShowSortIndicator( TRUE );
    setAllColumnsShowFocus( TRUE );
    header()->setClickEnabled( false );

    connect( this, SIGNAL( doubleClicked(QListViewItem *) ),
	     SLOT( selected( QListViewItem *) ) );
    connect( this, SIGNAL( returnPressed(QListViewItem *) ),
	     SLOT( selected( QListViewItem *) ) );
    connect( this, SIGNAL( currentChanged( QListViewItem *) ),
	     this, SLOT( highlighted( QListViewItem *)	) );
    connect( this, SIGNAL(rightButtonPressed ( QListViewItem *, const QPoint &, int )),
	     this, SLOT( rightButtonPressed ( QListViewItem * )));
    /*
      if ( KFileView::selectMode() == KFileView::Single )
      QListView::setSelectionMode( QListView::Single );
      else
      QListView::setSelectionMode( QListView::Extended );
    */
}

KFileDetailView::~KFileDetailView()
{
}

void KFileDetailView::highlightItem( const KFileViewItem *info )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    KFileListViewItem *item = (KFileListViewItem*)info->viewItem( this );

    if ( item != currentItem() ) {
        QListView::setCurrentItem( item );
	QListView::ensureItemVisible( item );
	setSelected( item, TRUE );
    }
}

void KFileDetailView::clearSelection()
{
    QListView::clearSelection();
}

void KFileDetailView::rightButtonPressed ( QListViewItem *item )
{
    if ( !item ) {
	activateMenu( 0 );
	return;
    }
    KFileListViewItem *i = (KFileListViewItem*)item;
    activateMenu( i->fileInfo() );
}

void KFileDetailView::clearView()
{
    QListView::clear();
    myLastItem = 0L;
}

void KFileDetailView::insertItem( KFileViewItem *i )
{
    KFileListViewItem *item = new KFileListViewItem( (QListView*) this,
						     i->name(), i->pixmap(), i,
						     myLastItem );

    item->setText( 1, QString::number( i->size() ) );
    item->setText( 2, i->access() );
    item->setText( 3, i->date() );
    item->setText( 4, i->owner() );
    item->setText( 5, i->group() );
    i->setViewItem( this, item );

    myLastItem = item;
}

void KFileDetailView::selected( QListViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi ) {
	select( const_cast<KFileViewItem*>( fi ) );
	// ContentsPos( 0, 0 ); // scroll to top left
    }
}

void KFileDetailView::highlighted( QListViewItem *item )
{
    if ( !item )
	return;
    const KFileViewItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
	highlight( const_cast<KFileViewItem*>( fi ) );
}

void KFileDetailView::setSelectMode( KFileView::SelectionMode sm )
{
    KFileView::setSelectMode( sm );
    /*
      if ( KFileView::selectMode() == KFileView::Single )
      QListView::setSelectionMode( QListView::Single );
      else
      QListView::setSelectionMode( QListView::Extended );
    */
}

bool KFileDetailView::isSelected( const KFileViewItem *i ) const
{
    if ( !i )
	return false;
    KFileListViewItem *item = (KFileListViewItem*) i->viewItem( this );
    return (item && item->isSelected());
}


void KFileDetailView::updateView( bool b )
{
    if ( !b )
	return;
    QListViewItemIterator it( (QListView*)this );
    for ( ; it.current(); ++it ) {
	KFileListViewItem *item = static_cast<KFileListViewItem *>( it.current() );
	item->setPixmap( 0, const_cast<KFileViewItem*>( item->fileInfo() )->pixmap() );
    }
}

void KFileDetailView::updateView( const KFileViewItem *i )
{
    if ( !i )
	return;

    // hack to make it not flicker
    qApp->processEvents();
    viewport()->setUpdatesEnabled( false );

    KFileListViewItem *item = (KFileListViewItem*) i->viewItem( this );
    item->setPixmap( 0, i->pixmap() );
    item->setText( 2, i->access() );

    qApp->processEvents();
    viewport()->setUpdatesEnabled( true );
    item->repaint(); // only repaints if visible
}

#include "kfiledetailview.moc"
