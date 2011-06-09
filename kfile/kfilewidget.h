// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
                  2001 Frerich Raabe <raabe@kde.org>
                  2007 David Faure <faure@kde.org>
                  2008 Rafael Fernández López <ereslibre@kde.org>

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


#ifndef KFILEWIDGET_H
#define KFILEWIDGET_H

#include "kfile_export.h"
#include "kabstractfilewidget.h"
#include <QtGui/QWidget>

class KJob;
class KFileItem;
class KDirOperator;

class KFILE_EXPORT KFileWidget : public QWidget, public KAbstractFileWidget
{
    Q_OBJECT
    Q_INTERFACES(KAbstractFileWidget)
public:
    /**
      * Constructs a file selector widget.
      *
      * @param startDir This can either be:
      *         @li An empty URL (KUrl()) to start in the current working directory,
      *             or the last directory where a file has been selected.
      *         @li The path or URL of a starting directory.
      *         @li An initial file name to select, with the starting directory being
      *             the current working directory or the last directory where a file
      *             has been selected.
      *         @li The path or URL of a file, specifying both the starting directory and
      *             an initially selected file name.
      *         @li A URL of the form @c kfiledialog:///&lt;keyword&gt; to start in the
      *             directory last used by a filedialog in the same application that
      *             specified the same keyword.
      *         @li A URL of the form @c kfiledialog:///&lt;keyword&gt;/&lt;filename&gt;
      *             to start in the directory last used by a filedialog in the same
      *             application that specified the same keyword, and to initially
      *             select the specified filename.
      *         @li A URL of the form @c kfiledialog:///&lt;keyword&gt;?global to start
      *             in the directory last used by a filedialog in any application that
      *             specified the same keyword.
      *         @li A URL of the form @c kfiledialog:///&lt;keyword&gt;/&lt;filename&gt;?global
      *             to start in the directory last used by a filedialog in any
      *             application that specified the same keyword, and to initially
      *             select the specified filename.
      *
      * @param parent The parent widget of this widget
      *
      */
    KFileWidget(const KUrl& startDir, QWidget *parent);

    /**
     * Destructor
     */
    virtual ~KFileWidget();

    /**
     * @returns The selected fully qualified filename.
     */
    virtual KUrl selectedUrl() const;

    /**
     * @returns The list of selected URLs.
     */
    virtual KUrl::List selectedUrls() const;

    /**
     * @returns the currently shown directory.
     */
    virtual KUrl baseUrl() const;

    /**
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    virtual QString selectedFile() const;

    /**
     * Returns a list of all selected local files.
     */
    virtual QStringList selectedFiles() const;

    /**
     * Sets the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    virtual void setUrl(const KUrl &url, bool clearforward = true);

    /**
     * Sets the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    virtual void setSelection(const QString& name);

    /**
     * Sets the operational mode of the filedialog to @p Saving, @p Opening
     * or @p Other. This will set some flags that are specific to loading
     * or saving files. E.g. setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileWidget::Saving ); sets
     * setKeepLocation for example.
     *
     * The mode @p Saving, together with a default filter set via
     * setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is @p Opening.
     *
     * Call this method right after instantiating KFileWidget.
     *
     * @see operationMode
     * @see KFileWidget::OperationMode
     */
    virtual void setOperationMode( OperationMode );

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see operationMode
     * @see KFileWidget::OperationMode
     */
    virtual OperationMode operationMode() const;

    /**
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when operationMode() is KFileWidget::Saving
     *
     * getSaveFileName() and getSaveUrl() set this to true by default, so that
     * you can type in the filename and change the directory without having
     * to type the name again.
     */
    virtual void setKeepLocation( bool keep );

    /**
     * @returns whether the contents of the location edit are kept when
     * changing directories.
     */
    virtual bool keepsLocation() const;

    /**
     * Sets the filter to be used to @p filter.
     *
     * You can set more
     * filters for the user to select separated by '\n'. Every
     * filter entry is defined through namefilter|text to display.
     * If no | is found in the expression, just the namefilter is
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
     * If the filter contains an unescaped '/', a mimetype-filter is assumed.
     * If you would like a '/' visible in your filter it can be escaped with
     * a '\'. You can specify multiple mimetypes like this (separated with
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
    virtual void setFilter(const QString& filter);

    /**
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilter().
     *
     * @see setFilter()
     * @see filterChanged()
     */
    virtual QString currentFilter() const;

    /**
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    virtual KMimeType::Ptr currentFilterMimeType();

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
    virtual void setMimeFilter( const QStringList& types,
                        const QString& defaultType = QString() );

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
     * previously.
     *
     * @see setMimeFilter()
     */
    virtual QString currentMimeFilter() const;

    /**
     *  Clears any mime- or namefilter. Does not reload the directory.
     */
    virtual void clearFilter();

    /**
     * Adds a preview widget and enters the preview mode.
     *
     * In this mode the dialog is split and the right part contains your
     * preview widget.
     *
     * Ownership is transferred to KFileWidget. You need to create the
     * preview-widget with "new", i.e. on the heap.
     *
     * @param w The widget to be used for the preview.
     */
    virtual void setPreviewWidget(KPreviewWidgetBase *w);

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
    virtual void setMode( KFile::Modes m );

    /**
     * Returns the mode of the filedialog.
     * @see setMode()
     */
    virtual KFile::Modes mode() const;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    virtual void setLocationLabel(const QString& text);

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
     * @returns a pointer to the OK-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    KPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    KPushButton *cancelButton() const;

    /**
     * @returns the combobox used to type the filename or full location of the file.
     */
    KUrlComboBox *locationEdit() const;

    /**
     * @returns the combobox that contains the filters
     */
    KFileFilterCombo *filterWidget() const;

    /**
     * @returns a pointer to the action collection, holding all the used
     * KActions.
     */
    KActionCollection *actionCollection() const;

    /**
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents directory, home directory or a recently
     * used directory.
     * @param startDir A URL specifying the initial directory, or using the
     *                 @c kfiledialog:/// syntax to specify a last used
     *                 directory.  If this URL specifies a file name, it is
     *                 ignored.  Refer to the KFileWidget::KFileWidget()
     *                 documentation for the @c kfiledialog:/// URL syntax.
     * @param recentDirClass If the @c kfiledialog:/// syntax is used, this
     *        will return the string to be passed to KRecentDirs::dir() and
     *        KRecentDirs::add().
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     * @see KFileWidget::KFileWidget()
     */
    static KUrl getStartUrl( const KUrl& startDir, QString& recentDirClass );

    /**
     * Similar to getStartUrl(const KUrl& startDir,QString& recentDirClass),
     * but allows both the recent start directory keyword and a suggested file name
     * to be returned.
     * @param startDir A URL specifying the initial directory and/or filename,
     *                 or using the @c kfiledialog:/// syntax to specify a
     *                 last used location.
     *                 Refer to the KFileWidget::KFileWidget()
     *                 documentation for the @c kfiledialog:/// URL syntax.
     * @param recentDirClass If the @c kfiledialog:/// syntax is used, this
     *        will return the string to be passed to KRecentDirs::dir() and
     *        KRecentDirs::add().
     * @param fileName The suggested file name, if specified as part of the
     *        @p StartDir URL.
     * @return The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     *
     * @see KFileWidget::KFileWidget()
     * @since 4.3
     */
    static KUrl getStartUrl( const KUrl& startDir, QString& recentDirClass, QString& fileName );

    /**
     * @internal
     * Used by KDirSelectDialog to share the dialog's start directory.
     */
    static void setStartDir( const KUrl& directory );

    /**
     * Set a custom widget that should be added to the file dialog.
     * @param widget A widget, or a widget of widgets, for displaying custom
     *               data in the file widget. This can be used, for example, to
     *               display a check box with the caption "Open as read-only".
     *               When creating this widget, you don't need to specify a parent,
     *               since the widget's parent will be set automatically by KFileWidget.
     */
    virtual void setCustomWidget(QWidget* widget);

    /**
     * Sets a custom widget that should be added below the location and the filter
     * editors.
     * @param text     Label of the custom widget, which is displayed below the labels
     *                 "Location:" and "Filter:".
     * @param widget   Any kind of widget, but preferable a combo box or a line editor
     *                 to be compliant with the location and filter layout.
     *                 When creating this widget, you don't need to specify a parent,
     *                 since the widget's parent will be set automatically by KFileWidget.
     */
    virtual void setCustomWidget(const QString& text, QWidget* widget);

    /// @internal for future extensions
    virtual void virtual_hook( int id, void* data );

public Q_SLOTS:
    /**
     * Called when clicking ok (when this widget is used in KFileDialog)
     * Might or might not call accept().
     */
    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void showEvent(QShowEvent* event);
    virtual bool eventFilter(QObject* watched, QEvent* event);

Q_SIGNALS:
    /**
      * Emitted when the user selects a file. It is only emitted in single-
      * selection mode. The best way to get notified about selected file(s)
      * is to connect to the okClicked() signal inherited from KDialog
      * and call selectedFile(), selectedFiles(),
      * selectedUrl() or selectedUrls().
      *
      * \since 4.4
      */
    void fileSelected(const KUrl&);

    /**
     * @deprecated, connect to fileSelected(const KUrl&) instead
     */
    QT_MOC_COMPAT void fileSelected(const QString&); // TODO KDE5: remove

    /**
     * Emitted when the user highlights a file.
     * \since 4.4
     */
    void fileHighlighted(const KUrl&);
    /**
     * @deprecated, connect to fileSelected(const KUrl&) instead
     */
    QT_MOC_COMPAT void fileHighlighted(const QString&); // TODO KDE5: remove

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

    /**
     * Emitted by slotOk() (directly or asynchronously) once everything has
     * been done. Should be used by the caller to call accept().
     */
    void accepted();

public:
    /**
     * @returns the KDirOperator used to navigate the filesystem
     * @since 4.3
     */
    KDirOperator* dirOperator();

    /**
     * reads the configuration for this widget from the given config group
     * @param group the KConfigGroup to read from
     * @since 4.4
     */
    void readConfig( KConfigGroup& group );

private:
    friend class KFileWidgetPrivate;
    KFileWidgetPrivate* const d;

    Q_PRIVATE_SLOT(d, void _k_slotLocationChanged(const QString&))
    Q_PRIVATE_SLOT(d, void _k_urlEntered(const KUrl&))
    Q_PRIVATE_SLOT(d, void _k_enterUrl(const KUrl&))
    Q_PRIVATE_SLOT(d, void _k_enterUrl(const QString&))
    Q_PRIVATE_SLOT(d, void _k_locationAccepted(const QString&))
    Q_PRIVATE_SLOT(d, void _k_slotFilterChanged())
    Q_PRIVATE_SLOT(d, void _k_fileHighlighted(const KFileItem&))
    Q_PRIVATE_SLOT(d, void _k_fileSelected(const KFileItem&))
    Q_PRIVATE_SLOT(d, void _k_slotLoadingFinished())
    Q_PRIVATE_SLOT(d, void _k_fileCompletion(const QString&))
    Q_PRIVATE_SLOT(d, void _k_toggleSpeedbar(bool))
    Q_PRIVATE_SLOT(d, void _k_toggleBookmarks(bool))
    Q_PRIVATE_SLOT(d, void _k_slotAutoSelectExtClicked())
    Q_PRIVATE_SLOT(d, void _k_placesViewSplitterMoved(int, int))
    Q_PRIVATE_SLOT(d, void _k_activateUrlNavigator())
    Q_PRIVATE_SLOT(d, void _k_zoomOutIconsSize())
    Q_PRIVATE_SLOT(d, void _k_zoomInIconsSize())
    Q_PRIVATE_SLOT(d, void _k_slotIconSizeSliderMoved(int))
    Q_PRIVATE_SLOT(d, void _k_slotIconSizeChanged(int))
};

#endif /* KABSTRACTFILEWIDGET_H */

