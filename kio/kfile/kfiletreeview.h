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

#ifndef kfile_tree_view_h
#define kfile_tree_view_h

#include <qmap.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <klistview.h>
#include <kdirnotify.h>
#include <kio/job.h>
#include <kfiletreeviewitem.h>
#include <kfiletreebranch.h>

class QTimer;



class KFileTreeViewToolTip : public QToolTip
{
public:
    KFileTreeViewToolTip( QListView *view ) : QToolTip( view ), m_view( view ) {}

protected:
    virtual void maybeTip( const QPoint & );

private:
    QListView *m_view;
};


/**
 * The multi-purpose tree (listview)
 * It parses its configuration (desktop files), each one corresponding to
 * a toplevel item, and creates the modules that will handle the contents
 * of those items.
 */
class KFileTreeView : public KListView
{
    Q_OBJECT
public:
    KFileTreeView( QWidget *parent, const char *name = 0 );
    virtual ~KFileTreeView();

   // void followURL( const KURL &url );

    /**
     * @return the current (i.e. selected) item
     */
    KFileTreeViewItem * currentKFileTreeViewItem() const;

    KURL currentURL() const;

   /**
    *  adds a branch to the treeview item. This highlevel function creates the branch, adds
    *  it to the treeview, starts the branches dirlister to list the directory and
    *  @returns a pointer to the new branch or zero
    *  @param path is the base url of the branch
    *  @param name is the name of the branch, which will be the text for column 0
    *  @param showHidden says if hidden files and directories should be visible
    */
   KFileTreeBranch* addBranch( const KURL &path, const QString& name, bool showHidden = false );

   /**
    *  same as the function above but with a pixmap to set for the branch.
    */
   virtual KFileTreeBranch* addBranch( const KURL &path, const QString& name ,const QPixmap& pix, bool showHidden = false  );

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
    *  @returns a pointer to the @ref KFileTreeBranch in the KFileTreeView or zero on failiure.
    *  @param branchno is the name of a branch
    */
   KFileTreeBranch *branch( const QString& searchName );


   /**
    *  @returns a list of pointers to all existing branches in the treeview.
    **/
   KFileTreeBranchList& branches();
   
   /**
    *  set the directory mode for branches. If true is passed, only directories will be loaded.
    *  @param branch is a pointer to a @ref KFileTreeBranch
    */
   virtual void setDirOnlyMode( KFileTreeBranch *brnch, bool );

   /**
    * searches a branch for a @ref KFileTreeViewItem identified by the relative url given as
    * second parameter. The method adds the branches base url to the relative path and finds
    * the item.
    * @returns a pointer to the item or zero if the item does not exist.
    * @param brnch  is a pointer to the branch to search in
    * @param relUrl is the branch relativ url
    */
   KFileTreeViewItem *findItem( KFileTreeBranch* brnch, const QString& relUrl );

   /**
    * see method above, differs only in the first parameter. Finds the branch by its name.
    */
   KFileTreeViewItem *findItem( const QString& branchName, const QString& relUrl );
   
   /**
    * @returns a flag indicating if extended folder pixmaps are displayed or not.
    */
   bool showFolderOpenPixmap() const { return m_wantOpenFolderPixmaps; };
   
public slots:
   /**
    * starts to list the filesystem for the branch. This is the last method to call after
    * having created a branch and set attributes like @ref setDirOnlyMode. This makes the
    * branch alive
    */
   virtual void populateBranch( KFileTreeBranch *brnch );

   
   /**
    * set the flag to show 'extended' folder icons on or off. If switched on, folders will
    * have an open folder pixmap displayed if their children are visible, and the standard
    * closed folder pixmap (from mimetype folder) if they are closed.
    * If switched off, the plain mime pixmap is displayed.
    * @param showIt = false displays mime type pixmap only
    */
   virtual void setShowFolderOpenPixmap( bool showIt = true )
      { m_wantOpenFolderPixmaps = showIt; }
   
protected:
    virtual QDragObject * dragObject();
    virtual void startAnimation( KFileTreeViewItem* item, const char * iconBaseName = "kde", uint iconCount = 6 );
    virtual void stopAnimation( KFileTreeViewItem* item );

    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *ev );

    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );

    virtual void leaveEvent( QEvent * );

protected slots:
    virtual void slotNewTreeViewItems( KFileTreeBranch*,
				       const KFileTreeViewItemList& );

    virtual void slotSetNextUrlToSelect( const KURL &url )
      { m_nextUrlToSelect = url; }

    virtual QPixmap itemIcon( KFileTreeViewItem*, int gap = 0 ) const;
   
private slots:
    void slotExecuted( QListViewItem * );
    void slotExpanded( QListViewItem * );
    void slotCollapsed( QListViewItem *item );
   
    void slotMouseButtonPressed(int _button, QListViewItem* _item, const QPoint&, int col);
    void slotSelectionChanged();

    void slotAnimation();

    void slotAutoOpenFolder();

    void slotOnItem( QListViewItem * );
    void slotItemRenamed(QListViewItem*, const QString &, int);

   void slotResult(  );
   void slotCanceled(  );

   void slotPopulateFinished( KFileTreeViewItem* );

   
signals:

   void onItem( const QString& );
   
protected:
   KURL m_nextUrlToSelect;

   
private:
    void clearTree();
    bool checkOnFilter( QString&);


   /* List that holds the branches */
    KFileTreeBranchList m_branches;


    struct AnimationInfo
    {
        AnimationInfo( const char * _iconBaseName, uint _iconCount, const QPixmap & _originalPixmap )
            : iconBaseName(_iconBaseName), iconCount(_iconCount), iconNumber(1), originalPixmap(_originalPixmap) {}
        AnimationInfo() : iconCount(0) {}
        QCString iconBaseName;
        uint iconCount;
        uint iconNumber;
        QPixmap originalPixmap;
    };
    typedef QMap<KFileTreeViewItem *, AnimationInfo> MapCurrentOpeningFolders;
    MapCurrentOpeningFolders m_mapCurrentOpeningFolders;


    QTimer *m_animationTimer;

    QPoint m_dragPos;
    bool m_bDrag;

   bool m_wantOpenFolderPixmaps; // Flag weather the folder should have open-folder pixmaps
   
    QListViewItem *m_currentBeforeDropItem; // The item that was current before the drag-enter event happened
    QListViewItem *m_dropItem; // The item we are moving the mouse over (during a drag)
    QStrList m_lstDropFormats;
   QPixmap  m_openFolderPixmap;
    QTimer *m_autoOpenTimer;

    KFileTreeViewToolTip m_toolTip;

   
   class KFileTreeViewPrivate;
   KFileTreeViewPrivate *d;
};

#endif
