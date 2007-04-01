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


#ifndef KFILEWIDGET_H
#define KFILEWIDGET_H

#include "kabstractfilewidget.h"
#include <QtGui/QWidget>

class KJob;
class KFileItem;
class KPushButton;
class KActionCollection;
class KToolBar;
class KUrlBar;
class KUrlComboBox;
class KFileFilterCombo;
class KFileWidgetPrivate;

class KIO_EXPORT KFileWidget : public QWidget, public KAbstractFileWidget
{
    Q_OBJECT
public:
    /**
      * Constructs a file selector widget.
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
      * @param parent The parent widget of this widget
      *
      * @param widget A widget, or a widget of widgets, for displaying custom
      *               data in the file widget. This can be used, for example, to
      *               display a check box with the caption "Open as read-only".
      *               When creating this widget, you don't need to specify a parent,
      *               since the widget's parent will be set automatically by KFileWidget.
      */
    KFileWidget( const KUrl& startDir, const QString& filter,
                 QWidget *parent, QWidget* widget = 0 );


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
     * filter entry is defined through namefilter|text to diplay.
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
   virtual void setPreviewWidget(const KPreviewWidgetBase *w);

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
     * @returns a pointer to the OK-Button in the filedialog. You may use it
     * e.g. to set a custom text to it.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    KPushButton *okButton() const;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog. You may use
     * it e.g. to set a custom text to it.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    KPushButton *cancelButton() const;

    /**
     * @returns the KUrlBar object used as the "speed bar". You can add custom
     * entries to it like that:
     * \code
     * KUrlBar *urlBar = fileDialog->speedBar();
     * if ( urlBar )
     *     urlBar->insertDynamicItem( someURL, i18n("The URL's description") );
     * \endcode
     *
     * Note that this method may return a null-pointer if the user configured
     * to not use the speed-bar.
     * @see KUrlBar
     * @see KUrlBar::insertDynamicItem
     */
    KUrlBar *speedBar();

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

protected Q_SLOTS:
    /**
     * Called when clicking ok (when this widget is used in KFileDialog)
     * Might or might not call accept().
     */
    virtual void slotOk();
    virtual void accept();
    virtual void slotCancel();

protected:
    virtual void showEvent(QShowEvent* event);

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

    /**
     * Emitted by slotOk() (directly or asynchronously) once everything has
     * been done. Should be used by the caller to call accept().
     */
    void accepted();

private Q_SLOTS:
    void slotLocationChanged( const QString& text );
    void urlEntered( const KUrl& );
    void enterUrl( const KUrl& url );
    void enterUrl( const QString& url );
    void locationActivated( const QString& url );

    void slotFilterChanged();
    void fileHighlighted(const KFileItem *i);
    void fileSelected(const KFileItem *i);
    void slotStatResult(KJob* job);
    void slotLoadingFinished();
    void fileCompletion( const QString& );
    void toggleSpeedbar( bool );
    void toggleBookmarks(bool show);

    void slotAutoSelectExtClicked();
    void addToRecentDocuments();

private:
    /**
     * Returns the absolute version of the URL specified in locationEdit.
     */
    KUrl getCompleteUrl(const QString&);

private:
    friend class KFileDialog; // calls slotOk etc.
    friend class KFileWidgetPrivate;
    KFileWidgetPrivate* const d;
};

#endif /* KABSTRACTFILEWIDGET_H */

