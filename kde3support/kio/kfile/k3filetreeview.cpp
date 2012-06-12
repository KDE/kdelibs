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

#include "k3filetreeview.h"
#include "kfiletreebranch.h"

#include <Qt3Support/Q3Header>
#include <QKeyEvent>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QApplication>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kfileitem.h>
#include <kmimetype.h>

#include <stdlib.h>
#include <assert.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kiconloader.h>


K3FileTreeView::K3FileTreeView( QWidget *parent )
    : K3ListView( parent ),
      m_wantOpenFolderPixmaps( true )
{
    setDragEnabled(true);
    setSelectionModeExt( K3ListView::Single );

    m_animationTimer = new QTimer( this );
    connect( m_animationTimer, SIGNAL(timeout()),
             this, SLOT(slotAnimation()) );

    m_currentBeforeDropItem = 0;
    m_dropItem = 0;

    m_autoOpenTimer = new QTimer( this );
    connect( m_autoOpenTimer, SIGNAL(timeout()),
             this, SLOT(slotAutoOpenFolder()) );

    /* The executed-Slot only opens  a path, while the expanded-Slot populates it */
    connect( this, SIGNAL(executed(Q3ListViewItem*)),
             this, SLOT(slotExecuted(Q3ListViewItem*)) );
    connect( this, SIGNAL(expanded(Q3ListViewItem*)),
             this, SLOT(slotExpanded(Q3ListViewItem*)));
    connect( this, SIGNAL(collapsed(Q3ListViewItem*)),
	     this, SLOT(slotCollapsed(Q3ListViewItem*)));


    /* connections from the konqtree widget */
    connect( this, SIGNAL(selectionChanged()),
             this, SLOT(slotSelectionChanged()) );
    connect( this, SIGNAL(onItem(Q3ListViewItem*)),
	     this, SLOT(slotOnItem(Q3ListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(Q3ListViewItem*,QString,int)),
             this, SLOT(slotItemRenamed(Q3ListViewItem*,QString,int)));


    m_bDrag = false;
    m_branches.setAutoDelete( true );

    m_openFolderPixmap = DesktopIcon( "folder-open",KIconLoader::SizeSmall,KIconLoader::ActiveState );
}

K3FileTreeView::~K3FileTreeView()
{
   // we must make sure that the KFileTreeViewItems are deleted _before_ the
   // branches are deleted. Otherwise, the KFileItems would be destroyed
   // and the KFileTreeViewItems had dangling pointers to them.
   hide();
   clear();
   m_branches.clear(); // finally delete the branches and KFileItems
}


bool K3FileTreeView::isValidItem( Q3ListViewItem *item)
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

void K3FileTreeView::contentsDragEnterEvent( QDragEnterEvent *ev )
{
   if ( ! acceptDrag( ev ) )
   {
      ev->ignore();
      return;
   }
   ev->acceptProposedAction();
   m_currentBeforeDropItem = selectedItem();

   Q3ListViewItem *item = itemAt( contentsToViewport( ev->pos() ) );
   if( item )
   {
      m_dropItem = item;
      m_autoOpenTimer->start( (QApplication::startDragTime() * 3) / 2 );
   }
   else
   {
   m_dropItem = 0;
}
}

void K3FileTreeView::contentsDragMoveEvent( QDragMoveEvent *e )
{
   if( ! acceptDrag( e ) )
   {
      e->ignore();
      return;
   }
   e->acceptProposedAction();


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
         m_autoOpenTimer->start( (QApplication::startDragTime() * 3) / 2 );
      }
   }
   else
   {
      m_autoOpenTimer->stop();
      m_dropItem = 0;
   }
}

void K3FileTreeView::contentsDragLeaveEvent( QDragLeaveEvent * )
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

void K3FileTreeView::contentsDropEvent( QDropEvent *e )
{

    m_autoOpenTimer->stop();
    m_dropItem = 0;

    kDebug(250) << "contentsDropEvent !";
    if( ! acceptDrag( e ) ) {
       e->ignore();
       return;
    }

    e->acceptProposedAction();
    Q3ListViewItem *afterme;
    Q3ListViewItem *parent;
    findDrop(e->pos(), parent, afterme);

    //kDebug(250) << " parent=" << (parent?parent->text(0):QString())
    //             << " afterme=" << (afterme?afterme->text(0):QString()) << endl;

    if (e->source() == viewport() && itemsMovable())
        movableDropEvent(parent, afterme);
    else
    {
       emit dropped(e, afterme);
       emit dropped(this, e, afterme);
       emit dropped(e, parent, afterme);
       emit dropped(this, e, parent, afterme);

       KUrl::List urls = KUrl::List::fromMimeData( e->mimeData() );
       if ( urls.isEmpty() )
           return;
       emit dropped( this, e, urls );

       KUrl parentURL;
       if( parent )
           parentURL = static_cast<K3FileTreeViewItem*>(parent)->url();
       else
           // can happen when dropping above the root item
           // Should we choose the first branch in such a case ??
           return;

       emit dropped( urls, parentURL );
       emit dropped( this , e, urls, parentURL );
    }
}

bool K3FileTreeView::acceptDrag(QDropEvent* e ) const
{

   bool ancestOK= acceptDrops();
   // kDebug(250) << "Do accept drops: " << ancestOK;
   ancestOK = ancestOK && itemsMovable();
   // kDebug(250) << "acceptDrag: " << ancestOK;
   // kDebug(250) << "canDecode: " << KUrl::List::canDecode(e->mimeData());
   // kDebug(250) << "action: " << e->action();

   /*  K3ListView::acceptDrag(e);  */
   /* this is what K3ListView does:
    * acceptDrops() && itemsMovable() && (e->source()==viewport());
    * ask acceptDrops and itemsMovable, but not the third
    */
   return ancestOK && KUrl::List::canDecode( e->mimeData() ) &&
       // Why this test? All DnDs are one of those AFAIK (DF)
      ( e->dropAction() == Qt::CopyAction
     || e->dropAction() == Qt::MoveAction
     || e->dropAction() == Qt::LinkAction );
}



Q3DragObject * K3FileTreeView::dragObject()
{

   KUrl::List urls;
   const QList<Q3ListViewItem *> fileList = selectedItems();
   for (int i = 0; i < fileList.size(); ++i)
   {
      urls.append( static_cast<K3FileTreeViewItem*>(fileList.at(i))->url() );
   }
   QPoint hotspot;
   QPixmap pixmap;
   if( urls.count() > 1 ){
      pixmap = DesktopIcon( "kmultiple", 16 );
   }
   if( pixmap.isNull() )
      pixmap = currentKFileTreeViewItem()->fileItem().pixmap( 16 );
   hotspot.setX( pixmap.width() / 2 );
   hotspot.setY( pixmap.height() / 2 );
#if 0 // there is no more kurldrag, this should use urls.setInMimeData( mimeData ) instead
   Q3DragObject* dragObject = new KUrlDrag( urls, this );
   if( dragObject )
      dragObject->setPixmap( pixmap, hotspot );
   return dragObject;
#endif
   return 0;
}



void K3FileTreeView::slotCollapsed( Q3ListViewItem *item )
{
   K3FileTreeViewItem *kftvi = static_cast<K3FileTreeViewItem*>(item);
   kDebug(250) << "hit slotCollapsed";
   if( kftvi && kftvi->isDir())
   {
      item->setPixmap( 0, itemIcon(kftvi));
   }
}

void K3FileTreeView::slotExpanded( Q3ListViewItem *item )
{
   kDebug(250) << "slotExpanded here !";

   if( ! item ) return;

   K3FileTreeViewItem *it = static_cast<K3FileTreeViewItem*>(item);
   KFileTreeBranch *branch = it->branch();

   /* Start the animation for the branch object */
   if( it->isDir() && branch && item->childCount() == 0 )
   {
      /* check here if the branch really needs to be populated again */
      kDebug(250 ) << "starting to open " << it->url().prettyUrl();
      startAnimation( it );
      bool branchAnswer = branch->populate( it->url(), it );
      kDebug(250) << "Branches answer: " << branchAnswer;
      if( ! branchAnswer )
      {
	 kDebug(250) << "ERR: Could not populate!";
	 stopAnimation( it );
      }
   }

   /* set a pixmap 'open folder' */
   if( it->isDir() && isOpen( item ) )
   {
      kDebug(250)<< "Setting open Pixmap";
      item->setPixmap( 0, itemIcon( it )); // 0, m_openFolderPixmap );
   }
}



void K3FileTreeView::slotExecuted( Q3ListViewItem *item )
{
    if ( !item )
        return;
    /* This opens the dir and causes the Expanded-slot to be called,
     * which strolls through the children.
     */
    if( static_cast<K3FileTreeViewItem*>(item)->isDir())
    {
       item->setOpen( !item->isOpen() );
    }
}


void K3FileTreeView::slotAutoOpenFolder()
{
   m_autoOpenTimer->stop();

   if ( !isValidItem(m_dropItem) || m_dropItem->isOpen() )
      return;

   m_dropItem->setOpen( true );
   m_dropItem->repaint();
}


void K3FileTreeView::slotSelectionChanged()
{
   if ( !m_dropItem ) // don't do this while the dragmove thing
   {
   }
}


KFileTreeBranch* K3FileTreeView::addBranch( const KUrl &path, const QString& name,
                              bool showHidden )
{
    const QPixmap& folderPix = KIconLoader::global()->loadMimeTypeIcon( KMimeType::mimeType("inode/directory")->iconName(),
                                                                        KIconLoader::Desktop, KIconLoader::SizeSmall );

    return addBranch( path, name, folderPix, showHidden);
}

KFileTreeBranch* K3FileTreeView::addBranch( const KUrl &path, const QString& name,
                              const QPixmap& pix, bool showHidden )
{
   kDebug(250) << "adding another root " << path.prettyUrl();

   /* Open a new branch */
   KFileTreeBranch *newBranch = new KFileTreeBranch( this, path, name, pix,
                                                     showHidden );
   return addBranch(newBranch);
}

KFileTreeBranch *K3FileTreeView::addBranch(KFileTreeBranch *newBranch)
{
   connect( newBranch, SIGNAL(populateFinished(K3FileTreeViewItem*)),
            this, SLOT(slotPopulateFinished(K3FileTreeViewItem*)));

   connect( newBranch, SIGNAL( newTreeViewItems( KFileTreeBranch*,
                               const K3FileTreeViewItemList& )),
            this, SLOT( slotNewTreeViewItems( KFileTreeBranch*,
                        const K3FileTreeViewItemList& )));

   m_branches.append( newBranch );
   return( newBranch );
}

KFileTreeBranch *K3FileTreeView::branch( const QString& searchName )
{
   KFileTreeBranch *branch = 0;
   Q3PtrListIterator<KFileTreeBranch> it( m_branches );

   while ( (branch = it.current()) != 0 ) {
      ++it;
      QString bname = branch->name();
      kDebug(250) << "This is the branches name: " << bname;
      if( bname == searchName )
      {
	 kDebug(250) << "Found branch " << bname << " and return ptr";
	 return( branch );
      }
   }
   return ( 0L );
}

KFileTreeBranchList& K3FileTreeView::branches()
{
   return( m_branches );
}


bool K3FileTreeView::removeBranch( KFileTreeBranch *branch )
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

void K3FileTreeView::setDirOnlyMode( KFileTreeBranch* branch, bool bom )
{
   if( branch )
   {
      branch->setDirOnlyMode( bom );
   }
}


void K3FileTreeView::slotPopulateFinished( K3FileTreeViewItem *it )
{
   if( it && it->isDir())
    stopAnimation( it );
}

void K3FileTreeView::slotNewTreeViewItems( KFileTreeBranch* branch, const K3FileTreeViewItemList& itemList )
{
   if( ! branch ) return;
   kDebug(250) << "hitting slotNewTreeViewItems";

   /* Sometimes it happens that new items should become selected, i.e. if the user
    * creates a new dir, he probably wants it to be selected. This can not be done
    * right after creating the directory or file, because it takes some time until
    * the item appears here in the treeview. Thus, the creation code sets the member
    * m_neUrlToSelect to the required url. If this url appears here, the item becomes
    * selected and the member nextUrlToSelect will be cleared.
    */
   if( ! m_nextUrlToSelect.isEmpty() )
   {
      K3FileTreeViewItemListIterator it( itemList );

      bool end = false;
      for( ; !end && it.current(); ++it )
      {
	 KUrl url = (*it)->url();

         if( m_nextUrlToSelect.equals(url, KUrl::CompareWithoutTrailingSlash ))   // ignore trailing / on dirs
	 {
	    setCurrentItem( static_cast<Q3ListViewItem*>(*it) );
	    m_nextUrlToSelect = KUrl();
	    end = true;
	 }
      }
   }
}

QPixmap K3FileTreeView::itemIcon( K3FileTreeViewItem *item, int gap ) const
{
   QPixmap pix;
   kDebug(250) << "Setting icon for column " << gap;

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
         pix = item->fileItem().pixmap( KIconLoader::SizeSmall ); // , KIconLoader::DefaultState);

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


void K3FileTreeView::slotAnimation()
{
   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.begin();
   MapCurrentOpeningFolders::Iterator end = m_mapCurrentOpeningFolders.end();
   for (; it != end;)
   {
      K3FileTreeViewItem *item = it.key();
      if (!isValidItem(item))
      {
         ++it;
         m_mapCurrentOpeningFolders.remove(item);
         continue;
      }

      uint & iconNumber = it.value().iconNumber;
      QString icon = QString::fromLatin1( it.value().iconBaseName ).append( QString::number( iconNumber ) );
      // kDebug(250) << "Loading icon " << icon;
      item->setPixmap( 0, DesktopIcon( icon,KIconLoader::SizeSmall,KIconLoader::ActiveState )); // KFileTreeViewFactory::instance() ) );

      iconNumber++;
      if ( iconNumber > it.value().iconCount )
	 iconNumber = 1;

      ++it;
   }
}


void K3FileTreeView::startAnimation( K3FileTreeViewItem * item, const char * iconBaseName, uint iconCount )
{
   /* TODO: allow specific icons */
   if( ! item )
   {
      kDebug(250) << " startAnimation Got called without valid item !";
      return;
   }

   m_mapCurrentOpeningFolders.insert( item,
                                      AnimationInfo( iconBaseName,
                                                     iconCount,
                                                     itemIcon(item, 0) ) );
   if ( !m_animationTimer->isActive() )
      m_animationTimer->start( 50 );
}

void K3FileTreeView::stopAnimation( K3FileTreeViewItem * item )
{
   if( ! item ) return;

   kDebug(250) << "Stoping Animation !";

   MapCurrentOpeningFolders::Iterator it = m_mapCurrentOpeningFolders.find(item);
   if ( it != m_mapCurrentOpeningFolders.end() )
   {
      if( item->isDir() && isOpen( item) )
      {
	 kDebug(250) << "Setting folder open pixmap !";
	 item->setPixmap( 0, itemIcon( item ));
      }
      else
      {
	 item->setPixmap( 0, it.value().originalPixmap );
      }
      m_mapCurrentOpeningFolders.remove( item );
   }
   else
   {
      if( item )
	 kDebug(250)<< "StopAnimation - could not find item " << item->url().prettyUrl();
      else
	 kDebug(250)<< "StopAnimation - item is zero !";
   }
   if (m_mapCurrentOpeningFolders.isEmpty())
      m_animationTimer->stop();
}

K3FileTreeViewItem * K3FileTreeView::currentKFileTreeViewItem() const
{
   return static_cast<K3FileTreeViewItem *>( selectedItem() );
}

KUrl K3FileTreeView::currentUrl() const
{
    K3FileTreeViewItem *item = currentKFileTreeViewItem();
    if ( item )
        return currentKFileTreeViewItem()->url();
    else
        return KUrl();
}

void K3FileTreeView::slotOnItem( Q3ListViewItem *item )
{
    K3FileTreeViewItem *i = static_cast<K3FileTreeViewItem *>( item );
    if( i )
    {
       const KUrl url = i->url();
       if ( url.isLocalFile() )
	  emit onItem( url.toLocalFile() );
       else
	  emit onItem( url.prettyUrl() );
    }
}

void K3FileTreeView::slotItemRenamed(Q3ListViewItem* item, const QString &name, int col)
{
   (void) item;
   kDebug(250) << "Do not bother: " << name << col;
}

K3FileTreeViewItem *K3FileTreeView::findItem( const QString& branchName, const QString& relUrl )
{
   KFileTreeBranch *br = branch( branchName );
   return( findItem( br, relUrl ));
}

K3FileTreeViewItem *K3FileTreeView::findItem( KFileTreeBranch* brnch, const QString& relUrl )
{
   K3FileTreeViewItem *ret = 0;
   if( brnch )
   {
      KUrl url = brnch->rootUrl();

      if( ! relUrl.isEmpty() && QDir::isRelativePath(relUrl) )
      {
         QString partUrl( relUrl );

         if( partUrl.endsWith('/'))
            partUrl.truncate( relUrl.length()-1 );

         url.addPath( partUrl );

         kDebug(250) << "assembled complete dir string " << url.prettyUrl();

         KFileItem fi = brnch->findByUrl( url );
         if( !fi.isNull() )
         {
            ret =
                const_cast<K3FileTreeViewItem *>(
                static_cast<const K3FileTreeViewItem*>(fi.extraData(brnch)));
            kDebug(250) << "Found item !" <<ret;
         }
      }
      else
      {
         ret = brnch->root();
      }
   }
   return( ret );
}

bool K3FileTreeView::showFolderOpenPixmap() const
{
    return m_wantOpenFolderPixmaps;
}

void K3FileTreeView::setShowFolderOpenPixmap( bool showIt )
{
    m_wantOpenFolderPixmaps = showIt;
}

void K3FileTreeView::slotSetNextUrlToSelect( const KUrl &url )
{
    m_nextUrlToSelect = url;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


#include "moc_k3filetreeview.cpp"
