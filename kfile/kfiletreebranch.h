
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kfile_tree_branch_h
#define kfile_tree_branch_h

#include <qlistview.h>
#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>
#include <qdict.h>

class KURL;
class KFileTreeView;
class KFileTreeViewItem;


/**
 *  This is the branch class of the KFileTreeViewWidget, which represents one
 *  branch in the treeview. Every branch has a root, which is an Url and lists
 *  the files unter the root. Every branch uses its own dirlister and can have
 *  its own filter etc, different from other branches in the treeview.
 *
 *  @short Branch object for @ref KFileTreeView object.
 *
 */

class KFileTreeBranch : public KDirLister
{
   Q_OBJECT
public:
   /**
    * constructs a branch, listing the file system.
    * @param url start url of the branch.
    * @param name the name of the branch, which is displayed in the first column of the treeview.
    * @param pix is a pixmap to display as an icon of the branch.
    * @param showHidden flag to make hidden files visible or not.
    */
   KFileTreeBranch( KFileTreeView*, const KURL& url, const QString& name,
                    const QPixmap& pix, bool showHidden = false);

   /**
    * @returns the root url of the branch.
    */
   KURL 	rootUrl() const { return( m_startURL ); }

   /**
    * sets a @ref KFileTreeViewItem as root widget for the branch.
    * That must be created outside of the branch. All KFileTreeViewItems
    * the branch is allocating will become children of that object.
    * @param the KFileTreeViewItem to become the root item.
    */
   void 	setRoot( KFileTreeViewItem *r ){ m_root = r; };

   /**
    * @returns the root item.
    */
   KFileTreeViewItem *root( ) { return( m_root );}

   /**
    * @returns the name of the branch.
    */
   QString      name() const { return( m_name ); }

   /**
    * sets the name of the branch.
    */
   void         setName( const QString n ) { m_name = n; };

   QPixmap& pixmap(){ return(m_rootIcon); }
   
public slots:
   void populate( );
   void populate( const KURL&, KFileTreeViewItem* );

signals:
   void populateFinished( KFileTreeViewItem * );
   
private slots:
   void addItems( const KFileItemList& );
   void slCompleted();
   void slotCanceled();
   void slotListerStarted( const KURL& );
   void slotDeleteItem( KFileItem* );

private:
   KFileTreeViewItem 	*m_root;
   KURL 		m_startURL;
   QString 		m_name;
   QPixmap 		m_rootIcon;
   KFileTreeViewItem    *m_currParent;
   KFileTreeViewItem    *m_nextChild;

   bool 		m_wantDotFiles    :1;
   bool                 m_recurseChildren :1;

   class KFileTreeBranchPrivate;
   KFileTreeBranchPrivate *d;
};



#endif

