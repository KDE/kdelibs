// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Frerich Raabe <raabe@kde.org>
                  2007 David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFILEDIALOG_H
#define KFILEDIALOG_H

#include <kdialog.h>
#include <kfile.h>
#include <kurl.h>
#include <kmimetype.h>

class KAbstractFileWidget;
class KFileWidget;

class KActionCollection;
class KUrlComboBox;
class KFileFilterCombo;
class KPushButton;
class KToolBar;
class KPreviewWidgetBase;

class KFileDialogPrivate;

/**
 * Provides a user (and developer) friendly way to
 * select files and directories.
 *
 * The widget can be used as a drop in replacement for the
 * QFileDialog widget, but has greater functionality and a nicer GUI.
 *
 * You will usually want to use one of the static methods
 * getOpenFileName(), getSaveFileName(), getOpenUrl()
 * or for multiple files getOpenFileNames() or getOpenUrls().
 *
 * The dialog has been designed to allow applications to customize it
 * by subclassing. It uses geometry management to ensure that subclasses
 * can easily add children that will be incorporated into the layout.
 *
 * \image html kfiledialog.png "KDE File Dialog"
 *
 * @short A file selection dialog.
 *
 * @author Richard J. Moore <rich@kde.org>, Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KIO_EXPORT KFileDialog : public KDialog
{
    Q_OBJECT

public:

    /**
     * Defines some default behavior of the filedialog.
     * E.g. in mode @p Opening and @p Saving, the selected files/urls will
     * be added to the "recent documents" list. The Saving mode also implies
     * setKeepLocation() being set.
     *
     * @p Other means that no default actions are performed.
     *
     * @see setOperationMode
     * @see operationMode
     */
    enum OperationMode { Other = 0, Opening, Saving };

    /**
      * Constructs a file dialog.
      *
      * @param startDir This can either be
      *         @li The URL of the directory to start in.
      *         @li A KUrl() to start in the current working
      *             directory, or the last directory where a file has been
      *             selected.
      *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
      *             directory last used by a filedialog in the same application that specified
      *             the same keyword.
      *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
      *             in the directory last used by a filedialog in any application that specified the
      *             same keyword.
      *
      * @param filter A shell glob or a mime-type-filter that specifies
      *               which files to display.
      * See setFilter() and setMimeFilter() for details on how to use this argument.
      *
      * @param parent The parent widget of this dialog
      *
      * @param widget A widget, or a widget of widgets, for displaying custom
      *               data in the dialog. This can be used, for example, to
      *               display a check box with the caption "Open as read-only".
      *               When creating this widget, you don't need to specify a parent,
      *               since the widget's parent will be set automatically by KFileDialog.
      */
    KFileDialog( const KUrl& startDir, const QString& filter,
                 QWidget *parent, QWidget* widget = 0 );


    /**
     * Destructs the file dialog.
     */
    ~KFileDialog();

    /**
     * @returns The selected fully qualified filename.
     */
    KUrl selectedUrl() const;

    /**
     * @returns The list of selected URLs.
     */
    KUrl::List selectedUrls() const;

    /**
     * @returns the currently shown directory.
     */
    KUrl baseUrl() const;

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
    void setUrl(const KUrl &url, bool clearforward = true);

    /**
     * Sets the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    void setSelection(const QString& name);

    /**
     * Sets the operational mode of the filedialog to @p Saving, @p Opening
     * or @p Other. This will set some flags that are specific to loading
     * or saving files. E.g. setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileDialog::Saving ); sets
     * setKeepLocation for example.
     *
     * The mode @p Saving, together with a default filter set via
     * setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is @p Opening.
     *
     * Call this method right after instantiating KFileDialog.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    void setOperationMode( KFileDialog::OperationMode );

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    OperationMode operationMode() const;

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when operationMode() is KFileDialog::Saving
     *
     * getSaveFileName() and getSaveUrl() set this to true by default, so that
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
     * filters for the user to select separated by @c '\\n'. Every
     * filter entry is defined through @c namefilter|text to display.
     * If no @c '|' is found in the expression, just the namefilter is
     * shown. Examples:
     *
     * \code
     * kfile->setFilter("*.cpp|C++ Source Files\n*.h|Header files");
     * kfile->setFilter("*.cpp");
     * kfile->setFilter("*.cpp|Sources (*.cpp)");
     * kfile->setFilter("*.cpp|" + i18n("Sources (*.cpp)"));
     * kfile->setFilter("*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files");
     * \endcode
     *
     * Note: The text to display is not parsed in any way. So, if you
     * want to show the suffix to select by a specific filter, you must
     * repeat it.
     *
     * If the filter contains an unescaped @c '/', a mimetype-filter is assumed.
     * If you would like a '/' visible in your filter it can be escaped with
     * a @c '\'. You can specify multiple mimetypes like this (separated with
     * space):
     *
     * \code
     * kfile->setFilter( "image/png text/html text/plain" );
     * kfile->setFilter( "*.cue|CUE\\/BIN Files (*.cue)" );
     * \endcode
     *
     * @see filterChanged
     * @see setMimeFilter
     */
    void setFilter(const QString& filter);

    /**
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilter().
     *
     * @see setFilter()
     * @see filterChanged()
     */
    QString currentFilter() const;

    /**
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
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
     * Do not use in conjunction with setFilter()
     */
    void setMimeFilter( const QStringList& types,
                        const QString& defaultType = QString() );

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
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
    void setPreviewWidget(KPreviewWidgetBase *w);

    /**
     * Forces the inline previews to be shown or hidden, depending on @p show.
     *
     * @param show Whether to show inline previews or not.
     * @since 4.2
     */
    void setInlinePreviewShown(bool show);

    /**
     * Sets whether the dialog should ask before accepting the selected file
     * when KFileDialog::OperationMode is set to Saving.
     *
     * In this case a KMessageBox appears for confirmation.
     *
     * @param enable Set this to true to enable checking.
     * @since 4.2
     */
    void setConfirmOverwrite(bool enable);

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

    /**
     * Creates a modal file dialog and return the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getOpenFileName( const KUrl& startDir= KUrl(),
                                    const QString& filter= QString(),
                                    QWidget *parent= 0,
                                    const QString& caption = QString() );


   /**
     * Use this version only if you have no QWidget available as
     * parent widget. This can be the case if the parent widget is
     * a widget in another process or if the parent widget is a
     * non-Qt widget. For example, in a GTK program.
    */
   static QString getOpenFileNameWId( const KUrl& startDir,
                                      const QString& filter,
                                      WId parent_id, const QString& caption );

    /**
     * Creates a modal file dialog and returns the selected
     * filenames or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QStringList getOpenFileNames( const KUrl& startDir= KUrl(),
                                         const QString& filter = QString(),
                                         QWidget *parent = 0,
                                         const QString& caption= QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * URL or an empty string if none was chosen.
     *
     * Note that with
     * this method the user must select an existing URL.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getOpenUrl( const KUrl& startDir = KUrl(),
                            const QString& filter = QString(),
                            QWidget *parent= 0,
                            const QString& caption = QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * URLs or an empty list if none was chosen.
     *
     * Note that with
     * this method the user must select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl::List getOpenUrls( const KUrl& startDir = KUrl(),
                                   const QString& filter = QString(),
                                   QWidget *parent = 0,
                                   const QString& caption = QString() );



    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static QString getSaveFileName( const KUrl& startDir = KUrl(),
                                    const QString& filter = QString(),
                                    QWidget *parent = 0,
                                    const QString& caption = QString() );


    /**
     * This function accepts the window id of the parent window, instead
     * of QWidget*. It should be used only when necessary.
     */
    static QString getSaveFileNameWId( const KUrl &startDir, const QString& filter,
                                       WId parent_id,
                                       const QString& caption );

    /**
     * Creates a modal file dialog and returns the selected
     * filename or an empty string if none was chosen.
     *
     * Note that with this
     * method the user need not select an existing filename.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param filter A shell glob or a mime-type-filter that specifies which files to display.
     *    The preferred option is to set a list of mimetype names, see setMimeFilter() for details.
     *    Otherwise you can set the text to be displayed for the each glob, and
     *    provide multiple globs, see setFilter() for details.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getSaveUrl( const KUrl& startDir = KUrl(),
                            const QString& filter = QString(),
                            QWidget *parent = 0,
                            const QString& caption = QString() );


    /**
     * Creates a modal directory-selection dialog and returns the selected
     * directory (local only) or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     * @return the path to an existing local directory.
     */
    static QString getExistingDirectory( const KUrl& startDir = KUrl(),
                                         QWidget * parent = 0,
                                         const QString& caption= QString() );

    /**
     * Creates a modal directory-selection dialog and returns the selected
     * directory or an empty string if none was chosen.
     * This version supports remote urls.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     * @return the url to an existing directory (local or remote).
     */
    static KUrl getExistingDirectoryUrl( const KUrl& startDir = KUrl(),
                                         QWidget * parent = 0,
                                         const QString& caption= QString() );

    /**
     * Creates a modal file dialog with an image previewer and returns the
     * selected url or an empty string if none was chosen.
     *
     * @param startDir This can either be
     *         @li The URL of the directory to start in.
     *         @li A KUrl() to start in the current working
     *             directory, or the last directory where a file has been
     *             selected.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;' to start in the
     *             directory last used by a filedialog in the same application that specified
     *             the same keyword.
     *         @li An URL starting with 'kfiledialog:///&lt;keyword&gt;?global' to start
     *             in the directory last used by a filedialog in any application that specified the
     *             same keyword.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The name of the dialog widget.
     */
    static KUrl getImageOpenUrl( const KUrl& startDir = KUrl(),
                                 QWidget *parent = 0,
                                 const QString& caption = QString() );

    /**
     * Sets the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * \code
     *    enum Mode {
     *         File         = 1,
     *         Directory    = 2,
     *         Files        = 4,
     *         ExistingOnly = 8,
     *         LocalOnly    = 16
     *    };
     * \endcode
     * You can OR the values, e.g.
     * \code
     * KFile::Modes mode = KFile::Files |
     *                     KFile::ExistingOnly |
     *                     KFile::LocalOnly );
     * setMode( mode );
     * \endcode
     */
    void setMode( KFile::Modes m );

    /**
     * Returns the mode of the filedialog.
     * @see setMode()
     */
    KFile::Modes mode() const;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString& text);

    /**
     * Returns the KFileWidget that implements most of this file dialog.
     * If you link to libkfile you can cast this to a KFileWidget*.
     */
    KAbstractFileWidget* fileWidget();

    /**
     * Returns a pointer to the toolbar.
     *
     * You can use this to insert custom
     * items into it, e.g.:
     * \code
     *      yourAction = new KAction( i18n("Your Action"), 0,
     *                                this, SLOT( yourSlot() ),
     *                                this, "action name" );
     *      yourAction->plug( kfileDialog->toolBar() );
     * \endcode
     */
    KToolBar *toolBar() const;

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
     * @returns the combobox used to type the filename or full location of the file.
     * You need to link to libkfile to use this widget.
     */
    KUrlComboBox *locationEdit() const;

    /**
     * @returns the combobox that contains the filters
     * You need to link to libkfile to use this widget.
     */
    KFileFilterCombo *filterWidget() const;

    /**
     * @returns a pointer to the action collection, holding all the used KActions.
     */
    KActionCollection *actionCollection() const;

    /**
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents directory, home directory or a recently
     * used directory.
     * @param startDir A url, to be used. May use the 'kfiledialog:///keyword' and
     *                 'kfiledialog:///keyword?global' syntax
     *                 as documented in the KFileDialog() constructor.
     * @param recentDirClass If the 'kfiledialog:///' syntax is used, recentDirClass
     *        will contain the string to be used later for KRecentDir::dir()
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     */
    static KUrl getStartUrl( const KUrl& startDir, QString& recentDirClass );

    /**
     * @internal
     * Used by KDirSelectDialog to share the dialog's start directory.
     */
    static void setStartDir( const KUrl& directory );

#ifdef Q_WS_WIN
public Q_SLOTS:
    int exec();
#endif

Q_SIGNALS:
    /**
      * Emitted when the user selects a file. It is only emitted in single-
      * selection mode. The best way to get notified about selected file(s)
      * is to connect to the okClicked() signal inherited from KDialog
      * and call selectedFile(), selectedFiles(),
      * selectedUrl() or selectedUrls().
      */
    void fileSelected(const QString&);

    /**
      * Emitted when the user highlights a file.
      */
    void fileHighlighted(const QString&);

    /**
     * Emitted when the user hilights one or more files in multiselection mode.
     *
     * Note: fileHighlighted() or fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     * @see fileSelected
     */
    void selectionChanged();

    /**
     * Emitted when the filter changed, i.e. the user entered an own filter
     * or chose one of the predefined set via setFilter().
     *
     * @param filter contains the new filter (only the extension part,
     * not the explanation), i.e. "*.cpp" or "*.cpp *.cc".
     *
     * @see setFilter()
     * @see currentFilter()
     */
    void filterChanged( const QString& filter );

protected:
    /**
     * Reimplemented to animate the cancel button.
     */
    virtual void keyPressEvent( QKeyEvent *e );

    /**
     * Reimplemented for saving the dialog geometry.
     */
    virtual void hideEvent( QHideEvent *event );

protected Q_SLOTS:
    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

private:
    Q_DISABLE_COPY(KFileDialog)

    KFileDialogPrivate * const d;
};

#endif
