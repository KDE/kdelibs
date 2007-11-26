
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001 Klaas Freitag <freitag@suse.de>

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

#ifndef kfile_tree_branch_h
#define kfile_tree_branch_h

class QColorGroup;

#include <Qt3Support/Q3ListView>

#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>
#include <k3filetreeviewitem.h>

class KUrl;
class K3FileTreeView;


/**
 * This is the branch class of the K3FileTreeView, which represents one
 * branch in the treeview. Every branch has a root which is an url. The branch
 * lists the files under the root. Every branch uses its own dirlister and can
 * have its own filter etc.
 *
 * @short Branch object for K3FileTreeView object.
 *
 */

class KDE3SUPPORT_EXPORT KFileTreeBranch : public KDirLister
{
   Q_OBJECT
public:
   /**
    * constructs a branch for K3FileTreeView. Does not yet start to list it.
    * @param url start url of the branch.
    * @param name the name of the branch, which is displayed in the first column of the treeview.
    * @param pix is a pixmap to display as an icon of the branch.
    * @param showHidden flag to make hidden files visible or not.
    * @param branchRoot is the K3FileTreeViewItem to use as the root of the
    *        branch, with the default 0 meaning to let KFileTreeBranch create
    *        it for you.
    */
   KFileTreeBranch( K3FileTreeView*, const KUrl& url, const QString& name,
                    const QPixmap& pix, bool showHidden = false,
		    K3FileTreeViewItem *branchRoot = 0 );

   /**
    * @returns the root url of the branch.
    */
   KUrl rootUrl() const;

   /**
    * sets a K3FileTreeViewItem as root widget for the branch.
    * That must be created outside of the branch. All KFileTreeViewItems
    * the branch is allocating will become children of that object.
    * @param r the K3FileTreeViewItem to become the root item.
    */
   virtual void setRoot( K3FileTreeViewItem *r );

   /**
    * @returns the root item.
    */
   K3FileTreeViewItem *root( );

   /**
    * @returns the name of the branch.
    */
   QString name() const;

   /**
    * sets the name of the branch.
    */
   virtual void setName( const QString n );

   /*
    * returns the current root item pixmap set in the constructor. The root
    * item pixmap defaults to the icon for directories.
    * @see openPixmap()
    */
   QPixmap pixmap() const;

   /*
    * returns the current root item pixmap set by setOpenPixmap()
    * which is displayed if the branch is expanded.
    * The root item pixmap defaults to the icon for directories.
    * @see pixmap()
    * Note that it depends on K3FileTreeView::showFolderOpenPximap weather
    * open pixmap are displayed or not.
    */
   QPixmap openPixmap() const;

   /**
    * @returns whether the items in the branch show their file extensions in the
    * tree or not. See setShowExtensions for more information.
    */
   bool showExtensions( ) const;

   /**
    * sets the root of the branch open or closed.
    */
   void setOpen( bool setopen = true );

   /**
    * sets if children recursion is wanted or not. If this is switched off, the
    * child directories of a just opened directory are not listed internally.
    * That means that it can not be determined if the sub directories are
    * expandable or not. If this is switched off there will be no call to
    * setExpandable.
    * @param t set to true to switch on child recursion
    */
   void setChildRecurse( bool t=true );

   /**
    * @returns if child recursion is on or off.
    * @see setChildRecurse
    */
   bool childRecurse();

public Q_SLOTS:
   /**
    * populates a branch. This method must be called after a branch was added
    * to  a K3FileTreeView using method addBranch.
    * @param url is the url of the root item where the branch starts.
    * @param currItem is the current parent.
    */
   virtual bool populate( const KUrl &url, K3FileTreeViewItem* currItem );

   /**
    * sets printing of the file extensions on or off. If you pass false to this
    * slot, all items of this branch will not show their file extensions in the
    * tree.
    * @param visible flags if the extensions should be visible or not.
    */
   virtual void setShowExtensions( bool visible = true );

   void setOpenPixmap( const QPixmap& pix );

protected:
   /**
    * allocates a K3FileTreeViewItem for the branch
    * for new items.
    */
   virtual K3FileTreeViewItem *createTreeViewItem( K3FileTreeViewItem *parent,
						  const KFileItem &fileItem );

public:
   /**
    * find the according K3FileTreeViewItem by an url
    */
   virtual K3FileTreeViewItem *findTVIByUrl( const KUrl& );

Q_SIGNALS:
   /**
    * emitted with the item of a directory which was finished to populate
    */
   void populateFinished( K3FileTreeViewItem * );

   /**
    * emitted with a list of new or updated K3FileTreeViewItem which were
    * found in a branch. Note that this signal is emitted very often and may slow
    * down the performance of the treeview !
    */
   void newTreeViewItems( KFileTreeBranch*, const K3FileTreeViewItemList& );

   /**
    * emitted with the exact count of children for a directory.
    */
   void directoryChildCount( K3FileTreeViewItem* item, int count );

private Q_SLOTS:
   void slotRefreshItems( const QList<QPair<KFileItem, KFileItem> >& );
   void addItems( const KFileItemList& );
   void slCompleted( const KUrl& );
   void slotCanceled( const KUrl& );
   void slotListerStarted( const KUrl& );
   void slotDeleteItem( const KFileItem& );
   void slotDirlisterClear();
   void slotDirlisterClearUrl( const KUrl& url );
   void slotRedirect( const KUrl& oldUrl, const KUrl&newUrl );

private:
   K3FileTreeViewItem    *parentKFTVItem( const KFileItem &item );
   static void           deleteChildrenOf( Q3ListViewItem *parent );
   K3FileTreeViewItem* treeItemForFileItem(const KFileItem &it);

   K3FileTreeViewItem 	*m_root;
   KUrl 		m_startURL;
   QString 		m_name;
   QPixmap 		m_rootIcon;
   QPixmap              m_openRootIcon;

   /* this list holds the url's which children are opened. */
   KUrl::List           m_openChildrenURLs;


   /* The next two members are used for caching purposes in findTVIByURL. */
   KUrl                 m_lastFoundURL;
   K3FileTreeViewItem   *m_lastFoundItem;

   bool                 m_recurseChildren :1;
   bool                 m_showExtensions  :1;

private:
   class KFileTreeBranchPrivate;
   KFileTreeBranchPrivate *d;
};


/**
 * List of KFileTreeBranches
 */
typedef Q3PtrList<KFileTreeBranch> KFileTreeBranchList;

/**
 * Iterator for KFileTreeBranchLists
 */
typedef Q3PtrListIterator<KFileTreeBranch> KFileTreeBranchIterator;

#endif

