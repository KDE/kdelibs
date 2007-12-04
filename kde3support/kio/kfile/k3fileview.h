// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KFILEVIEW_H
#define KFILEVIEW_H

class QPoint;
class QWidget;
class QDropEvent;
class KActionCollection;

#include "kde3support_export.h"
#include "kfileitem.h"
#include "kfile.h"

/**
 * internal class to make easier to use signals possible
 * @internal
 **/
class KDE3SUPPORT_EXPORT K3FileViewSignaler : public QObject
{
    Q_OBJECT

public:
    /**
      * Call this method when an item is selected (depends on single click /
      * double click configuration). Emits the appropriate signal.
      **/
    void activate( const KFileItem &item );

    /**
     * emits the highlighted signal for item. Call this in your view class
     * whenever the selection changes.
     */
    void highlightFile( const KFileItem &item );

    void activateMenu( const KFileItem &item, const QPoint& pos );

    void changeSorting( QDir::SortFlags sorting );

    void dropURLs( const KFileItem &item, QDropEvent *event, const KUrl::List &urls);

Q_SIGNALS:
    void dirActivated( const KFileItem &item );

    void sortingChanged( QDir::SortFlags );

    /**
     * the @p item maybe be a null item, indicating that we're in multiselection mode and
     * the selection has changed.
     */
    void fileHighlighted( const KFileItem &item );
    void fileSelected( const KFileItem &item );
    void activatedMenu( const KFileItem &item, const QPoint &pos );
    void dropped( const KFileItem &item, QDropEvent *event, const KUrl::List &urls );
};

/**
  * This class defines an interface to all file views. Its intent is
  * to allow to switch the view of the files in the selector very easily.
  * It defines some pure virtual functions, that must be implemented to
  * make a file view working.
  *
  * Since this class is not a widget, but it's meant to be added to other
  * widgets, its most important function is widget. This should return
  * a pointer to the implemented widget.
  *
  * @short A base class for views of the KDE file selector
  * @author Stephan Kulow <coolo@kde.org>
  **/
class KDE3SUPPORT_EXPORT K3FileView {

public:
    /**
     * Creates a new file view.
     */
    K3FileView();

    /**
     * Destroys the file view.
     */
    virtual ~K3FileView();

    /**
     * Inserts a list of items.
     **/
    void addItemList(const KFileItemList &list);

    /**
      * a pure virtual function to get a QWidget, that can be added to
      * other widgets. This function is needed to make it possible for
      * derived classes to derive from other widgets.
      **/
    virtual QWidget *widget() = 0;

    /**
     * Returns the widget.
     */
    QWidget *widget() const;

    /**
     * Sets @p filename the current item in the view, if available.
     */
    void setCurrentItem( const QString &filename );

    /**
     * Reimplement this to set @p item the current item in the view, e.g.
     * the item having focus.
     */
    virtual void setCurrentItem( const KFileItem &item ) = 0;

    /**
     * @returns the "current" KFileItem, e.g. where the cursor is.
     * Returns an null file item when there is no current item (e.g. in an empty view).
     * Subclasses have to implement this.
     */
    virtual KFileItem currentFileItem() const = 0;

    /**
     * Clears the view and all item lists.
     */
    virtual void clear();

    /**
      * does a repaint of the view.
      *
      * The default implementation calls
      * \code
      * widget()->repaint(f)
      * \endcode
      **/
    virtual void updateView(bool f = true);

    virtual void updateView(const KFileItem&);

    /**
     * Removes an item from the list; has to be implemented by the view.
     * Call K3FileView::removeItem( item ) after removing it.
     */
    virtual void removeItem(const KFileItem &item);

    /**
     * This hook is called when all items of the currently listed directory
     * are listed and inserted into the view, i.e. there won't come any new
     * items anymore.
     */
    virtual void listingCompleted();

    /**
      * Returns the sorting order of the internal list. Newly added files
      * are added through this sorting.
      */
    QDir::SortFlags sorting() const;

    /**
      * Sets the sorting order of the view.
      *
      * Default is QDir::Name | QDir::IgnoreCase | QDir::DirsFirst
      * Override this in your subclass and sort accordingly (usually by
      * setting the sorting-key for every item and telling QIconView
      * or QListView to sort.
      *
      * A view may choose to use a different sorting than QDir::Name, Time
      * or Size. E.g. to sort by mimetype or any possible string. Set the
      * sorting to QDir::Unsorted for that and do the rest internally.
      *
      * @see sortingKey
      */
    virtual void setSorting(QDir::SortFlags sort);

    /**
     * Tells whether the current items are in reversed order (shortcut to
     * sorting() & QDir::Reversed).
     */
    bool isReversed() const;

    void sortReversed();

    /**
      * @returns the number of dirs and files
      **/
    uint count() const;

    /**
      * @returns the number of files.
      **/
    uint numFiles() const;

    /**
      * @returns the number of directories
      **/
    uint numDirs() const;

    virtual void setSelectionMode( KFile::SelectionMode sm );
    virtual KFile::SelectionMode selectionMode() const;

    enum ViewMode {
        Files       = 1,
        Directories = 2,
        All = Files | Directories
    };
    virtual void setViewMode( ViewMode vm );
    virtual ViewMode viewMode() const;

    /**
     * @returns the localized name of the view, which could be displayed
     * somewhere, e.g. in a menu, where the user can choose between views.
     * @see setViewName
     */
    QString viewName() const;

    /**
     * Sets the name of the view, which could be displayed somewhere.
     * E.g. "Image Preview".
     */
    void setViewName( const QString& name );

    virtual void setParentView(K3FileView *parent);

    /**
     * The derived view must implement this function to add
     * the file in the widget.
     *
     * Make sure to call this implementation, i.e.
     * K3FileView::insertItem( i );
     *
     */
    virtual void insertItem( const KFileItem &i);

    /**
     * pure virtual function, that should be implemented to clear
     * the view. At this moment the list is already empty
     **/
    virtual void clearView() = 0;

    /**
     * pure virtual function, that should be implemented to make item i
     * visible, i.e. by scrolling the view appropriately.
     */
    virtual void ensureItemVisible( const KFileItem &i ) = 0;

    /**
     * Clears any selection, unhighlights everything. Must be implemented by
     * the view.
     */
    virtual void clearSelection() = 0;

    /**
     * Selects all items. You may want to override this, if you can implement
     * it more efficiently than calling setSelected() with every item.
     * This works only in Multiselection mode of course.
     */
    virtual void selectAll();

    /**
     * Inverts the current selection, i.e. selects all items, that were up to
     * now not selected and deselects the other.
     */
    virtual void invertSelection();

    /**
     * Tells the view that it should highlight the item.
     * This function must be implemented by the view.
     **/
    virtual void setSelected(const KFileItem &, bool enable) = 0;

    /**
     * @returns whether the given item is currently selected.
     * Must be implemented by the view.
     */
    virtual bool isSelected( const KFileItem & ) const = 0;

    /**
     * @returns all currently highlighted items.
     */
    KFileItemList selectedItems() const;

    /**
     * @returns all items currently available in the current sort-order
     */
    KFileItemList items() const;

    virtual KFileItem firstFileItem() const = 0;
    virtual KFileItem nextItem( const KFileItem & ) const = 0;
    virtual KFileItem prevItem( const KFileItem & ) const = 0;

    /**
     * This is a KFileDialog specific hack: we want to select directories with
     * single click, but not files. But as a generic class, we have to be able
     * to select files on single click as well.
     *
     * This gives us the opportunity to do both.
     *
     * Every view has to decide when to call select( item ) when a file was
     * single-clicked, based on onlyDoubleClickSelectsFiles().
     */
    void setOnlyDoubleClickSelectsFiles( bool enable );

    /**
     * @returns whether files (not directories) should only be select()ed by
     * double-clicks.
     * @see setOnlyDoubleClickSelectsFiles
     */
    bool onlyDoubleClickSelectsFiles() const;

    /**
     * increases the number of dirs and files.
     * @returns true if the item fits the view mode
     */
    bool updateNumbers(const KFileItem &i);

    /**
     * @returns the view-specific action-collection. Every view should
     * add its actions here (if it has any) to make them available to
     * e.g. the KDirOperator's popup-menu.
     */
    virtual KActionCollection * actionCollection() const;

    K3FileViewSignaler * signaler() const;

    virtual void readConfig( KConfigGroup *);
    virtual void writeConfig( KConfigGroup *);

    /**
     * Various options for drag and drop support.
     * These values can be or'd together.
     * @li @p AutoOpenDirs Automatically open directory after hovering above it
     * for a short while while dragging.
     */
    enum DropOptions {
        AutoOpenDirs  = 1
    };
    /**
     * Specify DND options. See DropOptions for details.
     * All options are disabled by default.
     */
    virtual void setDropOptions(int options);

    /**
     * Returns the DND options in effect.
     * See DropOptions for details.
     */
    int dropOptions();

    /**
     * This method calculates a QString from the given parameters, that is
     * suitable for sorting with e.g. QIconView or QListView. Their
     * Item-classes usually have a setKey( const QString& ) method or a virtual
     * method QString key() that is used for sorting.
     *
     * @param value Any string that should be used as sort criterion
     * @param isDir Tells whether the key is computed for an item representing
     *              a directory (directories are usually sorted before files)
     * @param SortFlags An ORed combination of QDir::SortFlag flags.
     *                  Currently, the values IgnoreCase, Reversed and
     *                  DirsFirst are taken into account.
     */
    static QString sortingKey( const QString& value, bool isDir, QDir::SortFlags SortFlags);

    /**
     * An overloaded method that takes not a QString, but a number as sort
     * criterion. You can use this for file-sizes or dates/times for example.
     * If you use a time_t, you need to cast that to KIO::filesize_t because
     * of ambiguity problems.
     */
    static QString sortingKey( KIO::filesize_t value, bool isDir,QDir::SortFlags SortFlags);

    /**
     * @internal
     * delay before auto opening a directory
     */
    static int autoOpenDelay();

protected:
    /**
     * @internal
     * class to distribute the signals
     **/
    K3FileViewSignaler *sig;

private:
    class K3FileViewPrivate;
    K3FileViewPrivate* const d;
};

#endif // KFILEINFOLISTWIDGET_H
