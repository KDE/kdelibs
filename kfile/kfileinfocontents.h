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

#ifndef KFILEINFOLISTWIDGET_H
#define KFILEINFOLISTWIDGET_H

class KFileInfo;
class QPixmap;
class QSignal;
#include <qwidget.h>
#include "kdir.h"

/**
  * A base class for views of the KDE file selector
  *
  * This class defines an interface to all file views. It's intent is 
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
class KFileInfoContents {
    
public:
    /** 
      * @short Constructor. Defines some used flags and lists
      * It loads the icons used to indicate files and folders in
      * all views, initializes some used lists and variables 
      *
      * @param useSingleClick defines, if the view should react on
      *        single click or double click
      * @param sorting defines, how the list should be sorted. 
      *        Currently Name, Size and Date are supported.
      **/
    KFileInfoContents( bool useSingleClick, QDir::SortSpec sorting );

    /** Destructor */
    virtual ~KFileInfoContents();
    
    /** 
      * Insert an item into the file view. The view will just maintain
      * the pointer. It doesn't free the object.
      *
      * This functions inserts the file at the correct place in the
      * sorted list
      * @return true, if a repaint is necessary
      * @param item the file to be inserted
      **/
    bool addItem(const KFileInfo *item);

    /**
      * @return the name of the file at position index. 
      * 
      * the meaning of index depends on the sorting and the other files
      * in the view. So use this function with care.
      **/
    virtual const char *text(uint index) const;

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
      * case the @see KFileInfo is used to find out the position of the
      * file. Since this is faster, this way is used in case a view has
      * other child views.
      **/
    virtual void setCurrentItem(const char *filename, 
				const KFileInfo * entry = 0);
    
    /**
      * set the autoUpdate flag for the specific widget. 
      * if set to false, no repaints should be done automaticly unless
      * repaint is called.
      *
      * this function is pure virtual, so it must be implemented
      **/
    virtual void setAutoUpdate(bool f) = 0;

    /**
      * clears all lists and calls clearView 
      */
    void clear();

    /**
      * does a repaint of the view. 
      *
      * The default implementation calls 
      * <code>widget()->repaint(f)</code>
      **/
    virtual void repaint(bool f = true);

    /**
      * specifies the sorting of the internal list. Newly added files
      * are added through this sorting.
      *
      **/
    QDir::SortSpec sorting() { return mySorting; }

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
    uint count() const { return itemsList->count(); }

    /**
      * to connect a slot to the internal signal, that is emited, if
      * the user selects a directory.
      * 
      * Before this signal is emited, the value for @ref selectedDir is
      * set to the correct value.
      **/
    void connectDirSelected( QObject *receiver, const char *member);

    /**
      * to connect a slot to the internal signal, that is emited, if
      * the user highlights a file.
      * 
      * Before this signal is emited, the value for @ref highlightedFile 
      * is set to the correct value.
      **/
    void connectFileHighlighted( QObject *receiver, const char *member);

    /**
      * to connect a slot to the internal signal, that is emited, if
      * the user selects a file.
      * 
      * Before this signal is emited, the value for @ref selectedFile is
      * set to the correct value.
      **/
    void connectFileSelected( QObject *receiver, const char *member);

    /**
      * @return the last selected directory
      **/
    KFileInfo *selectedDir()     { return lastSDir;  }
    
    /**
      * @return the last selected file
      **/
    KFileInfo *selectedFile()    { return lastSFile; }

    /**
      * @return the last highlighted file
      **/
    KFileInfo *highlightedFile() { return lastHFile; }

    /**
      * the number of files. 
      **/
    uint numFiles() const { return filesNumber; }
    
    /**
      * the number of directories
      **/
    uint numDirs() const { return dirsNumber; }

    /**
      * tries to find a completed filename, that starts with base
      *
      * if it fails, it returns a null string. 
      * @param activateFound if true, the file should select the 
      *        found file (or directory)
      **/
    virtual QString findCompletion( const char *base, 
				    bool activateFound = false );
    
    /**
      * a pure virtual function to indicate, if the view should list
      * files. 
      **/
    virtual bool acceptsFiles() = 0;

    /**
      * a pure virtual function to indicate, if the view should list
      * directories.
      **/
    virtual bool acceptsDirs() = 0;

protected:
    
    /**
      * this function is called after the class found out the correct
      * index. The derived view must implement this function to add
      * the file in the widget. 
      *
      * it's important, that the view keeps it's contents consistent
      * with the indicies in the sorted list of the base class
      * @return true, if a repaint is necessary
      **/
    virtual bool insertItem(const KFileInfo *i, int index) = 0;

    /**
      * this function does the actual sorting. It is called by 
      * addItem and calls itself insertItem 
      **/
    bool addItemInternal(const KFileInfo *);

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
      * pure virtual function, that should be implemented to clear
      * the view. At this moment the list is already empty
      **/
    virtual void clearView() = 0;

    /**
      * set the highlighted item to index. This function must be implemented
      * by the view
      **/
    virtual void highlightItem(unsigned int index) = 0;

    /** the pixmap for normal files */ 
    static QPixmap *file_pixmap;
    /** the pixmap for locked files */
    static QPixmap *locked_file;
    /** the pixmap for normal folders */
    static QPixmap *folder_pixmap;
    /** the pixmap for locked folders */
    static QPixmap *locked_folder;
    
    /** 
      * sets the value for the selected file and emits the correct
      * signal (depending on the type of the entry)
      **/
    void select( KFileInfo *entry);

    /**
      * gets the entry at index and calls the other select function
      * with it
      **/
    void select( int index );

    void highlight( int index );
    void highlight( KFileInfo *);
    
    bool useSingle() const { return useSingleClick; }
    bool isDir( uint index) const { return sortedArray[index]->isDir(); }

    const KFileInfo *at( uint index ) const { return sortedArray[index]; }

    /**
     * compares two items in the current context (sortMode and others) 
     * returns -1, if i1 is before i2 and 1, if the other case is true
     * in case, both are equal (in current context), the behaviour is 
     * undefined!
     **/
    int compareItems(const KFileInfo *fi1, const KFileInfo *fi2);

private:
    bool useSingleClick;

    bool reversed;
    bool keepDirsFirst;
    QDir::SortSpec mySorting;
    enum SortMode mySortMode;
    KFileInfo **sortedArray;
    uint sorted_length;
    uint sorted_max;

    KFileInfoList *itemsList;

    QSignal *sActivateDir;
    QSignal *sHighlightFile;
    QSignal *sSelectFile;
    QStrIList *nameList;
    
    KFileInfo *lastHFile;
    KFileInfo *lastSFile;
    KFileInfo *lastSDir;
    uint filesNumber, dirsNumber;

  /**
   * @internal
   * adds an item to sortedArray
   **/
  void insertSortedItem(const KFileInfo *item, uint pos);
};

#endif // KFILEINFOLISTWIDGET_H
