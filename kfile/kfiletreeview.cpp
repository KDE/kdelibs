/* This file is part of the KDEproject
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qheader.h>
#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <kdebug.h>
#include <kdirnotify_stub.h>
#include <kglobalsettings.h>
#include <kfileitem.h>
#include <kio/global.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kstddirs.h>
#include <kurldrag.h>
#include <stdlib.h>
#include <assert.h>
#include <kfiletreeview.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kurldrag.h>

#include "kfiletreeview.h"
#include "kfiletreebranch.h"
#include "kfiletreeviewitem.h"

static const int autoOpenTimeout = 750;


KFileTreeView::KFileTreeView( QWidget *parent, const char *name )
    : KListView( parent, name ),
      m_toolTip( this )
{
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );

    setSelectionMode( QListView::Single );

    m_animationTimer = new QTimer( this );
    connect( m_animationTimer, SIGNAL( timeout() ),
             this, SLOT( slotAnimation() ) );

    m_currentBeforeDropItem = 0;
    m_dropItem = 0;

    addColumn( QString::null );
    header()->hide();

    m_autoOpenTimer = new QTimer( this );
    connect( m_autoOpenTimer, SIGNAL( timeout() ),
             this, SLOT( slotAutoOpenFolder() ) );

    /* The executed-Slot only opens  a path, while the expanded-Slot populates it */
    connect( this, SIGNAL( executed( QListViewItem * ) ),
             this, SLOT( slotExecuted( QListViewItem * ) ) );
    connect( this, SIGNAL( expanded ( QListViewItem *) ),
    	     this, SLOT( slotExpanded( QListViewItem *) ));
    /* connections from the konqtree widget */
    connect( this, SIGNAL( mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
             this, SLOT( slotMouseButtonPressed(int, QListViewItem*, const QPoint&, int)) );
    connect( this, SIGNAL( selectionChanged() ),
             this, SLOT( slotSelectionChanged() ) );
    connect( this, SIGNAL( onItem( QListViewItem * )),
	     this, SLOT( slotOnItem( QListViewItem * ) ) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*, const QString &, int)),
             this, SLOT(slotItemRenamed(QListViewItem*, const QString &, int)));

    m_bDrag = false;
    m_branches.setAutoDelete( true );
}

KFileTreeView::~KFileTreeView()
{
    clearTree();
}

void KFileTreeView::clearTree()
{
//     for ( KFileTreeViewModule * module = m_lstModules.first() ; module ; module = m_lstModules.next() )
//         module->clearAll();
    m_mapCurrentOpeningFolders.clear();
    clear();
    setRootIsDecorated( true );
}

void KFileTreeView::contentsDragEnterEvent( QDragEnterEvent *ev )
{
    m_dropItem = 0;
    m_currentBeforeDropItem = selectedItem();
    // Save the available formats
    m_lstDropFormats.clear();
    for( int i = 0; ev->format( i ); i++ )
      if ( *( ev->format( i ) ) )
         m_lstDropFormats.append( ev->format( i ) );
}

void KFileTreeView::contentsDragMoveEvent( QDragMoveEvent *e )
{

    QListViewItem *item = itemAt( contentsToViewport( e->pos() ) );

    // Accept drops on the background, if URLs
    if ( !item && m_lstDropFormats.contains("text/uri-list") )
    {
        m_dropItem = 0;
        e->acceptAction();
        if (selectedItem())
        setSelected( selectedItem(), false ); // no item selected
        return;
    }
    if ( !item || !item->isSelectable() ) // || !item->acceptsDrops( m_lstDropFormats ))

    // if ( !item || !item->isSelectable() || !static_cast<KFileViewItem*>(item)->acceptsDrops( m_lstDropFormats ))
    {
        m_dropItem = 0;
        m_autoOpenTimer->stop();
        e->ignore();
        return;
    }

    e->acceptAction();

    setSelected( item, true );

    if ( item != m_dropItem )
    {
        m_autoOpenTimer->stop();
        m_dropItem = item;
        m_autoOpenTimer->start( autoOpenTimeout );
    }

}

void KFileTreeView::contentsDragLeaveEvent( QDragLeaveEvent * )
{

    // Restore the current item to what it was before the dragging (#17070)
    if ( m_currentBeforeDropItem )
        setSelected( m_currentBeforeDropItem, true );
    else
        setSelected( m_dropItem, false ); // no item selected
    m_currentBeforeDropItem = 0;
    m_dropItem = 0;
    m_lstDropFormats.clear();

}

void KFileTreeView::contentsDropEvent( QDropEvent *ev )
{

    m_autoOpenTimer->stop();

    kdDebug(1201) << "contentsDropEvent !" << endl;
    if ( !selectedItem() )
    {
       // KonqOperations::doDrop( 0L, m_dirtreeDir, ev, this );
    }
    else
    {
       KFileTreeViewItem *selection = currentKFileTreeViewItem();
       // selection->drop( ev );
    }

}

void KFileTreeView::contentsMousePressEvent( QMouseEvent *e )
{

    KListView::contentsMousePressEvent( e );

    QPoint p( contentsToViewport( e->pos() ) );
    QListViewItem *i = itemAt( p );

    if ( e->button() == LeftButton && i ) {
        // if the user clicked into the root decoration of the item, don't try to start a drag!
        if ( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
             treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
             p.x() < header()->cellPos( header()->mapToActual( 0 ) ) )
        {
            m_dragPos = e->pos();
            m_bDrag = true;
        }
    }

}

void KFileTreeView::contentsMouseMoveEvent( QMouseEvent *e )
{
    KListView::contentsMouseMoveEvent( e );
    if ( !m_bDrag || ( e->pos() - m_dragPos ).manhattanLength() <= KGlobalSettings::dndEventDelay() )
        return;

    m_bDrag = false;

    QListViewItem *item = itemAt( contentsToViewport( m_dragPos ) );
    if ( !item || !item->isSelectable() )
        return;

    // Start a drag
    // ### TODO: actually, there could be multiple items selected and dragged!
    QDragObject *drag = dragObject();
    if ( !drag )
        return;

    if ( drag->pixmap().isNull() ) {
        const QPixmap *pix = item->pixmap(0);
        if ( pix && !pix->isNull() ) {
            QPoint hotspot( pix->width() / 2, pix->height() / 2 );
            drag->setPixmap( *pix, hotspot );
        }
    }

    drag->drag();
}

QDragObject * KFileTreeView::dragObject()
{
    // ### somehow, quite broken...
    KURL::List urls;
    QListViewItem *item = firstChild();
    QListViewItem *current = currentItem();
    while ( item ) {
        if ( item->isSelected() || item == current ) {
            urls.append( static_cast<KFileTreeViewItem*>( item )->url() );
        }
        
        if ( !item->nextSibling() )
            item = item->firstChild();
        else
            item = item->nextSibling();
    }

    QDragObject *drag = 0L;
    if ( !urls.isEmpty() )
        drag = new KURLDrag( urls, viewport(), "url drag" );
    qDebug("*** DRAGOBJECT: %p!", drag);
    return drag;
}

void KFileTreeView::contentsMouseReleaseEvent( QMouseEvent *e )
{

    KListView::contentsMouseReleaseEvent( e );
    m_bDrag = false;

}

void KFileTreeView::leaveEvent( QEvent *e )
{

    KListView::leaveEvent( e );

}

void KFileTreeView::slotExpanded( QListViewItem *item )
{
   kdDebug(1201) << "slotExpanded here !" << endl;

#if 0
   if( !item->isOpen() )
#endif
   {
      KFileTreeViewItem *it = static_cast<KFileTreeViewItem*>(item);
      KFileTreeBranch *branch = it->branch();

      if( branch )
      {
	 kdDebug(1201 ) << "starting to open " << it->url().prettyURL() << endl;
	 KFileTreeViewItem *currItem = static_cast<KFileTreeViewItem*>(currentItem());
	 startAnimation( currItem );
	 branch->populate( it->url(), currItem);
      }
   }
   // item->setOpen( !item->isOpen() );
}



void KFileTreeView::slotExecuted( QListViewItem *item )
{
    if ( !item )
        return;
#if 0
    if ( !static_cast<KFileViewItem*>(item)->isClickable() )
        return;
#endif
    /* This opens the dir and causes the Expanded-slot to be called,
     * which strolls through the children.
     */
    if( static_cast<KFileTreeViewItem*>(item)->isDir())
    {
       item->setOpen( !item->isOpen() );
    }
}


void KFileTreeView::slotMouseButtonPressed(int _button, QListViewItem* _item, 
                                           const QPoint&, int col)
{
   KFileTreeViewItem * item = static_cast<KFileTreeViewItem*>(_item);
   if(_item && col < 2)
      if (_button == MidButton)
	 item->middleButtonPressed();
      else if (_button == RightButton)
      {
	 item->setSelected( true );
	 item->rightButtonPressed();
      }
}

void KFileTreeView::slotAutoOpenFolder()
{
    m_autoOpenTimer->stop();

    if ( !m_dropItem || m_dropItem->isOpen() )
        return;

    m_dropItem->setOpen( true );
    m_dropItem->repaint();
}


void KFileTreeView::slotSelectionChanged()
{
    if ( !m_dropItem ) // don't do this while the dragmove thing
    {
    }
}

#if 0
void KFileTreeView::FilesAdded( const KURL & dir )
{
    kdDebug(1201) << "KFileTreeView::FilesAdded " << dir.url() << endl;
    /* TODO */
}

void KFileTreeView::FilesRemoved( const KURL::List & urls )
{
    //kdDebug(1201) << "KFileTreeView::FilesRemoved " << urls.count() << endl;
    for ( KURL::List::ConstIterator it = urls.begin() ; it != urls.end() ; ++it )
    {
        //kdDebug(1201) <<  "KFileTreeView::FilesRemoved " << (*it).prettyURL() << endl;
       // TODO
    }
}

void KFileTreeView::FilesChanged( const KURL::List & urls )
{
    //kdDebug(1201) << "KFileTreeView::FilesChanged" << endl;
    // not same signal, but same implementation
    FilesRemoved( urls );
}
#endif

int KFileTreeView::addBranch( const KURL &path, const QString& name,
                              bool showHidden )
{
    const QPixmap& folderPix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Small );

    return addBranch( path, name, folderPix, showHidden);
}

int KFileTreeView::addBranch( const KURL &path, const QString& name,
                              const QPixmap& pix, bool showHidden )
{
   kdDebug(1201) << "adding another root " << path.prettyURL() << endl;

   /* Open a new branch */
   KFileTreeBranch *newBranch = new KFileTreeBranch( this, path, name, pix,
                                                     showHidden );
   connect( newBranch, SIGNAL(populateFinished( KFileTreeViewItem* )),
	    this, SLOT( slotPopulateFinished( KFileTreeViewItem* )));

   m_branches.append( newBranch );
   return( m_branches.at());
}

KFileTreeBranch *KFileTreeView::branch( int branchno )
{
   KFileTreeBranch *branch = m_branches.at( branchno );
   return( branch );
}


bool KFileTreeView::removeBranch( int branchno )
{
   return( m_branches.remove( branchno ));
}

void KFileTreeView::setDirOnlyMode( int branchno, bool bom )
{
   KFileTreeBranch *branch = m_branches.at( branchno );
   if( branch )
   {
      branch->setDirOnlyMode( bom );
   }
}

void KFileTreeView::populateBranch(int branchno)
{
   KFileTreeBranch *branch = m_branches.at( branchno );
   if( branch )
   {
      branch->populate();
   }

   startAnimation( branch->root() );
   kdDebug() << "Starting to populate !" << endl;
}

void KFileTreeView::slotPopulateFinished( KFileTreeViewItem *it )
{
    stopAnimation( it );
}

void KFileTreeView::slotResult( )
{
}

void KFileTreeView::slotCanceled( )
{
}


/*
 * This method checks if a file should be displayed or not depending
 * on the current filter
 */
bool KFileTreeView::checkOnFilter( QString& fi )
{
   // dummy
   return( true );
}

QPixmap KFileTreeView::itemIcon( KFileTreeViewItem *item, int gap ) const
{
   if( item )
   {
      /* Check on it's own pixmap */
       const QPixmap *cp = item->pixmap( gap );
       if( cp )
           return *cp;
       else
           return item->fileItem()->pixmap( KIcon::Small, KIcon::DefaultState);
   }

   return QPixmap();
}


void KFileTreeView::slotAnimation()
{
    MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.begin();
    MapCurrentOpeningFolders::Iterator end = m_mapCurrentOpeningFolders.end();
    for (; it != end; ++it )
    {
        uint & iconNumber = it.data().iconNumber;
        QString icon = QString::fromLatin1( it.data().iconBaseName ).append( QString::number( iconNumber ) );
	kdDebug() << "Loading icon " << icon << endl;
        it.key()->setPixmap( 0, SmallIcon( icon )); // KFileTreeViewFactory::instance() ) );

        iconNumber++;
        if ( iconNumber > it.data().iconCount )
            iconNumber = 1;
    }
}


void KFileTreeView::startAnimation( KFileTreeViewItem * item, const char * iconBaseName, uint iconCount )
{
   /* TODO: allow specific icons */
   if( ! item )
   {
      kdDebug() << " startAnimation Got called without valid item !" << endl;
      return;
   }

   KIconLoader *loader = KGlobal::iconLoader();
   m_mapCurrentOpeningFolders.insert( item,
                                      AnimationInfo( iconBaseName,
                                                     iconCount,
                                                     itemIcon(item, 0) ) );
    if ( !m_animationTimer->isActive() )
        m_animationTimer->start( 50 );
}

void KFileTreeView::stopAnimation( KFileTreeViewItem * item )
{
    MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.find(item);
    if ( it != m_mapCurrentOpeningFolders.end() )
    {
       item->setPixmap( 0, it.data().originalPixmap );
       // TODO set pix here.
        m_mapCurrentOpeningFolders.remove( item );
    }
    if (m_mapCurrentOpeningFolders.isEmpty())
        m_animationTimer->stop();
}

KFileTreeViewItem * KFileTreeView::currentKFileTreeViewItem() const
{
   return static_cast<KFileTreeViewItem *>( selectedItem() );
}

KURL KFileTreeView::currentURL() const
{
    KFileTreeViewItem *item = currentKFileTreeViewItem();
    if ( item )
        currentKFileTreeViewItem()->url();
    else
        return KURL();
}

void KFileTreeView::slotOnItem( QListViewItem *item )
{
    KFileTreeViewItem *i = static_cast<KFileTreeViewItem *>( item );
    if( i )
    {
       const KURL& url = i->url();
       if ( url.isLocalFile() )
	  emit( onItem( url.path() ));
       else
	  emit( onItem( url.prettyURL()));
    }
}

void KFileTreeView::slotItemRenamed(QListViewItem* item, const QString &name, int col)
{
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


void KFileTreeViewToolTip::maybeTip( const QPoint &point )
{
#if 0
    QListViewItem *item = m_view->itemAt( point );
    if ( item ) {
	QString text = static_cast<KFileViewItem*>( item )->toolTipText();
	if ( !text.isEmpty() )
	    tip ( m_view->itemRect( item ), text );
    }
#endif
}

#include "kfiletreeview.moc"
