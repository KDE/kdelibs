/* This file is part of the KDE project
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

#ifndef kfile_tree_view_item_h
#define kfile_tree_view_item_h

#include <qptrlist.h>
#include <klistview.h>

#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>

class KURL;
class KFileTreeView;
class KFileTreeBranch;
class KFileTreeItem;


/**
 * An item for @ref a KFileTreeView that knows about its own @ref KFileItem.
 */
class KFileTreeViewItem : public KListViewItem
{
public:
   KFileTreeViewItem( KFileTreeViewItem*, KFileItem*, KFileTreeBranch * );
   KFileTreeViewItem( KFileTreeView*, KFileItem*, KFileTreeBranch * );

   /**
    * @return the @ref KFileTreeBranch the item is sorted in.
    */
   KFileTreeBranch* branch() const     { return m_branch; }

   /**
    * @return the @ref KFileItem the viewitem is representing.
    */
   KFileItem *fileItem() const         { return m_kfileitem; }

   /**
    * @return the path of the item.
    */
   QString path() const;

   /**
    * @return the items @ref KURL
    */
   KURL    url() const;

   /**
    * @return if the item represents a directory 
    */ 
   bool    isDir() const;

   /**
    * @return if this directory was already seen by a @ref KDirLister.
    */ 
   bool    alreadyListed() const;

   /**
    * set the flag if the directory was already listed.
    */ 
   void    setListed( bool wasListed );
   
protected:

private:

   KFileItem *m_kfileitem;
   KFileTreeBranch *m_branch;
   bool  m_wasListed;
   class KFileTreeViewItemPrivate;
   KFileTreeViewItemPrivate *d;
};


/**
 * List of KFileTreeViewItems
 */
typedef QPtrList<KFileTreeViewItem> KFileTreeViewItemList;

/**
 * Iterator for KFileTreeViewItemList
 */
typedef QPtrListIterator<KFileTreeViewItem> KFileTreeViewItemListIterator;


#endif

