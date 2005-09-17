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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qevent.h>
#include <qnamespace.h>
#include <q3header.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kapplication.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
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

class KFileDetailView::KFileDetailViewPrivate
{
public:
   KFileDetailViewPrivate() : dropItem(0)
   { }

   KFileListViewItem *dropItem;
   QTimer autoOpenTimer;
};

KFileDetailView::KFileDetailView(QWidget *parent)
    : KListView(parent), KFileView(), d(new KFileDetailViewPrivate())
{
    // this is always the static section, not the index depending on column order
    m_sortingCol = COL_NAME;
    m_blockSortingSignal = false;
    setViewName( i18n("Detailed View") );

    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Size" ) );
    addColumn( i18n( "Date" ) );
    addColumn( i18n( "Permissions" ) );
    addColumn( i18n( "Owner" ) );
    addColumn( i18n( "Group" ) );
    setShowSortIndicator( true );
    setAllColumnsShowFocus( true );
    setDragEnabled(true);

    connect( header(), SIGNAL( clicked(int)),
             SLOT(slotSortingChanged(int) ));


    connect( this, SIGNAL( returnPressed(Q3ListViewItem *) ),
	     SLOT( slotActivate( Q3ListViewItem *) ) );

    connect( this, SIGNAL( clicked(Q3ListViewItem *, const QPoint&, int)),
	     SLOT( selected( Q3ListViewItem *) ) );
    connect( this, SIGNAL( doubleClicked(Q3ListViewItem *, const QPoint&, int)),
	     SLOT( slotActivate( Q3ListViewItem *) ) );

    connect( this, SIGNAL(contextMenuRequested( Q3ListViewItem *,
                                                const QPoint &, int )),
	     this, SLOT( slotActivateMenu( Q3ListViewItem *, const QPoint& )));

    KFile::SelectionMode sm = KFileView::selectionMode();
    switch ( sm ) {
    case KFile::Multi:
	Q3ListView::setSelectionMode( Q3ListView::Multi );
	break;
    case KFile::Extended:
	Q3ListView::setSelectionMode( Q3ListView::Extended );
	break;
    case KFile::NoSelection:
	Q3ListView::setSelectionMode( Q3ListView::NoSelection );
	break;
    default: // fall through
    case KFile::Single:
	Q3ListView::setSelectionMode( Q3ListView::Single );
	break;
    }

    // for highlighting
    if ( sm == KFile::Multi || sm == KFile::Extended )
	connect( this, SIGNAL( selectionChanged() ),
		 SLOT( slotSelectionChanged() ));
    else
	connect( this, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
		 SLOT( highlighted( Q3ListViewItem * ) ));

    // DND
    connect( &(d->autoOpenTimer), SIGNAL( timeout() ),
             this, SLOT( slotAutoOpen() ));

    setSorting( sorting() );

    m_resolver =
        new KMimeTypeResolver<KFileListViewItem,KFileDetailView>( this );
}

KFileDetailView::~KFileDetailView()
{
    delete m_resolver;
    delete d;
}

void KFileDetailView::readConfig( KConfig *config, const QString& group )
{
    restoreLayout( config, group );
}

void KFileDetailView::writeConfig( KConfig *config, const QString& group )
{
    saveLayout( config, group );
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

void KFileDetailView::slotActivateMenu (Q3ListViewItem *item,const QPoint& pos )
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

    KFileListViewItem *item = new KFileListViewItem( (Q3ListView*) this, i );

    setSortingKey( item, i );

    i->setExtraData( this, item );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );
}

void KFileDetailView::slotActivate( Q3ListViewItem *item )
{
    if ( !item )
        return;

    const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
    if ( fi )
        sig->activate( fi );
}

void KFileDetailView::selected( Q3ListViewItem *item )
{
    if ( !item )
        return;

    if ( KGlobalSettings::singleClick() ) {
        const KFileItem *fi = ( (KFileListViewItem*)item )->fileInfo();
        if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
            sig->activate( fi );
    }
}

void KFileDetailView::highlighted( Q3ListViewItem *item )
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
    disconnect( this, SIGNAL( selectionChanged( Q3ListViewItem * ) ));

    KFileView::setSelectionMode( sm );

    switch ( KFileView::selectionMode() ) {
    case KFile::Multi:
        Q3ListView::setSelectionMode( Q3ListView::Multi );
        break;
    case KFile::Extended:
        Q3ListView::setSelectionMode( Q3ListView::Extended );
        break;
    case KFile::NoSelection:
        Q3ListView::setSelectionMode( Q3ListView::NoSelection );
        break;
    default: // fall through
    case KFile::Single:
        Q3ListView::setSelectionMode( Q3ListView::Single );
        break;
    }

    if ( sm == KFile::Multi || sm == KFile::Extended )
        connect( this, SIGNAL( selectionChanged() ),
                 SLOT( slotSelectionChanged() ));
    else
        connect( this, SIGNAL( selectionChanged( Q3ListViewItem * )),
                 SLOT( highlighted( Q3ListViewItem * )));
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

    Q3ListViewItemIterator it( (Q3ListView*)this );
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
    // col is the section here, not the index!

    QDir::SortSpec sort = sorting();
    int sortSpec = -1;
    bool reversed = (col == m_sortingCol) && (sort & QDir::Reversed) == 0;
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

    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter ) {
        if ( e->state() & Qt::ControlModifier )
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

// Qt4 porting: once we use QListWidget, this becomes a reimplementation of
// virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;
// or better: something at the model level, instead?
Q3DragObject *KFileDetailView::dragObject()
{
    // create a list of the URL:s that we want to drag
    KURL::List urls;
    KFileItemListIterator it( * KFileView::selectedItems() );
    for ( ; it.current(); ++it ){
        urls.append( (*it)->url() );
    }
    QPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "kmultiple", KIcon::SizeSmall );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( KIcon::SizeSmall );

    QPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
#if 0 // there is no more kurldrag, this should use urls.addToMimeData( mimeData ) instead
    Q3DragObject* myDragObject = new KURLDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
#endif
    return 0;
}

void KFileDetailView::slotAutoOpen()
{
    d->autoOpenTimer.stop();
    if( !d->dropItem )
        return;

    KFileItem *fileItem = d->dropItem->fileInfo();
    if (!fileItem)
        return;

    if( fileItem->isFile() )
        return;

    if ( fileItem->isDir() || fileItem->isLink())
        sig->activate( fileItem );
}

bool KFileDetailView::acceptDrag(QDropEvent* e) const
{
    return KURL::List::canDecode( e->mimeData() ) &&
       (e->source()!= const_cast<KFileDetailView*>(this)) &&
       ( e->action() == QDropEvent::Copy
      || e->action() == QDropEvent::Move
      || e->action() == QDropEvent::Link );
}

void KFileDetailView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       d->dropItem = item;
       d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileDetailView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    if ( item ) {  // are we over an item ?
       if (d->dropItem != item)
       {
           d->dropItem = item;
           d->autoOpenTimer.start( autoOpenDelay() ); // restart timer
       }
    }
    else
    {
       d->dropItem = 0;
       d->autoOpenTimer.stop();
    }
}

void KFileDetailView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void KFileDetailView::contentsDropEvent( QDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptAction();     // Yes

    KFileListViewItem *item = dynamic_cast<KFileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KURL::List urls = KURL::List::fromMimeData( e->mimeData() );
    if ( !urls.isEmpty() )
    {
        emit dropped(e, urls, fileItem ? fileItem->url() : KURL());
        sig->dropURLs(fileItem, e, urls);
    }
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
