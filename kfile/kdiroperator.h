// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
		  2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KDIROPERATOR_H_
#define KDIROPERATOR_H_

#include <qwidget.h>
#include <qstack.h>

#include <kaction.h>
#include <kcompletion.h>

#include <kfileview.h>
#include <kfileviewitem.h>
#include <kfile.h>

class QPopupMenu;
class QTimer;

class KAction;
class KToggleAction;
class KActionSeparator;
class KActionMenu;
class KFileReader;
class QWidgetStack;
class KProgress;
namespace KIO {
  class DeleteJob;
}

class KDirOperator : public QWidget
{
    Q_OBJECT

 public:
    /**
     * You need to call listDirectory() to actually load the directory,
     * this constructor only initializes and doesn't start loading.
     */
    KDirOperator(const KURL& urlName = KURL(),
		 QWidget *parent = 0, const char* name = 0);
    virtual ~KDirOperator();

    void setShowHiddenFiles ( bool s ) { showHiddenAction->setChecked( s ); }
    bool showHiddenFiles () const { return showHiddenAction->isChecked(); }

    void close();

    /**
     * Set a filter like "*.cpp *.h *.o". Call @ref rereadDir() to apply it.
     */
    void setNameFilter(const QString& filter);

    /**
     * @returns the current namefilter.
     */
    const QString& nameFilter() const { return dir->nameFilter(); }

    void setMimeFilter( const QStringList& mimetypes );

    void clearFilter();

    /**
     * local URLs (i.e. directories) always have file: prepended.
     */
    KURL url() const;

    void setURL(const KURL& url, bool clearforward);

    /**
     * Clears the current selection and attempts to set @p filename
     * the current file. filename is just the name, no path or url.
     */
    void setCurrentItem( const QString& filename );

    //this also reads the current url(), so you better call this after setURL()
    void setView(KFileView *view);
    // ### make non-const
    const KFileView * view() const { return fileView; }

    /**
     * Returns the widget of the current view. 0L if there is no view/widget.
     */
    QWidget * viewWidget() const { return fileView ? fileView->widget() : 0L; }

    void setView(KFile::FileView view);

    void setSorting( QDir::SortSpec );
    QDir::SortSpec sorting() const { return mySorting; }

    bool isRoot() const;

    KFileReader *fileReader() const { return dir; }

    /**
     * @returns the progress widget, that is shown during directory listing.
     * You can for example reparent() it to put it into a statusbar.
     */
    KProgress * progressBar() const;

    /**
     * Sets the listing/selection mode for the views, an OR'ed combination of
     * @li File
     * @li Directory
     * @li Files
     * @li ExistingOnly
     * @li LocalOnly
     *
     * You cannot mix File and Files of course.
     */
    void setMode( KFile::Mode m );
    KFile::Mode mode() const;

    void setPreviewWidget(const QWidget *w);

    /**
     * @returns a list of all currently selected items. If there is no view, then
     * 0L is returned.
     */
    const KFileViewItemList * selectedItems() const {
	return ( fileView ? fileView->selectedItems() : 0L );
    }
    inline bool isSelected( const KFileViewItem *item ) const {
	return ( fileView ? fileView->isSelected( item ) : false );
    }


    int numDirs() const;
    int numFiles() const;

    /**
     * @returns a KCompletion object, containing all filenames and
     * directories of the current directory/URL.
     * You can use it to insert it into a @ref KLineEdit or @ref KComboBox
     * Note: it will only contain files, after @ref prepareCompletionObjects()
     * has been called. It will be implicitly called from @ref makeCompletion()
     * or @ref makeDirCompletion()
     */
    KCompletion * completionObject() const {
	return const_cast<KCompletion *>( &myCompletion );
    }

    /**
     * @returns a KCompletion object, containing only all directories of the
     * current directory/URL.
     * You can use it to insert it into a @ref KLineEdit or @ref KComboBox
     * Note: it will only contain directories, after
     * @ref prepareCompletionObjects() has been called. It will be implicitly
     * called from @ref makeCompletion() or @ref makeDirCompletion()
     */
    KCompletion *dirCompletionObject() const {
	return const_cast<KCompletion *>( &myDirCompletion );
    }
    /**
     * an accessor to a collection of all available Actions. The actions
     * are static, they will be there all the time (no need to connect to
     * the signals KActionCollection::inserted() or removed().
     *
     * There are the following actions:
     *
     * @li popupMenu : an ActionMenu presenting a popupmenu with all actions
     * @li up : changes to the parent directory
     * @li back : goes back to the previous directory
     * @li forward : goes forward in the history
     * @li home : changes to the user's home directory
     * @li reload : reloads the current directory
     * @li separator : a separator
     * @li mkdir : opens a dialog box to create a directory
     * @li delete : deletes the selected files/directories
     * @li sorting menu : an ActionMenu containing all sort-options
     * @li by name : sorts by name
     * @li by date : sorts by date
     * @li by size : sorts by size
     * @li reversed : reverses the sort order
     * @li dirs first : sorts directories before files
     * @li case insensitive : sorts case insensitively
     * @li view menu : an ActionMenu containing all actions concerning the view
     * @li short view : shows a simple fileview
     * @li detailed view : shows a detailed fileview (dates, permissions ,...)
     * @li show hidden : shows hidden files
     * @li separate dirs : shows directories in a separate pane
     * @li preview  : shows a preview next to the fileview
     * @li single : hides the separate view for directories or the preview
     *
     * The short and detailed view are in an exclusive group. The sort-by
     * actions are in an exclusive group as well. Also the "separate dirs",
     * "preview" and "single" actions are in an exclusive group.
     *
     * You can e.g. use
     * <pre>actionCollection()->action( "up" )->plug( someToolBar );</pre>
     * to add a button into a toolbar, which makes the dirOperator change to
     * its parent directory.
     *
     * @returns all available Actions
     */
    KActionCollection * actionCollection() const { return myActionCollection; }

    /**
     * Reads the default settings for a view, i.e. the default KFile::FileView.
     * Also reads the sorting and whether hidden files should be shown.
     * Note: the default view will not be set - you have to call
     * <pre>@ref setView( KFile::Default )</pre> to apply it.
     */
    virtual void readConfig( KConfig *, const QString& group = QString::null );

    virtual void saveConfig( KConfig *, const QString& group = QString::null );


    /**
     * This is a KFileDialog specific hack: we want to select directories with
     * single click, but not files. But as a generic class, we have to be able
     * to select files on single click as well.
     *
     * This gives us the opportunity to do both.
     *
     * The default is false, set it to true if you don't want files selected
     * with single click.
     */
    void setOnlyDoubleClickSelectsFiles( bool enable );

    /**
     * @returns whether files (not directories) should only be select()ed by
     * double-clicks.
     * @see #setOnlyDoubleClickSelectsFiles
     */
    bool onlyDoubleClickSelectsFiles() const;

    /**
     * Creates the given directory/url. If it is a relative path,
     * it will be completed with the current directory.
     * If enterDirectory is true, the directory will be entered after a
     * successful operation. If unsuccessful, a messagebox will be presented
     * to the user.
     * @returns true if the directory could be created.
     */
    bool mkdir( const QString& directory, bool enterDirectory = true );

    KIO::DeleteJob * del( const KFileViewItemList& items,
                          bool ask = true, bool showProgress = true );

    /**
     * Clears the forward and backward history.
     */
    void clearHistory();

    /**
     * When going up in the directory hierarchy, KDirOperator can highlight
     * the directory that was just left.
     *
     * I.e. when you go from /home/gis/src to /home/gis, the item "src" will
     * be made the current item.
     *
     * Default is off.
     */
    void setEnableDirHighlighting( bool enable );

    /**
     * @returns whether the last directory will be made the current item
     * when going up in the directory hierarchy.
     *
     * Default is false.
     */
    bool dirHighlighting() const;

protected:
    void setFileReader( KFileReader *reader );
    void resizeEvent( QResizeEvent * );
    void setupActions();
    void updateSortActions();
    void updateViewActions();
    void setupMenu();
    void prepareCompletionObjects();
    bool checkPreviewSupport();

private:
    /**
     * Contains all URLs you can reach with the back button.
     */
    QStack<KURL> backStack;

    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<KURL> forwardStack;

    static KURL *lastDirectory;

    KFileReader *dir;

    KCompletion myCompletion;
    KCompletion myDirCompletion;
    bool myCompleteListDirty;
    QDir::SortSpec mySorting;

    /**
      * takes action on the new location. If it's a directory, change
      * into it, if it's a file, correct the name, etc.
      */
    void checkPath(const QString& txt, bool takeFiles = false);

    void connectView(KFileView *);

    // for the handling of the cursor
    bool finished;

    KFileView *fileView;
    KFileView *oldView;

    KFileViewItemList pendingMimeTypes;

    // the enum KFile::FileView as an int
    int viewKind;
    int defaultView;

    KFile::Mode myMode;
    KProgress *progress;

    const QWidget *myPreview;    // temporary pointer for the preview widget

    // actions for the popupmenus
    // ### clean up all those -- we have them all in the actionMenu!
    KActionMenu *actionMenu;

    KAction 	*backAction;
    KAction 	*forwardAction;
    KAction 	*homeAction;
    KAction 	*upAction;
    KAction 	*reloadAction;
    KActionSeparator *actionSeparator;
    KAction 	*mkdirAction;

    KActionMenu *sortActionMenu;
    KRadioAction *byNameAction;
    KRadioAction *byDateAction;
    KRadioAction *bySizeAction;
    KToggleAction *reverseAction;
    KToggleAction *dirsFirstAction;
    KToggleAction *caseInsensitiveAction;

    KActionMenu *viewActionMenu;
    KRadioAction *shortAction;
    KRadioAction *detailedAction;
    KToggleAction *showHiddenAction;
    KToggleAction *separateDirsAction;

    KActionCollection *myActionCollection;

 public slots:
    void back();
    void forward();
    void home();
    void cdUp();
    void rereadDir();
    void mkdir();
    /**
     * Deletes the currently selected files/directories.
     */
    void deleteSelected();

    QString makeCompletion(const QString&);
    QString makeDirCompletion(const QString&);

  protected slots:
    void resetCursor();
    void readNextMimeType();
    void pathChanged();
    void filterChanged();
    void insertNewFiles(const KFileItemList &newone);
    void itemDeleted(KFileItem *);

    void selectDir(const KFileViewItem*);
    void selectFile(const KFileViewItem*);
    void highlightFile(const KFileViewItem* i) { emit fileHighlighted( i ); }

    /**
     * Called upon right-click to activate the popupmenu.
     */
    virtual void activatedMenu( const KFileViewItem * );

    void sortByName() 		{ byNameAction->setChecked( true ); }
    void sortBySize() 		{ bySizeAction->setChecked( true ); }
    void sortByDate() 		{ byDateAction->setChecked( true ); }
    void sortReversed() 	{ reverseAction->setChecked( !reverseAction->isChecked() ); }
    void toggleDirsFirst() 	{ dirsFirstAction->setChecked( !dirsFirstAction->isChecked() ); }
    void toggleIgnoreCase() 	{ caseInsensitiveAction->setChecked( !caseInsensitiveAction->isChecked() ); }

    void deleteOldView();

    void slotCompletionMatch(const QString&);

private slots:
    void slotDetailedView();
    void slotSimpleView();
    void slotToggleHidden( bool );

    void slotSingleView();
    void slotSeparateDirs();
    void slotDefaultPreview();

    void slotSortByName();
    void slotSortBySize();
    void slotSortByDate();
    void slotSortReversed();
    void slotToggleDirsFirst();
    void slotToggleIgnoreCase();

    void slotStarted();
    void slotProgress( KIO::Job *, unsigned long );
    void slotShowProgress();
    void slotIOFinished();
    void slotRedirected( const KURL& );

    void slotViewActionAdded( KAction * );
    void slotViewActionRemoved( KAction * );
    void slotViewSortingChanged();

signals:
    void urlEntered(const KURL& );
    void updateInformation(int files, int dirs);
    void completion(const QString&);
    void finishedLoading();

    /**
     * Emitted whenever the current fileview is changed, either by an explicit
     * call to @ref setView() or by the user selecting a different view thru
     * the GUI.
     */
    void viewChanged( KFileView * newView );

    void fileHighlighted(const KFileViewItem*);
    void dirActivated(const KFileViewItem*);
    void fileSelected(const KFileViewItem*);

private:
    class KDirOperatorPrivate;
    KDirOperatorPrivate *d;
};

#endif
