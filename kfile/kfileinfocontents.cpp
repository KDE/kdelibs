/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    
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

#include "kfileinfocontents.h"
#include <qpixmap.h>
#include <qsignal.h>
#include <kapp.h>

#ifdef Unsorted // the "I hate X.h" modus
#undef Unsorted
#endif

QPixmap *KFileInfoContents::folder_pixmap = 0;
QPixmap *KFileInfoContents::locked_folder = 0;   
QPixmap *KFileInfoContents::file_pixmap = 0;
QPixmap *KFileInfoContents::locked_file = 0;   

KFileInfoContents::KFileInfoContents( bool use, QDir::SortSpec sorting )
{
    sortedList = new KFileInfoList;
    itemsList  = new KFileInfoList;
    reversed   = false;
    mySortMode = Increasing; // defaults
    mySorting  = sorting; 

    useSingleClick = use;

    sActivateDir   = new QSignal(0, "activateDir");
    sHighlightFile = new QSignal(0, "highlightFile");
    sSelectFile    = new QSignal(0, "selectFile");

    nameList = new QStrList();
    firstfile = 0;
    // don't use IconLoader to always get the same icon,
    // it looks very strange, if the icons differ from application
    // to application.
    if (!folder_pixmap) 
        folder_pixmap = new QPixmap(KApplication::kde_icondir() +
                                    "/mini/folder.xpm"); 
    if (!locked_folder)
        locked_folder = new QPixmap(KApplication::kde_icondir() +
                                    "/mini/lockedfolder.xpm");
    if (!file_pixmap)
	file_pixmap = new QPixmap(KApplication::kde_icondir() +
				  "/mini/mini-ball.xpm");
    if (!locked_file)
	locked_file = new QPixmap(KApplication::kde_icondir() +
				  "/mini/mini-rball.xpm");

    filesNumber = 0;
    dirsNumber = 0;
}

KFileInfoContents::~KFileInfoContents()
{
    delete sortedList;
    delete itemsList;
    delete nameList;

    delete sActivateDir;
    delete sHighlightFile;
    delete sSelectFile;
}


bool KFileInfoContents::addItem(const KFileInfo *i) 
{
    if (!acceptsFiles() && i->isFile())
	return false;
    if (!acceptsDirs() && i->isDir())
	return false;

    if (i->isDir())
	dirsNumber++;
    else
	filesNumber++;

    itemsList->append(i);

    // we add it nonetheless, we could need it.
    // TODO: think about it
    // NOTE: usd in completion - could use sortedList?? dg
    nameList->inSort(i->fileName());
    
    return addItemInternal(i);
}

void KFileInfoContents::setSorting(QDir::SortSpec new_sort)
{
    debug("reorder");
    QDir::SortSpec old_sort = static_cast<QDir::SortSpec>(this->sorting() & QDir::SortByMask);
    QDir::SortSpec sortflags = static_cast<QDir::SortSpec>(this->sorting() & (~QDir::SortByMask));

    if (mySortMode == Switching) {
	if (new_sort == old_sort)
	    reversed = !reversed;
	else
	    reversed = false;
    } else 
	reversed = (mySortMode == Decreasing);

    mySorting = static_cast<QDir::SortSpec>(new_sort | sortflags);
   
    if (count() <= 1) // nothing to do in this case
	return;

    setAutoUpdate(false);
    sortedList->clear();
    firstfile = 0;
    clearView();

    KFileInfoListIterator lFileInfoIterator(*itemsList);
    // get all entries and add them sorted into the list using the iterator
    for ( KFileInfo *lCurrentFileInfo = lFileInfoIterator.toFirst(); 
	  lCurrentFileInfo; 
	  lCurrentFileInfo = ++lFileInfoIterator ) 
	addItemInternal(lCurrentFileInfo);
    
    setAutoUpdate(true);
    repaint(true);
}

void KFileInfoContents::clear()
{
    lastHFile = 0;
    lastSFile = 0;
    lastSDir = 0;
    sortedList->clear();
    itemsList->clear();
    nameList->clear();
    clearView();
    firstfile = 0;
    filesNumber = 0;
    dirsNumber = 0;
}

void KFileInfoContents::connectDirSelected( QObject *receiver, 
					    const char *member)
{
    sActivateDir->connect(receiver, member);
}

void KFileInfoContents::connectFileHighlighted( QObject *receiver, 
					 const char *member)
{
    sHighlightFile->connect(receiver, member);
}

void KFileInfoContents::connectFileSelected( QObject *receiver, 
				      const char *member)
{
    sSelectFile->connect(receiver, member);
}


bool KFileInfoContents::addItemInternal(const KFileInfo *i)
{
    int pos = -1;

    if ( mySorting != QDir::Unsorted) 
    {
	bool found = false;
	bool isDir = i->isDir();
	bool keepDirsFirst;
	QDir::SortSpec sort = static_cast<QDir::SortSpec>(mySorting & QDir::SortByMask);

	if (i->isDir())
	    sort = QDir::Name;
        
        if (reversed)
            isDir = !isDir;
        
        if ( mySorting & QDir::DirsFirst )
            keepDirsFirst = 1;
        else
            keepDirsFirst = 0;

	if ( keepDirsFirst ) {
            pos = isDir ? 0 : firstfile;
        } else {
            pos = 0;
        }
	
	
	if (pos >= static_cast<int>(sortedList->count()))
	    found = true;
            
        if (keepDirsFirst && isDir && (pos == firstfile))
            found = true;
	
	while (!found) {
	    
	    bool bigger;
	    switch (sort) {
	    case QDir::Size:
		bigger = (i->size() > sortedList->at(pos)->size());
		break;
	    case QDir::Name:
	    default: 
		bigger = (stricmp(i->fileName(),
				  sortedList->at(pos)->fileName()) > 0);
	    }
	    
	    if (reversed)
		bigger = !bigger;
	    
	    if (bigger)
		pos++;
	    else
		found = true;

	    if (pos >= static_cast<int>(sortedList->count())) {
	        if (keepDirsFirst) {
                    if (!isDir)
	                pos = sortedList->count();
	            else
	                pos = firstfile;
                } else {
                    pos = sortedList->count();
                }
		found = true;
	    }
	    
            if ( keepDirsFirst && isDir && (pos >= firstfile))
	    	found = true;
	}
	
        if (keepDirsFirst && isDir) 
	    firstfile++;
    }
    
    if (pos < 0) {
	sortedList->append(i);
	pos = sortedList->at(); // to avoid the -1
    }  else
	sortedList->insert(pos, i);

    return insertItem(i, pos);
}


const char *KFileInfoContents::text(uint index) const
{
    if (index < count())
	return sortedList->at(index)->fileName();
    else
	return "";
}

void KFileInfoContents::select( int index )
{
    KFileInfo *i = sortedList->at(index);
    select(i);
}

void KFileInfoContents::select( KFileInfo *entry)
{
    if ( entry->isDir() ) {
	lastSDir = entry;
	debug("selectDir %s",entry->fileName().data());
	sActivateDir->activate();
    } else {
	lastSFile = entry;
	sSelectFile->activate();
    }
}

void KFileInfoContents::highlight( KFileInfo *entry )
{
    lastHFile = entry;
    sHighlightFile->activate();
}

void KFileInfoContents::highlight( int index )
{
    KFileInfo *i = sortedList->at(index);
    highlight(i);
}


void  KFileInfoContents::repaint(bool f)
{
    widget()->repaint(f);
}

void KFileInfoContents::setCurrentItem(const char *item, 
				       const KFileInfo *entry)
{
    if (item != 0) {
	debug("setCurrentItem %s",item);
	for (KFileInfo *i = sortedList->first(); i; i = sortedList->next())
	    if (sortedList->current()->fileName() == item) {
		highlightItem(sortedList->at());
		highlight(sortedList->at());
		return;
	    }
    } else
	for (KFileInfo *i = sortedList->first(); i; i = sortedList->next())
	    if (i == entry) {
		highlightItem(sortedList->at());
		return;
	    }
    
    warning("setCurrentItem: no match found.");
}

QString KFileInfoContents::findCompletion( const char *base, 
					   bool activateFound )
{
    QString remainder = base;
    const char *name;
    for ( name = nameList->first(); name; name = nameList->next() ) {
	if ( strlen(name) < remainder.length())
	    continue;
	if (strncmp(name, remainder, remainder.length()) == 0)
	    break;
    }
    
    if (name) {
	
        QString body = name;

        // get the possible text completions and store the least 
	// common denominator in QString body
        for ( const char *extra = nameList->next(); extra; 
	      extra = nameList->next() ) {
	    
            unsigned int counter;
            if ( strncmp(extra, remainder, remainder.length()) != 0 )
                break;
            for ( counter=0; counter < strlen(extra) ; counter++ ) {
                if (strncmp( extra, body, counter) != 0)
                    break;
            }
            body.truncate(counter-1);
        }
	
	debug("completion %s %s %s", base, name, body.data());

	bool matchExactly = (name == body);

	if (matchExactly || activateFound)
	    {
		for (KFileInfo *i = sortedList->first(); i; 
		     i = sortedList->next())
		    
		    if (i->fileName() == name) {
			int found = sortedList->at();
			if (sortedList->at(found)->isDir())
			    body += "/";
			highlightItem(found);
			if (activateFound)
			    select(found);
			else
			    highlight(found);
			break;
		    }
	    } else
		setCurrentItem(name); // the first matching name

	return body; 
    } else {
	debug("no completion for %s", base);
	return 0;
    }
    
}
