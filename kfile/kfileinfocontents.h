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

class KFileInfoContents {
    
public:
    KFileInfoContents( bool useSingleClick, QDir::SortSpec sorting );
    virtual ~KFileInfoContents();
    
    bool addItem(const KFileInfo *);
    virtual const char *text(uint) const;
    virtual QWidget *widget() = 0;
    virtual void setCurrentItem(const char *filename, const KFileInfo * i = 0);
    virtual void setAutoUpdate(bool) = 0;
    void clear();
    virtual void repaint(bool f = true);
    QDir::SortSpec sorting() { return mySorting; }
    void setSorting(QDir::SortSpec sort); // a little bit complexer
    uint count() const { return itemsList->count(); }

    void connectDirSelected( QObject *receiver, const char *member);
    void connectFileHighlighted( QObject *receiver, const char *member);
    void connectFileSelected( QObject *receiver, const char *member);

    KFileInfo *selectedDir()     { return lastSDir;  }
    KFileInfo *selectedFile()    { return lastSFile; }
    KFileInfo *highlightedFile() { return lastHFile; }

    uint numFiles() const { return filesNumber; }
    uint numDirs() const { return dirsNumber; }

    virtual QString findCompletion( const char *base, 
				    bool activateFound = false );
    
    virtual bool acceptsFiles() = 0;
    virtual bool acceptsDirs() = 0;

protected:
    virtual bool insertItem(const KFileInfo *i, int index) = 0;
    bool addItemInternal(const KFileInfo *);
    enum SortMode { Increasing, Decreasing, Switching };
    void setSortMode(SortMode mode) { mySortMode = mode; }
    virtual void clearView() = 0;
    virtual void highlightItem(unsigned int item) = 0;

    static QPixmap *file_pixmap;
    static QPixmap *locked_file;
    static QPixmap *folder_pixmap;
    static QPixmap *locked_folder;
    
    void select( int index );
    void select( KFileInfo *);
    void highlight( int index );
    void highlight( KFileInfo *);
    
    bool useSingle() const { return useSingleClick; }
    bool isDir( uint index) const { return sortedList->at(index)->isDir(); }
    int firstFile() const { return firstfile; }

    const KFileInfo *at( uint index ) const { return sortedList->at(index); }
    
private:
    int firstfile;
    bool useSingleClick;

    bool reversed;
    bool keepDirsFirst;
    QDir::SortSpec mySorting;
    enum SortMode mySortMode;
    KFileInfoList *sortedList;
    KFileInfoList *itemsList;

    QSignal *sActivateDir;
    QSignal *sHighlightFile;
    QSignal *sSelectFile;
    QStrIList *nameList;
    
    KFileInfo *lastHFile;
    KFileInfo *lastSFile;
    KFileInfo *lastSDir;
    uint filesNumber, dirsNumber;

};

#endif // KFILEINFOLISTWIDGET_H
