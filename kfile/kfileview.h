// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow <coolo@kde.org>

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

#ifndef KFILEVIEW_H
#define KFILEVIEW_H

class QSignal;
class QPoint;
class KActionCollection;

#include <qwidget.h>

#include "kfileviewitem.h"
#include "kfilereader.h"
#include "kfile.h"

/**
 * internal class to make easier to use signals possible
 * @internal
 **/
class KFileViewSignaler : public QObject {
    Q_OBJECT

public:
    void activateDir(const KFileViewItem *i) { emit dirActivated(i); }
    void highlightFile(const KFileViewItem *i) { emit fileHighlighted(i); }
    void activateFile(const KFileViewItem *i) { emit fileSelected(i); }
    void activateMenu( const KFileViewItem *i ) { emit activatedMenu( i ); }

signals:
    void dirActivated(const KFileViewItem*);

    /**
     * the item maybe be 0L, indicating that we're in multiselection mode and
     * the selection has changed.
     */
    void fileHighlighted(const KFileViewItem*);
    void fileSelected(const KFileViewItem*);
    void activatedMenu( const KFileViewItem *i );
};

/**
  * A base class for views of the KDE file selector
  *
  * This class defines an interface to all file views. Its intent is
  * to allow to switch the view of the files in the selector very easily.
  * It defines some pure virtual functions, that must be implemented to
  * make a file view working.
  *
  * Since this class is not a widget, but it's meant to be added to other
  * widgets, its most important function is @ref widget. This should return
  * a pointer to the implemented widget.
  *
  * @short A base class for views of the KDE file selector
  * @author Stephan Kulow <coolo@kde.org>
  * @version $Id$
  **/
class KFileView {

public:
    KFileView();

    /** Destructor */
    virtual ~KFileView();

    /**
     * inserts a list of items.
     **/
    virtual void addItemList(const KFileViewItemList &list);

    /**
     * Inserts a list of items. Use this method <b>only</b> if you are sure
     * that the list really contains KFileViewItems or items  subclassing
     * KFileViewItem. You can use KDirLister to create a KFileItemList with
     * any item, subclassing KFileItem. E.g. KFileReader creates KFileViewItems
     *
     * @see KDirLister::createFileItem
     */
    void addItemList( const KFileItemList& list );

    /**
      * a pure virtual function to get a QWidget, that can be added to
      * other widgets. This function is needed to make it possible for
      * derived classes to derive from other widgets.
      **/
    virtual QWidget *widget() = 0;

    /**
     * As const-method, to be fixed in 3.0
     */
    QWidget *widget() const { return const_cast<KFileView*>(this)->widget(); }

    /**
      * set the current item in the view.
      *
      * the normal use of this function is to use the parameter filename to
      * set the highlight on this file.
      *
      * the extend use of this function is to set filename to 0. In this
      * case the @ref KFileViewItem is used to find out the position of the
      * file. Since this is faster, this way is used in case a view has
      * other child views.
      **/
    virtual void setCurrentItem(const QString &filename,
				const KFileViewItem * entry = 0);

    // virtual KFileViewItem *currentFileItem() const; // ### add after 3.0

    /**
     * clears the view and all item lists
      */
    virtual void clear();

    /**
      * does a repaint of the view.
      *
      * The default implementation calls
      * <pre>widget()->repaint(f)</pre>
      **/
    virtual void updateView(bool f = true);

    virtual void updateView(const KFileViewItem*);

    /**
     * Remove an item from the list; has to be implemented by the view.
     * Call KFileView::removeItem( item ) after removing it.
     */
    virtual void removeItem(const KFileViewItem *item);

    /**
      * specifies the sorting of the internal list. Newly added files
      * are added through this sorting.
      */
    QDir::SortSpec sorting() const { return mySorting; }

    /**
      * set the sorting of the view
      *
      * Default is QDir::Name | QDir::IgnoreCase | QDir::DirsFirst
      * Don't use QDir::Reversed, use @ref sortReversed() if you want to
      * reverse the sort order.
      * Calling this method keeps the reversed-setting
      * If your view wants to get notified about sorting-changes (e.g. to show
      * a sorting indicator), override this method and call this implementation
      * in the beginning of your method.
      * @see #setSortMode
      **/
    virtual void setSorting(QDir::SortSpec sort);

    /**
      * set the sorting mode. Default mode is Increasing. Affects only
      * newly added items.
      * @see #setSorting
      **/
    void setSortMode(KFile::SortMode mode) { mySortMode = mode; }

    /**
     * @returns the current sort mode
     * @see #setSortMode
     * @see #setSorting
     */
    KFile::SortMode sortMode() const { return mySortMode; }

    /**
     * Toggles the current sort order, i.e. the order is reversed.
     * @see #isReversed
     */
    virtual void sortReversed();

    /**
     * Tells whether the current items are in reversed order (= contrary to
     * @ref sortMode).
     */
    bool isReversed() const { return reversed; }

    /**
      * @returns the number of dirs and files
      **/
    uint count() const { return filesNumber + dirsNumber; }

    /**
      * @returns the number of files.
      **/
    uint numFiles() const { return filesNumber; }

    /**
      * @returns the number of directories
      **/
    uint numDirs() const { return dirsNumber; }

    virtual void setSelectionMode( KFile::SelectionMode sm );
    virtual KFile::SelectionMode selectionMode() const;

    enum ViewMode {
	Files       = 1,
	Directories = 2,
	All = Files | Directories
    };
    virtual void setViewMode( ViewMode vm );
    virtual ViewMode viewMode() const { return view_mode; }

    /**
     * @returns the localized name of the view, which could be displayed
     * somewhere, e.g. in a menu, where the user can choose between views.
     * @see #setViewName
     */
    // ### make const!
    QString viewName() { return viewname; }

    /**
     * Sets the name of the view, which could be displayed somewhere.
     * E.g. "Image Preview".
     */
    void setViewName( const QString& name ) { viewname = name; }

    virtual void setOperator(QObject *ops);

    /**
     * The derived view must implement this function to add
     * the file in the widget.
     *
     **/
    virtual void insertItem( KFileViewItem *i) = 0;

    /**
     * pure virtual function, that should be implemented to clear
     * the view. At this moment the list is already empty
     **/
    virtual void clearView() = 0;

    /**
     * pure virtual function, that should be implemented to make item i
     * visible, i.e. by scrolling the view appropriately.
     */
    virtual void ensureItemVisible( const KFileViewItem *i ) = 0;

    /**
     * Clear any selection, unhighlight everything. Must be implemented by the
     * view.
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
      * This function must be implemented by the view
      **/
    virtual void setSelected(const KFileViewItem *, bool enable) = 0;

    /**
     * @returns whether the given item is currently selected.
     * Must be implemented by the view.
     */
    virtual bool isSelected( const KFileViewItem * ) const = 0;

    /**
     * @returns all currently highlighted items.
     */
    const KFileViewItemList * selectedItems() const;

    /**
     * @returns all items currently available in the current sort-order
     */
    const KFileViewItemList * items() const;

    /**
     * Inserts "counter" KFileViewItems and sorts them conforming to the
     * current sort-order.
     * If you override this method, you have to call @ref setFirstItem()
     * afterwards, to set the first item of your newly sorted items.
     */
    virtual void insertSorted(KFileViewItem *tfirst, uint counter);

    /**
     * @returns the first (depending on sort order) item. It forms sort of a
     * list, as each item holds a pointer to the next item.
     * This is only public for internal reasons, DON'T call it unless you
     * implement a View yourself and really need to.
     */
    KFileViewItem *firstItem() const { return myFirstItem; }

    /**
     * This is a KFileDialog specific hack: we want to select directories with
     * single click, but not files. But as a generic class, we have to be able
     * to select files on single click as well.
     *
     * This gives us the opportunity to do both.
     *
     * Every view has to decide when to call select( item ) when a file was
     * single-clicked, based on @ref onlyDoubleClickSelectsFiles().
     */
    void setOnlyDoubleClickSelectsFiles( bool enable ) {
	myOnlyDoubleClickSelectsFiles = enable;
    }

    /**
     * @returns whether files (not directories) should only be select()ed by
     * double-clicks.
     * @see #setOnlyDoubleClickSelectsFiles
     */
    bool onlyDoubleClickSelectsFiles() const {
	return myOnlyDoubleClickSelectsFiles;
    }

    /**
     * increases the number of dirs and files.
     * @returns true if the item fits the view mode
     */
    bool updateNumbers(const KFileViewItem *i);

    // ### make virtual and override in kfilepreview and kcombiview
    /**
     * @returns the view-specific action-collection. Every view should
     * add its actions here (if it has any) to make them available to
     * e.g. the KDirOperator's popup-menu.
     */
    KActionCollection * actionCollection() const;

protected:

    /**
      * Call this method when an item is selected (depends on single click /
      * double click configuration). Emits the appropriate signal.
      **/
    void select( const KFileViewItem *item );

    /**
     * emits the highlighted signal for item. Call this in your subclass,
     * whenever the selection changes.
     */
    void highlight( const KFileViewItem *item) { sig->highlightFile( item ); }

    /**
     * compares two items in the current context (sortMode and others)
     * returns -1, if i1 is before i2 and 1, if the other case is true
     * in case, both are equal (in current context), the behaviour is
     * undefined!
     **/
    int compareItems(const KFileViewItem *fi1, const KFileViewItem *fi2) const;

    /**
     * this is a help function for sorting, since I can't use the libc
     * version (because I have a variable sort function)
     *
     */
    void QuickSort(KFileViewItem* a[], int lo0, int hi0) const;

    KFileViewItem *mergeLists(KFileViewItem *list1, KFileViewItem *list2);

    void activateMenu( const KFileViewItem *i ) { sig->activateMenu(i); }

    /**
     * @internal
     * class to distribute the signals
     **/
    KFileViewSignaler *sig;

    /**
     * Call this method to set the first item after you call your own
     * insertSorted(). You only need to call it when you override
     * insertSorted().
     */
    void setFirstItem( KFileViewItem * item ) { myFirstItem = item; }

    /**
     * Call this if you changed the sort order and want to perform the actual
     * sorting and show the new items.
     */
    void resort() {
	if ( count() > 1 ) {
            const KFileViewItemList *selected = KFileView::selectedItems();

	    KFileViewItem *item = myFirstItem;
	    myFirstItem = 0L;
	    insertSorted( item, count() );

            // restore the old selection
            KFileViewItemListIterator it( *selected );
            for ( ; it.current(); ++it ) {
                setSelected( it.current(), true );
            }
	}
    }

    /**
     * You should probably never change this variable, but call setSorting().
     * It's here for the combi-view, that needs to set the sorting without
     * resorting (the childviews do that themselves).
     */
    QDir::SortSpec mySorting;

    /**
     * You should probably never change this variable, but call sortReversed().
     * It's here for the combi-view, that needs to set the sorting without
     * resorting (the childviews do that themselves).
     */
    bool reversed;

private:
    static QDir::SortSpec defaultSortSpec;
    KFile::SortMode mySortMode;
    QString viewname;

    /**
     * counters
     **/
    uint filesNumber;
    uint dirsNumber;

    ViewMode view_mode;
    KFile::SelectionMode selection_mode;

    KFileViewItem *myFirstItem;
    mutable KFileViewItemList *itemList, *selectedList;
    mutable  bool itemListDirty;
    bool myOnlyDoubleClickSelectsFiles;

private:
    class KFileViewPrivate;
    KFileViewPrivate *d;
};

#endif // KFILEINFOLISTWIDGET_H
