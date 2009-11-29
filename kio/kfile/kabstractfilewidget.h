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
#ifndef KABSTRACTFILEWIDGET_H
#define KABSTRACTFILEWIDGET_H

class KPreviewWidgetBase;

#include "kurl.h"
#include "kfile.h"
#include <kmimetype.h>

class KPushButton;
class KActionCollection;
class KToolBar;
class KFileWidgetPrivate;
class KUrlComboBox;
class KFileFilterCombo;

/**
 * Base class for KFileWidget.
 *
 * This abstract interface allows KFileDialog (in kio) to call methods
 * on the dlopened KFileWidget (from kfilemodule.so)
 *
 * In addition to the pure virtual methods defined below, the implementations
 * of KAbstractFileWidget are expected to define the following signals:
 * <ul>
 * <li>fileSelected(const KUrl&)</li>
 * <li>fileHighlighted(const KUrl&)</li>
 * <li>selectionChanged()</li>
 * <li>filterChanged(const QString&)</li>
 * <li>accepted()</li>
 * </ul>
 */
class KIO_EXPORT KAbstractFileWidget
{
public:
    virtual ~KAbstractFileWidget() {}

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
     * @returns The selected fully qualified filename.
     */
    virtual KUrl selectedUrl() const = 0;

    /**
     * @returns The list of selected URLs.
     */
    virtual KUrl::List selectedUrls() const = 0;

    /**
     * @returns the currently shown directory.
     */
    virtual KUrl baseUrl() const = 0;

    /**
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    virtual QString selectedFile() const = 0;

    /**
     * Returns a list of all selected local files.
     */
    virtual QStringList selectedFiles() const = 0;

    /**
     * Sets the directory to view.
     *
     * @param url URL to show.
     * @param clearforward Indicates whether the forward queue
     * should be cleared.
     */
    virtual void setUrl(const KUrl &url, bool clearforward = true) = 0;

    /**
     * Sets the file name to preselect to @p name
     *
     * This takes absolute URLs and relative file names.
     */
    virtual void setSelection(const QString& name) = 0;

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
    virtual void setOperationMode( OperationMode ) = 0;

    /**
     * @returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * @see operationMode
     * @see KFileDialog::OperationMode
     */
    virtual OperationMode operationMode() const = 0;

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
    virtual void setKeepLocation( bool keep ) = 0;

    /**
     * @returns whether the contents of the location edit are kept when
     * changing directories.
     */
    virtual bool keepsLocation() const = 0;

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
    virtual void setFilter(const QString& filter) = 0;

    /**
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilter().
     *
     * @see setFilter()
     * @see filterChanged()
     */
    virtual QString currentFilter() const = 0;

    /**
     * Returns the mimetype for the desired output format.
     *
     * This is only valid if setFilterMimeType() has been called
     * previously.
     *
     * @see setFilterMimeType()
     */
    virtual KMimeType::Ptr currentFilterMimeType() = 0;

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
                                const QString& defaultType = QString() ) = 0;

    /**
     * The mimetype for the desired output format.
     *
     * This is only valid if setMimeFilter() has been called
     * previously.
     *
     * @see setMimeFilter()
     */
    virtual QString currentMimeFilter() const = 0;

    /**
     *  Clears any mime- or namefilter. Does not reload the directory.
     */
    virtual void clearFilter() = 0;

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
   virtual void setPreviewWidget(KPreviewWidgetBase *w) = 0;

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
    virtual void setMode( KFile::Modes m ) = 0;

    /**
     * Returns the mode of the filedialog.
     * @see setMode()
     */
    virtual KFile::Modes mode() const = 0;

    /**
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    virtual void setLocationLabel(const QString& text) = 0;

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
    virtual KToolBar *toolBar() const = 0;

    /**
     * @returns a pointer to the OK-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    virtual KPushButton *okButton() const = 0;

    /**
     * @returns a pointer to the Cancel-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    virtual KPushButton *cancelButton() const = 0;

    /**
     * @returns the combobox used to type the filename or full location of the file.
     */
    virtual KUrlComboBox *locationEdit() const = 0;

    /**
     * @returns the combobox that contains the filters
     */
    virtual KFileFilterCombo *filterWidget() const = 0;

    /**
     * @returns a pointer to the action collection, holding all the used
     * KActions.
     */
    virtual KActionCollection *actionCollection() const = 0;

    /**
     * Set a custom widget that should be added to the bottom of the file dialog.
     * @param widget A widget, or a widget of widgets, for displaying custom
     *               data in the file widget. This can be used, for example, to
     *               display a check box with the caption "Open as read-only".
     *               When creating this widget, you don't need to specify a parent,
     *               since the widget's parent will be set automatically by KFileWidget.
     */
    virtual void setCustomWidget(QWidget* widget) = 0;

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
    virtual void setCustomWidget(const QString& text, QWidget* widget) = 0;

    /**
     * Called when clicking ok (when this widget is used in KFileDialog)
     * Might or might not call accept().
     */
    virtual void slotOk() = 0;
    virtual void accept() = 0;
    virtual void slotCancel() = 0;

    /// @internal for future extensions
    virtual void virtual_hook( int id, void* data ) = 0;

    /**
     * Sets whether the user should be asked for confirmation
     * when an overwrite might occurr.
     *
     * @param enable Set this to true to enable checking.
     * @since 4.2
     */
    void setConfirmOverwrite(bool enable){  // KDE5 TODO: make this virtual
        virtual_hook(0, static_cast<void*>(&enable));
    }

    /**
     * Forces the inline previews to be shown or hidden, depending on @p show.
     *
     * @param show Whether to show inline previews or not.
     * @since 4.2
     */
    void setInlinePreviewShown(bool show) { // KDE5 TODO: make this virtual
        virtual_hook(1, static_cast<void*>(&show));
    }
};

Q_DECLARE_INTERFACE(KAbstractFileWidget, "org.kde.KAbstractFileWidget")

#endif /* KABSTRACTFILEWIDGET_H */

