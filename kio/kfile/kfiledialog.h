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

class KActionCollection;
class KDirOperator;
class KURLComboBox;
class KFileFilterCombo;
class KFileView;
class KFileItem;
class KPushButton;
class KToolBar;
class KPreviewWidgetBase;

struct KFileDialogPrivate;

/**
 * Provides a user (and developer) friendly way to
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
 * \image html kfiledialog.png "KDE File Dialog"
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
      * Constructs a file dialog.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li QString::null to start in the current working
      *		    directory, or the last directory where a file has been
      *		    selected.
      *         @li ':&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in the same application that specified
      *             the same keyword.
      *         @li '::&lt;keyword&gt;' to start in the direcotry last used
      *             by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
                      which files to display.
      * See @ref setFilter() for details on how to use this argument.
      *
      * @param acceptURLs If set to @p false, the dialog will
      * just accept files on the local filesystem.
      */
    KFileDialog(const QString& startDir, const QString& filter,
		QWidget *parent, const char *name,
		bool modal);

    /**
      * Constructs a file dialog.
      *
      * The parameters here are identical to the first constructor except
      * for the addition of a QWidget parameter.
      *
      * Historical note: The original version of KFileDialog did not have this extra
      * parameter. It was added later, and, in order to maintain binary compatibility,
      * it was placed in a new constructor instead of added to the original one.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li QString::null to start in the current working
      *		    directory, or the last directory where a file has been
      *		    selected.
      *         @li ':&lt;keyword&gt;' to start in the directory last used
      *             by a filedialog in the same application that specified
      *             the same keyword.
      *         @li '::&lt;keyword&gt;' to start in the direcotry last used
      *             by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
      *               which files to display.
      * See @ref setFilter() for details on how to use this argument.
      *
      * @param acceptURLs If set to @p false, the dialog will
      * just accept files on the local filesystem.
      *
      * @param widget A widget, or a widget of widgets, for displaying custom
      *               data in the dialog. This can be used, for example, to
      *               display a check box with the caption "Open as read-only".
      *               When creating this widget, you don't need to specify a parent,
      *               since the widget's parent will be set automatically by KFileDialog.
      * @since 3.1
      */
    KFileDialog(const QString& startDir, const QString& filter,
		QWidget *parent, const char *name,
		bool modal, QWidget* widget);

    /**
     * Destructs the file dialog.
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
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    QString selectedFile() const;

    /**
     * Returns a list of all selected local files.
     */
    QStringList selectedFiles() const;

    /**
     * Sets the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setURL(const KURL &url, bool clearforward = true);

    /**
     * Sets the file name to preselect to @p name
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
     * The mode @p Saving, together with a default filter set via
     * @ref setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is @p Opening.
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
     * Sets the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select seperated by '\n'. Every
     * filter entry is defined through namefilter|text to diplay.
     * If no | is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * <code>
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * kfile->setFilter("*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files");
     * </code>
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     *
     * If the filter contains an unescaped '/', a mimetype-filter is assumed.
     * If you would like a '/' visible in your filter it can be escaped with
     * a '\'. You can specify multiple mimetypes like this (separated with
     * space):
     *
     * <code>
     * kfile->setFilter( "image/png text/html text/plain" );
     * kfile->setFilter( "*.cue|CUE\\/BIN Files (*.cue)" );
     * </code>
     *
     * @see #filterChanged
     * @see #setMimeFilter
     */
    void setFilter(const QString& filter);

    /**
     * Returns the current filter as entered by the user or one of the
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
     * Sets the filter up to specify the output type.
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
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if @ref setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    KMimeType::Ptr currentFilterMimeType();

    /**
     * Sets the filter up to specify the output type.
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
     * Adds a preview widget and enters the preview mode.
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
     * Creates a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getOpenFileName(const QString& startDir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);


    /**
     * Creates a modal file dialog and returns the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QStringList getOpenFileNames(const QString& startDir= QString::null,
					const QString& filter= QString::null,
					QWidget *parent = 0,
					const QString& caption= QString::null);

    /**
     * Creates a modal file dialog and returns the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getOpenURL(const QString& startDir = QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);


    /**
     * Creates a modal file dialog and returns the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL::List getOpenURLs(const QString& startDir= QString::null,
				  const QString& filter= QString::null,
				  QWidget *parent = 0,
				  const QString& caption= QString::null);

    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName(const QString& startDir= QString::null,
				   const QString& filter= QString::null,
				   QWidget *parent= 0,
				   const QString& caption = QString::null);

    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li a relative path or a filename determining the
     *             directory to start in and the file to be selected.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param filter This is a space seperated list of shell globs.
     * You can set the text to be displayed for the glob, and
     * provide multiple globs.  See @ref setFilter() for details on
     * how to do this...
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KURL getSaveURL(const QString& startDir= QString::null,
			   const QString& filter= QString::null,
			   QWidget *parent= 0,
			   const QString& caption = QString::null);
    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getExistingDirectory(const QString & startDir = QString::null,
					QWidget * parent = 0,
					const QString& caption= QString::null);

    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     *
     * Contrary to @ref getExistingDirectory(), this method allows the
     * selection of a remote directory.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
     *             by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     * @since 3.1
     */
    static KURL getExistingURL(const QString & startDir = QString::null,
                                  QWidget * parent = 0,
                                  const QString& caption= QString::null);
    /**
     * Creates a modal file dialog with an image previewer and returns the
     * selected url or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li QString::null to start in the current working
     *		    directory, or the last directory where a file has been
     *		    selected.
     *         @li ':&lt;keyword&gt;' to start in the directory last used
     *             by a filedialog in the same application that specified
     *             the same keyword.
     *         @li '::&lt;keyword&gt;' to start in the direcotry last used
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
     * Sets the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * <code>
     *    enum Mode {
     *         File         = 1,
     *         Directory    = 2,
     *         Files        = 4,
     *         ExistingOnly = 8,
     *         LocalOnly    = 16
     *    };
     * </code>
     * You can OR the values, e.g.
     * <code>
     * KFile::Mode mode = static_cast<KFile::Mode>( KFile::Files |
     *                                              KFile::ExistingOnly |
     *                                              KFile::LocalOnly );
     * setMode( mode );
     * </code>
     */
    void setMode( unsigned int m );

    /**
     * Returns the mode of the filedialog.
     * @see #setMode()
     */
    KFile::Mode mode() const;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * Returns a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * <code>
     *      yourAction = new KAction( i18n("Your Action"), 0,
     *                                this, SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( kfileDialog->toolBar() );
     * </code>
     */
    KToolBar *toolBar() const { return toolbar; }

    /**
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     */
    KPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     */
    KPushButton *cancelButton() const;

    /**
     * @returns a pointer to the action collection, holding all the used
     * KActions.
     */
    KActionCollection *actionCollection() const;

    /**
     * @returns the index of the path combobox so when inserting widgets into
     * the dialog (e.g. subclasses) they can do so without hardcoding in an index
     */
    int pathComboIndex();

    /**
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents direcory, home directory or a recently 
     * used directory.
     * @param startDir A url/directory, to be used. May use the ':' and '::' syntax 
     *        as documented in the @ref KFileDialog() constructor.
     * @param recentDirClass If the ':' or '::' syntax is used, recentDirClass
     *        will contain the string to be used later for @ref KRecentDir::dir()
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     * @since 3.1
     */
    static KURL getStartURL( const QString& startDir, QString& recentDirClass );


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

    KURLComboBox *locationEdit;

    KFileFilterCombo *filterWidget;

    /**
     * adds a entry of the current directory. If disableUpdating is set
     * to true, it will care about clever updating
     **/
    void addDirEntry(KFileItem *entry, bool disableUpdating);

    /**
      * Perform basic initialization tasks. Called by constructors.
      * @since 3.1
      */
    void init(const QString& startDir, const QString& filter, QWidget* widget);

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
    virtual void writeConfig( KConfig *, const QString& group = QString::null );

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
    void enterURL( const KURL& url );
    void enterURL( const QString& url );
    void locationActivated( const QString& url );

    /**
     * @deprecated
     */
    void toolbarCallback(int);

    void slotFilterChanged();
    void pathComboChanged( const QString& );
    void fileHighlighted(const KFileItem *i);
    void fileSelected(const KFileItem *i);
    void slotStatResult(KIO::Job* job);
    void slotLoadingFinished();

    void dirCompletion( const QString& );
    void fileCompletion( const QString& );
    /**
     * @since 3.1
     */
    void toggleSpeedbar( bool );

    /**
     * @deprecated
     */
    virtual void updateStatusLine(int dirs, int files);

    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

    void addToRecentDocuments();
    void initSpeedbar();

private:
    KFileDialog(const KFileDialog&);
    KFileDialog operator=(const KFileDialog&);

protected:
    KDirOperator *ops;
    bool autoDirectoryFollowing;

    KURL::List& parseSelectedURLs() const;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    KFileDialogPrivate *d;
};

#endif
