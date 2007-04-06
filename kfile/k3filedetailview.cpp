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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k3filedetailview.h"
#include <QtGui/QActionEvent>
#include <QtCore/QCOORD>
#include <Qt3Support/Q3Header>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <kapplication.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kdebug.h>

#include "config-kfile.h"

#define COL_NAME 0
#define COL_SIZE 1
#define COL_DATE 2
#define COL_PERM 3
#define COL_OWNER 4
#define COL_GROUP 5

class K3FileDetailView::K3FileDetailViewPrivate
{
public:
   K3FileDetailViewPrivate() : dropItem(0)
   { }

   K3FileListViewItem *dropItem;
   QTimer autoOpenTimer;
};

K3FileDetailView::K3FileDetailView(QWidget *parent)
    : K3ListView(parent), KFileView(), d(new K3FileDetailViewPrivate())
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
        new K3MimeTypeResolver<K3FileListViewItem,K3FileDetailView>( this );
}

K3FileDetailView::~K3FileDetailView()
{
    delete m_resolver;
    delete d;
}

void K3FileDetailView::readConfig( KConfig *config, const QString& group )
{
    restoreLayout( config, group );
}

void K3FileDetailView::writeConfig( KConfig *config, const QString& group )
{
    saveLayout( config, group );
}

void K3FileDetailView::setSelected( const KFileItem *info, bool enable )
{
    if ( !info )
	return;

    // we can only hope that this casts works
    K3FileListViewItem *item = (K3FileListViewItem*)info->extraData( this );

    if ( item )
	K3ListView::setSelected( item, enable );
}

void K3FileDetailView::setCurrentItem( const KFileItem *item )
{
    if ( !item )
        return;
    K3FileListViewItem *it = (K3FileListViewItem*) item->extraData( this );
    if ( it )
        K3ListView::setCurrentItem( it );
}

KFileItem * K3FileDetailView::currentFileItem() const
{
    K3FileListViewItem *current = static_cast<K3FileListViewItem*>( currentItem() );
    if ( current )
        return current->fileInfo();

    return 0L;
}

void K3FileDetailView::clearSelection()
{
    K3ListView::clearSelection();
}

void K3FileDetailView::selectAll()
{
    if (KFileView::selectionMode() == KFile::NoSelection ||
        KFileView::selectionMode() == KFile::Single)
	return;

    K3ListView::selectAll( true );
}

void K3FileDetailView::invertSelection()
{
    K3ListView::invertSelection();
}

void K3FileDetailView::slotActivateMenu (Q3ListViewItem *item,const QPoint& pos )
{
    if ( !item ) {
        sig->activateMenu( 0, pos );
        return;
    }
    K3FileListViewItem *i = (K3FileListViewItem*) item;
    sig->activateMenu( i->fileInfo(), pos );
}

void K3FileDetailView::clearView()
{
    m_resolver->m_lstPendingMimeIconItems.clear();
    K3ListView::clear();
}

void K3FileDetailView::insertItem( KFileItem *i )
{
    KFileView::insertItem( i );

    K3FileListViewItem *item = new K3FileListViewItem( (Q3ListView*) this, i );

    setSortingKey( item, i );

    i->setExtraData( this, item );

    if ( !i->isMimeTypeKnown() )
        m_resolver->m_lstPendingMimeIconItems.append( item );
}

void K3FileDetailView::slotActivate( Q3ListViewItem *item )
{
    if ( !item )
        return;

    const KFileItem *fi = ( (K3FileListViewItem*)item )->fileInfo();
    if ( fi )
        sig->activate( fi );
}

void K3FileDetailView::selected( Q3ListViewItem *item )
{
    if ( !item )
        return;

    if ( KGlobalSettings::singleClick() ) {
        const KFileItem *fi = ( (K3FileListViewItem*)item )->fileInfo();
        if ( fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()) )
            sig->activate( fi );
    }
}

void K3FileDetailView::highlighted( Q3ListViewItem *item )
{
    if ( !item )
        return;

    const KFileItem *fi = ( (K3FileListViewItem*)item )->fileInfo();
    if ( fi )
        sig->highlightFile( fi );
}


void K3FileDetailView::setSelectionMode( KFile::SelectionMode sm )
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

bool K3FileDetailView::isSelected( const KFileItem *i ) const
{
    if ( !i )
        return false;

    K3FileListViewItem *item = (K3FileListViewItem*) i->extraData( this );
    return (item && item->isSelected());
}


void K3FileDetailView::updateView( bool b )
{
    if ( !b )
        return;

    Q3ListViewItemIterator it( (Q3ListView*)this );
    for ( ; it.current(); ++it ) {
        K3FileListViewItem *item=static_cast<K3FileListViewItem *>(it.current());
        item->setPixmap( 0, item->fileInfo()->pixmap(K3Icon::SizeSmall) );
    }
}

void K3FileDetailView::updateView( const KFileItem *i )
{
    if ( !i )
        return;

    K3FileListViewItem *item = (K3FileListViewItem*) i->extraData( this );
    if ( !item )
        return;

    item->init();
    setSortingKey( item, i );

    //item->repaint(); // only repaints if visible
}

void K3FileDetailView::setSortingKey( K3FileListViewItem *item,
                                     const KFileItem *i )
{
    // see also setSorting()
    QDir::SortFlags spec = KFileView::sorting();

    if ( spec & QDir::Time )
        item->setKey( sortingKey( i->time( KIO::UDS_MODIFICATION_TIME ),
                                  i->isDir(), spec ));
    else if ( spec & QDir::Size )
        item->setKey( sortingKey( i->size(), i->isDir(), spec ));

    else // Name or Unsorted
        item->setKey( sortingKey( i->text(), i->isDir(), spec ));
}


void K3FileDetailView::removeItem( const KFileItem *i )
{
    if ( !i )
        return;

    K3FileListViewItem *item = (K3FileListViewItem*) i->extraData( this );
    m_resolver->m_lstPendingMimeIconItems.removeAll( item );
    delete item;

    KFileView::removeItem( i );
}

void K3FileDetailView::slotSortingChanged( int col )
{
    // col is the section here, not the index!

    QDir::SortFlags sort = sorting();
    QDir::SortFlags sortSpec = QDir::Unsorted;
    bool reversed = (col == m_sortingCol) && (sort & QDir::Reversed) == 0;
    m_sortingCol = col;

    switch( col ) {
        case COL_NAME:
            sortSpec = ((sort & ~QDir::SortByMask) | QDir::Name);
            break;
        case COL_SIZE:
            sortSpec = ((sort & ~QDir::SortByMask) | QDir::Size);
            break;
        case COL_DATE:
            sortSpec = ((sort & ~QDir::SortByMask) | QDir::Time);
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


    KFileView::setSorting( sortSpec );

    const KFileItemList itemList = *items();
    KFileItemList::const_iterator kit = itemList.begin();
    const KFileItemList::const_iterator kend = itemList.end();
    for ( ; kit != kend; ++kit ) {
        KFileItem *item = *kit;
        K3FileListViewItem *i = viewItem( item );
        QString key;
        if ( sortSpec & QDir::Time ) {
            // warning, time_t is often signed -> cast it
            key = sortingKey( (unsigned long)item->time( KIO::UDS_MODIFICATION_TIME ), item->isDir(), sortSpec );
        }
        else if ( sortSpec & QDir::Size ) {
            key = sortingKey( item->size(), item->isDir(), sortSpec );
        }
        else { // Name or Unsorted -> use column text
            key = sortingKey( i->text(m_sortingCol), item->isDir(), sortSpec );
        }
        i->setKey( key );
    }

    K3ListView::setSorting( m_sortingCol, !reversed );
    K3ListView::sort();

    if ( !m_blockSortingSignal )
        sig->changeSorting( sortSpec );
}


void K3FileDetailView::setSorting( QDir::SortFlags spec )
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
        spec &= ~QDir::Reversed;
    else
        spec |= QDir::Reversed;

    m_sortingCol = col;
    KFileView::setSorting( spec );


    // don't emit sortingChanged() when called via setSorting()
    m_blockSortingSignal = true; // can't use blockSignals()
    slotSortingChanged( col );
    m_blockSortingSignal = false;
}

void K3FileDetailView::ensureItemVisible( const KFileItem *i )
{
    if ( !i )
        return;

    K3FileListViewItem *item = (K3FileListViewItem*) i->extraData( this );

    if ( item )
        K3ListView::ensureItemVisible( item );
}

// we're in multiselection mode
void K3FileDetailView::slotSelectionChanged()
{
    sig->highlightFile( 0L );
}

KFileItem * K3FileDetailView::firstFileItem() const
{
    K3FileListViewItem *item = static_cast<K3FileListViewItem*>( firstChild() );
    if ( item )
        return item->fileInfo();
    return 0L;
}

KFileItem * K3FileDetailView::nextItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        K3FileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemBelow() )
            return ((K3FileListViewItem*) item->itemBelow())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

KFileItem * K3FileDetailView::prevItem( const KFileItem *fileItem ) const
{
    if ( fileItem ) {
        K3FileListViewItem *item = viewItem( fileItem );
        if ( item && item->itemAbove() )
            return ((K3FileListViewItem*) item->itemAbove())->fileInfo();
        else
            return 0L;
    }
    else
        return firstFileItem();
}

void K3FileDetailView::keyPressEvent( QKeyEvent *e )
{
    K3ListView::keyPressEvent( e );

    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter ) {
        if ( e->modifiers() & Qt::ControlModifier )
            e->ignore();
        else
            e->accept();
    }
}

//
// mimetype determination on demand
//
void K3FileDetailView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void K3FileDetailView::determineIcon( K3FileListViewItem *item )
{
    (void) item->fileInfo()->determineMimeType();
    updateView( item->fileInfo() );
}

void K3FileDetailView::listingCompleted()
{
    m_resolver->start();
}

// Qt4 porting: once we use QListWidget, this becomes a reimplementation of
// virtual QMimeData *mimeData(const QList<QListWidgetItem*> items) const;
// or better: something at the model level, instead?
Q3DragObject *K3FileDetailView::dragObject()
{
    // create a list of the URL:s that we want to drag
    const KUrl::List urls = KFileView::selectedItems()->urlList();
    QPixmap pixmap;
    if( urls.count() > 1 )
        pixmap = DesktopIcon( "kmultiple", K3Icon::SizeSmall );
    if( pixmap.isNull() )
        pixmap = currentFileItem()->pixmap( K3Icon::SizeSmall );

    QPoint hotspot;
    hotspot.setX( pixmap.width() / 2 );
    hotspot.setY( pixmap.height() / 2 );
#if 0 // there is no more kurldrag, this should use urls.populateMimeData( mimeData ) instead
    Q3DragObject* myDragObject = new KUrlDrag( urls, widget() );
    myDragObject->setPixmap( pixmap, hotspot );
    return myDragObject;
#endif
    return 0;
}

void K3FileDetailView::slotAutoOpen()
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

bool K3FileDetailView::acceptDrag(QDropEvent* e) const
{
    return KUrl::List::canDecode( e->mimeData() ) &&
       (e->source()!= const_cast<K3FileDetailView*>(this)) &&
       ( e->dropAction() == Qt::CopyAction
      || e->dropAction() == Qt::MoveAction
      || e->dropAction() == Qt::LinkAction );
}

void K3FileDetailView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    K3FileListViewItem *item = dynamic_cast<K3FileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
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

void K3FileDetailView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    if ((dropOptions() & AutoOpenDirs) == 0)
       return;

    K3FileListViewItem *item = dynamic_cast<K3FileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
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

void K3FileDetailView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();
}

void K3FileDetailView::contentsDropEvent( QDropEvent *e )
{
    d->dropItem = 0;
    d->autoOpenTimer.stop();

    if ( ! acceptDrag( e ) ) { // can we decode this ?
        e->ignore();            // No
        return;
    }
    e->acceptProposedAction();     // Yes

    K3FileListViewItem *item = dynamic_cast<K3FileListViewItem*>(itemAt( contentsToViewport( e->pos() ) ));
    KFileItem * fileItem = 0;
    if (item)
        fileItem = item->fileInfo();

    emit dropped(e, fileItem);

    KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
    if ( !urls.isEmpty() )
    {
        emit dropped(e, urls, fileItem ? fileItem->url() : KUrl());
        sig->dropURLs(fileItem, e, urls);
    }
}


/////////////////////////////////////////////////////////////////


void K3FileListViewItem::init()
{
    K3FileListViewItem::setPixmap( COL_NAME, inf->pixmap(K3Icon::SizeSmall));

    setText( COL_NAME, inf->text() );
    setText( COL_SIZE, KGlobal::locale()->formatNumber( inf->size(), 0));
    setText( COL_DATE,  inf->timeString() );
    setText( COL_PERM,  inf->permissionsString() );
    setText( COL_OWNER, inf->user() );
    setText( COL_GROUP, inf->group() );
}

#include "k3filedetailview.moc"
