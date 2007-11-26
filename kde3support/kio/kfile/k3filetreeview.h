/* This file is part of the KDE project
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kfile_tree_view_h
#define kfile_tree_view_h

#include <QtCore/QMap>
#include <QtCore/QPoint>
#include <QtGui/QPixmap>



#include <k3listview.h>
#include <kdirnotify.h>
#include <kio/job.h>
#include <k3filetreeviewitem.h>
#include <kfiletreebranch.h>

class QTimer;



/**
 * The filetreeview offers a treeview on the file system which behaves like
 * a QTreeView showing files and/or directories in the file system.
 *
 * K3FileTreeView is able to handle more than one URL, represented by
 * KFileTreeBranch.
 *
 * Typical usage:
 * 1. create a K3FileTreeView fitting in your layout and add columns to it
 * 2. call addBranch to create one or more branches
 * 3. retrieve the root item with KFileTreeBranch::root() and set it open
 *    if desired. That starts the listing.
 */
class KDE3SUPPORT_EXPORT K3FileTreeView : public K3ListView
{
    Q_OBJECT
public:
    K3FileTreeView( QWidget *parent );
    virtual ~K3FileTreeView();

    /**
     * @return the current (i.e. selected) item
     */
    K3FileTreeViewItem * currentKFileTreeViewItem() const;

   /**
    * @return the URL of the current selected item.
    */
    KUrl currentUrl() const;

   /**
    *  Adds a branch to the treeview item.
    *
    *  This high-level function creates the branch, adds it to the treeview and
    *  connects some signals. Note that directory listing does not start until
    *  a branch is expanded either by opening the root item by user or by setOpen
    *  on the root item.
    *
    *  @returns a pointer to the new branch or zero
    *  @param path is the base url of the branch
    *  @param name is the name of the branch, which will be the text for column 0
    *  @param showHidden says if hidden files and directories should be visible
    */
   KFileTreeBranch* addBranch( const KUrl &path, const QString& name, bool showHidden = false );

   /**
    *  same as the function above but with a pixmap to set for the branch.
    */
   virtual KFileTreeBranch* addBranch( const KUrl &path, const QString& name ,
				       const QPixmap& pix, bool showHidden = false  );

   /**
    *  same as the function above but letting the user create the branch.
    */
   virtual KFileTreeBranch* addBranch( KFileTreeBranch * );

   /**
    *  removes the branch from the treeview.
    *  @param branch is a pointer to the branch
    *  @returns true on success.
    */
   virtual bool removeBranch( KFileTreeBranch *branch );

   /**
    *  @returns a pointer to the KFileTreeBranch in the K3FileTreeView or zero on failure.
    *  @param searchName is the name of a branch
    */
   KFileTreeBranch *branch( const QString& searchName );


   /**
    *  @returns a list of pointers to all existing branches in the treeview.
    **/
   KFileTreeBranchList& branches();

   /**
    *  set the directory mode for branches. If true is passed, only directories will be loaded.
    *  @param branch is a pointer to a KFileTreeBranch
    */
   virtual void setDirOnlyMode( KFileTreeBranch *branch, bool );

   /**
    * searches a branch for a K3FileTreeViewItem identified by the relative url given as
    * second parameter. The method adds the branches base url to the relative path and finds
    * the item.
    * @returns a pointer to the item or zero if the item does not exist.
    * @param brnch  is a pointer to the branch to search in
    * @param relUrl is the branch relativ url
    */
   K3FileTreeViewItem *findItem( KFileTreeBranch* brnch, const QString& relUrl );

   /**
    * see method above, differs only in the first parameter. Finds the branch by its name.
    */
   K3FileTreeViewItem *findItem( const QString& branchName, const QString& relUrl );

   /**
    * @returns a flag indicating if extended folder pixmaps are displayed or not.
    */
   bool showFolderOpenPixmap() const;

public Q_SLOTS:

   /**
    * set the flag to show 'extended' folder icons on or off. If switched on, folders will
    * have an open folder pixmap displayed if their children are visible, and the standard
    * closed folder pixmap (from mimetype folder) if they are closed.
    * If switched off, the plain mime pixmap is displayed.
    * @param showIt = false displays mime type pixmap only
    */
   virtual void setShowFolderOpenPixmap( bool showIt = true );

protected:
   /**
    * @returns true if we can decode the drag and support the action
    */

    virtual bool acceptDrag(QDropEvent* event) const;
    virtual Q3DragObject * dragObject();

    virtual void startAnimation( K3FileTreeViewItem* item, const char * iconBaseName = "kde", uint iconCount = 6 );
    virtual void stopAnimation( K3FileTreeViewItem* item );
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *ev );

protected Q_SLOTS:
    virtual void slotNewTreeViewItems( KFileTreeBranch*,
				       const K3FileTreeViewItemList& );

    virtual void slotSetNextUrlToSelect( const KUrl &url );

    virtual QPixmap itemIcon( K3FileTreeViewItem*, int gap = 0 ) const;

private Q_SLOTS:
    void slotExecuted( Q3ListViewItem * );
    void slotExpanded( Q3ListViewItem * );
    void slotCollapsed( Q3ListViewItem *item );

    void slotSelectionChanged();

    void slotAnimation();

    void slotAutoOpenFolder();

    void slotOnItem( Q3ListViewItem * );
    void slotItemRenamed(Q3ListViewItem*, const QString &, int);

   void slotPopulateFinished( K3FileTreeViewItem* );


Q_SIGNALS:

   void onItem( const QString& );
   /* New signals if you like it ? */
   void dropped( QWidget*, QDropEvent* );
   void dropped( QWidget*, QDropEvent*, KUrl::List& );
   void dropped( KUrl::List&, KUrl& );
   // The drop event allows to differentiate between move and copy
   void dropped( QWidget*, QDropEvent*, KUrl::List&, KUrl& );

   void dropped( QDropEvent *, Q3ListViewItem * );
   void dropped(K3FileTreeView *, QDropEvent *, Q3ListViewItem *);
   void dropped(QDropEvent *, Q3ListViewItem *, Q3ListViewItem *);
   void dropped(K3FileTreeView *, QDropEvent *, Q3ListViewItem *, Q3ListViewItem *);

protected:
   KUrl m_nextUrlToSelect;


private:
    // Returns whether item is still a valid item in the tree
    bool isValidItem( Q3ListViewItem *item);
    void clearTree();


   /* List that holds the branches */
    KFileTreeBranchList m_branches;


    struct AnimationInfo
    {
        AnimationInfo( const char * _iconBaseName, uint _iconCount, const QPixmap & _originalPixmap )
            : iconBaseName(_iconBaseName), iconCount(_iconCount), iconNumber(1), originalPixmap(_originalPixmap) {}
        AnimationInfo() : iconCount(0) {}
        QByteArray iconBaseName;
        uint iconCount;
        uint iconNumber;
        QPixmap originalPixmap;
    };
    typedef QMap<K3FileTreeViewItem *, AnimationInfo> MapCurrentOpeningFolders;
    MapCurrentOpeningFolders m_mapCurrentOpeningFolders;


    QTimer *m_animationTimer;

    QPoint m_dragPos;
    bool m_bDrag;

    bool m_wantOpenFolderPixmaps; // Flag weather the folder should have open-folder pixmaps

    Q3ListViewItem *m_currentBeforeDropItem; // The item that was current before the drag-enter event happened
    Q3ListViewItem *m_dropItem; // The item we are moving the mouse over (during a drag)
    QPixmap  m_openFolderPixmap;
    QTimer *m_autoOpenTimer;

private:
   class K3FileTreeViewPrivate;
   K3FileTreeViewPrivate *d;
};

#endif
