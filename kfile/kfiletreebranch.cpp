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
				  const QPixmap& pix, bool showHidden  )
    : KDirLister( false ),
      m_root( 0L ),
      m_startURL( url ),
      m_name ( name ),
      m_rootIcon( pix ),
      m_currParent( 0L ),
      m_nextChild( 0L ),
      m_wantDotFiles( showHidden ),
      m_recurseChildren( false )
{
   m_root = new KFileTreeViewItem( parent,
				   new KFileItem( url, "inode/directory",
                                                  S_IFDIR  ),
				   this );
   m_root->setPixmap( 0, pix );
   m_root->setText( 0, name );
   m_root->setOpen( true );
   m_currParent= m_root;

   connect( this, SIGNAL( newItems(const KFileItemList&)),
	    this,     SLOT  ( addItems( const KFileItemList& )));

   connect( this, SIGNAL( completed()),
	    this,     SLOT(slCompleted()));

   connect( this, SIGNAL( dirty( const QString& )),
	    this,     SLOT(slotDirty( const QString& )));

   connect( this, SIGNAL( started( const QString& )),
	    this,   SLOT( slotListerStarted( const QString& )));

   connect( this, SIGNAL( deleteItem( KFileItem* )),
	    this,   SLOT( slotDeleteItem( KFileItem* )));
   
   connect( this, SIGNAL( canceled() ),
            this,   SLOT( slotCanceled() ));
}


void KFileTreeBranch::slotListerStarted( const QString &url )
{
   kdDebug(1201) << "Slot Lister Started" << endl;

   /* set the parent correct if it is zero. */
   if( ! m_currParent )
   {
      kdDebug( 1201) << "No parent available at the moment" << endl;

      KFileItem *fi = findByName( url );
      if( fi )
      {
	 m_currParent = static_cast<KFileTreeViewItem*>( fi->extraData( this ));
      }
      else
      {
	 kdDebug(1201) << "No parent found in KFileItem for " << url << endl;
      }
   }
}

void KFileTreeBranch::slotDirty( const QString& dir )
{
   kdDebug(1201) << "Slot Dirty Started" << endl;

   /* set the parent correct if it is zero. */
   kdDebug( 1201) << "No parent available at the moment" << endl;

   KFileItem *fi = findByName( dir );
   if( fi )
   {
      m_currParent = static_cast<KFileTreeViewItem*>( fi->extraData( this ));
   }
   else
   {
      kdDebug(1201) << "No parent found in KFileItem for " << dir << endl;
   }
}


void KFileTreeBranch::addItems( const KFileItemList& list )
{
   /* Put the new items under the current url */
   KFileItemListIterator it( list );

   KFileItem *currItem;
   while ( m_currParent&& (currItem = it.current()) != 0 )
   {
      KFileTreeViewItem *newKFTVI = new KFileTreeViewItem( m_currParent,
                                                           currItem,
                                                           this);
      currItem->setExtraData( this, newKFTVI );

      /* Now try to find out if there are children for dirs in the treeview */
      /* This stats a directory on the local file system and checks the */
      /* hardlink entry in the stat-buf. This works only for directory. */
      if( currItem->isLocalFile( ) && currItem->isDir() )
      {
	 KURL url = currItem->url();
	 QString filename = url.directory( false, true ) + url.filename();
	 kdDebug(1201) << "Doing stat on " << filename << endl;
	 /* do the stat trick of Carsten. The problem is, that the hardlink
	 *  count only contains directory links. Thus, this method only seem
         * to work in dir-only mode */
	 struct stat statBuf;
	 if( stat( QFile::encodeName( filename ), &statBuf ) == 0 )
	 {
	    int hardLinks = statBuf.st_nlink;  /* Count of dirs */
	    kdDebug(1201) << "Hard link count: " << hardLinks << endl;
	    if( hardLinks > 2 )
	    {
	       newKFTVI->setExpandable(true);
	    }
	    else
	    {
	       newKFTVI->setExpandable(false);
	    }
	 }
	 else
	 {
	    kdDebug(1201) << "stat of " << filename << " failed !" << endl;
	 }
      }
      ++it;
   }
}



void KFileTreeBranch::slotDeleteItem( KFileItem *it )
{
   kdDebug(1201) << "Slot Delete Item hitted" << endl;

   KFileTreeViewItem *kfti = static_cast<KFileTreeViewItem*>(it->extraData(it));

   if( kfti )
   {
      kdDebug(1201) << "Found corresponding KFileTreeViewItem" << endl;
      delete( kfti );
   }
}


void KFileTreeBranch::slotCanceled()
{
    // ### anything else to do?
    emit populateFinished( m_currParent );
}

void KFileTreeBranch::slCompleted()
{
   kdDebug(1201) << "SlotCompleted hit !" << endl;
   emit( populateFinished( m_currParent));

   /* This is a walk through the children of the last populated directory.
    * Here we start the dirlister on every child of the dir and wait for its
    * finish. When it has finished, we go to the next child.
    * This must be done for non local file systems in dirOnly- and Full-Mode
    * and for local file systems only in full mode, because the stat trick
    * (see addItem-Method).
    * does only work for dirs, not for files in the directory.
    */
   if( !m_startURL.isLocalFile() || ! dirOnlyMode() )
   {
      /* now walk again through the tree and populate the children to get +-signs */
      if( ! m_currParent ) return;

      if( m_recurseChildren && ! m_nextChild )
      {
	 /* This is the starting point. The visible folder has finished,
            processing the children has not yet started */
	 m_nextChild = static_cast<KFileTreeViewItem*>
	    (static_cast<QListViewItem*>(m_currParent)->firstChild());

	 m_recurseChildren = false;
	 if( ! m_nextChild )
	 {
	    /* This happens if there is no child at all */
	    kdDebug( 1201 ) << "No children to recuse" << endl;
	 }
	 else
	 {
	    KFileItem *fi = m_currParent->fileItem();
	 }

      }

      if( m_nextChild )
      {
	 /* Next child is defined, the dirlister job must be started with on the
	  * closed child of an open item to find out if there are children to make
	  * the +-sign in the treeview. */

	 /* Skip non directory entries */
	 while( m_nextChild && !m_nextChild->isDir())
	 {
	    m_nextChild = static_cast<KFileTreeViewItem*>(static_cast<QListViewItem*>(m_nextChild->nextSibling()));
	    kdDebug(1201) << "Next child " << m_nextChild << endl;
	 }

	 /* at this point, m_nextChildren is a dir if defined or m_nextChild is zero, both OK. */
	 if( m_nextChild )
	 {
	    KFileItem *kfi = m_nextChild->fileItem();
	    if( kfi )
	    {
	       KURL url = kfi->url();
	       kdDebug(1201) << "Looking for children of <" << url.prettyURL() << ">" << endl;
	       m_currParent = m_nextChild;

	       /* now switch the nextChild pointer to the next to have the next item when
		* this slot is hit again.
		*/
	       m_nextChild = static_cast<KFileTreeViewItem*>(static_cast<QListViewItem*>(m_nextChild->nextSibling()));
	       if( kfi->isReadable() )
		  openURL( url, m_wantDotFiles, true );
	       else
		  kdDebug(1201) << "Can not recurse to " << url.prettyURL() << endl;
	    }
	 }
	 else
	 {
	    kdDebug(1201) << "** parsing of children finished" << endl;
	    m_recurseChildren = false;
	    m_currParent = 0L;
	 }
      }
   }
   else
   {
      kdDebug(1201) << "skipping to recurse in complete-slot" << endl;
      m_currParent = 0L;

   }
}

void KFileTreeBranch::populate( const KURL& url,  KFileTreeViewItem *currItem )
{
   if( ! currItem )
      return;
   m_currParent = currItem;

   kdDebug(1201) << "Populating <" << url.prettyURL() << ">" << endl;

   /* Recurse to the first row of children of the new files to get info about children */
   m_recurseChildren = true;

   openURL( url, m_wantDotFiles, true );
}

void KFileTreeBranch::populate( )
{
   populate( m_startURL, m_currParent );
}

#include "kfiletreebranch.moc"

