// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qkeycode.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kfileviewitem.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>

#include "kfiledetailview.h"
#include "config-kfile.h"

#define COL_NAME 0
#define COL_SIZE 1
#define COL_PERM 2
#define COL_DATE 3
#define COL_OWNER 4
#define COL_GROUP 5

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

    QHeader *h = header();
    h->setClickEnabled( false, COL_PERM );
    h->setClickEnabled( false, COL_OWNER);
    h->setClickEnabled( false, COL_GROUP );
    connect( h, SIGNAL( sectionClicked(int)), SLOT(slotSortingChanged(int) ));


    connect( this, SIGNAL( returnPressed(QListViewItem *) ),
	     SLOT( selected( QListViewItem *) ) );

    connect( this, SIGNAL( clicked(QListViewItem *, const QPoint&, int)),
	     SLOT( selected( QListViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(QListViewItem *, const QPoint&, int)),
	     SLOT( slotDoubleClicked( QListViewItem *) ) );

    connect( this, SIGNAL(rightButtonPressed( QListViewItem *, const QPoint &,
					      int )),
	     this, SLOT( rightButtonPressed ( QListViewItem * )));

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	QListView::setSelectionMode( QListView::Multi );
	break;
    case KFile::Extended:
	QListView::setSelectionMode( QListView::Extended );
	break;
    case KFile::NoSelection:
	QListView::setSelectionMode( QListView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	QListView::setSelectionMode( QListView::Single );
	break;
    }

    // for highlighting
    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( QListViewItem * ) ),
		 SLOT( highlighted( QListViewItem * ) ));

    setSorting( sorting() );
}

KFileDetailView::~KFileDetailView()
{
}

void KFileDetailView::setSelected( const KFileViewItem *info, bool enable )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    KFileListViewItem *item = (KFileListViewItem*)info->viewItem( this );

    if ( item && item != currentItem() ) {
        KListView::setCurrentItem( item );
	KListView::setSelected( item, enable );
    }
}

void KFileDetailView::clearSelection()
{
    KListView::clearSelection();
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
    KListView::clear();
    myLastItem = 0L;
}

void KFileDetailView::insertItem( KFileViewItem *i )
{
    KFileListViewItem *item = new KFileListViewItem( (QListView*) this,
						     i->name(), i->pixmap(), i,
						     myLastItem );

    item->setText( 1, KGlobal::locale()->formatNumber( i->size(), 0 ) );
    item->setText( 2, i->access() );
    item->setText( 3, i->date() );
    item->setText( 4, i->user() );
    item->setText( 5, i->group() );
    i->setViewItem( this, item );

    myLastItem = item;
}

void KFileDetailView::slotDoubleClicked( QListViewItem *item )
{
    if ( !item )
	return;

    const KFileViewItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
	select( const_cast<KFileViewItem*>( fi ) );
}

void KFileDetailView::selected( QListViewItem *item )
{
    if ( !item )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileViewItem *fi = ( (KFileListViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    select( const_cast<KFileViewItem*>( fi ) );
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


void KFileDetailView::setSelectionMode( KFile::SelectionMode sm )
{
    disconnect( this, SIGNAL( selectionChanged() ));
    disconnect( this, SIGNAL( selectionChanged( QListViewItem * ) ));

    KFileView::setSelectionMode( sm );

    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
	QListView::setSelectionMode( QListView::Multi );
	break;
    case KFile::Extended:
	QListView::setSelectionMode( QListView::Extended );
	break;
    case KFile::NoSelection:
	QListView::setSelectionMode( QListView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	QListView::setSelectionMode( QListView::Single );
	break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( QListViewItem * )),
		 SLOT( highlighted( QListViewItem * )));
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
	KFileListViewItem *item=static_cast<KFileListViewItem *>(it.current());
	item->setPixmap( 0, const_cast<KFileViewItem*>( item->fileInfo() )->pixmap() );
    }
}

void KFileDetailView::updateView( const KFileViewItem *i )
{
    if ( !i )
	return;

    KFileListViewItem *item = (KFileListViewItem*) i->viewItem( this );
    if ( !item )
	return;

    item->setPixmap( 0, i->pixmap() );
    item->setText( 2, i->access() );

    //item->repaint(); // only repaints if visible
}


void KFileDetailView::removeItem( const KFileViewItem *i )
{
    if ( !i )
	return;

    delete (KFileListViewItem*) i->viewItem( this );
    KFileView::removeItem( i );
}

void KFileDetailView::slotSortingChanged( int col )
{
    KListView::setSorting( -1 );
    QDir::SortSpec sort = sorting();
    int sortSpec = -1;
    bool reversed = false;

    switch( col ) {
    case COL_NAME:
	if ( (sort & QDir::Size) == 0 && (sort & QDir::Time) == 0 )
	    reversed = true;
	else
	    sortSpec = sort & ~QDir::SortByMask | QDir::Name;
	break;
    case COL_SIZE:
	if ( (sort & QDir::Size) == QDir::Size )
	    reversed = true;
	else
	    sortSpec = sort & ~QDir::SortByMask | QDir::Size;
	break;
    case COL_DATE:
	if ( (sort & QDir::Time) == QDir::Time )
	    reversed = true;
	else
	    sortSpec = sort & ~QDir::SortByMask | QDir::Time;
	break;
    default: // FIXME sort by group.... only this view can do this for now
	break;
    }

    const KFileViewItem *current = 0L;
    if ( currentItem() )
        current = ((KFileListViewItem*)currentItem())->fileInfo();

    if ( sortSpec != -1 )
	KFileView::setSorting( static_cast<QDir::SortSpec>( sortSpec ) );

    if ( reversed )
	sortReversed();
    else
	header()->setSortIndicator( col, !isReversed() );

    if ( current && (sortSpec != -1 || reversed) )
        KListView::setCurrentItem((QListViewItem*)current->viewItem(this));
}


void KFileDetailView::setSorting( QDir::SortSpec spec )
{
    kdDebug(kfile_area) << "KFileDetailView::setSorting " << count() << endl;
    KFileView::setSorting( spec );
    setSortIndicator();
}


void KFileDetailView::sortReversed()
{
    KFileView::sortReversed();
    setSortIndicator();
}

void KFileDetailView::setSortIndicator()
{
    kdDebug(kfile_area) << "setSortingIndicator " << endl;

    QDir::SortSpec spec = sorting();
    int col = -1;

    if ( (spec & QDir::Size) == QDir::Size )
	col = COL_SIZE;
    else if (  (spec & QDir::Time) == QDir::Time )
	col = COL_DATE;
    else if ( (spec & QDir::Name) == QDir::Name )
	col = COL_NAME;

    header()->setSortIndicator( col, !isReversed() );
}


void KFileDetailView::ensureItemVisible( const KFileViewItem *i )
{
    if ( !i )
	return;
    KFileListViewItem *item = (KFileListViewItem*) i->viewItem( this );
    if ( item )
	KListView::ensureItemVisible( item );
}

// we're in multiselection mode
void KFileDetailView::slotSelectionChanged()
{
    highlight( 0L );
}

#include "kfiledetailview.moc"
