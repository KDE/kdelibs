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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kfile_tree_view_item_h
#define kfile_tree_view_item_h

#include "kde3support_export.h"

#include <Qt3Support/Q3PtrList>
#include <k3listview.h>

#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>

class KUrl;
class K3FileTreeView;
class KFileTreeBranch;


/**
 * An item for a K3FileTreeView that knows about its own KFileItem.
 */
class KDE3SUPPORT_EXPORT K3FileTreeViewItem : public K3ListViewItem
{
public:
   K3FileTreeViewItem( K3FileTreeViewItem*, const KFileItem&, KFileTreeBranch * );
   K3FileTreeViewItem( K3FileTreeView*, const KFileItem&, KFileTreeBranch * );
   ~K3FileTreeViewItem();

   /**
    * @return the KFileTreeBranch the item is sorted in.
    */
   KFileTreeBranch* branch() const     { return m_branch; }

   /**
    * @return the KFileItem the viewitem is representing.
    */
   KFileItem fileItem() const         { return m_kfileitem; }

   /**
    * @return the path of the item.
    */
   QString path() const;

   /**
    * @return the items KUrl
    */
   KUrl    url() const;

   /**
    * @return if the item represents a directory
    */
   bool    isDir() const;

   /**
    * @return if this directory was already seen by a KDirLister.
    */
   bool    alreadyListed() const;

   /**
    * set the flag if the directory was already listed.
    */
   void    setListed( bool wasListed );

protected:

private:

   KFileItem m_kfileitem;
   KFileTreeBranch *m_branch;
   bool  m_wasListed;
   class K3FileTreeViewItemPrivate;
   K3FileTreeViewItemPrivate *d;
};


/**
 * List of KFileTreeViewItems
 */
typedef Q3PtrList<K3FileTreeViewItem> K3FileTreeViewItemList;

/**
 * Iterator for KFileTreeViewItemList
 */
typedef Q3PtrListIterator<K3FileTreeViewItem> K3FileTreeViewItemListIterator;


#endif

