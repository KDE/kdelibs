// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Frerich Raabe <raabe@kde.org>

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

#ifndef __KFILEDIALOG_H__
#define __KFILEDIALOG_H__

#include <qstring.h>

#include <kdialogbase.h>

#include <kfile.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kio/jobclasses.h>

class QCheckBox;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QPopupMenu;
class QVBoxLayout;

class KDirOperator;
class KFileBookmark;
class KFileBookmarkManager;
class KURLComboBox;
class KFileFilter;
class KFileView;
class KFileViewItem;
class KToolBar;
class KPreviewWidgetBase;

struct KFileDialogPrivate;

/**
 * Provide a user (and developer) friendly way to
 * select files and directories.
 *
 * The widget can be used as a drop in replacement for the
 * @ref  QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the static methods
 * @ref getOpenFileName(), @ref getSaveFileName(), @ref getOpenURL()
 * or for multiple files @ref getOpenFileNames() or @ref getOpenURLs().
 *
 * The dialog has been designed to allow applications to customise it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 *
 * @image kfiledialog.png KFileDialog
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore <rich@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KFileDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Defines some default behavior of the filedialog.
     * E.g. in mode @p Opening and @p Saving, the selected files/urls will
     * be added to the "recent documents" list. The Saving mode also implies
     * @ref setKeepLocation() being set.
     *
     * @p Other means that no default actions are performed.
     *
     * @see #setOperationMode
     * @see #operationMode
     */
    enum OperationMode { Other = 0, Opening, Saving };

    /**
      * Construct a file dialog.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li @ref QString::null to start in the current working
      *		    directory, or the last directory where a file has been
      *		    selected.
      *         @li ':<keyword>' to start in the directory last used
      *             by a filedialog in the same application that specified
      *             the same keyword.
      *         @li '::<keyword' to start in the direcotry last used
      *             by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
                      which files to display.
      * See @ref setFilter() for details on how to use this argument.
      *
      * @param acceptURLs If set to @p false, @ref KFileDialog will
      * just accept files on the local filesystem.
      */
    KFileDialog(const QString& startDir, const QString& filter,
		QWidget *parent, const char *name,
		bool modal);

    /**
     * Destruct the file dialog.
     */
    ~KFileDialog();

    /**
     * @returns The selected fully qualified filename.
     */
    KURL selectedURL() const;

    /**
     * @returns The list of selected URLs.
     */
    KURL::List selectedURLs() const;

    /**
     * @returns the currently shown directory.
     */
    KURL baseURL() const;

    /**
     * Retrieve the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    QString selectedFile() const;

    /**
     * Retrieve a list of all selected local files.
     */
    QStringList selectedFiles() const;

    /**
     * Set the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Set the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    void setSelection(const QString& name);

    /**
     * Sets the operational mode of the filedialog to @p Saving, @p Opening
     * or @p Other. This will set some flags that are specific to loading
     * or saving files. E.g. @ref setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileDialog::Saving ); sets
     * setKeepLocation for example.
     *
     * The default mode is @p Other.
     *
     * Call this method right after instantiating KFileDialog.
     *
     * @see #operationMode
     * @see KFileDialog::OperationMode
     */
    void setOperationMode( KFileDialog::OperationMode );

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see #operationMode
     * @see KFileDialog::OperationMode
     */    
    OperationMode operationMode() const;

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when @ref operationMode() is KFileDialog::Saving
     *
     * getSaveFileName() and getSaveURL() set this to true by default, so that
     * you can type in the filename and change the directory without having
     * to type the name again.
     */
    void setKeepLocation( bool keep );

    /**
     * @returns whether the contents of the location edit are kept when
     * changing directories.
     */
    bool keepsLocation() const;

    /**
     * Set the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select seperated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * <pre>
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * kfile->setFilter("*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files");
     * </pre>
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     *
     * If the filter contains a '/', a mimetype-filter is assumed. You can
     * specify multiple mimetypes like this (separated with space):
     *
     * <pre>
     * kfile->setFilter( "image/png text/html text/plain" );
     * </pre>
     *
     * @see #filterChanged
     * @see #setMimeFilter
     */
    void setFilter(const QString& filter);

    /**
     * Retrieve the current filter as entered by the user or one of the
     * predefined set via @ref setFilter().
     *
     * @param filter Contains the new filter (only the extension part,
     * not the explanation), for example, "*.cpp" or "*.cpp *.cc".
     *
     * @see setFilter()
     * @see filterChanged()
     */
    QString currentFilter() const;

    /**
     * Set the filter up to specify the output type.
     *
     * @param label the label to use instead of "Filter:"
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format.
     *
     * Do not use in conjunction with @ref setFilter()
     * @deprecated
     */
    void setFilterMimeType(const QString &label, const KMimeType::List &types, const KMimeType::Ptr &defaultType);

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if @ref setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    KMimeType::Ptr currentFilterMimeType();

    /**
     * Set the filter up to specify the output type.
     *
     * @param types a list of mimetypes that can be used as output format
     * @param defaultType the default mimetype to use as output format, if any.
     * If @p defaultType is set, it will be set as the current item.
     * Otherwise, a first item showing all the mimetypes will be created.
     * Typically, @p defaultType should be empty for loading and set for saving.
     *
     * Do not use in conjunction with @ref setFilter()
     */
    void setMimeFilter( const QStringList& types,
                        const QString& defaultType = QString::null );

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if @ref setMimeFilter() has been called
     * previously.
     *
     * @see setMimeFilter()
     */
    QString currentMimeFilter() const;

    /**
     *  Clears any mime- or namefilter. Does not reload the directory.
     */
    void clearFilter();

    /**
     * @deprecated
     * Add a preview widget and enter the preview mode.
     *
     * In this mode
     * the dialog is split and the right part contains your widget.
     * This widget has to inherit @ref QWidget and it has to implement
     * a slot @ref showPreview(const KURL &); which is called
     * every time the file changes. You may want to look at
     * koffice/lib/kofficecore/koFilterManager.cc for some hints :)
     *
     * Ownership is transferred to KFileDialog. You need to create the
     * preview-widget with "new", i.e. on the heap.
     */
    void setPreviewWidget(const QWidget *w);

    /**
     * Add a preview widget and enter the preview mode.
     *
     * In this mode the dialog is split and the right part contains your
     * preview widget.
     *
     * Ownership is transferred to KFileDialog. You need to create the
     * preview-widget with "new", i.e. on the heap.
     *
     * @param w The widget to be used for the preview.
     */
   void setPreviewWidget(const KPreviewWidgetBase *w);

    /**
     * Create a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getOpenFileName(const QString& startDir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);


    /**
     * Create a modal file dialog and return the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QStringList getOpenFileNames(const QString& startDir= QString::null,
					const QString& filter= QString::null,
					QWidget *parent = 0,
					const QString& caption= QString::null);

    /**
     * Create a modal file dialog and return the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getOpenURL(const QString& startDir = QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);


    /**
     * Create a modal file dialog and return the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL::List getOpenURLs(const QString& startDir= QString::null,
				  const QString& filter= QString::null,
				  QWidget *parent = 0,
				  const QString& caption= QString::null);

    /**
     * Create a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName(const QString& startDir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);

    /**
     * Create a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getSaveURL(const QString& startDir= QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);
    /**
     * Create a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing directory.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory(const QString & startDir = QString::null,
					QWidget * parent = 0,
					const QString& caption= QString::null);

    /**
     * Create a modal file dialog with an image previewer and returns the
     * selected url or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li @ref QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':<keyword>' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::<keyword' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getImageOpenURL( const QString& startDir = QString::null,
				 QWidget *parent = 0,
				 const QString& caption = QString::null );
    /**
     * @reimplemented
     **/
    virtual void show();

    /**
     * @deprecated use @ref setMode( unsigned int ) instead
     */
    void setMode( KFile::Mode m );

    /**
     * Set the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * <pre>    enum Mode {
     *                      File         = 1,
     *                      Directory    = 2,
     *                      Files        = 4,
     *                      ExistingOnly = 8,
     *                      LocalOnly    = 16
     *                    };
     * </pre>
     * You can OR the values, e.g.
     * <pre>
     * KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files |
     *                                              KFile::ExistingOnly |
     *                                              KFile::LocalOnly );
     * setMode( mode );
     * </pre>
     */
    void setMode( unsigned int m );

    /**
     * Retrieve the mode of the filedialog.
     * @see #setMode()
     */
    KFile::Mode mode() const;

    /**
     * Set the text to be displayed in front of the
     * selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * Retrieve a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * <pre>
     *      yourAction = new KAction( i18n("Your Action"), 0,
     *                                this, SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( kfileDialog->toolBar() );
     * </pre>
     */
    KToolBar *toolBar() const { return toolbar; }

    /**
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     */
    QPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     */
    QPushButton *cancelButton() const;

signals:
    /**
      * Emitted when the user selects a file. It is only emitted in single-
      * selection mode. The best way to get notified about selected file(s)
      * is to connect to the okClicked() signal inherited from KDialogBase
      * and call @ref selectedFile(), @ref selectedFiles(),
      * @ref selectedURL() or @ref selectedURLs().
      */
    void fileSelected(const QString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const QString&);

    /**
     * Emitted when the user hilights one or more files in multiselection mode.
     *
     * Note: @ref fileHighlighted() or @ref fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     * @see #fileSelected
     */
    void selectionChanged();

    /**
     * Emitted when the filter changed, i.e. the user entered an own filter
     * or chose one of the predefined set via @ref setFilter().
     *
     * @param filter contains the new filter (only the extension part,
     * not the explanation), i.e. "*.cpp" or "*.cpp *.cc".
     *
     * @see setFilter()
     * @see currentFilter()
     */
    void filterChanged( const QString& filter );

protected:
    KToolBar *toolbar;

    static KURL *lastDirectory;

    QPopupMenu *bookmarksMenu;
    KURLComboBox *locationEdit;

    KFileFilter *filterWidget;

    KFileBookmarkManager *bookmarks;
    QStringList history; // FIXME: remove it, this is not used at all

    /**
     * adds a entry of the current directory. If disableUpdating is set
     * to true, it will care about clever updating
     **/
    void addDirEntry(KFileViewItem *entry, bool disableUpdating);

    /**
      * rebuild geometry managment.
      *
      */
    virtual void initGUI();

    /**
     * called when an item is highlighted/selected in multiselection mode.
     * handles setting the locationEdit.
     */
    void multiSelectionChanged();

    /**
     * Reads configuration and applies it (size, recent directories, ...)
     */
    virtual void readConfig( KConfig *, const QString& group = QString::null );

    /**
     * Saves the current configuration
     */
    virtual void saveConfig( KConfig *, const QString& group = QString::null );

    /**
     * Reads the recent used files and inserts them into the location combobox
     */
    virtual void readRecentFiles( KConfig * );

    /**
     * Saves the entries from the location combobox.
     */
    virtual void saveRecentFiles( KConfig * );

    /**
     * Parses the string "line" for files. If line doesn't contain any ", the
     * whole line will be interpreted as one file. If the number of " is odd,
     * an empty list will be returned. Otherwise, all items enclosed in " "
     * will be returned as correct urls.
     */
    KURL::List tokenize(const QString& line) const;

protected slots:
    void urlEntered( const KURL& );
    void pathComboActivated( const KURL& url );
    void pathComboReturnPressed( const QString& url );
    void locationActivated( const QString& url );
    void toolbarCallback(int);
    void toolbarPressedCallback(int); // ### remove
    void slotFilterChanged();
    void pathComboChanged( const QString& );
    void fileHighlighted(const KFileViewItem *i);
    void fileSelected(const KFileViewItem *i);
    void slotStatResult(KIO::Job* job);
    void slotLoadingFinished();

    void dirCompletion( const QString& );
    void fileCompletion( const QString& );

    virtual void updateStatusLine(int dirs, int files);
    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();


    /**
      * Add the current location to the global bookmarks list
      */
    void addToBookmarks();
    void bookmarksChanged();
    void fillBookmarkMenu( KFileBookmark *parent, QPopupMenu *menu, int &id );
    void addToRecentDocuments();

private slots:
    void buildBookmarkPopup();
    void bookmarkMenuActivated( int choice );

private:
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);

protected:
    KFileDialogPrivate *d;
    KDirOperator *ops;
    bool autoDirectoryFollowing;

    KURL::List& parseSelectedURLs() const;

};

#endif
