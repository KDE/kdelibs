// -*- c++ -*-
/* This file is part of the KDE libraries
   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
                 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qevent.h>
#include <qkeycode.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapplication.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kdebug.h>

#include "kfiledetailview.h"
#include "config-kfile.h"

#define COL_NAME 0
#define COL_SIZE 1
#define COL_DATE 2
#define COL_PERM 3
#define COL_OWNER 4
#define COL_GROUP 5

KFileDetailView::KFileDetailView(QWidget *parent, const char *name)
    : KListView(parent, name), KFileView()
{
    m_sortingCol = COL_NAME;
    m_blockSortingSignal = false;
    setViewName( i18n("Detailed View") );

    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Size" ) );
    addColumn( i18n( "Date" ) );
    addColumn( i18n( "Permissions" ) );
    addColumn( i18n( "Owner" ) );
    addColumn( i18n( "Group" ) );
    setShowSortIndicator( TRUE );
    setAllColumnsShowFocus( TRUE );

    connect( header(), SIGNAL( sectionClicked(int)),
             SLOT(slotSortingChanged(int) ));


    connect( this, SIGNAL( returnPressed(QListViewItem *) ),
	     SLOT( slotActivate( QListViewItem *) ) );

    connect( this, SIGNAL( clicked(QListViewItem *, const QPoint&, int)),
	     SLOT( selected( QListViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(QListViewItem *, const QPoint&, int)),
	     SLOT( slotActivate( QListViewItem *) ) );

    connect( this, SIGNAL(contextMenuRequested( QListViewItem *,
                                                const QPoint &, int )),
	     this, SLOT( slotActivateMenu( QListViewItem *, const QPoint& )));

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


    m_resolver =
        new KMimeTypeResolver<KFileListViewItem,KFileDetailView>( this );
}

KFileDetailView::~KFileDetailView()
{
    delete m_resolver;
}

void KFileDetailView::setSelected( const KFileItem *info, bool enable )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    KFileListViewItem *item = (KFileListViewItem*)info->extraData( this );

    if ( item )
	KListView::setSelected( item, enable );
}

void KFileDetailView::setCurrentItem( const KFileItem *item )
{
    if ( !item )
        return;
    KFileListViewItem *it = (KFileListViewItem*) item->extraData( this );
    if ( it )
        KListView::setCurrentItem( it );
}

KFileItem * KFileDetailView::currentFileItem() const
{
    KFileListViewItem *current = static_cast<KFileListViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
}

void KFileDetailView::clearSelection()
{
    KListView::clearSelection();
}

void KFileDetailView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    KListView::selectAll( true );
}

void KFileDetailView::invertSelection()
{
    KListView::invertSelection();
}

void KFileDetailView::slotActivateMenu (QListViewItem *item,const QPoint& pos )
{
    if ( !item ) {
	sig->activateMenu( 0, pos );
	return;
    }
    KFileListViewItem *i = (KFileListViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void KFileDetailView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();
    KListView::clear();
}

void KFileDetailView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    KFileListViewItem *item = new KFileListViewItem( (QListView*) this, i );

    setSortingKey( item, i );

    i->setExtraData( this, item );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );
}

void KFileDetailView::slotActivate( QListViewItem *item )
{
    if ( !item )
	return;

    const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
	sig->activate( fi );
}

void KFileDetailView::selected( QListViewItem *item )
{
    if ( !item )
	return;

    if ( KGlobalSettings::singleClick() ) {
	const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
	if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
	    sig->activate( fi );
    }
}

void KFileDetailView::highlighted( QListViewItem *item )
{
    if ( !item )
	return;

    const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
	sig->highlightFile( fi );
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

bool KFileDetailView::isSelected( const KFileItem *i ) const
{
    if ( !i )
	return false;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    return (item && item->isSelected());
}


void KFileDetailView::updateView( bool b )
{
    if ( !b )
	return;

    QListViewItemIterator it( (QListView*)this );
    for ( ; it.current(); ++it ) {
	KFileListViewItem *item=static_cast<KFileListViewItem *>(it.current());
	item->setPixmap( 0, item->fileInfo()->pixmap(KIcon::SizeSmall) );
    }
}

void KFileDetailView::updateView( const KFileItem *i )
{
    if ( !i )
	return;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    if ( !item )
	return;

    item->init();
    setSortingKey( item, i );

    //item->repaint(); // only repaints if visible
}

void KFileDetailView::setSortingKey( KFileListViewItem *item,
                                     const KFileItem *i )
{
    // see also setSorting()
    QDir::SortSpec spec = KFileView::sorting();

    if ( spec & QDir::Time )
        item->setKey( sortingKey( i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & QDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));
}


void KFileDetailView::removeItem( const KFileItem *i )
{
    if ( !i )
	return;

    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    m_resolver->m_lstPendingMimeIconItems.remove( item );
    delete item;

    KFileView::removeItem( i );
}

void KFileDetailView::slotSortingChanged( int col )
{
    QDir::SortSpec sort = sorting();
    int sortSpec = -1;
    bool reversed = col == m_sortingCol && (sort & QDir::Reversed) == 0;
    m_sortingCol = col;

    switch( col ) {
        case COL_NAME:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Name);
            break;
        case COL_SIZE:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Size);
            break;
        case COL_DATE:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Time);
            break;

        // the following columns have no equivalent in QDir, so we set it
        // to QDir::Unsorted and remember the column (m_sortingCol)
        case COL_OWNER:
        case COL_GROUP:
        case COL_PERM:
            // grmbl, QDir::Unsorted == SortByMask.
            sortSpec = (sort & ~QDir::SortByMask);// | QDir::Unsorted;
            break;
        default:
            break;
    }

    if ( reversed )
        sortSpec |= QDir::Reversed;
    else
        sortSpec &= ~QDir::Reversed;

    if ( sort & QDir::IgnoreCase )
        sortSpec |= QDir::IgnoreCase;
    else
        sortSpec &= ~QDir::IgnoreCase;


    KFileView::setSorting( static_cast<QDir::SortSpec>( sortSpec ) );

    KFileItem *item;
    KFileItemListIterator it( *items() );

    if ( sortSpec & QDir::Time ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), sortSpec ));
    }

    else if ( sortSpec & QDir::Size ) {
        for ( ; (item = it.current()); ++it )
            viewItem(item)->setKey( sortingKey( item->size(), item->isDir(),
                                                sortSpec ));
    }
    else { // Name or Unsorted -> use column text
        for ( ; (item = it.current()); ++it ) {
            KFileListViewItem *i = viewItem( item );
            i->setKey( sortingKey( i->text(m_sortingCol), item->isDir(),
                                   sortSpec ));
        }
    }

    KListView::setSorting( m_sortingCol, !reversed );
    KListView::sort();

    if ( !m_blockSortingSignal )
        sig->changeSorting( static_cast<QDir::SortSpec>( sortSpec ) );
}


void KFileDetailView::setSorting( QDir::SortSpec spec )
{
    int col = 0;
    if ( spec & QDir::Time )
        col = COL_DATE;
    else if ( spec & QDir::Size )
        col = COL_SIZE;
    else if ( spec & QDir::Unsorted )
        col = m_sortingCol;
    else
        col = COL_NAME;

    // inversed, because slotSortingChanged will reverse it
    if ( spec & QDir::Reversed )
        spec = (QDir::SortSpec) (spec & ~QDir::Reversed);
    else
        spec = (QDir::SortSpec) (spec | QDir::Reversed);

    m_sortingCol = col;
    KFileView::setSorting( (QDir::SortSpec) spec );


    // don't emit sortingChanged() when called via setSorting()
    m_blockSortingSignal = true; // can't use blockSignals()
    slotSortingChanged( col );
    m_blockSortingSignal = false;
}


void KFileDetailView::ensureItemVisible( const KFileItem *i )
{
    if ( !i )
	return;
    KFileListViewItem *item = (KFileListViewItem*) i->extraData( this );
    if ( item )
	KListView::ensureItemVisible( item );
}

// we're in multiselection mode
void KFileDetailView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

KFileItem * KFileDetailView::firstFileItem() const
{
    KFileListViewItem *item = static_cast<KFileListViewItem*>( firstChild() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * KFileDetailView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemBelow() )
            return ((KFileListViewItem*) item->itemBelow())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

KFileItem * KFileDetailView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        KFileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemAbove() )
            return ((KFileListViewItem*) item->itemAbove())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

void KFileDetailView::keyPressEvent( QKeyEvent *e )
{
    KListView::keyPressEvent( e );

    if ( e->key() == Key_Return || e->key() == Key_Enter ) {
        if ( e->state() & ControlButton )
            e->ignore();
        else
            e->accept();
    }
}

//
// mimetype determination on demand
//
void KFileDetailView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void KFileDetailView::determineIcon( KFileListViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void KFileDetailView::listingCompleted()
{
    m_resolver->start();
}
/////////////////////////////////////////////////////////////////


void KFileListViewItem::init()
{
    KFileListViewItem::setPixmap( COL_NAME, inf->pixmap(KIcon::SizeSmall));

    setText( COL_NAME, inf->text() );
    setText( COL_SIZE, KGlobal::locale()->formatNumber( inf->size(), 0));
    setText( COL_DATE,  inf->timeString() );
    setText( COL_PERM,  inf->permissionsString() );
    setText( COL_OWNER, inf->user() );
    setText( COL_GROUP, inf->group() );
}


void KFileDetailView::virtual_hook( int id, void* data )
{ KListView::virtual_hook( id, data );
  KFileView::virtual_hook( id, data ); }

#include "kfiledetailview.moc"
