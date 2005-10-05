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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>
#include <q3header.h>
#include <qevent.h>
#include <qtimer.h>
#include <kdebug.h>
#include <kdirnotify_stub.h>
#include <kglobalsettings.h>
#include <kfileitem.h>
#include <kfileview.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <assert.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kiconloader.h>


#include "kfiletreeview.h"
#include "kfiletreebranch.h"
#include "kfiletreeviewitem.h"

KFileTreeView::KFileTreeView( QWidget *parent )
    : KListView( parent ),
      m_wantOpenFolderPixmaps( true )
{
    setDragEnabled(true);
    setSelectionModeExt( KListView::Single );

    m_animationTimer = new QTimer( this );
    connect( m_animationTimer, SIGNAL( timeout() ),
             this, SLOT( slotAnimation() ) );

    m_currentBeforeDropItem = 0;
    m_dropItem = 0;

    m_autoOpenTimer = new QTimer( this );
    connect( m_autoOpenTimer, SIGNAL( timeout() ),
             this, SLOT( slotAutoOpenFolder() ) );

    /* The executed-Slot only opens  a path, while the expanded-Slot populates it */
    connect( this, SIGNAL( executed( Q3ListViewItem * ) ),
             this, SLOT( slotExecuted( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( expanded ( Q3ListViewItem *) ),
    	     this, SLOT( slotExpanded( Q3ListViewItem *) ));
    connect( this, SIGNAL( collapsed( Q3ListViewItem *) ),
	     this, SLOT( slotCollapsed( Q3ListViewItem* )));


    /* connections from the konqtree widget */
    connect( this, SIGNAL( selectionChanged() ),
             this, SLOT( slotSelectionChanged() ) );
    connect( this, SIGNAL( onItem( Q3ListViewItem * )),
	     this, SLOT( slotOnItem( Q3ListViewItem * ) ) );
    connect( this, SIGNAL(itemRenamed(Q3ListViewItem*, const QString &, int)),
             this, SLOT(slotItemRenamed(Q3ListViewItem*, const QString &, int)));


    m_bDrag = false;
    m_branches.setAutoDelete( true );

    m_openFolderPixmap = DesktopIcon( "folder_open",KIcon::SizeSmall,KIcon::ActiveState );
}

KFileTreeView::~KFileTreeView()
{
   // we must make sure that the KFileTreeViewItems are deleted _before_ the
   // branches are deleted. Otherwise, the KFileItems would be destroyed
   // and the KFileTreeViewItems had dangling pointers to them.
   hide();
   clear();
   m_branches.clear(); // finally delete the branches and KFileItems
}


bool KFileTreeView::isValidItem( Q3ListViewItem *item)
{
   if (!item)
      return false;
   Q3PtrList<Q3ListViewItem> lst;
   Q3ListViewItemIterator it( this );
   while ( it.current() )
   {
      if ( it.current() == item )
         return true;
      ++it;
   }
   return false;
}

void KFileTreeView::contentsDragEnterEvent( QDragEnterEvent *ev )
{
   if ( ! acceptDrag( ev ) )
   {
      ev->ignore();
      return;
   }
   ev->acceptAction();
   m_currentBeforeDropItem = selectedItem();

   Q3ListViewItem *item = itemAt( contentsToViewport( ev->pos() ) );
   if( item )
   {
      m_dropItem = item;
      m_autoOpenTimer->start( KFileView::autoOpenDelay() );
   }
   else
   {
   m_dropItem = 0;
}
}

void KFileTreeView::contentsDragMoveEvent( QDragMoveEvent *e )
{
   if( ! acceptDrag( e ) )
   {
      e->ignore();
      return;
   }
   e->acceptAction();


   Q3ListViewItem *afterme;
   Q3ListViewItem *parent;

   findDrop( e->pos(), parent, afterme );

   // "afterme" is 0 when aiming at a directory itself
   Q3ListViewItem *item = afterme ? afterme : parent;

   if( item && item->isSelectable() )
   {
      setSelected( item, true );
      if( item != m_dropItem ) {
	 m_autoOpenTimer->stop();
	 m_dropItem = item;
	 m_autoOpenTimer->start( KFileView::autoOpenDelay() );
      }
   }
   else
   {
      m_autoOpenTimer->stop();
      m_dropItem = 0;
   }
}

void KFileTreeView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
   // Restore the current item to what it was before the dragging (#17070)
   if ( isValidItem(m_currentBeforeDropItem) )
   {
      setSelected( m_currentBeforeDropItem, true );
      ensureItemVisible( m_currentBeforeDropItem );
   }
   else if ( isValidItem(m_dropItem) )
      setSelected( m_dropItem, false ); // no item selected
   m_currentBeforeDropItem = 0;
   m_dropItem = 0;

}

void KFileTreeView::contentsDropEvent( QDropEvent *e )
{

    m_autoOpenTimer->stop();
    m_dropItem = 0;

    kdDebug(250) << "contentsDropEvent !" << endl;
    if( ! acceptDrag( e ) ) {
       e->ignore();
       return;
    }

    e->acceptAction();
    Q3ListViewItem *afterme;
    Q3ListViewItem *parent;
    findDrop(e->pos(), parent, afterme);

    //kdDebug(250) << " parent=" << (parent?parent->text(0):QString::null)
    //             << " afterme=" << (afterme?afterme->text(0):QString::null) << endl;

    if (e->source() == viewport() && itemsMovable())
        movableDropEvent(parent, afterme);
    else
    {
       emit dropped(e, afterme);
       emit dropped(this, e, afterme);
       emit dropped(e, parent, afterme);
       emit dropped(this, e, parent, afterme);

       KURL::List urls = KURL::List::fromMimeData( e->mimeData() );
       if ( urls.isEmpty() )
           return;
       emit dropped( this, e, urls );

       KURL parentURL;
       if( parent )
           parentURL = static_cast<KFileTreeViewItem*>(parent)->url();
       else
           // can happen when dropping above the root item
           // Should we choose the first branch in such a case ??
           return;

       emit dropped( urls, parentURL );
       emit dropped( this , e, urls, parentURL );
    }
}

bool KFileTreeView::acceptDrag(QDropEvent* e ) const
{

   bool ancestOK= acceptDrops();
   // kdDebug(250) << "Do accept drops: " << ancestOK << endl;
   ancestOK = ancestOK && itemsMovable();
   // kdDebug(250) << "acceptDrag: " << ancestOK << endl;
   // kdDebug(250) << "canDecode: " << KURL::List::canDecode(e->mimeData()) << endl;
   // kdDebug(250) << "action: " << e->action() << endl;

   /*  KListView::acceptDrag(e);  */
   /* this is what KListView does:
    * acceptDrops() && itemsMovable() && (e->source()==viewport());
    * ask acceptDrops and itemsMovable, but not the third
    */
   return ancestOK && KURL::List::canDecode( e->mimeData() ) &&
       // Why this test? All DnDs are one of those AFAIK (DF)
      ( e->action() == QDropEvent::Copy
	|| e->action() == QDropEvent::Move
	|| e->action() == QDropEvent::Link );
}



Q3DragObject * KFileTreeView::dragObject()
{

   KURL::List urls;
   const Q3PtrList<Q3ListViewItem> fileList = selectedItems();
   Q3PtrListIterator<Q3ListViewItem> it( fileList );
   for ( ; it.current(); ++it )
   {
      urls.append( static_cast<KFileTreeViewItem*>(it.current())->url() );
   }
   QPoint hotspot;
   QPixmap pixmap;
   if( urls.count() > 1 ){
      pixmap = DesktopIcon( "kmultiple", 16 );
   }
   if( pixmap.isNull() )
      pixmap = currentKFileTreeViewItem()->fileItem()->pixmap( 16 );
   hotspot.setX( pixmap.width() / 2 );
   hotspot.setY( pixmap.height() / 2 );
#if 0 // there is no more kurldrag, this should use urls.setInMimeData( mimeData ) instead
   Q3DragObject* dragObject = new KURLDrag( urls, this );
   if( dragObject )
      dragObject->setPixmap( pixmap, hotspot );
   return dragObject;
#endif
   return 0;
}



void KFileTreeView::slotCollapsed( Q3ListViewItem *item )
{
   KFileTreeViewItem *kftvi = static_cast<KFileTreeViewItem*>(item);
   kdDebug(250) << "hit slotCollapsed" << endl;
   if( kftvi && kftvi->isDir())
   {
      item->setPixmap( 0, itemIcon(kftvi));
   }
}

void KFileTreeView::slotExpanded( Q3ListViewItem *item )
{
   kdDebug(250) << "slotExpanded here !" << endl;

   if( ! item ) return;

   KFileTreeViewItem *it = static_cast<KFileTreeViewItem*>(item);
   KFileTreeBranch *branch = it->branch();

   /* Start the animation for the branch object */
   if( it->isDir() && branch && item->childCount() == 0 )
   {
      /* check here if the branch really needs to be populated again */
      kdDebug(250 ) << "starting to open " << it->url().prettyURL() << endl;
      startAnimation( it );
      bool branchAnswer = branch->populate( it->url(), it );
      kdDebug(250) << "Branches answer: " << branchAnswer << endl;
      if( ! branchAnswer )
      {
	 kdDebug(250) << "ERR: Could not populate!" << endl;
	 stopAnimation( it );
      }
   }

   /* set a pixmap 'open folder' */
   if( it->isDir() && isOpen( item ) )
   {
      kdDebug(250)<< "Setting open Pixmap" << endl;
      item->setPixmap( 0, itemIcon( it )); // 0, m_openFolderPixmap );
   }
}



void KFileTreeView::slotExecuted( Q3ListViewItem *item )
{
    if ( !item )
        return;
    /* This opens the dir and causes the Expanded-slot to be called,
     * which strolls through the children.
     */
    if( static_cast<KFileTreeViewItem*>(item)->isDir())
    {
       item->setOpen( !item->isOpen() );
    }
}


void KFileTreeView::slotAutoOpenFolder()
{
   m_autoOpenTimer->stop();

   if ( !isValidItem(m_dropItem) || m_dropItem->isOpen() )
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


KFileTreeBranch* KFileTreeView::addBranch( const KURL &path, const QString& name,
                              bool showHidden )
{
    const QPixmap& folderPix = KMimeType::mimeType("inode/directory")->pixmap( KIcon::Desktop,KIcon::SizeSmall );

    return addBranch( path, name, folderPix, showHidden);
}

KFileTreeBranch* KFileTreeView::addBranch( const KURL &path, const QString& name,
                              const QPixmap& pix, bool showHidden )
{
   kdDebug(250) << "adding another root " << path.prettyURL() << endl;

   /* Open a new branch */
   KFileTreeBranch *newBranch = new KFileTreeBranch( this, path, name, pix,
                                                     showHidden );
   return addBranch(newBranch);
}

KFileTreeBranch *KFileTreeView::addBranch(KFileTreeBranch *newBranch)
{
   connect( newBranch, SIGNAL(populateFinished( KFileTreeViewItem* )),
            this, SLOT( slotPopulateFinished( KFileTreeViewItem* )));

   connect( newBranch, SIGNAL( newTreeViewItems( KFileTreeBranch*,
                               const KFileTreeViewItemList& )),
            this, SLOT( slotNewTreeViewItems( KFileTreeBranch*,
                        const KFileTreeViewItemList& )));

   m_branches.append( newBranch );
   return( newBranch );
}

KFileTreeBranch *KFileTreeView::branch( const QString& searchName )
{
   KFileTreeBranch *branch = 0;
   Q3PtrListIterator<KFileTreeBranch> it( m_branches );

   while ( (branch = it.current()) != 0 ) {
      ++it;
      QString bname = branch->name();
      kdDebug(250) << "This is the branches name: " << bname << endl;
      if( bname == searchName )
      {
	 kdDebug(250) << "Found branch " << bname << " and return ptr" << endl;
	 return( branch );
      }
   }
   return ( 0L );
}

KFileTreeBranchList& KFileTreeView::branches()
{
   return( m_branches );
}


bool KFileTreeView::removeBranch( KFileTreeBranch *branch )
{
   if(m_branches.contains(branch))
   {
      delete (branch->root());
      m_branches.remove( branch );
      return true;
   }
   else
   {
      return false;
   }
}

void KFileTreeView::setDirOnlyMode( KFileTreeBranch* branch, bool bom )
{
   if( branch )
   {
      branch->setDirOnlyMode( bom );
   }
}


void KFileTreeView::slotPopulateFinished( KFileTreeViewItem *it )
{
   if( it && it->isDir())
    stopAnimation( it );
}

void KFileTreeView::slotNewTreeViewItems( KFileTreeBranch* branch, const KFileTreeViewItemList& itemList )
{
   if( ! branch ) return;
   kdDebug(250) << "hitting slotNewTreeViewItems" << endl;

   /* Sometimes it happens that new items should become selected, i.e. if the user
    * creates a new dir, he probably wants it to be selected. This can not be done
    * right after creating the directory or file, because it takes some time until
    * the item appears here in the treeview. Thus, the creation code sets the member
    * m_neUrlToSelect to the required url. If this url appears here, the item becomes
    * selected and the member nextUrlToSelect will be cleared.
    */
   if( ! m_nextUrlToSelect.isEmpty() )
   {
      KFileTreeViewItemListIterator it( itemList );

      bool end = false;
      for( ; !end && it.current(); ++it )
      {
	 KURL url = (*it)->url();

	 if( m_nextUrlToSelect.equals(url, true ))   // ignore trailing / on dirs
	 {
	    setCurrentItem( static_cast<Q3ListViewItem*>(*it) );
	    m_nextUrlToSelect = KURL();
	    end = true;
	 }
      }
   }
}

QPixmap KFileTreeView::itemIcon( KFileTreeViewItem *item, int gap ) const
{
   QPixmap pix;
   kdDebug(250) << "Setting icon for column " << gap << endl;

   if( item )
   {
      /* Check if it is a branch root */
      KFileTreeBranch *brnch = item->branch();
      if( item == brnch->root() )
      {
	 pix = brnch->pixmap();
	 if( m_wantOpenFolderPixmaps && brnch->root()->isOpen() )
	 {
	    pix = brnch->openPixmap();
	 }
      }
      else
      {
         // TODO: different modes, user Pixmaps ?
         pix = item->fileItem()->pixmap( KIcon::SizeSmall ); // , KIcon::DefaultState);

         /* Only if it is a dir and the user wants open dir pixmap and it is open,
          * change the fileitem's pixmap to the open folder pixmap. */
         if( item->isDir() && m_wantOpenFolderPixmaps )
         {
            if( isOpen( static_cast<Q3ListViewItem*>(item)))
               pix = m_openFolderPixmap;
         }
      }
   }

   return pix;
}


void KFileTreeView::slotAnimation()
{
   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.begin();
   MapCurrentOpeningFolders::Iterator end = m_mapCurrentOpeningFolders.end();
   for (; it != end;)
   {
      KFileTreeViewItem *item = it.key();
      if (!isValidItem(item))
      {
         ++it;
         m_mapCurrentOpeningFolders.remove(item);
         continue;
      }

      uint & iconNumber = it.data().iconNumber;
      QString icon = QString::fromLatin1( it.data().iconBaseName ).append( QString::number( iconNumber ) );
      // kdDebug(250) << "Loading icon " << icon << endl;
      item->setPixmap( 0, DesktopIcon( icon,KIcon::SizeSmall,KIcon::ActiveState )); // KFileTreeViewFactory::instance() ) );

      iconNumber++;
      if ( iconNumber > it.data().iconCount )
	 iconNumber = 1;

      ++it;
   }
}


void KFileTreeView::startAnimation( KFileTreeViewItem * item, const char * iconBaseName, uint iconCount )
{
   /* TODO: allow specific icons */
   if( ! item )
   {
      kdDebug(250) << " startAnimation Got called without valid item !" << endl;
      return;
   }

   m_mapCurrentOpeningFolders.insert( item,
                                      AnimationInfo( iconBaseName,
                                                     iconCount,
                                                     itemIcon(item, 0) ) );
   if ( !m_animationTimer->isActive() )
      m_animationTimer->start( 50 );
}

void KFileTreeView::stopAnimation( KFileTreeViewItem * item )
{
   if( ! item ) return;

   kdDebug(250) << "Stoping Animation !" << endl;

   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.find(item);
   if ( it != m_mapCurrentOpeningFolders.end() )
   {
      if( item->isDir() && isOpen( item) )
      {
	 kdDebug(250) << "Setting folder open pixmap !" << endl;
	 item->setPixmap( 0, itemIcon( item ));
      }
      else
      {
	 item->setPixmap( 0, it.data().originalPixmap );
      }
      m_mapCurrentOpeningFolders.remove( item );
   }
   else
   {
      if( item )
	 kdDebug(250)<< "StopAnimation - could not find item " << item->url().prettyURL()<< endl;
      else
	 kdDebug(250)<< "StopAnimation - item is zero !" << endl;
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
        return currentKFileTreeViewItem()->url();
    else
        return KURL();
}

void KFileTreeView::slotOnItem( Q3ListViewItem *item )
{
    KFileTreeViewItem *i = static_cast<KFileTreeViewItem *>( item );
    if( i )
    {
       const KURL url = i->url();
       if ( url.isLocalFile() )
	  emit onItem( url.path() );
       else
	  emit onItem( url.prettyURL() );
    }
}

void KFileTreeView::slotItemRenamed(Q3ListViewItem* item, const QString &name, int col)
{
   (void) item;
   kdDebug(250) << "Do not bother: " << name << col << endl;
}

KFileTreeViewItem *KFileTreeView::findItem( const QString& branchName, const QString& relUrl )
{
   KFileTreeBranch *br = branch( branchName );
   return( findItem( br, relUrl ));
}

KFileTreeViewItem *KFileTreeView::findItem( KFileTreeBranch* brnch, const QString& relUrl )
{
   KFileTreeViewItem *ret = 0;
   if( brnch )
   {
      KURL url = brnch->rootUrl();

      if( ! relUrl.isEmpty() && QDir::isRelativePath(relUrl) )
      {
         QString partUrl( relUrl );

         if( partUrl.endsWith("/"))
            partUrl.truncate( relUrl.length()-1 );

         url.addPath( partUrl );

         kdDebug(250) << "assembled complete dir string " << url.prettyURL() << endl;

         KFileItem *fi = brnch->findByURL( url );
         if( fi )
         {
            ret = static_cast<KFileTreeViewItem*>( fi->extraData( brnch ));
            kdDebug(250) << "Found item !" <<ret << endl;
         }
      }
      else
      {
         ret = brnch->root();
      }
   }
   return( ret );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



void KFileTreeView::virtual_hook( int id, void* data )
{ KListView::virtual_hook( id, data ); }

#include "kfiletreeview.moc"
