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

class KFileViewItem;
class QSignal;
class QPoint;

#include "kfilereader.h"
#include <qwidget.h>

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
    void activateMenu( const KFileViewItem *i )
    { emit activatedMenu( i ); }

signals:
    void dirActivated(const KFileViewItem*);
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
  * widgets, its most important function is @code widget. This should return
  * a pointer to the implemented widget.
  *
  * @short A base class for views of the KDE file selector
  * @author Stephan Kulow <coolo@kde.org>
  * @version $Id$
  **/
class KFileView {

public:
    enum ViewMode {
	Files = 1,
	Directories = 2,
	All = Files | Directories
    };
    enum SelectionMode {
	Single,
	Multi
    };

    KFileView( );

    /** Destructor */
    virtual ~KFileView();

    /**
     * inserts a list of items. This is a speed optimized version of
     * function above.
     **/
    virtual void addItemList(const KFileViewItemList *list);

    /**
      * a pure virtual function to get a QWidget, that can be added to
      * other widgets. This function is needed to make it possible for
      * derived classes to derive from other widgets.
      **/
    virtual QWidget *widget() = 0;

    /**
      * set the current item in the view.
      *
      * the normal use of this function is to use the parameter filename to
      * set the highlight on this file.
      *
      * the extend use of this function is to set filename to 0. In this
      * case the @see KFileViewItem is used to find out the position of the
      * file. Since this is faster, this way is used in case a view has
      * other child views.
      **/
    virtual void setCurrentItem(const QString &filename,
				const KFileViewItem * entry = 0);

    /**
     * clears all lists and calls clear
      */
    virtual void clear();

    /**
      * does a repaint of the view.
      *
      * The default implementation calls
      * <code>widget()->repaint(f)</code>
      **/
    virtual void updateView(bool f = true);

    virtual void updateView(const KFileViewItem*);

    /**
      * specifies the sorting of the internal list. Newly added files
      * are added through this sorting.
      *
      **/
    QDir::SortSpec sorting() const { return mySorting; }

    /**
      * set the sorting of the view. If the sorting is the same as
      * the current value and sortMode is Switching, it switches the
      * order in the list.
      *
      * Default is QDir::Name
      **/
    void setSorting(QDir::SortSpec sort); // a little bit complexer

    /**
      * returns the number of added files
      **/
    uint count() const { return filesNumber + dirsNumber; }

    /**
      * the number of files.
      **/
    uint numFiles() const { return filesNumber; }

    /**
      * the number of directories
      **/
    uint numDirs() const { return dirsNumber; }

    virtual void setSelectMode( SelectionMode sm );
    virtual SelectionMode selectMode() const;

    virtual void setViewMode( ViewMode vm );
    virtual ViewMode viewMode() const;

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
     * Clear any selection, unhighlight everything. Must be implemented by the
     * view.
     */
    virtual void clearSelection() = 0;

    virtual void insertSorted(KFileViewItem *first, uint counter);

protected:

    /**
      * Increasing means greater indicies means bigger values
      *
      * Decrease means greater indicies means smaller values
      *
      * Switching means, that it should switch between Increasing and
      * Decreasing
      **/
    enum SortMode { Increasing, Decreasing, Switching };

    /**
      * set the sorting mode. Default mode is Increasing
      **/
    void setSortMode(SortMode mode) { mySortMode = mode; }

    /**
      * set the highlighted item to index. This function must be implemented
      * by the view
      **/
    virtual void highlightItem(const KFileViewItem *) = 0;

    /**
      * sets the value for the selected file and emits the correct
      * signal (depending on the type of the entry)
      **/
    void select( const KFileViewItem *entry);

    void highlight( const KFileViewItem *);

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

    bool updateNumbers(const KFileViewItem *i);

    KFileViewItem *mergeLists(KFileViewItem *list1, KFileViewItem *list2);

    void activateMenu( const KFileViewItem *i );

    /**
     * @internal
     * class to distribute the signals
     **/
    KFileViewSignaler *sig;

    KFileViewItem *firstItem() const { return first; }

private:

    bool reversed;
    QDir::SortSpec mySorting;
    enum SortMode mySortMode;

    /**
     * counters
     **/
    uint filesNumber;
    uint dirsNumber;

    ViewMode view_mode;
    SelectionMode selection_mode;

    KFileViewItem *first;
};

#endif // KFILEINFOLISTWIDGET_H
