/* This file is part of the KDEproject
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2002 Klaas Freitag <freitag@suse.de>

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

#include <qfile.h>

#include <kfileitem.h>
#include <kdebug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kfiletreeviewitem.h"
#include "kfiletreebranch.h"

class KFileTreeView;


/* --- KFileTreeViewToplevelItem --- */
KFileTreeBranch::KFileTreeBranch( KFileTreeView *parent, const KURL& url,
                                  const QString& name,
				  const QPixmap& pix, bool showHidden,
				  KFileTreeViewItem *branchRoot )

    : KDirLister( false ),
      m_root( branchRoot ),
      m_startURL( url ),
      m_name ( name ),
      m_rootIcon( pix ),
      m_openRootIcon( pix ),
      m_recurseChildren(true),
	  m_showExtensions(true)
{
   kdDebug( 250) << "Creating branch for url " << url.prettyURL() << endl;

   /* if non exists, create one */
   if( ! branchRoot )
   {
      m_root =  new KFileTreeViewItem( parent,
				       new KFileItem( url, "inode/directory",
						      S_IFDIR  ),
				       this );
   }

   m_root->setExpandable( true );
   m_root->setPixmap( 0, pix );
   m_root->setText( 0, name );

   setShowingDotFiles( showHidden );

   connect( this, SIGNAL( newItems(const KFileItemList&)),
	    this, SLOT  ( addItems( const KFileItemList& )));

   connect( this, SIGNAL( completed(const KURL& )),
	    this,   SLOT(slCompleted(const KURL&)));

   connect( this, SIGNAL( started( const KURL& )),
	    this,   SLOT( slotListerStarted( const KURL& )));

   connect( this, SIGNAL( deleteItem( KFileItem* )),
	    this,   SLOT( slotDeleteItem( KFileItem* )));

   connect( this, SIGNAL( canceled(const KURL&) ),
            this,   SLOT( slotCanceled(const KURL&) ));

   connect( this, SIGNAL( clear()),
	    this, SLOT( slotDirlisterClear()));

   connect( this, SIGNAL( clear(const KURL&)),
	    this, SLOT( slotDirlisterClearURL(const KURL&)));

   connect( this, SIGNAL( redirection( const KURL& , const KURL& ) ),
	    this, SLOT( slotRedirect( const KURL&, const KURL& )));

   m_openChildrenURLs.append( url );
}

void KFileTreeBranch::setOpenPixmap( const QPixmap& pix )
{
   m_openRootIcon = pix;

   if( root()->isOpen())
   {
      root()->setPixmap( 0, pix );
   }
}

void KFileTreeBranch::slotListerStarted( const KURL &url )
{
   /* set the parent correct if it is zero. */
   kdDebug( 250) << "Starting to list " << url.prettyURL() << endl;
}


KFileTreeViewItem *KFileTreeBranch::parentKFTVItem( KFileItem *item )
{
   KFileTreeViewItem *parent = 0;

   if( ! item ) return 0;

   /* If it is a directory, check, if it exists in the dict. If not, go one up
    * and check again.
    */
   KURL url = item->url();
   // kdDebug(250) << "Item's url is " << url.prettyURL() << endl;
   KURL dirUrl( url );
    dirUrl.setFileName( QString::null );
   // kdDebug(250) << "Directory url is " << dirUrl.prettyURL() << endl;

   parent  = findTVIByURL( dirUrl );
   // kdDebug(250) << "Returning as parent item <" << parent <<  ">" << endl;
   return( parent );
}


void KFileTreeBranch::addItems( const KFileItemList& list )
{
   KFileItemListIterator it( list );
   kdDebug(250) << "Adding " << list.count() << " items !" << endl;
   KFileItem *currItem;
   KFileTreeViewItemList treeViewItList;
   KFileTreeViewItem *parentItem = 0;

   while ( (currItem = it.current()) != 0 )
   {
      parentItem = parentKFTVItem( currItem );

      /* Only create a new KFileTreeViewItem if it does not yet exist */
      KFileTreeViewItem *newKFTVI =
	 static_cast<KFileTreeViewItem *>(currItem->extraData( this ));

      if( ! newKFTVI )
      {
         newKFTVI = createTreeViewItem( parentItem, currItem );
         currItem->setExtraData( this, newKFTVI );


	 /* Cut off the file extension in case it is not a directory */
	 if( !m_showExtensions && !currItem->isDir() )	/* Need to cut the extension */
	 {
	    QString n = currItem->text();
	    int mPoint = n.findRev( '.' );
                if( mPoint > 0 )
                    n = n.left( mPoint );
	    newKFTVI->setText( 0, n );
	 }
      }

      /* Now try to find out if there are children for dirs in the treeview */
      /* This stats a directory on the local file system and checks the */
      /* hardlink entry in the stat-buf. This works only for local directories. */
      if( dirOnlyMode() && m_recurseChildren && currItem->isLocalFile( ) && currItem->isDir() )
      {
	 KURL url = currItem->url();
	 QString filename = url.directory( false, true ) + url.filename();
	 /* do the stat trick of Carsten. The problem is, that the hardlink
	 *  count only contains directory links. Thus, this method only seem
         * to work in dir-only mode */
	 kdDebug(250) << "Doing stat on " << filename << endl;
	 struct stat statBuf;
	 if( stat( QFile::encodeName( filename ), &statBuf ) == 0 )
	 {
	    int hardLinks = statBuf.st_nlink;  /* Count of dirs */
	    kdDebug(250) << "stat succeeded, hardlinks: " << hardLinks << endl;
        // If the link count is > 2, the directory likely has subdirs. If it's < 2
        // it's something weird like a mounted SMB share. In that case we don't know
        // if there are subdirs, thus show it as expandable.
	    if( hardLinks != 2 )
	    {
	       newKFTVI->setExpandable(true);
	    }
	    else
	    {
	       newKFTVI->setExpandable(false);
	    }
	    if( hardLinks >= 2 ) // "Normal" directory with subdirs
	    {
	       kdDebug(250) << "Emitting for " << url.prettyURL() << endl;
	       emit( directoryChildCount( newKFTVI, hardLinks-2)); // parentItem, hardLinks-1 ));
        }
	 }
	 else
	 {
	    kdDebug(250) << "stat of " << filename << " failed !" << endl;
	 }
      }
      ++it;

      treeViewItList.append( newKFTVI );
   }

   emit( newTreeViewItems( this, treeViewItList ));
}

KFileTreeViewItem* KFileTreeBranch::createTreeViewItem( KFileTreeViewItem *parent,
							KFileItem *fileItem )
{
   KFileTreeViewItem  *tvi = 0;

   if( parent && fileItem )
   {
      tvi = new KFileTreeViewItem( parent,
				   fileItem,
				   this );
   }
   else
   {
      kdDebug(250) << "createTreeViewItem: Have no parent" << endl;
   }
   return( tvi );
}

void KFileTreeBranch::setChildRecurse( bool t )
{
   m_recurseChildren = t;
   if( t == false )
      m_openChildrenURLs.clear();
}


void KFileTreeBranch::setShowExtensions( bool visible )
{
   m_showExtensions = visible;
}

bool KFileTreeBranch::showExtensions( ) const
{
   return( m_showExtensions );
}

/*
 * The signal that tells that a directory was deleted may arrive before the signal
 * for its children arrive. Thus, we must walk through the children of a dir and
 * remove them before removing the dir itself.
 */
void KFileTreeBranch::slotDeleteItem( KFileItem *it )
{

   if( !it ) return;
   kdDebug(250) << "Slot Delete Item hitted for " << it->url().prettyURL() << endl;

   KFileTreeViewItem *kfti = static_cast<KFileTreeViewItem*>(it->extraData(this));

   if( kfti )
   {
      kdDebug( 250 ) << "Child count: " << kfti->childCount() << endl;
      if( kfti->childCount() > 0 )
      {
	 KFileTreeViewItem *child = static_cast<KFileTreeViewItem*>(kfti->firstChild());

	 while( child )
	 {
	    kdDebug(250) << "Calling child to be deleted !" << endl;
	    KFileTreeViewItem *nextChild = static_cast<KFileTreeViewItem*>(child->nextSibling());
	    slotDeleteItem( child->fileItem());
	    child = nextChild;
	 }
      }

      kdDebug(250) << "Found corresponding KFileTreeViewItem" << endl;
      delete( kfti );
   }
   else
   {
      kdDebug(250) << "Error: kfiletreeviewitem: "<< kfti << endl;
   }
}


void KFileTreeBranch::slotCanceled( const KURL& url )
{
    // ### anything else to do?
   // remove the url from the childrento-recurse-list
   m_openChildrenURLs.remove( url);

   // stop animations etc.
   emit populateFinished( findTVIByURL(url));
}

void KFileTreeBranch::slotDirlisterClear()
{
   kdDebug(250)<< "*** Clear all !" << endl;
   /* this slots needs to clear all listed items, but NOT the root item */
   if( ! m_root ) return;

   QListViewItem *child = m_root->firstChild();
   QListViewItem *next = child;

   while( child )
   {
      next = child->nextSibling();
      delete child;
      child  = next;
   }
}

void KFileTreeBranch::slotRedirect( const KURL& oldUrl, const KURL&newUrl )
{
    if( oldUrl.equals( m_startURL, true ))
   {
      m_startURL = newUrl;
   }

}

void KFileTreeBranch::slotDirlisterClearURL( const KURL& url )
{
   kdDebug(250)<< "*** Clear for URL !" << url.prettyURL() << endl;
   KFileItem *item = find( url );
   if( item )
   {
      KFileTreeViewItem *ftvi =
	 static_cast<KFileTreeViewItem *>(item->extraData( this ));
      delete ftvi;
   }
}


KFileTreeViewItem* KFileTreeBranch::findTVIByURL( const KURL& url )
{
   KFileTreeViewItem *resultItem = 0;

    if( m_startURL.equals(url, true) )
   {
      kdDebug(250) << "findByURL: Returning root as a parent !" << endl;
      resultItem = m_root;
   }
    else if( m_lastFoundURL.equals( url, true ))
   {
      kdDebug(250) << "findByURL: Returning from lastFoundURL!" << endl;
      resultItem = m_lastFoundItem;
   }
   else
   {
        kdDebug(250) << "findByURL: searching by dirlister: " << url.url() << endl;

      KFileItem *it = findByURL( url );

      if( it )
      {
	 resultItem = static_cast<KFileTreeViewItem*>(it->extraData(this));
	 m_lastFoundItem = resultItem;
	 m_lastFoundURL = url;
      }
   }

    if ( !resultItem ) // file not found? startDir not existant?
        resultItem = m_root;

   return( resultItem );
}


void KFileTreeBranch::slCompleted( const KURL& url )
{
   kdDebug(250) << "SlotCompleted hit for " << url.prettyURL() << endl;
   KFileTreeViewItem *currParent = findTVIByURL( url );
   if( ! currParent ) return;

   kdDebug(250) << "current parent " << currParent << " is already listed: "
		<< currParent->alreadyListed() << endl;

   emit( populateFinished(currParent));
   emit( directoryChildCount(currParent, currParent->childCount()));

   /* This is a walk through the children of the last populated directory.
    * Here we start the dirlister on every child of the dir and wait for its
    * finish. When it has finished, we go to the next child.
    * This must be done for non local file systems in dirOnly- and Full-Mode
    * and for local file systems only in full mode, because the stat trick
    * (see addItem-Method) does only work for dirs, not for files in the directory.
    */
   /* Set bit that the parent dir was listed completely */
   currParent->setListed(true);

   kdDebug(250) << "recurseChildren: " << m_recurseChildren << endl;
   kdDebug(250) << "isLocalFile: " << m_startURL.isLocalFile() << endl;
   kdDebug(250) << "dirOnlyMode: " << dirOnlyMode() << endl;


   if( m_recurseChildren && (!m_startURL.isLocalFile() || ! dirOnlyMode()) )
   {
      bool wantRecurseUrl = false;
      /* look if the url is in the list for url to recurse */
      for ( KURL::List::Iterator it = m_openChildrenURLs.begin();
	    it != m_openChildrenURLs.end(); ++it )
      {
	 /* it is only interesting that the url _is_in_ the list. */
            if( (*it).equals( url, true ) )
	    wantRecurseUrl = true;
      }

      KFileTreeViewItem    *nextChild = 0;
      kdDebug(250) << "Recursing " << url.prettyURL() << "? " << wantRecurseUrl << endl;

      if( wantRecurseUrl && currParent )
      {
	
	 /* now walk again through the tree and populate the children to get +-signs */
	 /* This is the starting point. The visible folder has finished,
            processing the children has not yet started */
	 nextChild = static_cast<KFileTreeViewItem*>
	    (static_cast<QListViewItem*>(currParent)->firstChild());

	 if( ! nextChild )
	 {
	    /* This happens if there is no child at all */
	    kdDebug( 250 ) << "No children to recuse" << endl;
	 }

	 /* Since we have listed the children to recurse, we can remove the entry
	  * in the list of the URLs to see the children.
	  */
	 m_openChildrenURLs.remove(url);
      }

      if( nextChild ) /* This implies that idx > -1 */
      {
	 /* Next child is defined. We start a dirlister job on every child item
	  * which is a directory to find out how much children are in the child
	  * of the last opened dir
	  */

	 /* Skip non directory entries */
	 while( nextChild )
	 {
	    if( nextChild->isDir() && ! nextChild->alreadyListed())
	    {
	       KFileItem *kfi = nextChild->fileItem();
	       if( kfi && kfi->isReadable())
	       {
		  KURL recurseUrl = kfi->url();
		  kdDebug(250) << "Starting to recurse NOW " << recurseUrl.prettyURL() << endl;
		  openURL( recurseUrl, true );
	       }
	    }
	    nextChild = static_cast<KFileTreeViewItem*>(static_cast<QListViewItem*>(nextChild->nextSibling()));
	    // kdDebug(250) << "Next child " << m_nextChild << endl;
	 }
      }
   }
   else
   {
      kdDebug(250) << "skipping to recurse in complete-slot" << endl;
   }
}

/* This slot is called when a treeviewitem is expanded in the gui */
bool KFileTreeBranch::populate( const KURL& url,  KFileTreeViewItem *currItem )
{
   bool ret = false;
   if( ! currItem )
      return ret;

   kdDebug(250) << "Populating <" << url.prettyURL() << ">" << endl;

   /* Add this url to the list of urls to recurse for children */
   if( m_recurseChildren )
   {
      m_openChildrenURLs.append( url );
      kdDebug(250) << "Appending to list " << url.prettyURL() << endl;
   }

   if( ! currItem->alreadyListed() )
   {
      /* start the lister */
      ret = openURL( url, true );
   }
   else
   {
      kdDebug(250) << "Children already existing in treeview!" << endl;
      slCompleted( url );
      ret = true;
   }
   return ret;
}

void KFileTreeBranch::virtual_hook( int id, void* data )
{ KDirLister::virtual_hook( id, data ); }

#include "kfiletreebranch.moc"

