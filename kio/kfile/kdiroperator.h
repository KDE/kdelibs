// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
		  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <qptrstack.h>

#include <kaction.h>
#include <kcompletion.h>

#include <kdirlister.h>

#include <kfileview.h>
#include <kfileitem.h>
#include <kfile.h>

class QPopupMenu;
class QTimer;

class KAction;
class KDirLister;
class KToggleAction;
class KActionSeparator;
class KActionMenu;
class QWidgetStack;
class KProgress;
namespace KIO {
  class DeleteJob;
}

/**
 * This widget works as a network transparent filebrowser. You specify a URL
 * to display and this url will be loaded via @ref KDirLister. The user can
 * browse through directories, highlight and select files, delete or rename
 * files.
 *
 * It supports different views, e.g. a detailed view (see @ref KFileDetailView),
 * a simple icon view (see @ref KFileIconView), a combination of two views,
 * separating directories and files (@ref KCombiView).
 *
 * Additionally, a preview view is available (see @ref KFilePreview), which can
 * show either a simple or detailed view and additionally a preview widget
 * (see @ref setPreviewWidget()). @ref KImageFilePreview is one implementation
 * of a preview widget, that displays previews for all supported filetypes
 * utilizing KIO::PreviewJob.
 *
 * Currently, those classes don't support Drag&Drop out of the box -- there
 * you have to use your own view-classes. You can use some DnD-aware views
 * from Björn Sahlström <bjorn@kbear.org> until they will be integrated
 * into this library. See http://devel-home.kde.org/~pfeiffer/DnD-classes.tar.gz
 *
 * This widget is the one used in the KFileDialog.
 *
 * Basic usage is like this:
 * <pre>
 *   KDirOperator *op = new KDirOperator( KURL( "file:/home/gis" ), this );
 *   // some signals you might be interested in
 *   connect(op, SIGNAL(updateInformation(int, int)),
 *           SLOT(updateStatusLine(int, int)));
 *   connect(op, SIGNAL(urlEntered(const KURL&)),
 *           SLOT(urlEntered(const KURL&)));
 *   connect(op, SIGNAL(fileHighlighted(const KFileItem *)),
 *           SLOT(fileHighlighted(const KFileItem *)));
 *   connect(op, SIGNAL(fileSelected(const KFileItem *)),
 *           SLOT(fileSelected(const KFileItem *)));
 *   connect(op, SIGNAL(finishedLoading()),
 *           SLOT(slotLoadingFinished()));
 *
 *   op->readConfig( KGlobal::config(), "Your KDiroperator ConfigGroup" );
 *   op->setView(KFile::Default);
 * </pre>
 *
 * This will create a childwidget of 'this' showing the directory contents
 * of /home/gis in the default-view. The view is determined by the readConfig()
 * call, which will read the KDirOperator settings, the user left your program
 * with (and which you saved with op->writeConfig()).
 *
 * @short A widget for displaying files and browsing directories.
 * @author Stephan Kulow <coolo@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KDirOperator : public QWidget
{
    Q_OBJECT

 public:
    /**
     * Constructs the KDirOperator with no initial view. As the views are
     * configurable, call @ref readConfig() to load the user's configuration
     * and then @ref setView to explicitly set a view.
     *
     * This constructor doesn't start loading the url, @ref setView will do it.
     */
    KDirOperator(const KURL& urlName = KURL(),
		 QWidget *parent = 0, const char* name = 0);
    /**
     * Destroys the KDirOperator.
     */
    virtual ~KDirOperator();

    /**
     * Enables/disables showing hidden files.
     */
    void setShowHiddenFiles ( bool s ) { showHiddenAction->setChecked( s ); }

    /**
     * @returns true when hidden files are shown or false otherwise.
     */
    bool showHiddenFiles () const { return showHiddenAction->isChecked(); }

    /**
     * Stops loading immediately. You don't need to call this, usually.
     */
    void close();

    /**
     * Sets a filter like "*.cpp *.h *.o". Only files matching that filter
     * will be shown. Call @ref updateDir() to apply it.
     *
     * @see KDirLister::setNameFilter
     * @see #nameFilter
     */
    void setNameFilter(const QString& filter);

    /**
     * @returns the current namefilter.
     * @see #setNameFilter
     */
    const QString& nameFilter() const { return dir->nameFilter(); }

    /**
     * Sets a list of mimetypes as filter. Only files of those mimetypes
     * will be shown.
     *
     * Example:
     * <pre>
     * QStringList filter;
     * filter << "text/html" << "image/png" << "inode/directory";
     * dirOperator->setMimefilter( filter );
     * </pre>
     *
     * Node: Without the mimetype inode/directory, only files would be shown.
     * Call @ref updateDir() to apply it.
     *
     * @see KDirLister::setMimeFilter
     * @see #mimeFilter
     */
    void setMimeFilter( const QStringList& mimetypes );

    /**
     * @returns the current mime filter.
     */
    QStringList mimeFilter() const { return dir->mimeFilters(); }

    /**
     * Clears both the namefilter and mimetype filter, so that all files and
     * directories will be shown. Call @ref updateDir() to apply it.
     *
     * @see #setMimeFilter
     * @see #setNameFilter
     */
    void clearFilter();

    /**
     * @returns the current url
     */
    KURL url() const;

    /**
     * Sets a new url to list.
     * @param clearforward specifies whether the "forward" history should be
     * cleared.
     */
    void setURL(const KURL& url, bool clearforward);

    /**
     * Clears the current selection and attempts to set @p filename
     * the current file. filename is just the name, no path or url.
     */
    void setCurrentItem( const QString& filename );

    /**
     * Sets a new KFileView to be used for showing and browsing files.
     * Note: this will read the current @ref url() to fill the view.
     *
     * @see KFileView
     * @see KFileIconView
     * @see KFileDetailView
     * @see KFileCombiView
     * @see #view
     */
    void setView(KFileView *view);

    /**
     * @returns the currently used view.
     * @see #setView
     */
    KFileView * view() const { return m_fileView; }

    /**
     * Returns the widget of the current view. 0L if there is no view/widget.
     * (KFileView itself is not a widget.)
     */
    QWidget * viewWidget() const { return m_fileView ? m_fileView->widget() : 0L; }

    /**
     * Sets one of the predefined fileviews
     * @see KFile::FileView
     */
    void setView(KFile::FileView view);

    /**
     * Sets the way to sort files and directories.
     */
    void setSorting( QDir::SortSpec );

    /**
     * @returns the current way of sorting files and directories
     */
    QDir::SortSpec sorting() const { return mySorting; }

    /**
     * @returns true if we are displaying the root directory of the current url
     */
    bool isRoot() const { return url().path() == QChar('/'); }

    /**
     * @returns the object listing the directory
     */
    KDirLister *dirLister() const { return dir; }

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
     * You cannot mix File and Files of course, as the former means
     * single-selection mode, the latter multi-selection.
     */
    void setMode( KFile::Mode m );
    /**
     * @returns the listing/selection mode.
     */
    KFile::Mode mode() const;

    /**
     * Sets a preview-widget to be shown next to the file-view.
     * The ownership of @p w is transferred to KDirOperator, so don't
     * delete it yourself!
     */
    void setPreviewWidget(const QWidget *w);

    /**
     * @returns a list of all currently selected items. If there is no view,
     * then 0L is returned.
     */
    const KFileItemList * selectedItems() const {
	return ( m_fileView ? m_fileView->selectedItems() : 0L );
    }

    /**
     * @returns true if @p item is currently selected, or false otherwise.
     */
    inline bool isSelected( const KFileItem *item ) const {
	return ( m_fileView ? m_fileView->isSelected( item ) : false );
    }

    /**
     * @returns the number of directories in the currently listed url.
     * Returns 0 if there is no view.
     */
    int numDirs() const;

    /**
     * @returns the number of files in the currently listed url.
     * Returns 0 if there is no view.
     */
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
     * @li properties : shows a KPropertiesDialog for the selected files
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
     * <pre>
     * setView( KFile::Default )
     * </pre>
     * to apply it.
     *
     * @see setView
     * @see #writeConfig
     */
    virtual void readConfig( KConfig *, const QString& group = QString::null );

    /**
     * Saves the current settings like sorting, simple or detailed view.
     *
     * @see #readConfig
     */
    virtual void writeConfig( KConfig *, const QString& group = QString::null );


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

    /**
     * Starts and returns a @ref KIO::DeleteJob to delete the given @p items.
     *
     * @param ask specifies whether a confirmation dialog should be shown
     * @param showProgress passed to the DeleteJob to show a progress dialog
     */
    KIO::DeleteJob * del( const KFileItemList& items,
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

    /**
     * @returns true if we are in directory-only mode, that is, no files are
     * shown.
     */
    bool dirOnlyMode() const { return dirOnlyMode( myMode ); }

    static bool dirOnlyMode( uint mode ) {
        return ( (mode & KFile::Directory) &&
                 (mode & (KFile::File | KFile::Files)) == 0 );
    }

protected:
    /**
     * A view factory for creating predefined fileviews. Called internally by @ref #setView
	 * , but you can also call it directly. Reimplement this if you depend on self defined fileviews.
	 * @param parent is the  @ref QWidget to be set as parent
	 * @param view is the predefined view to be set, note: this can be several ones OR:ed together.
	 * @returns the created @ref KFileView
     * @see KFileView
     * @see KCombiView
     * @see KFileDetailView
     * @see KFileIconView
     * @see KFilePreview
     * @see KFile::FileView
     * @see #setView
     */
     virtual KFileView* createView( QWidget* parent, KFile::FileView view );
     /**
     * Sets a custom KFileReader to list directories.
     */
    void setDirLister( KDirLister *lister );

    /**
     * @reimplemented
     */
    void resizeEvent( QResizeEvent * );

    /**
     * Sets up all the actions. Called from the constructor, you usually
     * better not call this.
     */
    void setupActions();

    /**
     * Updates the sorting-related actions to comply with the current sorting
     * @see #sorting
     */
    void updateSortActions();

    /**
     * Updates the view-related actions to comply with the current
     * KFile::FileView
     */
    void updateViewActions();

    /**
     * Sets up the context-menu with all the necessary actions. Called from the
     * constructor, you usually don't need to call this.
     */
    void setupMenu();

    /**
     * Synchronizes the completion objects with the entries of the
     * currently listed url.
     *
     * Automatically called from @ref makeCompletion() and
     * @ref makeDirCompletion()
     */
    void prepareCompletionObjects();

    /**
     * Checks if there support from @ref KIO::PreviewJob for the currently
     * shown files, taking @ref mimeFilter() and @ref nameFilter() into account
     * Enables/disables the preview-action accordingly.
     */
    bool checkPreviewSupport();

public slots:
    /**
     * Goes one step back in the history and opens that url.
     */
    void back();

    /**
     * Goes one step forward in the history and opens that url.
     */
    void forward();

    /**
     * Enters the home directory.
     */
    void home();

    /**
     * Goes one directory up from the current url.
     */
    void cdUp();

    /**
     * to update the view after changing the settings
     */
    void updateDir();

    /**
     * Re-reads the current url.
     */
    void rereadDir();

    /**
     * Opens a dialog to create a new directory.
     */
    void mkdir();

    /**
     * Deletes the currently selected files/directories.
     */
    void deleteSelected();

    /**
     * Enables/disables actions that are selection dependent. Call this e.g.
     * when you are about to show a popup menu using some of KDirOperators
     * actions.
     */
    void updateSelectionDependentActions();

    /**
     * Tries to complete the given string (only completes files).
     */
    QString makeCompletion(const QString&);

    /**
     * Tries to complete the given string (only completes directores).
     */
    QString makeDirCompletion(const QString&);

protected slots:
    /**
     * Restores the normal cursor after showing the busy-cursor. Also hides
     * the progressbar.
     */
    void resetCursor();

    /**
     * Called after @ref setURL() to load the directory, update the history,
     * etc.
     */
    void pathChanged();

    /**
     * Adds a new list of KFileItems to the view
     * (coming from KDirLister)
     */
    void insertNewFiles(const KFileItemList &newone);

    /**
     * Removes the given KFileItem item from the view (usually called from
     * KDirLister).
     */
    void itemDeleted(KFileItem *);

    /**
     * Enters the directory specified by the given @p item.
     */
    void selectDir(const KFileItem *item );

    /**
     * Emits fileSelected( item )
     */
    void selectFile(const KFileItem *item);

    /**
     * Emits fileHighlighted( i )
     */
    void highlightFile(const KFileItem* i) { emit fileHighlighted( i ); }

    /**
     * Called upon right-click to activate the popupmenu.
     */
    virtual void activatedMenu( const KFileItem *, const QPoint& pos );

    /**
     * Changes sorting to sort by name
     */
    void sortByName() 		{ byNameAction->setChecked( true ); }

    /**
     * Changes sorting to sort by size
     */
    void sortBySize() 		{ bySizeAction->setChecked( true ); }

    /**
     * Changes sorting to sort by date
     */
    void sortByDate() 		{ byDateAction->setChecked( true ); }

    /**
     * Changes sorting to reverse sorting
     */
    void sortReversed() 	{ reverseAction->setChecked( !reverseAction->isChecked() ); }

    /**
     * Toggles showing directories first / having them sorted like files.
     */
    void toggleDirsFirst() 	{ dirsFirstAction->setChecked( !dirsFirstAction->isChecked() ); }

    /**
     * Toggles case sensitive / case insensitive sorting
     */
    void toggleIgnoreCase() 	{ caseInsensitiveAction->setChecked( !caseInsensitiveAction->isChecked() ); }

    /**
     * Tries to make the given @p match as current item in the view and emits
     * completion( match )
     */
    void slotCompletionMatch(const QString& match);

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

    void fileHighlighted(const KFileItem*);
    void dirActivated(const KFileItem*);
    void fileSelected(const KFileItem*);

private:
    /**
     * Contains all URLs you can reach with the back button.
     */
    QPtrStack<KURL> backStack;

    /**
     * Contains all URLs you can reach with the forward button.
     */
    QPtrStack<KURL> forwardStack;

    KDirLister *dir;
    KURL currUrl;

    KCompletion myCompletion;
    KCompletion myDirCompletion;
    bool myCompleteListDirty;
    QDir::SortSpec mySorting;

    /**
     * Checks whether we preview support is available for the current
     * mimetype/namefilter
     */
    bool checkPreviewInternal() const;

    /**
      * takes action on the new location. If it's a directory, change
      * into it, if it's a file, correct the name, etc.
      */
    void checkPath(const QString& txt, bool takeFiles = false);

    void connectView(KFileView *);

    KFileView *m_fileView;
    KFileItemList pendingMimeTypes;

    // the enum KFile::FileView as an int
    int m_viewKind;
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
    KActionCollection *viewActionCollection;

private slots:
    /**
     * @internal
     */
    void slotDetailedView();
    void slotSimpleView();
    void slotToggleHidden( bool );

    void slotSingleView();
    void slotSeparateDirs();
    void slotDefaultPreview();
    void togglePreview( bool );

    void slotSortByName();
    void slotSortBySize();
    void slotSortByDate();
    void slotSortReversed();
    void slotToggleDirsFirst();
    void slotToggleIgnoreCase();

    void slotStarted();
    void slotProgress( int );
    void slotShowProgress();
    void slotIOFinished();
    void slotCanceled();
    void slotRedirected( const KURL& );

    void slotViewActionAdded( KAction * );
    void slotViewActionRemoved( KAction * );
    void slotViewSortingChanged( QDir::SortSpec );

    void slotClearView();
    void slotRefreshItems( const KFileItemList& items );

    void slotProperties();

    static bool isReadable( const KURL& url );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDirOperatorPrivate;
    KDirOperatorPrivate *d;
};

#endif
